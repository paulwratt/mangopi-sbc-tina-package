#include <errno.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "bt_queue.h"
#include "bt_log.h"
#include "bt_reactor.h"
#include "bt_semaphore.h"
#include "bt_thread.h"

static void *btmg_run_thread(void *start_arg);
static void work_queue_read_cb(void *context);

static const size_t DEFAULT_WORK_QUEUE_CAPACITY = 128;

thread_t *btmg_thread_new_sized(const char *name, size_t work_queue_capacity)
{
    struct start_arg start;
    thread_t *ret = NULL;

    if (!name || work_queue_capacity == 0) {
        BTMG_ERROR("thread name is NULL");
        return NULL;
    }

    ret = calloc(1, sizeof(thread_t));
    if (!ret) {
        BTMG_ERROR("calloc for thread_t failed");
        goto error;
    }

    ret->reactor = btmg_reactor_new();
    if (!ret->reactor) {
        BTMG_ERROR("reactor_new for new thread failed");
        goto error;
    }

    ret->work_queue = btmg_fixed_queue_new(work_queue_capacity);
    if (!ret->work_queue)
        goto error;

    // Start is on the stack, but we use a semaphore, so it's safe
    start.start_sem = btmg_semaphore_new(0);
    if (!start.start_sem)
        goto error;

    strncpy(ret->name, name, THREAD_NAME_MAX_LEN);

    start.thread = ret;
    start.error = 0;
    pthread_create(&ret->pthread, NULL, btmg_run_thread, &start);
    btmg_semaphore_wait(start.start_sem);
    btmg_semaphore_free(start.start_sem);

    if (start.error)
        goto error;

    return ret;

error:
    if (ret) {
        btmg_fixed_queue_free(ret->work_queue, free);
        btmg_reactor_free(ret->reactor);
    }
    free(ret);
    return NULL;
}

thread_t *btmg_thread_new(const char *name)
{
    return btmg_thread_new_sized(name, DEFAULT_WORK_QUEUE_CAPACITY);
}

void btmg_thread_free(thread_t *thread)
{
    if (!thread) {
        BTMG_ERROR("thread is NULL");
        return;
    }

    btmg_thread_stop(thread);
    btmg_thread_join(thread);

    btmg_fixed_queue_free(thread->work_queue, free);
    btmg_reactor_free(thread->reactor);
    free(thread);
}

bool btmg_thread_set_priority(thread_t *thread, int priority)
{
    int rc = 0;

    if (!thread)
        return false;

    rc = setpriority(PRIO_PROCESS, thread->tid, priority);
    if (rc < 0) {
        BTMG_ERROR("unable to set thread priority %d for tid %d, error %d", priority, thread->tid,
                   rc);
        return false;
    }

    return true;
}

void btmg_thread_join(thread_t *thread)
{
    if (!thread) {
        BTMG_ERROR("thread is NULL");
        return;
    }

    // TODO(zachoverflow): use a compare and swap when ready
    if (!thread->is_joined) {
        thread->is_joined = true;
        pthread_cancel(thread->pthread);
        pthread_join(thread->pthread, NULL);
    }
}

bool btmg_thread_post(thread_t *thread, thread_fn func, void *context)
{
    work_item_t *item = NULL;

    if (!thread || !func) {
        BTMG_ERROR("thread or func is NULL");
        return false;
    }

    // TODO(sharvil): if the current thread == |thread| and we've run out
    // of queue space, we should abort this operation, otherwise we'll
    // deadlock.

    // Queue item is freed either when the queue itself is destroyed
    // or when the item is removed from the queue for dispatch.

    item = (work_item_t *)calloc(1, sizeof(work_item_t));
    if (!item) {
        BTMG_ERROR("unable to allocate memory: %s", strerror(errno));
        return false;
    }
    item->func = func;
    item->context = context;
    btmg_fixed_queue_enqueue(thread->work_queue, item);
    return true;
}

void btmg_thread_stop(thread_t *thread)
{
    if (!thread) {
        BTMG_ERROR("thread is NULL");
        return;
    }

    btmg_reactor_stop(thread->reactor);
}

bool btmg_thread_is_self(const thread_t *thread)
{
    if (thread = NULL) {
        BTMG_ERROR("thread is NULL");
        return false;
    }

    return !pthread_equal(pthread_self(), thread->pthread);
}

reactor_t *btmg_thread_get_reactor(const thread_t *thread)
{
    if (thread = NULL) {
        BTMG_ERROR("thread is NULL");
        return NULL;
    }

    return thread->reactor;
}

const char *btmg_thread_name(const thread_t *thread)
{
    if (!thread) {
        BTMG_ERROR("thread is NULL");
        return NULL;
    }

    return thread->name;
}

static void *btmg_run_thread(void *start_arg)
{
    struct start_arg *start = NULL;
    thread_t *thread = NULL;
    int fd = 0;
    void *context = NULL;
    reactor_object_t *work_queue_object = NULL;
    size_t count = 0;
    work_item_t *item = NULL;

    if (!start_arg) {
        BTMG_ERROR("arg is NULL");
        return NULL;
    }

    start = start_arg;
    thread = start->thread;

    if (!thread) {
        BTMG_ERROR("thread is NULL");
        return NULL;
    }

    if (prctl(PR_SET_NAME, (unsigned long)thread->name) == -1) {
        BTMG_ERROR("unable to set thread name: %s", strerror(errno));
        start->error = errno;
        btmg_semaphore_post(start->start_sem);
        return NULL;
    }

#ifdef SYS_gettid
    thread->tid = syscall(SYS_gettid);
#else
    thread->tid = gettid();
#endif

    BTMG_DEBUG("thread id %d ", thread->tid);

    btmg_semaphore_post(start->start_sem);

    fd = btmg_fixed_queue_get_dequeue_fd(thread->work_queue);
    context = thread->work_queue;

    work_queue_object =
            btmg_reactor_register(thread->reactor, fd, context, work_queue_read_cb, NULL);
    btmg_reactor_start(thread->reactor);
    btmg_reactor_unregister(work_queue_object);

    // Make sure we dispatch all queued work items before exiting the thread.
    // This allows a caller to safely tear down by enqueuing a teardown
    // work item and then joining the thread.
    item = btmg_fixed_queue_try_dequeue(thread->work_queue);
    while (item && count <= btmg_fixed_queue_capacity(thread->work_queue)) {
        item->func(item->context);
        free(item);
        item = btmg_fixed_queue_try_dequeue(thread->work_queue);
        ++count;
    }

    if (count > btmg_fixed_queue_capacity(thread->work_queue))
        BTMG_DEBUG("growing event queue on shutdown");

    return NULL;
}

static void work_queue_read_cb(void *context)
{
    fixed_queue_t *queue = NULL;
    work_item_t *item = NULL;

    if (!context) {
        BTMG_ERROR("thread context is NULL");
        return;
    }

    queue = (fixed_queue_t *)context;
    item = btmg_fixed_queue_dequeue(queue);

    if (item != NULL) {
        item->func(item->context);
        free(item);
    }
}
