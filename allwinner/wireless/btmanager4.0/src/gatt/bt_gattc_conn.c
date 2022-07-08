#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>
#include "bt_manager.h"
#include "common.h"
#include "bt_log.h"
#include "bt_le_hci.h"
#include "bt_gatt_client.h"
#include "src/shared/gatt-server.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bt_mainloop.h>
#include <os_net_sync_notify.h>
#include "bt_gatt_inner.h"

struct queue *conn_list;

int sock_fd = -1;
static bool verbose = true;
snfy_handle_t *gattc_snfy_handle;

static void gatt_client_close(bt_conn_t *cli);

bt_conn_t *bt_conn_ref(bt_conn_t *cli)
{
    atomic_val_t old;

    BTMG_DEBUG("enter");
    /* Reference counter must be checked to avoid incrementing ref fr
	* zero, then we should return NULL instead.
	* Loop on clear-and-set in case someone has modified the referen
	* count since the read, and start over again when that happens.
	*/
    if (!cli) {
        BTMG_ERROR("client struct is NULL");
        return NULL;
    }
    do {
        old = atomic_get(&cli->ref);

        if (!old) {
            return NULL;
        }
    } while (!atomic_cas(&cli->ref, old, old + 1));

    BTMG_DEBUG("handle 0x%04x ref %u -> %u", cli->conn.handle, old, old + 1);

    return cli;
}

void bt_conn_unref(bt_conn_t *cli)
{
    BTMG_DEBUG("enter");

    if (!cli) {
        BTMG_ERROR("client struct is NULL");
        return;
    }
    atomic_val_t old = atomic_dec(&cli->ref);

    BTMG_DEBUG("handle %u ref %u -> %u", cli->conn.handle, old, atomic_get(&cli->ref));

    if (atomic_get(&cli->ref) == 0) {
        BTMG_INFO("atomic_get ref == 0, destory");
        // __ASSERT(old > 0, "Conn reference counter is 0");
        gatt_client_close(cli);
    }
}

static void gatt_client_close(bt_conn_t *cli)
{
    BTMG_DEBUG("enter");
    bool ret = 0;
    if (cli == NULL) {
        return;
    }
    ret = queue_remove(conn_list, cli);
    if (ret == false) {
        BTMG_WARNG("queue_remove failed");
    }

    if (cli->fd > 0) {
        close(cli->fd);
        cli->fd = -1;
    }

    if (cli->dbs) {
        gatt_db_unref(cli->dbs);
        cli->dbs = NULL;
    }
    if (cli->gattc) {
        bt_gatt_client_unref(cli->gattc);
        cli->gattc = NULL;
    }
    if (cli->att) {
        bt_att_unref(cli->att);
        cli->att = NULL;
    }
    free(cli);
}

static bool gattc_match_conn_handle(const void *data, const void *user_data)
{
    const bt_conn_t *cli = data;
    int conn_handle = *((int *)user_data);
    // BTMG_DEBUG("cli = %p, cli_handle=0x%04X, conn_handle=%x, iseq=%s", cli, cli->conn.handle,
    //            conn_handle, (cli->fd == conn_handle) ? "yes" : "no");

    return cli->conn.handle == conn_handle;
}

bt_conn_t *bt_conn_conn_id_to_conn_handle(int conn_handle)
{
    bt_conn_t *cli = NULL;

    cli = queue_find(conn_list, gattc_match_conn_handle, &(conn_handle));
    if (cli == NULL) {
        BTMG_ERROR("Error, no conn found for conn_handle 0x%04x", conn_handle);
        return NULL;
    }
    return bt_conn_ref(cli);
}

int bt_conn_conn_handle_to_conn_id(bt_conn_t *cli)
{
    if (cli) {
        return cli->conn.handle;
        /* code */
    }
    BTMG_ERROR("Error, no conn found for cli %p", cli);
    return -1;
}

static void att_connected_cb(bool success, uint8_t att_ecode, void *user_data)
{
    BTMG_DEBUG("enter");
    bt_conn_t *cli = (bt_conn_t *)user_data;

    gattc_conn_cb_para_t para;
    para.success = success;
    para.att_ecode = att_ecode;
    para.conn_id = bt_conn_conn_handle_to_conn_id(user_data);

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_conn_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_conn_cb(&para);
    }
}

static void att_disconnected_cb(int err, void *user_data)
{
    BTMG_DEBUG("enter");

    int ret = 0;
    gattc_disconn_cb_para_t para;
    bt_conn_t *cli = user_data;
    char addr_str[18];

    ba2str(&(cli->conn.peer_bdaddr), addr_str);

    BTMG_INFO("Device disconnected: %s, handle:%04x, role:%d, addr_type:%d", strerror(err),
              cli->conn.handle, cli->conn.role, cli->conn.peer_bdaddr_type);

    BTMG_DEBUG("peer addr %s, status=%d, supervision_timeout=%d", addr_str, cli->conn.status,
               cli->conn.supervision_timeout);

    switch (err) {
    case ECONNABORTED:
        para.reason = LOCAL_HOST_TERMINATED;
        break;
    case ETIMEDOUT:
        para.reason = CONNECTION_TIMEOUT;
        break;
    case ECONNRESET:
        para.reason = REMOTE_USER_TERMINATED;
        break;
    default:
        para.reason = UNKNOWN_OTHER_ERROR;
    }

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_disconn_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_disconn_cb(&para);
    }
    bt_conn_unref(cli);
}

static void att_debug_cb(const char *str, void *user_data)
{
    const char *prefix = user_data;
    BTMG_DEBUG("%s, %s", prefix, str);
}

static void gatt_debug_cb(const char *str, void *user_data)
{
    const char *prefix = user_data;
    BTMG_DEBUG("%s,%s", prefix, str);
}

static void log_service_event(struct gatt_db_attribute *attr, const char *str)
{
    char uuid_str[MAX_LEN_UUID_STR];
    bt_uuid_t uuid;
    uint16_t start, end;

    gatt_db_attribute_get_service_uuid(attr, &uuid);
    bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));

    gatt_db_attribute_get_service_handles(attr, &start, &end);
    BTMG_DEBUG("%s - UUID: %s start: 0x%04x end: 0x%04x\n", str, uuid_str, start, end);
}

static void service_added_cb(struct gatt_db_attribute *attr, void *user_data)
{
    log_service_event(attr, "Service Added");
}

static void service_removed_cb(struct gatt_db_attribute *attr, void *user_data)
{
    log_service_event(attr, "Service Removed");
}

static void service_changed_cb(uint16_t start_handle, uint16_t end_handle, void *user_data)
{
    BTMG_DEBUG("enter");

    gattc_service_changed_cb_para_t para;
    para.start_handle = start_handle;
    para.end_handle = end_handle;

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_service_changed_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_service_changed_cb(&para);
    }
}

static bt_conn_t *gatt_client_create(int fd, uint16_t mtu, bdaddr_t *dst, uint8_t dst_type)
{
    bt_conn_t *cli;

    cli = new0(bt_conn_t, 1);
    if (!cli) {
        BTMG_ERROR("Failed to allocate memory for client");
        return NULL;
    }

    bacpy(&cli->conn.peer_bdaddr, dst);
    cli->conn.peer_bdaddr_type = dst_type;
    if (bt_le_gap_find_conn(&cli->conn) < 0) {
        BTMG_ERROR("Failed to get GATT connection");
        free(cli);
        return NULL;
    }

    cli->att = bt_att_new(fd, false);
    if (!cli->att) {
        BTMG_ERROR("Failed to initialze ATT transport layer");
        bt_att_unref(cli->att);
        free(cli);
        return NULL;
    }
    atomic_set(&(cli->ref), 1);

    if (!bt_att_set_close_on_unref(cli->att, true)) {
        BTMG_ERROR("Failed to set up ATT transport layer");
        bt_att_unref(cli->att);
        free(cli);
        return NULL;
    }

    if (!bt_att_register_disconnect(cli->att, att_disconnected_cb, cli, NULL)) {
        fprintf(stderr, "Failed to set ATT disconnect handler\n");
        bt_att_unref(cli->att);
        free(cli);
        return NULL;
    }

    cli->fd = fd;
    cli->dbc = gatt_db_new();
    if (!cli->dbc) {
        fprintf(stderr, "Failed to create GATT database\n");
        bt_att_unref(cli->att);
        free(cli);
        return NULL;
    }

    cli->gattc = bt_gatt_client_new(cli->dbc, cli->att, mtu, 0);
    if (!cli->gattc) {
        fprintf(stderr, "Failed to create GATT client\n");
        gatt_db_unref(cli->dbc);
        bt_att_unref(cli->att);
        free(cli);
        return NULL;
    }

    gatt_db_register(cli->dbc, service_added_cb, service_removed_cb, NULL, NULL);

    if (verbose) {
        bt_att_set_debug(cli->att, att_debug_cb, "att: ", NULL);
        bt_gatt_client_set_debug(cli->gattc, gatt_debug_cb, "gatt: ", NULL);
    }

    if (gatt_server && gatt_server->dbs) {
        cli->dbs = gatt_db_ref(gatt_server->dbs);
        if (!cli->dbs) {
            fprintf(stderr, "Failed to create GATT database\n");
            gatt_db_unref(cli->dbs);
            cli->dbs = NULL;
            gatt_db_unref(cli->dbc);
            bt_att_unref(cli->att);
            free(cli);
            return NULL;
        }
        cli->gatts = bt_gatt_server_new(cli->dbs, cli->att, 517, 0);
        if (!cli->gatts) {
            fprintf(stderr, "Failed to create GATT server\n");
            gatt_db_unref(cli->dbs);
            cli->dbs = NULL;
            gatt_db_unref(cli->dbc);
            bt_att_unref(cli->att);
            free(cli);
            return NULL;
        }
    }
    if (conn_list == NULL) {
        conn_list = queue_new();
        if (conn_list == NULL) {
            BTMG_ERROR("Failed to new conn queue");
        }
    }
    if (queue_push_tail(conn_list, cli) == false) {
        BTMG_ERROR("Failed to push queuet");
        bt_conn_unref(cli);
    }
    bt_gatt_client_ready_register(cli->gattc, att_connected_cb, cli, NULL);
    bt_gatt_client_set_service_changed(cli->gattc, service_changed_cb, cli, NULL);

    /* bt_gatt_client already holds a reference */
    gatt_db_unref(cli->dbc);

    return cli;
}

static int l2cap_le_att_connect(bdaddr_t *src, bdaddr_t *dst, uint8_t dst_type, int sec)
{
    int sock;
    struct sockaddr_l2 srcaddr, dstaddr;
    struct bt_security btsec;
    char srcaddr_str[18], dstaddr_str[18];

    ba2str(src, srcaddr_str);
    ba2str(dst, dstaddr_str);
    BTMG_INFO("gatt-client: Opening L2CAP LE connection on ATT "
              "channel:\n\t src: %s\n\tdest: %s",
              srcaddr_str, dstaddr_str);

    sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (sock < 0) {
        BTMG_ERROR("Failed to create L2CAP socket:%s,(%d)", strerror(errno), errno);
        return -1;
    }

    /* Set up source address */
    memset(&srcaddr, 0, sizeof(srcaddr));
    srcaddr.l2_family = AF_BLUETOOTH;
    srcaddr.l2_cid = htobs(ATT_CID);
    srcaddr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    bacpy(&srcaddr.l2_bdaddr, src);

    if (bind(sock, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0) {
        BTMG_ERROR("Failed to bind L2CAP socket");
        close(sock);
        return -1;
    }

    /* Set the security level */
    memset(&btsec, 0, sizeof(btsec));
    btsec.level = sec;
    if (setsockopt(sock, SOL_BLUETOOTH, BT_SECURITY, &btsec, sizeof(btsec)) != 0) {
        BTMG_DEBUG("Failed to set L2CAP security level: %s", strerror(errno));
        close(sock);
        return -1;
    }

    /* Set up destination address */
    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.l2_family = AF_BLUETOOTH;
    dstaddr.l2_cid = htobs(ATT_CID);
    if (dst_type == BTMG_LE_PUBLIC_ADDRESS)
        dstaddr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    else if (dst_type == BTMG_LE_RANDOM_ADDRESS)
        dstaddr.l2_bdaddr_type = BDADDR_LE_RANDOM;
    bacpy(&dstaddr.l2_bdaddr, dst);

    BTMG_INFO("Connecting to device:%s", dstaddr_str);
    if (connect(sock, (struct sockaddr *)&dstaddr, sizeof(dstaddr)) < 0) {
        BTMG_DEBUG("Failed to connect: %s", strerror(errno));
        close(sock);
        return -1;
    }
    BTMG_INFO("Connect Done, sock:%d", sock);

    return sock;
}

// extern struct bt_hci *bt_hci_handle;
int bt_conn_gattc_connect(int dev_id, bdaddr_t peer_addr, uint8_t addr_type, uint16_t mtu,
                          uint8_t security_level)
{
    BTMG_DEBUG("enter");
    int ret = 0;
    int fd = -1;
    // gattc_cn_args_t *cn_args = (gattc_cn_args_t*)call_args[0];
    bdaddr_t src_addr;
    bt_conn_t *cli;
    // gattc_conn_cb_para_t para;

    if (dev_id == -1) {
        bacpy(&src_addr, BDADDR_ANY);
    } else if (hci_devba(dev_id, &src_addr) < 0) {
        BTMG_ERROR("Adapter not available");
        goto end;
    }

    // if(str2ba(cn_args->addr, &dst_addr) < 0) {
    // 	BTMG_ERROR("Invalid remote address:%s",cn_args->addr);
    // 	goto end;
    // }

    fd = l2cap_le_att_connect(&src_addr, &peer_addr, addr_type, security_level);
    if (fd < 0) {
        BTMG_ERROR("LE ATT connect failed.");
        goto end;
    }

    cli = gatt_client_create(fd, mtu, &peer_addr, addr_type);
    if (!cli) {
        BTMG_ERROR("Failed to create gatt client");
        close(fd);
        goto end;
    }
    return 0;
end:

    return -1;
}

static int bt_conn_gatt_destory_cli(int dev_id, bt_conn_t *cli)
{
    BTMG_DEBUG("enter");

    int dd, ret;
    uint16_t handle;
    uint8_t reason;

    if (cli == NULL) {
        return -1;
    }
    // queue_remove(conn_list, cli);
    handle = cli->conn.handle;
    bt_conn_unref(cli);
    bt_le_disconnect(dev_id, handle, HCI_OE_USER_ENDED_CONNECTION);
    return 0;
}

// todo may have problem
int bt_conn_gattc_disconnect(int dev_id, bdaddr_t peer_addr, uint8_t addr_type)
{
    BTMG_DEBUG("enter");

    char addr_str[18];
    ba2str(&peer_addr, addr_str);

    // int dd, ret;
    uint16_t conn_id;
    // uint8_t reason;
    bt_conn_t *cli;

    conn_id = find_ledev_conn_handle(dev_id, addr_str);
    if (conn_id < 0) {
        BTMG_ERROR("Could not find conn_id.");
        return -1;
    }

    cli = bt_conn_conn_id_to_conn_handle(conn_id);
    bt_conn_unref(cli);
    if (bt_conn_gatt_destory_cli(dev_id, cli) < 0) {
        return -1;
    }

    return 0;
}

int bt_conn_gattc_init()
{
    if (conn_list == NULL) {
        conn_list = queue_new();
        if (conn_list == NULL) {
            BTMG_ERROR("Failed to new conns queue");
            // bt_hci_unref(bt_hci_handle);
            // bt_hci_handle = NULL;
            return -1;
        }
    }
    return 0;
    // return bt_bluez_mainloop_init();
}

static void bt_conn_gattc_conn_list_destroy(void *data)
{
    bt_conn_t *cli = (bt_conn_t *)data;
    if (bt_conn_gatt_destory_cli(0, cli) < 0) {
        BTMG_ERROR("destory_cli failed");
        return;
    }
    //todo disconnect
}

int bt_conn_gattc_show_connlist();
int bt_conn_gattc_deinit()
{
    if (conn_list) {
        bt_conn_gattc_show_connlist();
    }
    if (conn_list) {
        queue_destroy(conn_list, bt_conn_gattc_conn_list_destroy);
        conn_list = NULL;
    }
    return 0;
}

static void show_connlist_handler(void *data, void *user_data)
{
    bt_conn_t *cli = data;
    if (cli) {
        char addr_str[18];
        ba2str(&(cli->conn.peer_bdaddr), addr_str);

        printf("[Device] handle:0x%04x, role:%d, peeraddr:%s(%s), status=%d,\n\t"
               "conn.interval=%d, cli->conn.latency=%d, supervision_timeout=%d\n",
               cli->conn.handle, cli->conn.role, addr_str,
               cli->conn.peer_bdaddr_type ? "random" : "public", cli->conn.status,
               cli->conn.interval, cli->conn.latency, cli->conn.supervision_timeout);
    }
}

void bt_conn_foreach(int type, void (*func)(void *cli, void *data), void *data)
{
    if (conn_list) {
        queue_foreach(conn_list, func, data);
    }
    return;
}

int bt_conn_gattc_show_connlist()
{
    bt_conn_foreach(0, show_connlist_handler, NULL);
    return 0;
}
