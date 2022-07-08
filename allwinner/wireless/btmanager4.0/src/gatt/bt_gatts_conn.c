#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include "lib/l2cap.h"
#include "lib/uuid.h"
#include "src/shared/mainloop.h"
#include "src/shared/util.h"
#include "src/shared/att.h"
#include "src/shared/queue.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-server.h"
#include "bt_manager.h"
#include "common.h"
#include "bt_log.h"
#include "bt_mainloop.h"
#include "bt_gatt_server.h"
#include "bt_gatt_inner.h"

struct service_data {
    struct gatt_db_attribute *match;
    bool found;
    uint16_t handle;
};

static pthread_t gatt_tid;
gatt_server_t *gatt_server = NULL;
static int fd = -1;
static bool verbose = true;

static void att_debug_cb(const char *str, void *user_data)
{
    const char *prefix = user_data;

    BTMG_DEBUG("%s""%s",prefix, str);
}

static void gatt_debug_cb(const char *str, void *user_data)
{
    const char *prefix = user_data;

    BTMG_DEBUG("%s %s", prefix, str);
}

static void server_destroy(gatt_server_t *server)
{
    BTMG_DEBUG("Freeing gatt");
    bt_gatt_server_unref(server->gatts);
    BTMG_DEBUG("Freeing db");
    gatt_db_unref(server->dbs);
}

static void att_disconnect_cb(int err, void *user_data)
{
    BTMG_DEBUG("enter");
    BTMG_DEBUG("Device disconnected: %s", strerror(err));

    /* TODO: Report Disconnection Event */

    /* TODO: Whether we should  re-enable advertising */
    gatts_connection_event_t event = BT_GATT_DISCONNECT;

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_server_cb.gatts_connection_event_cb) {
        if (gatt_server && gatt_server->bd_addr) {
            btmg_cb_p->btmg_gatt_server_cb.gatts_connection_event_cb(gatt_server->bd_addr, event, err);
        }
    }
}

static int l2cap_le_att_accept(int sk)
{
    int nsk;
    socklen_t optlen;
    struct sockaddr_l2 addr;
    char ba[18];

    BTMG_DEBUG("enter");

    if (sk < 0)
        goto fail;

    memset(&addr, 0, sizeof(addr));
    optlen = sizeof(addr);
    nsk = accept(sk, (struct sockaddr *)&addr, &optlen);
    if (nsk < 0) {
        BTMG_ERROR("Accept failed");
        goto fail;
    }

    ba2str(&addr.l2_bdaddr, ba);
    memcpy(gatt_server->bd_addr, ba, AG_MAX_BDADDR_LEN);
    BTMG_DEBUG("Connect from %s", ba);

    gatts_connection_event_t event = BT_GATT_CONNECTION;

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_server_cb.gatts_connection_event_cb) {
        btmg_cb_p->btmg_gatt_server_cb.gatts_connection_event_cb(ba, event, 0);
    }

    return nsk;
fail:
    return -1;
}

static void server_listen_cb(int fd, uint32_t events, void *user_data)
{
    gatt_server_t *server = user_data;
    int accept_fd;
    int mtu = 517;

    BTMG_DEBUG("enter");
    if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        mainloop_remove_fd(fd);
        return;
    }

    accept_fd = l2cap_le_att_accept(fd);
    if (accept_fd < 0) {
        BTMG_ERROR("Accept error");
        return;
    }

    BTMG_DEBUG("accept_fd %d", accept_fd);
    server->fd = accept_fd;
    server->att = bt_att_new(accept_fd, false);
    if (!server->att) {
        BTMG_ERROR("Failed to initialze ATT transport layer");
        goto fail;
    }

    if (!bt_att_set_close_on_unref(server->att, true)) {
        BTMG_ERROR("Failed to set up ATT transport layer");
        goto fail;
    }

    if (!bt_att_register_disconnect(server->att, att_disconnect_cb, server, NULL)) {
        BTMG_ERROR("Failed to set ATT disconnect handler");
        goto fail;
    }

    server->gatts = bt_gatt_server_new(server->dbs, server->att, mtu, 0);
    if (!server->gatts) {
        BTMG_ERROR("Failed to create GATT server");
        goto fail;
    }

    if (verbose) {
        bt_att_set_debug(server->att, att_debug_cb, "att: ", NULL);
        bt_gatt_server_set_debug(server->gatts, gatt_debug_cb, "server: ", NULL);
    }

fail:
    return;
}

static int l2cap_le_att_listen(bdaddr_t *src, int sec, uint8_t src_type)
{
    int sk;
    struct sockaddr_l2 srcaddr;
    struct bt_security btsec;

    BTMG_DEBUG("enter");
    sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (sk < 0) {
        perror("Failed to create L2CAP socket");
        return -1;
    }

    /* Set up source address */
    memset(&srcaddr, 0, sizeof(srcaddr));
    srcaddr.l2_family = AF_BLUETOOTH;
    srcaddr.l2_cid = htobs(ATT_CID);
    srcaddr.l2_bdaddr_type = src_type;
    bacpy(&srcaddr.l2_bdaddr, src);

    if (bind(sk, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0) {
        BTMG_ERROR("Failed to bind L2CAP socket");
        goto fail;
    }

    /* Set the security level */
    memset(&btsec, 0, sizeof(btsec));
    btsec.level = sec;
    if (setsockopt(sk, SOL_BLUETOOTH, BT_SECURITY, &btsec, sizeof(btsec)) != 0) {
        BTMG_ERROR("Failed to set L2CAP security level");
        goto fail;
    }

    if (listen(sk, 10) < 0) {
        BTMG_ERROR("Listening on socket failed");
        goto fail;
    }

    BTMG_DEBUG("Started listening on ATT channel");

    return sk;

fail:
    close(sk);
    return -1;
}

int bt_conn_gatts_init()
{
    struct gatt_db *db;
    bdaddr_t src_addr;
    int sec = BT_SECURITY_LOW;
    uint8_t src_type = BDADDR_LE_PUBLIC;
    int ret;

    BTMG_DEBUG("enter");

    gatt_server = new0(gatt_server_t, 1);
    if (!gatt_server)
        return -1;

    gatt_server->dbs = gatt_db_new();
    if (!gatt_server->dbs) {
        BTMG_ERROR("failed to create GATT database");
        ret = -1;
        goto free_server;
    }

    if (bt_bluez_mainloop_init() != 0) {
        goto free_db;
    }

    int bt_gatt_db_add_default_services(gatt_client_t * server);
    bt_gatt_db_add_default_services(gatt_server);

    bacpy(&src_addr, BDADDR_ANY);

    fd = l2cap_le_att_listen(&src_addr, sec, src_type);

    ret = mainloop_add_fd(fd, EPOLLIN, server_listen_cb, gatt_server, NULL);
    if (ret < 0) {
        BTMG_ERROR("Failed to add listen socket(%d),error(%d)", fd, ret);
        close(fd);
        goto free_db;
    }

    return 0;

free_db:
    free(gatt_server->dbs);
free_server:
    free(gatt_server);

    return ret;
}

int bt_conn_gatts_deinit()
{
    BTMG_DEBUG("enter");
    if (fd >= 0) {
        mainloop_remove_fd(fd);
        close(fd);
        fd = -1;
    }
    if (gatt_server) {
        bt_gatt_server_unref(gatt_server->gatts);
        gatt_db_unref(gatt_server->dbs);
        if (gatt_server) {
            free(gatt_server);
            gatt_server = NULL;
        }
    }

    bt_bluez_mainloop_deinit();

    return 0;
}
