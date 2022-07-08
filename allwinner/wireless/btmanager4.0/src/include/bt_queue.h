#ifndef __BTMG_QUEUE_H
#define __BTMG_QUEUE_H

#include "bt_semaphore.h"
#include "bt_list.h"
#include "bt_reactor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fixed_queue_t fixed_queue_t;
typedef void (*fixed_queue_free_cb)(void *data);
typedef void (*fixed_queue_cb)(fixed_queue_t *queue, void *context);

struct fixed_queue_t {
    list_t *list;
    semaphore_t *enqueue_sem;
    semaphore_t *dequeue_sem;
    pthread_mutex_t lock;
    size_t capacity;
    reactor_object_t *dequeue_object;
    fixed_queue_cb dequeue_ready;
    void *dequeue_context;
};

fixed_queue_t *btmg_fixed_queue_new(size_t capacity);
void btmg_fixed_queue_free(fixed_queue_t *queue, fixed_queue_free_cb free_cb);
bool btmg_fixed_queue_is_empty(fixed_queue_t *queue);
size_t btmg_fixed_queue_capacity(fixed_queue_t *queue);
void btmg_fixed_queue_enqueue(fixed_queue_t *queue, void *data);
void *btmg_fixed_queue_dequeue(fixed_queue_t *queue);
bool btmg_fixed_queue_try_enqueue(fixed_queue_t *queue, void *data);
void *btmg_fixed_queue_try_dequeue(fixed_queue_t *queue);
void *btmg_fixed_queue_try_peek(fixed_queue_t *queue);
int btmg_fixed_queue_get_dequeue_fd(const fixed_queue_t *queue);
int btmg_fixed_queue_get_enqueue_fd(const fixed_queue_t *queue);
void btmg_fixed_queue_register_dequeue(fixed_queue_t *queue, reactor_t *reactor,
                                       fixed_queue_cb ready_cb, void *context);
void btmg_fixed_queue_unregister_dequeue(fixed_queue_t *queue);

#ifdef __cplusplus
}; /*extern "C"*/
#endif

#endif
