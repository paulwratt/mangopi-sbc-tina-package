#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <termios.h>
#include <poll.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include "channel.h"
#include "bt_rfcomm.h"
#include "bt_device.h"
#include "common.h"
#include "bt_log.h"

bt_rfcomm_t *rfcomm_t = NULL;
static pthread_mutex_t rfcomm_mutex = PTHREAD_MUTEX_INITIALIZER;

int rfcomm_init(void)
{
    rfcomm_t = (bt_rfcomm_t *)malloc(sizeof(bt_rfcomm_t));
    if (NULL == rfcomm_t) {
        BTMG_ERROR("malloc for rfcomm_t failed");
        return BT_ERROR;
    }
    memset(rfcomm_t, 0, sizeof(bt_rfcomm_t));

    return BT_OK;
}

int rfcomm_deinit(void)
{
    pthread_mutex_lock(&rfcomm_mutex);
    spp_client_stop_recv_thread();
    spp_service_stop_recv_thread();

    if (rfcomm_t->sock > 0) {
        rfcomm_release_dev(rfcomm_t->sock, rfcomm_t->dev);
    }
    if (rfcomm_t) {
        free(rfcomm_t);
        rfcomm_t = NULL;
    }
    pthread_mutex_unlock(&rfcomm_mutex);
    pthread_mutex_destroy(&rfcomm_mutex);

    return BT_OK;
}

int rfcomm_release_dev(int ctl, int dev)
{
    struct rfcomm_dev_req req;
    int err;

    memset(&req, 0, sizeof(req));
    req.dev_id = dev;
    req.flags = (1 << RFCOMM_HANGUP_NOW);
    err = ioctl(ctl, RFCOMMRELEASEDEV, &req);

    if (err < 0)
        BTMG_ERROR("Can't release device");

    if (rfcomm_t->rfcomm_fd > 0)
        close(rfcomm_t->rfcomm_fd);

    close(rfcomm_t->sock);
    rfcomm_t->sock = 0;
    rfcomm_t->rfcomm_fd = 0;

    return err;
}

int rfcomm_release_all_dev(int ctl)
{
    struct rfcomm_dev_list_req *dl;
    struct rfcomm_dev_info *di;
    int i;

    dl = malloc(sizeof(*dl) + RFCOMM_MAX_DEV * sizeof(*di));
    if (!dl) {
        BTMG_ERROR("Can't allocate memory");
        return BT_ERROR;
    }

    dl->dev_num = RFCOMM_MAX_DEV;
    di = dl->dev_info;

    if (ioctl(ctl, RFCOMMGETDEVLIST, (void *)dl) < 0) {
        BTMG_ERROR("Can't get device list");
        free(dl);
        return BT_ERROR;
    }

    for (i = 0; i < dl->dev_num; i++)
        rfcomm_release_dev(ctl, (di + i)->id);

    free(dl);

    return BT_OK;
}

int find_conn(int s, int dev_id, long arg)
{
    struct hci_conn_list_req *cl = NULL;
    struct hci_conn_info *ci = NULL;
    int i;

    if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
        BTMG_ERROR("Can't allocate memory");
        return BT_ERROR;
    }
    cl->dev_id = dev_id;
    cl->conn_num = 10;
    ci = cl->conn_info;

    if (ioctl(s, HCIGETCONNLIST, (void *)cl)) {
        BTMG_ERROR("Can't get connection list");
        free(cl);
        cl = NULL;
        return BT_ERROR;
    }

    for (i = 0; i < cl->conn_num; i++, ci++)
        if (!bacmp((bdaddr_t *)arg, &ci->bdaddr)) {
            free(cl);
            cl = NULL;
            return BT_ERROR;
        }

    free(cl);
    cl = NULL;

    return BT_OK;
}
