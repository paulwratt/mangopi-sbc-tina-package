#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "bt_list.h"
#include "bt_log.h"
#include "bt_reactor.h"

static reactor_status_t btmg_run_reactor(reactor_t *reactor, int iterations);

static const size_t MAX_EVENTS = 64;
static const eventfd_t EVENT_REACTOR_STOP = 1;

reactor_t *btmg_reactor_new(void)
{
    struct epoll_event event;
    reactor_t *ret = NULL;

    ret = (reactor_t *)calloc(1, sizeof(reactor_t));
    if (!ret)
        return NULL;

    ret->epoll_fd = INVALID_FD;
    ret->event_fd = INVALID_FD;

    ret->epoll_fd = epoll_create(MAX_EVENTS);
    if (ret->epoll_fd == INVALID_FD) {
        BTMG_ERROR("unable to create epoll instance: %s", strerror(errno));
        goto error;
    }

    ret->event_fd = eventfd(0, 0);
    if (ret->event_fd == INVALID_FD) {
        BTMG_ERROR("unable to create eventfd: %s", strerror(errno));
        goto error;
    }

    pthread_mutex_init(&ret->list_lock, NULL);
    ret->invalidation_list = btmg_list_new(NULL);
    if (!ret->invalidation_list) {
        BTMG_ERROR("unable to allocate object invalidation list.");
        goto error;
    }

    memset(&event, 0, sizeof(event));
    event.events = EPOLLIN;
    event.data.ptr = NULL;
    if (epoll_ctl(ret->epoll_fd, EPOLL_CTL_ADD, ret->event_fd, &event) == -1) {
        BTMG_ERROR("unable to register eventfd with epoll set: %s", strerror(errno));
        goto error;
    }

    BTMG_DEBUG("create reactor success");
    return ret;

error:
    btmg_reactor_free(ret);
    return NULL;
}

void btmg_reactor_free(reactor_t *reactor)
{
    BTMG_DEBUG("ENTER");

    if (reactor == NULL) {
        BTMG_ERROR("reactor is NULL");
        return;
    }

    btmg_list_free(reactor->invalidation_list);
    close(reactor->event_fd);
    close(reactor->epoll_fd);
    free(reactor);
}

reactor_status_t btmg_reactor_start(reactor_t *reactor)
{
    if (reactor)
        return btmg_run_reactor(reactor, 0);
    else {
        BTMG_ERROR("reactor is NULL");
        return REACTOR_STATUS_ERROR;
    }
}

reactor_status_t btmg_reactor_run_once(reactor_t *reactor)
{
    if (reactor)
        return btmg_run_reactor(reactor, 1);
    else {
        BTMG_ERROR("reactor is NULL");
        return REACTOR_STATUS_ERROR;
    }
}

void btmg_reactor_stop(reactor_t *reactor)
{
    if (reactor == NULL) {
        BTMG_ERROR("reactor is NULL");
        return;
    }

    eventfd_write(reactor->event_fd, EVENT_REACTOR_STOP);
}

reactor_object_t *btmg_reactor_register(reactor_t *reactor, int fd, void *context,
                                        void (*read_ready)(void *context),
                                        void (*write_ready)(void *context))
{
    reactor_object_t *object = NULL;
    struct epoll_event event;

    if (reactor == NULL || fd == INVALID_FD) {
        BTMG_ERROR("reactor is NULL or fd is INVALID_FD");
        return NULL;
    }

    object = (reactor_object_t *)calloc(1, sizeof(reactor_object_t));
    if (!object) {
        BTMG_ERROR("unable to allocate reactor object: %s", strerror(errno));
        return NULL;
    }

    object->reactor = reactor;
    object->fd = fd;
    object->context = context;
    object->read_ready = read_ready;
    object->write_ready = write_ready;
    pthread_mutex_init(&object->lock, NULL);

    memset(&event, 0, sizeof(event));
    if (read_ready)
        event.events |= (EPOLLIN | EPOLLRDHUP);
    if (write_ready)
        event.events |= EPOLLOUT;
    event.data.ptr = object;

    if (epoll_ctl(reactor->epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        BTMG_ERROR("unable to register fd %d to epoll set: %s", fd, strerror(errno));
        pthread_mutex_destroy(&object->lock);
        free(object);
        return NULL;
    }

    return object;
}

bool btmg_reactor_change_registration(reactor_object_t *object, void (*read_ready)(void *context),
                                      void (*write_ready)(void *context))
{
    struct epoll_event event;

    if (object == NULL) {
        BTMG_ERROR("object is NULL");
        return false;
    }

    memset(&event, 0, sizeof(event));
    if (read_ready)
        event.events |= (EPOLLIN | EPOLLRDHUP);
    if (write_ready)
        event.events |= EPOLLOUT;
    event.data.ptr = object;

    if (epoll_ctl(object->reactor->epoll_fd, EPOLL_CTL_MOD, object->fd, &event) == -1) {
        BTMG_ERROR("unable to modify interest set for fd %d: %s", object->fd, strerror(errno));
        return false;
    }

    pthread_mutex_lock(&object->lock);
    object->read_ready = read_ready;
    object->write_ready = write_ready;
    pthread_mutex_unlock(&object->lock);

    return true;
}

void btmg_reactor_unregister(reactor_object_t *obj)
{
    reactor_t *reactor = NULL;

    if (obj == NULL) {
        BTMG_ERROR("object is NULL");
        return;
    }

    reactor = obj->reactor;

    if (epoll_ctl(reactor->epoll_fd, EPOLL_CTL_DEL, obj->fd, NULL) == -1)
        BTMG_ERROR("unable to unregister fd %d from epoll set: %s", obj->fd, strerror(errno));

    if (reactor->is_running && pthread_equal(pthread_self(), reactor->run_thread)) {
        reactor->object_removed = true;
        return;
    }

    pthread_mutex_lock(&reactor->list_lock);
    btmg_list_append(reactor->invalidation_list, obj);
    pthread_mutex_unlock(&reactor->list_lock);

    // Taking the object lock here makes sure a callback for |obj| isn't
    // currently executing. The reactor thread must then either be before
    // the callbacks or after. If after, we know that the object won't be
    // referenced because it has been taken out of the epoll set. If before,
    // it won't be referenced because the reactor thread will check the
    // invalidation_list and find it in there. So by taking this lock, we
    // are waiting until the reactor thread drops all references to |obj|.
    // One the wait completes, we can unlock and destroy |obj| safely.
    pthread_mutex_lock(&obj->lock);
    pthread_mutex_unlock(&obj->lock);
    pthread_mutex_destroy(&obj->lock);
    free(obj);
}

// Runs the reactor loop for a maximum of |iterations|.
// 0 |iterations| means loop forever.
// |reactor| may not be NULL.
reactor_status_t btmg_run_reactor(reactor_t *reactor, int iterations)
{
    struct epoll_event events[MAX_EVENTS];
    int ret = 0;
    reactor_object_t *object = NULL;

    if (reactor == NULL) {
        BTMG_ERROR("reactor is NULL");
        return REACTOR_STATUS_ERROR;
    }

    reactor->run_thread = pthread_self();
    reactor->is_running = true;

    for (int i = 0; iterations == 0 || i < iterations; ++i) {
        pthread_mutex_lock(&reactor->list_lock);
        btmg_list_clear(reactor->invalidation_list);
        pthread_mutex_unlock(&reactor->list_lock);

        do {
            ret = epoll_wait(reactor->epoll_fd, events, MAX_EVENTS, -1);
        } while (ret == -1 && errno == EINTR);

        pthread_testcancel();

        if (ret == -1) {
            BTMG_ERROR("error in epoll_wait: %s", strerror(errno));
            reactor->is_running = false;
            return REACTOR_STATUS_ERROR;
        }

        for (int j = 0; j < ret; ++j) {
            // The event file descriptor is the only one that registers with
            // a NULL data pointer. We use the NULL to identify it and break
            // out of the reactor loop.
            if (events[j].data.ptr == NULL) {
                eventfd_t value;
                eventfd_read(reactor->event_fd, &value);
                reactor->is_running = false;
                return REACTOR_STATUS_STOP;
            }

            object = (reactor_object_t *)events[j].data.ptr;

            pthread_mutex_lock(&reactor->list_lock);
            if (btmg_list_contains(reactor->invalidation_list, object)) {
                pthread_mutex_unlock(&reactor->list_lock);
                continue;
            }

            // Downgrade the list lock to an object lock.
            pthread_mutex_lock(&object->lock);
            pthread_mutex_unlock(&reactor->list_lock);

            reactor->object_removed = false;
            if (events[j].events & (EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR) &&
                object->read_ready)
                object->read_ready(object->context);
            if (!reactor->object_removed && events[j].events & EPOLLOUT && object->write_ready)
                object->write_ready(object->context);
            pthread_mutex_unlock(&object->lock);

            if (reactor->object_removed) {
                pthread_mutex_destroy(&object->lock);
                free(object);
            }
        } /*for (int j =0 ,j < ret; ++j)*/
    }

    reactor->is_running = false;

    return REACTOR_STATUS_DONE;
}
