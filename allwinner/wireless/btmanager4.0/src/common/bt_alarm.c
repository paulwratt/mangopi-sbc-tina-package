#include <errno.h>
#include <inttypes.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "bt_alarm.h"
#include "bt_list.h"
#include "bt_log.h"
#include "bt_semaphore.h"
#include "bt_thread.h"

// Make callbacks run at high thread priority. Some callbacks are used for audio
// related timer tasks as well as re-transmissions etc. Since we at this point
// cannot differentiate what callback we are dealing with, assume high priority
// for now.
// TODO(eisenbach): Determine correct thread priority (from parent?/per alarm?)
static const int CALLBACK_THREAD_PRIORITY_HIGH = -19;

// If the next wakeup time is less than this threshold, we should acquire
// a wakelock instead of setting a wake alarm so we're not bouncing in
// and out of suspend frequently. This value is externally visible to allow
// unit tests to run faster. It should not be modified by production code.
int64_t TIMER_INTERVAL_FOR_WAKELOCK_IN_MS = 3000;
static const clockid_t CLOCK_ID = CLOCK_BOOTTIME;
static const char *WAKE_LOCK_ID = "bluedroid_timer";

// This mutex ensures that the |alarm_set|, |alarm_cancel|, and alarm callback
// functions execute serially and not concurrently. As a result, this mutex also
// protects the |alarms| list.
static pthread_mutex_t monitor;
static list_t *alarms;
static timer_t timer;
static bool timer_set;

// All alarm callbacks are dispatched from |callback_thread|
static thread_t *callback_thread;
static bool callback_thread_active;
static semaphore_t *alarm_expired;

static bool btmg_lazy_initialize(void);
static period_ms_t btmg_now(void);
static void btmg_schedule_next_instance(alarm_t *alarm, bool force_reschedule);
static void btmg_reschedule_root_alarm(void);
static void btmg_timer_callback(void *data);
static void btmg_callback_dispatch(void *context);
static void btmg_alarm_set_internal(alarm_t *alarm, period_ms_t period, alarm_callback_t cb,
                                    void *data, bool is_periodic);

alarm_t *btmg_alarm_new(void)
{
    int error = 0;

    // Make sure we have a list we can insert alarms into.
    if (!alarms && !btmg_lazy_initialize())
        return NULL;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);

    alarm_t *ret = calloc(1, sizeof(alarm_t));
    if (!ret) {
        BTMG_ERROR("unable to allocate memory for alarm");
        goto error;
    }

    // Make this a recursive mutex to make it safe to call |alarm_cancel| from
    // within the callback function of the alarm.
    error = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if (error) {
        BTMG_ERROR("unable to create a recursive mutex: %s", strerror(error));
        goto error;
    }

    error = pthread_mutex_init(&ret->callback_lock, &attr);
    if (error) {
        BTMG_ERROR("unable to initialize mutex: %s", strerror(error));
        goto error;
    }

    pthread_mutexattr_destroy(&attr);
    return ret;

error:
    pthread_mutexattr_destroy(&attr);
    free(ret);

    return NULL;
}

void btmg_alarm_free(alarm_t *alarm)
{
    if (!alarm) {
        BTMG_ERROR("alarm is NULL");
        return;
    }

    btmg_alarm_cancel(alarm);
    pthread_mutex_destroy(&alarm->callback_lock);
    free(alarm);
}

period_ms_t btmg_alarm_get_remaining_ms(const alarm_t *alarm)
{
    if (alarm == NULL) {
        BTMG_ERROR("alarm is NULL");
        return 0;
    }

    period_ms_t remaining_ms = 0;

    pthread_mutex_lock(&monitor);
    if (alarm->deadline)
        remaining_ms = alarm->deadline - btmg_now();
    pthread_mutex_unlock(&monitor);

    return remaining_ms;
}

void btmg_alarm_set(alarm_t *alarm, period_ms_t deadline, alarm_callback_t cb, void *data)
{
    btmg_alarm_set_internal(alarm, deadline, cb, data, false);
}

void btmg_alarm_set_periodic(alarm_t *alarm, period_ms_t period, alarm_callback_t cb, void *data)
{
    btmg_alarm_set_internal(alarm, period, cb, data, true);
}

// Runs in exclusion with alarm_cancel and timer_callback.
void btmg_alarm_set_internal(alarm_t *alarm, period_ms_t period, alarm_callback_t cb, void *data,
                             bool is_periodic)
{
    BTMG_DEBUG("ENTER");

    if (alarms == NULL) {
        BTMG_ERROR("alarms list is NULL");
        return;
    }

    if (alarm == NULL) {
        BTMG_ERROR("alarm is NULL");
        return;
    }

    if (cb == NULL) {
        BTMG_ERROR("calback function is NULL");
        return;
    }

    pthread_mutex_lock(&monitor);

    alarm->created = btmg_now();
    alarm->is_periodic = is_periodic;
    alarm->period = period;
    alarm->callback = cb;
    alarm->data = data;

    btmg_schedule_next_instance(alarm, false);

    pthread_mutex_unlock(&monitor);
}

void btmg_alarm_cancel(alarm_t *alarm)
{
    bool needs_reschedule = false;

    if (alarms == NULL) {
        BTMG_ERROR("alarms list is NULL");
        return;
    }

    if (alarm == NULL) {
        BTMG_ERROR("alarm is NULL");
        return;
    }

    pthread_mutex_lock(&monitor);

    needs_reschedule = (!btmg_list_is_empty(alarms) && btmg_list_front(alarms) == alarm);

    btmg_list_remove(alarms, alarm);
    alarm->deadline = 0;
    alarm->callback = NULL;
    alarm->data = NULL;

    if (needs_reschedule)
        btmg_reschedule_root_alarm();

    pthread_mutex_unlock(&monitor);

    // If the callback for |alarm| is in progress, wait here until it completes.
    pthread_mutex_lock(&alarm->callback_lock);
    pthread_mutex_unlock(&alarm->callback_lock);
}

void btmg_alarm_cleanup(void)
{
    // If lazy_initialize never ran there is nothing to do
    if (!alarms) {
        BTMG_ERROR("alarms are already cleanup");
        return;
    }

    callback_thread_active = false;
    btmg_semaphore_post(alarm_expired);
    btmg_thread_free(callback_thread);
    callback_thread = NULL;

    btmg_semaphore_free(alarm_expired);
    alarm_expired = NULL;
    timer_delete(timer);
    btmg_list_free(alarms);
    alarms = NULL;

    pthread_mutex_destroy(&monitor);
}

static bool btmg_lazy_initialize(void)
{
    struct sigevent sigevent;

    if (alarms != NULL) {
        BTMG_WARNG("alarms list is already exists");
        return false;
    }

    pthread_mutex_init(&monitor, NULL);

    alarms = btmg_list_new(NULL);
    if (!alarms) {
        BTMG_ERROR("unable to allocate alarm list");
        return false;
    }

    memset(&sigevent, 0, sizeof(sigevent));
    sigevent.sigev_notify = SIGEV_THREAD;
    sigevent.sigev_notify_function = (void (*)(union sigval))btmg_timer_callback;
    if (timer_create(CLOCK_ID, &sigevent, &timer) == -1) {
        BTMG_ERROR("unable to create timer: %s", strerror(errno));
        return false;
    }

    alarm_expired = btmg_semaphore_new(0);
    if (!alarm_expired) {
        BTMG_ERROR("unable to create alarm expired semaphore");
        return false;
    }

    callback_thread_active = true;
    callback_thread = btmg_thread_new("alarm_callbacks");
    if (!callback_thread) {
        BTMG_ERROR("unable to create alarm callback thread");
        return false;
    }

    if (!callback_thread->reactor) {
        BTMG_ERROR("thread reactor is NULL");
        return false;
    }

    btmg_thread_set_priority(callback_thread, CALLBACK_THREAD_PRIORITY_HIGH);
    btmg_thread_post(callback_thread, btmg_callback_dispatch, NULL);
    return true;
}

static period_ms_t btmg_now(void)
{
    struct timespec ts;

    if (alarms == NULL) {
        BTMG_ERROR("alarms list is NULL");
        return 0;
    }

    if (clock_gettime(CLOCK_ID, &ts) == -1) {
        BTMG_ERROR("unable to get current time: %s", strerror(errno));
        return 0;
    }

    return (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
}

// Must be called with monitor held
static void btmg_schedule_next_instance(alarm_t *alarm, bool force_reschedule)
{
    // If the alarm is currently set and it's at the start of the list,
    // we'll need to re-schedule since we've adjusted the earliest deadline.
    bool needs_reschedule = false;

    needs_reschedule = (!btmg_list_is_empty(alarms) && btmg_list_front(alarms) == alarm);
    if (alarm->callback)
        btmg_list_remove(alarms, alarm);

    // Calculate the next deadline for this alarm
    period_ms_t just_now = btmg_now();
    period_ms_t ms_into_period =
            alarm->is_periodic ? ((just_now - alarm->created) % alarm->period) : 0;
    alarm->deadline = just_now + (alarm->period - ms_into_period);

    // Add it into the timer list sorted by deadline (earliest deadline first).
    if (btmg_list_is_empty(alarms) ||
        ((alarm_t *)btmg_list_front(alarms))->deadline >= alarm->deadline)
        btmg_list_prepend(alarms, alarm);
    else
        for (list_node_t *node = btmg_list_begin(alarms); node != btmg_list_end(alarms);
             node = btmg_list_next(node)) {
            list_node_t *next = btmg_list_next(node);
            if (next == btmg_list_end(alarms) ||
                ((alarm_t *)btmg_list_node(next))->deadline >= alarm->deadline) {
                btmg_list_insert_after(alarms, node, alarm);
                break;
            }
        }

    // If the new alarm has the earliest deadline, we need to re-evaluate our schedule.
    if (force_reschedule || needs_reschedule ||
        (!btmg_list_is_empty(alarms) && btmg_list_front(alarms) == alarm))
        btmg_reschedule_root_alarm();
}

// NOTE: must be called with monitor lock.
static void btmg_reschedule_root_alarm(void)
{
    bool timer_was_set = timer_set;
    struct itimerspec wakeup_time;
    struct itimerspec time_to_expire;
    alarm_t *next = NULL;

    if (alarms == NULL) {
        BTMG_ERROR("alarms list is NULL");
        return;
    }

    // If used in a zeroed state, disarms the timer
    memset(&wakeup_time, 0, sizeof(wakeup_time));

    if (btmg_list_is_empty(alarms))
        goto done;

    next = btmg_list_front(alarms);
    wakeup_time.it_value.tv_sec = (next->deadline / 1000);
    wakeup_time.it_value.tv_nsec = (next->deadline % 1000) * 1000000LL;

done:
    timer_set = wakeup_time.it_value.tv_sec != 0 || wakeup_time.it_value.tv_nsec != 0;

    if (timer_settime(timer, TIMER_ABSTIME, &wakeup_time, NULL) == -1)
        BTMG_ERROR("unable to set timer: %s", strerror(errno));

    // If next expiration was in the past (e.g. short timer that got context switched)
    // then the timer might have diarmed itself. Detect this case and work around it
    // by manually signalling the |alarm_expired| semaphore.
    //
    // It is possible that the timer was actually super short (a few milliseconds)
    // and the timer expired normally before we called |timer_gettime|. Worst case,
    // |alarm_expired| is signaled twice for that alarm. Nothing bad should happen in
    // that case though since the callback dispatch function checks to make sure the
    // timer at the head of the list actually expired.
    if (timer_set) {
        timer_gettime(timer, &time_to_expire);
        if (time_to_expire.it_value.tv_sec == 0 && time_to_expire.it_value.tv_nsec == 0) {
            BTMG_ERROR("alarm expiration too close for posix timers, switching to guns");
            btmg_semaphore_post(alarm_expired);
        }
    }
}

// Callback function for wake alarms and our posix timer
static void btmg_timer_callback(UNUSED_ATTR void *ptr)
{
    btmg_semaphore_post(alarm_expired);
}

// Function running on |callback_thread| that dispatches alarm callbacks upon
// alarm expiration, which is signaled using |alarm_expired|.
static void btmg_callback_dispatch(UNUSED_ATTR void *context)
{
    alarm_t *alarm = NULL;
    alarm_callback_t callback = NULL;
    void *data = NULL;

    while (true) {
        btmg_semaphore_wait(alarm_expired);
        if (!callback_thread_active)
            break;

        pthread_mutex_lock(&monitor);

        // Take into account that the alarm may get cancelled before we get to it.
        // We're done here if there are no alarms or the alarm at the front is in
        // the future. Release the monitor lock and exit right away since there's
        // nothing left to do.
        if (btmg_list_is_empty(alarms) ||
            (alarm = btmg_list_front(alarms))->deadline > btmg_now()) {
            btmg_reschedule_root_alarm();
            pthread_mutex_unlock(&monitor);
            continue;
        }

        btmg_list_remove(alarms, alarm);
        callback = alarm->callback;
        data = alarm->data;

        if (alarm->is_periodic) {
            btmg_schedule_next_instance(alarm, true);
        } else {
            btmg_reschedule_root_alarm();
            alarm->deadline = 0;
            alarm->callback = NULL;
            alarm->data = NULL;
        }

        // Downgrade lock.
        pthread_mutex_lock(&alarm->callback_lock);
        pthread_mutex_unlock(&monitor);

        callback(data);

        pthread_mutex_unlock(&alarm->callback_lock);
    }

    BTMG_DEBUG("Alarm callback thread exited");
}

#define INTERVAL_TIME_NUM 5

struct interval_time {
    struct timespec start;
    void *tag;
    bool enable;
};

static struct interval_time diff_time[INTERVAL_TIME_NUM];

uint64_t btmg_interval_time(void *tag, uint64_t expect_time_ms)
{
    static bool init = false;

    int i;
    if (init == false) {
        memset(diff_time, 0, sizeof(diff_time));
        for (i = 0; i < INTERVAL_TIME_NUM; i++) {
            diff_time[i].enable = false;
            diff_time[i].tag = NULL;
        }
        init = true;
    }

    for (i = 0; i < INTERVAL_TIME_NUM; i++) {
        if (diff_time[i].enable == true && diff_time[i].tag == tag)
            break;
    }

    if (i >= INTERVAL_TIME_NUM) {
        for (i = 0; i < INTERVAL_TIME_NUM; i++) {
            if (diff_time[i].enable == false && diff_time[i].tag == NULL) {
                diff_time[i].enable = true;
                diff_time[i].tag = tag;
                clock_gettime(CLOCK_REALTIME, &(diff_time[i].start));
                return BT_OK;
            }
        }
    }

    for (i = 0; i < INTERVAL_TIME_NUM; i++) {
        if (diff_time[i].enable == true && diff_time[i].tag == tag) {
            struct timespec cur_time;
            uint64_t t = 0;
            clock_gettime(CLOCK_REALTIME, &cur_time);

            if ((cur_time.tv_nsec - diff_time[i].start.tv_nsec) < 0) {
                t = (1000000000 + cur_time.tv_nsec - diff_time[i].start.tv_nsec) / 1000000; //ms
                t = (cur_time.tv_sec - diff_time[i].start.tv_sec - 1) * 1000 + t;
            } else {
                t = (cur_time.tv_nsec - diff_time[i].start.tv_nsec) / 1000000; //ms
                t = (cur_time.tv_sec - diff_time[i].start.tv_sec) * 1000 + t;
            }
            /*
			BTMG_INFO("i:%d,%p,%p,cur:%llu,%llu,start:%llu,%llu,t:%lu,ex:%lu",i,diff_time[i].tag,tag,
					cur_time.tv_sec,cur_time.tv_nsec,diff_time[i].start.tv_sec,
					diff_time[i].start.tv_nsec,t,expect_time_ms);
			*/
            if (t >= expect_time_ms) {
                clock_gettime(CLOCK_REALTIME, &diff_time[i].start);
                return t;
            }
        }
    }
    return BT_OK;
}
