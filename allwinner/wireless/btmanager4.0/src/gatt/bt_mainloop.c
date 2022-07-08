#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <src/shared/mainloop.h>
#include "bt_log.h"

typedef struct {
    pthread_t thread_id;
    bool thread_enable;
    int sockfds[2];
    int ref;
    pthread_t tid;
} mainloop_handle_t;

static mainloop_handle_t mainloop_handle = {
    .thread_enable = false,
    .ref = 0,
};

static pthread_mutex_t mainloop_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAIN_LOOP_EXIT_CODE 0xf0

static void bt_mainloop_thread_quit_cb(int fd, uint32_t events, void *user_data)
{
    ssize_t result;
    uint8_t c;

    BTMG_DEBUG("enter");

    if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        BTMG_ERROR("Events error");
        mainloop_quit();
        return;
    }

    result = read(fd, &c, 1);
    if (result < 0) {
        BTMG_ERROR("Read channel error");
        mainloop_quit();
        return;
    }
    if (result == 1 && c == MAIN_LOOP_EXIT_CODE) {
        BTMG_DEBUG("Received bta thread exit code");
        mainloop_quit();
        return;
    }
    return;
}

static void *bt_mainloop_thread(void *arg)
{
    int ret;

    BTMG_DEBUG("enter");

    ret = socketpair(PF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
                     mainloop_handle.sockfds);

    if (ret == -1) {
        BTMG_ERROR("Couldn't create paired sockets");
        return NULL;
    }

    ret = mainloop_add_fd(mainloop_handle.sockfds[0], EPOLLIN, bt_mainloop_thread_quit_cb, NULL,
                          NULL);
    if (ret < 0) {
        BTMG_ERROR("Failed to add fd (%d)\n", ret);
        goto end;
    }

    mainloop_handle.thread_enable = true;
    mainloop_handle.tid = pthread_self();
    mainloop_run();
    mainloop_handle.thread_enable = false;

end:
    close(mainloop_handle.sockfds[0]);
    close(mainloop_handle.sockfds[1]);
    mainloop_handle.sockfds[0] = -1;
    mainloop_handle.sockfds[1] = -1;

    return NULL;
}

pthread_t bt_bluez_thread_tid_get(void)
{
    return mainloop_handle.tid;
}

int bt_bluez_mainloop_deinit(void)
{
    uint8_t c;
    int result;

    BTMG_DEBUG("enter");

    if (mainloop_handle.thread_enable == false || mainloop_handle.ref == 0)
        return 0;

    pthread_mutex_lock(&mainloop_mutex);

    mainloop_handle.ref--;

    BTMG_DEBUG("reference:%d", mainloop_handle.ref);

    if (mainloop_handle.ref > 0) {
        pthread_mutex_unlock(&mainloop_mutex);
        return 0;
    }

    c = MAIN_LOOP_EXIT_CODE;
    result = write(mainloop_handle.sockfds[1], &c, 1);
    if (result < 0)
        BTMG_ERROR("Couldn't send exit code, %s", strerror(errno));
    mainloop_handle.thread_enable = false;
    pthread_join(mainloop_handle.thread_id, NULL);
    pthread_mutex_unlock(&mainloop_mutex);
}

int bt_bluez_mainloop_init(void)
{
    int ret = -1;

    BTMG_DEBUG("enter");

    pthread_mutex_lock(&mainloop_mutex);

    mainloop_handle.ref++;

    if (mainloop_handle.thread_enable || mainloop_handle.ref > 1) {
        BTMG_DEBUG("mainloop thread has been alread running");
        goto end;
    }

    mainloop_init();

    ret = pthread_create(&mainloop_handle.thread_id, NULL, bt_mainloop_thread, NULL);
    if (ret != 0) {
        BTMG_ERROR("mainloop thread create failed.");
        pthread_mutex_unlock(&mainloop_mutex);
        return -1;
    }

end:

    pthread_mutex_unlock(&mainloop_mutex);
    BTMG_DEBUG("reference:%d", mainloop_handle.ref);
    return 0;
}
