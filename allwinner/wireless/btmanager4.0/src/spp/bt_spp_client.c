
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

#define BUFFER_SIZE 1024

extern bt_rfcomm_t *rfcomm_t;
struct sockaddr_rc laddr;

static int auth = 0;
static int encryption = 0;
static int secure = 0;
static int master = 0;
static int rfcomm_raw_tty = 1;
static pthread_t recv_thread;
static bool recv_thread_start = false;
static pthread_mutex_t rfcomm_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t clean_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *recv_thread_process(void *arg);
static void clean(void *arg);

static void *recv_thread_process(void *arg)
{
    pthread_cleanup_push((void *)clean, "spp_client_recv_thread_process");
    recv_thread_start = true;
    char recv_msg[BUFFER_SIZE];
    int byte_num = 0;

    if (prctl(PR_SET_NAME, "spp_client_recv_thread") == -1) {
        BTMG_ERROR("unable to set recv thread name: %s", strerror(errno));
    }
    while (recv_thread_start) {
        bzero(recv_msg, BUFFER_SIZE);
        if (rfcomm_t->rfcomm_fd > 0) {
            byte_num = read(rfcomm_t->rfcomm_fd, recv_msg, BUFFER_SIZE);
            if (byte_num <= 0) {
                BTMG_DEBUG("SPP clisent disconnect...");
                break;
            }
            if (btmg_cb_p && btmg_cb_p->btmg_spp_client_cb.spp_client_recvdata_cb) {
                btmg_cb_p->btmg_spp_client_cb.spp_client_recvdata_cb(rfcomm_t->dst_addr, recv_msg,
                                                                     byte_num);
            }
        }
        usleep(1000 * 10);
    }

    pthread_cleanup_pop(1);
    pthread_exit(NULL);
}

int spp_client_send(char *data, uint32_t len)
{
    int status = 0;
    pthread_mutex_lock(&rfcomm_mutex);
    status = send(rfcomm_t->sock, data, len, 0);
    BTMG_DEBUG("send status:%d", status);
    pthread_mutex_unlock(&rfcomm_mutex);
    if (status < 0) {
        BTMG_ERROR("rfcomm send fail");
        return BT_ERROR;
    }

    return status;
}

void spp_client_regesiter_dev(int dev, const char *dst)
{
    struct rfcomm_dev_req req;
    dev_node_t *dev_node = NULL;
    char remote_name[256] = { 0 };
    struct termios ti;
    int try = 30, ctl;

    memset(&req, 0, sizeof(req));
    req.dev_id = dev;
    req.flags = (1 << RFCOMM_REUSE_DLC) | (1 << RFCOMM_RELEASE_ONHUP);

    memcpy(&req.dst, &laddr.rc_bdaddr, sizeof(bdaddr_t));
    req.channel = laddr.rc_channel;

    dev = ioctl(rfcomm_t->sock, RFCOMMCREATEDEV, &req);
    if (dev < 0) {
        BTMG_ERROR("Can't create RFCOMM TTY");
        close(rfcomm_t->sock);
        return;
    }

    snprintf(rfcomm_t->dev_name, MAXPATHLEN - 1, "/dev/rfcomm%d", dev);
    while ((rfcomm_t->rfcomm_fd = open(rfcomm_t->dev_name, O_RDWR | O_NOCTTY)) < 0) {
        if (errno == EACCES) {
            BTMG_ERROR("Can't open RFCOMM device");
            goto release;
        }
        snprintf(rfcomm_t->dev_name, MAXPATHLEN - 1, "/dev/bluetooth/rfcomm/%d", dev);
        if ((rfcomm_t->rfcomm_fd = open(rfcomm_t->dev_name, O_RDWR | O_NOCTTY)) < 0) {
            if (try--) {
                snprintf(rfcomm_t->dev_name, MAXPATHLEN - 1, "/dev/rfcomm%d", dev);
                usleep(100 * 1000);
                continue;
            }
            BTMG_ERROR("Can't open RFCOMM device");
            goto release;
        }
    }

    if (rfcomm_raw_tty) {
        tcflush(rfcomm_t->rfcomm_fd, TCIOFLUSH);
        cfmakeraw(&ti);
        tcsetattr(rfcomm_t->rfcomm_fd, TCSANOW, &ti);
    }

    bt_device_get_name(dst, remote_name);
    dev_node = btmg_dev_list_find_device(connected_devices, dst);
    if (dev_node == NULL) {
        btmg_dev_list_add_device(connected_devices, remote_name, dst, BTMG_REMOTE_DEVICE_SPP);
        BTMG_DEBUG("add spp device %s into connected_devices", dst);
    }

    if (btmg_cb_p && btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb) {
        BTMG_DEBUG("spp connected,device:%s", dst);
        btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb(dst,
                                                                     BTMG_SPP_CLIENT_CONNECTED);
    }

    memcpy(rfcomm_t->dst_addr, dst, sizeof(rfcomm_t->dst_addr));
    rfcomm_t->dev = dev;

release:
    memset(&req, 0, sizeof(req));
    req.dev_id = dev;
    req.flags = (1 << RFCOMM_HANGUP_NOW);
    ioctl(ctl, RFCOMMRELEASEDEV, &req);
}

int spp_client_connect(int dev, const char *dst)
{
    int ret = 0, fd = 0;
    static uint8_t SPP_UUID[16] = { 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00,
                                    0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };

    if (rfcomm_t == NULL) {
        BTMG_ERROR("rfcomm not init,need enable spp profile");
        return BT_ERROR;
    }
    int channel = getChannel(SPP_UUID, dst);
    BTMG_DEBUG("channel %d", channel);
    str2ba(dst, &laddr.rc_bdaddr);
    laddr.rc_family = AF_BLUETOOTH;
    laddr.rc_channel = channel;

    if (channel < 0) {
        BTMG_ERROR("spp client getChannel error");
        return BT_ERROR;
    }

    if (btmg_cb_p && btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb) {
        btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb(dst,
                                                                     BTMG_SPP_CLIENT_CONNECTING);
    }

    if (rfcomm_t->sock > 0) {
        rfcomm_release_dev(rfcomm_t->sock, dev);
    }

    rfcomm_t->sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (rfcomm_t->sock < 0) {
        BTMG_ERROR("Can't create RFCOMM socket");
        goto end;
    }

    if (connect(rfcomm_t->sock, (struct sockaddr *)&laddr, sizeof(laddr)) < 0) {
        BTMG_ERROR("Can't connect RFCOMM socket");
        close(rfcomm_t->sock);
        goto end;
    }

    spp_client_regesiter_dev(dev, dst);

    if (spp_client_start_recv_thread() == -1) {
        goto release;
    }

    return BT_OK;

release:
    close(rfcomm_t->sock);

end:
    if (btmg_cb_p && btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb) {
        btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb(
                dst, BTMG_SPP_CLIENT_CONNECT_FAILED);
    }
    return BT_ERROR;
}

int spp_client_disconnect(int dev, const char *dst)
{
    struct rfcomm_dev_req req;
    int err;

    BTMG_DEBUG("disconnect addr:%s", dst);

    if (btmg_cb_p && btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb) {
        btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb(dst,
                                                                     BTMG_SPP_CLIENT_DISCONNECTING);
    }

    recv_thread_start = false;
    if (rfcomm_t->sock > 0)
        rfcomm_release_dev(rfcomm_t->sock, dev);

    return spp_client_bt_disconnect(dst);
}

int spp_client_bt_disconnect(const char *addr)
{
    struct hci_conn_info_req *cr = NULL;
    bdaddr_t bdaddr;
    int opt, dd, dev_id;

    str2ba(addr, &bdaddr);

    dev_id = hci_for_each_dev(HCI_UP, find_conn, (long)&bdaddr);
    if (dev_id < 0) {
        BTMG_ERROR("Device:%s not connected", addr);
        return BT_ERROR;
    }

    dd = hci_open_dev(dev_id);
    if (dd < 0) {
        BTMG_ERROR("HCI device open failed");
        return BT_ERROR;
    }

    cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
    if (!cr) {
        BTMG_ERROR("Can't allocate memory");
        goto end;
    }

    bacpy(&cr->bdaddr, &bdaddr);
    cr->type = ACL_LINK;
    if (ioctl(dd, HCIGETCONNINFO, (unsigned long)cr) < 0) {
        BTMG_ERROR("Get connection info failed");
        goto end;
    }

    if (hci_disconnect(dd, htobs(cr->conn_info->handle), HCI_OE_USER_ENDED_CONNECTION, 10000) < 0) {
        BTMG_ERROR("Disconnect failed");
        goto end;
    }

    if (cr) {
        free(cr);
        cr = NULL;
    }
    hci_close_dev(dd);
    return BT_OK;

end:
    if (cr) {
        free(cr);
        cr = NULL;
    }
    hci_close_dev(dd);
    if (btmg_cb_p && btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb) {
        btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb(
                addr, BTMG_SPP_CLIENT_DISCONNEC_FAILED);
    }
    return BT_ERROR;
}

bool is_spp_client_run(void)
{
    return recv_thread_start;
}

static void clean(void *arg)
{
    pthread_mutex_lock(&clean_mutex);
    BTMG_DEBUG("enter...");
    if (recv_thread_start == true) {
        char dst[1024] = { 0 };
        ba2str(&laddr.rc_bdaddr, dst);
        spp_client_disconnect(0, dst);
    }
    recv_thread_start = false;
    BTMG_DEBUG("end...");
    pthread_mutex_unlock(&clean_mutex);
}

int spp_client_start_recv_thread(void)
{
    int ret = pthread_create(&recv_thread, NULL, recv_thread_process, NULL);

    if (ret < 0) {
        BTMG_ERROR("revc thread error");
        return BT_ERROR;
    }

    return BT_OK;
}

void spp_client_stop_recv_thread(void)
{
    if (recv_thread_start) {
        clean((void *)NULL);
    }
}
