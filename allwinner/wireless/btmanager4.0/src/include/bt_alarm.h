#ifndef __BTMG_ALARM_H
#define __BTMG_ALARM_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNUSED_ATTR
#define UNUSED_ATTR __attribute__((unused))
#endif

typedef uint64_t period_ms_t;
typedef void (*alarm_callback_t)(void *data);

struct alarm_t {
    // The lock is held while the callback for this alarm is being executed.
    // It allows us to release the coarse-grained monitor lock while a potentially
    // long-running callback is executing. |alarm_cancel| uses this lock to provide
    // a guarantee to its caller that the callback will not be in progress when it
    // returns.
    pthread_mutex_t callback_lock;
    period_ms_t created;
    period_ms_t period;
    period_ms_t deadline;
    bool is_periodic;
    alarm_callback_t callback;
    void *data;
};

typedef struct alarm_t alarm_t;

alarm_t *btmg_alarm_new(void);
void btmg_alarm_free(alarm_t *alarm);
period_ms_t btmg_alarm_get_remaining_ms(const alarm_t *alarm);
void btmg_alarm_set(alarm_t *alarm, period_ms_t deadline, alarm_callback_t cb, void *data);
void btmg_alarm_set_periodic(alarm_t *alarm, period_ms_t period, alarm_callback_t cb, void *data);
void btmg_alarm_cancel(alarm_t *alarm);
void btmg_alarm_cleanup(void);
uint64_t btmg_interval_time(void *tag, uint64_t expect_time_ms);

#ifdef __cplusplus
}; /*extern "C"*/
#endif

#endif
