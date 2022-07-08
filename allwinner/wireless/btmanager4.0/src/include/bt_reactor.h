#ifndef __BTMG_REACTOR_H
#define __BTMG_REACTOR_H

#include <pthread.h>
#include <stdbool.h>
#include "bt_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(EFD_SEMAPHORE)
#define EFD_SEMAPHORE (1 << 0)
#endif

#ifndef INVALID_FD
#define INVALID_FD (-1)
#endif

typedef enum {
    REACTOR_STATUS_STOP, // |reactor_stop| was called.
    REACTOR_STATUS_ERROR, // there was an error during the operation.
    REACTOR_STATUS_DONE, // the reactor completed its work (for the _run_once* variants).
} reactor_status_t;

typedef struct reactor_t {
    int epoll_fd;
    int event_fd;
    pthread_mutex_t list_lock; // protects invalidation_list.
    list_t *invalidation_list; // reactor objects that have been unregistered.
    pthread_t run_thread; // the pthread on which reactor_run is executing.
    bool is_running; // indicates whether |run_thread| is valid.
    bool object_removed;
} reactor_t;

typedef struct reactor_object_t {
    int fd; // the file descriptor to monitor for events.
    void *context; // a context that's passed back to the *_ready functions.
    reactor_t *reactor; // the reactor instance this object is registered with.
    pthread_mutex_t lock; // protects the lifetime of this object and all variables.
    void (*read_ready)(void *context); // function to call when the file descriptor becomes readable.
    void (*write_ready)(
            void *context); // function to call when the file descriptor becomes writeable.
} reactor_object_t;

reactor_t *btmg_reactor_new(void);
void btmg_reactor_free(reactor_t *reactor);
reactor_status_t btmg_reactor_start(reactor_t *reactor);
reactor_status_t btmg_reactor_run_once(reactor_t *reactor);
void btmg_reactor_stop(reactor_t *reactor);
reactor_object_t *btmg_reactor_register(reactor_t *reactor, int fd, void *context,
                                        void (*read_ready)(void *context),
                                        void (*write_ready)(void *context));
bool btmg_reactor_change_registration(reactor_object_t *object, void (*read_ready)(void *context),
                                      void (*write_ready)(void *context));
void btmg_reactor_unregister(reactor_object_t *obj);

#ifdef __cplusplus
}; /*extern "C"*/
#endif

#endif
