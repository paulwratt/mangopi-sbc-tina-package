#ifndef __BT_GATT_INNER_H__
#define __BT_GATT_INNER_H__

#include "os_net_atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bt_conn {
    int fd;
    struct bt_att *att;

    struct gatt_db *dbc;
    struct bt_gatt_client *gattc;

    struct gatt_db *dbs;
    struct bt_gatt_server *gatts;

    evt_le_connection_complete conn;
    struct queue *notify_list;

    uint8_t *device_name;
    unsigned int name_len;
    char bd_addr[18];

    /* transaction id for a req/rsp sequence. */
    uint32_t trans_id;
    /* for current transaction */
    uint32_t cur_trans_id;
    unsigned int id; /* For internal att */

    bool in_read_req;
    uint8_t curr_opcode;

    uint16_t gatt_svc_chngd_handle;
    bool svc_chngd_enabled;

    atomic_t ref;
};

typedef struct bt_conn gatt_client_t;
typedef struct bt_conn gatt_server_t;
typedef struct bt_conn bt_conn_t;

extern struct queue *conn_list;
extern gatt_server_t *gatt_server;

#define BT_LE_GAP_HANDLE_INVALID ((uint16_t)(0xFFFF))

#define ATT_CID 4
#define LE_LINK 0x80

int bt_le_gap_find_conn(evt_le_connection_complete *conn);
bt_conn_t *bt_conn_ref(bt_conn_t *cli);
void bt_conn_unref(bt_conn_t *cli);

bt_conn_t *bt_conn_conn_id_to_conn_handle(int conn_handle);
int bt_conn_conn_handle_to_conn_id(bt_conn_t *cli);

int bt_conn_gattc_connect(int dev_id, bdaddr_t peer_addr, uint8_t addr_type, uint16_t mtu,
                          uint8_t security_level);
int bt_conn_gattc_disconnect(int dev_id, bdaddr_t peer_addr, uint8_t addr_type);
int bt_conn_gattc_init();
int bt_conn_gattc_deinit();
void bt_conn_foreach(int type, void (*func)(void *cli, void *data), void *data);

#ifdef __cplusplus
}
#endif

#endif // __BT_GATT_INNER_H__
