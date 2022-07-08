#ifndef __BT_GATT_CLIENT_H__
#define __BT_GATT_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <lib/bluetooth.h>
#include <lib/hci.h>
#include <lib/hci_lib.h>
#include <lib/l2cap.h>
#include <lib/uuid.h>
#include <api_action.h>

#include <src/shared/util.h>
#include <src/shared/att.h>
#include <src/shared/queue.h>
#include <src/shared/gatt-db.h>
#include <src/shared/gatt-client.h>

typedef struct {
    uint8_t *addr;
    uint8_t dst_type;
    uint16_t mtu;
    uint8_t sec;
} gattc_cn_args_t;

typedef struct {
    uint8_t *addr;
    uint8_t addr_type;
} gattc_dcn_args_t;

typedef struct {
    int conn_id;
    int sec_level;
} gattc_set_sec_args_t;

typedef struct {
    int conn_id;
    int id;
} gattc_unreg_notify_indicate_args_t;

typedef struct {
    int conn_id;
    int char_handle;
} gattc_reg_notify_indicate_args_t;

typedef struct {
    int conn_id;
    int char_handle;
    uint8_t *value;
    uint16_t len;
} gattc_write_req_args_t;

typedef struct {
    int conn_id;
    int char_handle;
    bool signed_write;
    uint8_t *value;
    uint16_t len;
} gattc_write_cmd_args_t;

typedef struct {
    int conn_id;
    bool reliable_writes;
    int char_handle;
    int offset;
    uint8_t *value;
    uint16_t len;
} gattc_write_long_req_args_t;

typedef struct {
    int conn_id;
    int char_handle;
    int offset;
} gattc_read_long_req_args_t;

typedef struct {
    int conn_id;
    int char_handle;
} gattc_read_req_args_t;

typedef struct {
    int conn_id;
    uint16_t start_handle;
    uint16_t end_handle;
} gattc_dis_all_svs_args_t;

typedef struct {
    int conn_id;
    const char *uuid;
    uint16_t start_handle;
    uint16_t end_handle;
} gattc_dis_svs_by_uuid_args_t;

typedef struct {
    int conn_id;
    void *svc_handle;
} gattc_dis_svc_all_char_args_t;

typedef struct {
    int conn_id;
    void *char_handle;
} gattc_dis_char_all_disc_args_t;

typedef enum {
    GATTC_INIT = 0,
    GATTC_DEINIT,
    GATTC_CONNECT,
    GATTC_DISCONNECT,
    GATTC_SET_SEC,
    GATTC_GET_SEC,
    GATTC_REG_NOTIFY_INDICATE,
    GATTC_UNREG_NOTIFY_INDICATE,
    GATTC_WRITE_REQ,
    GATTC_WRITE_CMD,
    GATTC_READ_REQ,
    GATTC_WRITE_LONG_REQ,
    GATTC_READ_LONG_REQ,
    GATTC_DIS_ALL_SVS,
    GATTC_DIS_SVS_BY_UUID,
    GATTC_DIS_SVC_ALL_CHAR,
    GATTC_DIS_CHAR_ALL_DESC,
    GATTC_GET_MTU,
    GATTC_GET_CONNECTED_LIST,
} gattc_call_t;

const char *bt_gattc_ecode_to_string(uint8_t ecode);
extern act_func_t gatt_client_action_table[];

#ifdef __cplusplus
}
#endif
#endif
