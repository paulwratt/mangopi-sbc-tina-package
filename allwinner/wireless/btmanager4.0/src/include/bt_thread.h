#ifndef __BTMG_THREAD_H
#define __BTMG_THREAD_H

#include "bt_reactor.h"
#include "bt_queue.h"
#include "bt_semaphore.h"
#include "bt_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define THREAD_NAME_MAX_LEN 64

typedef void (*thread_fn)(void *context);

typedef struct thread_t {
    bool is_joined;
    pthread_t pthread;
    pid_t tid;
    char name[THREAD_NAME_MAX_LEN + 1];
    reactor_t *reactor;
    fixed_queue_t *work_queue;
} thread_t;

typedef struct start_arg {
    thread_t *thread;
    semaphore_t *start_sem;
    int error;
} start_arg;

typedef struct work_item_t {
    thread_fn func;
    void *context;
} work_item_t;

thread_t *btmg_thread_new(const char *name);
void btmg_thread_free(thread_t *thread);
bool btmg_thread_set_priority(thread_t *thread, int priority);
void btmg_thread_join(thread_t *thread);
bool btmg_thread_post(thread_t *thread, thread_fn func, void *context);
void btmg_thread_stop(thread_t *thread);
bool btmg_thread_is_self(const thread_t *thread);
reactor_t *btmg_thread_get_reactor(const thread_t *thread);
const char *btmg_thread_name(const thread_t *thread);

#ifdef __cplusplus
}; /*extern "C"*/
#endif

#endif
