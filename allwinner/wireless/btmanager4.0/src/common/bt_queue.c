#include <pthread.h>
#include <stdlib.h>

#include "bt_queue.h"
#include "bt_list.h"
#include "bt_semaphore.h"
#include "bt_reactor.h"
#include "bt_log.h"

static void internal_dequeue_ready(void *context);

fixed_queue_t *btmg_fixed_queue_new(size_t capacity)
{
    fixed_queue_t *ret = NULL;

    ret = calloc(1, sizeof(fixed_queue_t));

    if (!ret) {
        BTMG_ERROR("Failed to allocate memory");
        goto error;
    }

    pthread_mutex_init(&ret->lock, NULL);
    ret->capacity = capacity;

    ret->list = btmg_list_new(NULL);
    if (!ret->list) {
        BTMG_ERROR("List is NULL");
        goto error;
    }

    ret->enqueue_sem = btmg_semaphore_new(capacity);
    if (!ret->enqueue_sem) {
        BTMG_ERROR("enqueue_sem is NULL");
        goto error;
    }

    ret->dequeue_sem = btmg_semaphore_new(0);
    if (!ret->dequeue_sem) {
        BTMG_ERROR("dequeue_sem is NULL");
        goto error;
    }

    return ret;

error:
    btmg_fixed_queue_free(ret, NULL);
    return NULL;
}

void btmg_fixed_queue_free(fixed_queue_t *queue, fixed_queue_free_cb free_cb)
{
    if (!queue)
        return;

    btmg_fixed_queue_unregister_dequeue(queue);

    if (free_cb)
        for (const list_node_t *node = btmg_list_begin(queue->list);
             node != btmg_list_end(queue->list); node = btmg_list_next(node))
            free_cb(btmg_list_node(node));

    btmg_list_free(queue->list);
    btmg_semaphore_free(queue->enqueue_sem);
    btmg_semaphore_free(queue->dequeue_sem);
    pthread_mutex_destroy(&queue->lock);
    free(queue);
}

bool btmg_fixed_queue_is_empty(fixed_queue_t *queue)
{
    bool is_empty = false;

    if (queue == NULL) {
        BTMG_ERROR("queue is NULL");
        return false;
    }

    pthread_mutex_lock(&queue->lock);
    is_empty = btmg_list_is_empty(queue->list);
    pthread_mutex_unlock(&queue->lock);

    return is_empty;
}

size_t btmg_fixed_queue_capacity(fixed_queue_t *queue)
{
    if (queue == NULL) {
        BTMG_ERROR("queue is NULL");
        return 0;
    }

    return queue->capacity;
}

void btmg_fixed_queue_enqueue(fixed_queue_t *queue, void *data)
{
    if (queue == NULL || data == NULL) {
        BTMG_ERROR("queue or data is NULL");
        return;
    }

    btmg_semaphore_wait(queue->enqueue_sem);

    pthread_mutex_lock(&queue->lock);
    btmg_list_append(queue->list, data);
    pthread_mutex_unlock(&queue->lock);

    btmg_semaphore_post(queue->dequeue_sem);
}

void *btmg_fixed_queue_dequeue(fixed_queue_t *queue)
{
    void *ret = NULL;

    if (queue == NULL) {
        BTMG_ERROR("queue is NULL");
        return NULL;
    }

    btmg_semaphore_wait(queue->dequeue_sem);

    pthread_mutex_lock(&queue->lock);
    ret = btmg_list_front(queue->list);
    btmg_list_remove(queue->list, ret);
    pthread_mutex_unlock(&queue->lock);

    btmg_semaphore_post(queue->enqueue_sem);

    return ret;
}

bool btmg_fixed_queue_try_enqueue(fixed_queue_t *queue, void *data)
{
    if (queue == NULL || data == NULL) {
        BTMG_ERROR("queue or data is NULL");
        return false;
    }

    if (!btmg_semaphore_try_wait(queue->enqueue_sem))
        return false;

    pthread_mutex_lock(&queue->lock);
    btmg_list_append(queue->list, data);
    pthread_mutex_unlock(&queue->lock);

    btmg_semaphore_post(queue->dequeue_sem);
    return true;
}

void *btmg_fixed_queue_try_dequeue(fixed_queue_t *queue)
{
    void *ret = NULL;

    if (queue == NULL) {
        BTMG_ERROR("queue is NULL");
        return NULL;
    }

    if (!btmg_semaphore_try_wait(queue->dequeue_sem)) {
        BTMG_ERROR("Failed to dequeue_sem");
        return NULL;
    }

    pthread_mutex_lock(&queue->lock);
    ret = btmg_list_front(queue->list);
    btmg_list_remove(queue->list, ret);
    pthread_mutex_unlock(&queue->lock);

    btmg_semaphore_post(queue->enqueue_sem);

    return ret;
}

void *btmg_fixed_queue_try_peek(fixed_queue_t *queue)
{
    void *ret = NULL;

    if (queue == NULL) {
        BTMG_ERROR("queue is NULL");
        return NULL;
    }

    pthread_mutex_lock(&queue->lock);
    // Because protected by the lock, the empty and front calls are atomic and not a race condition
    ret = btmg_list_is_empty(queue->list) ? NULL : btmg_list_front(queue->list);
    pthread_mutex_unlock(&queue->lock);

    return ret;
}

int btmg_fixed_queue_get_dequeue_fd(const fixed_queue_t *queue)
{
    if (queue == NULL) {
        BTMG_ERROR("queue is NULL");
        return -1;
    }

    return btmg_semaphore_get_fd(queue->dequeue_sem);
}

int btmg_fixed_queue_get_enqueue_fd(const fixed_queue_t *queue)
{
    if (queue == NULL) {
        BTMG_ERROR("queue is NULL");
        return -1;
    }

    return btmg_semaphore_get_fd(queue->enqueue_sem);
}

void btmg_fixed_queue_register_dequeue(fixed_queue_t *queue, reactor_t *reactor,
                                       fixed_queue_cb ready_cb, void *context)
{
    if (queue == NULL || reactor == NULL || ready_cb == NULL) {
        BTMG_ERROR("queue, reactor or ready_cb is NULL");
        return;
    }

    // Make sure we're not already registered
    btmg_fixed_queue_unregister_dequeue(queue);

    queue->dequeue_ready = ready_cb;
    queue->dequeue_context = context;
    queue->dequeue_object = btmg_reactor_register(reactor, btmg_fixed_queue_get_dequeue_fd(queue),
                                                  queue, internal_dequeue_ready, NULL);
}

void btmg_fixed_queue_unregister_dequeue(fixed_queue_t *queue)
{
    if (queue == NULL) {
        BTMG_ERROR("queue is NULL");
        return;
    }

    if (queue->dequeue_object) {
        btmg_reactor_unregister(queue->dequeue_object);
        queue->dequeue_object = NULL;
    }
}

static void internal_dequeue_ready(void *context)
{
    if (context == NULL) {
        BTMG_ERROR("context is NULL");
        return;
    }

    fixed_queue_t *queue = context;
    queue->dequeue_ready(queue, queue->dequeue_context);
}
