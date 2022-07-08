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
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bt_mainloop.h>
#include <os_net_sync_notify.h>
#include "bt_gatt_inner.h"

snfy_handle_t *gattc_snfy_handle;

const char *bt_gattc_ecode_to_string(uint8_t ecode)
{
    switch (ecode) {
    case BT_ATT_ERROR_INVALID_HANDLE:
        return "Invalid Handle";
    case BT_ATT_ERROR_READ_NOT_PERMITTED:
        return "Read Not Permitted";
    case BT_ATT_ERROR_WRITE_NOT_PERMITTED:
        return "Write Not Permitted";
    case BT_ATT_ERROR_INVALID_PDU:
        return "Invalid PDU";
    case BT_ATT_ERROR_AUTHENTICATION:
        return "Authentication Required";
    case BT_ATT_ERROR_REQUEST_NOT_SUPPORTED:
        return "Request Not Supported";
    case BT_ATT_ERROR_INVALID_OFFSET:
        return "Invalid Offset";
    case BT_ATT_ERROR_AUTHORIZATION:
        return "Authorization Required";
    case BT_ATT_ERROR_PREPARE_QUEUE_FULL:
        return "Prepare Write Queue Full";
    case BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND:
        return "Attribute Not Found";
    case BT_ATT_ERROR_ATTRIBUTE_NOT_LONG:
        return "Attribute Not Long";
    case BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE:
        return "Insuficient Encryption Key Size";
    case BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN:
        return "Invalid Attribute value len";
    case BT_ATT_ERROR_UNLIKELY:
        return "Unlikely Error";
    case BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION:
        return "Insufficient Encryption";
    case BT_ATT_ERROR_UNSUPPORTED_GROUP_TYPE:
        return "Group type Not Supported";
    case BT_ATT_ERROR_INSUFFICIENT_RESOURCES:
        return "Insufficient Resources";
    case BT_ERROR_CCC_IMPROPERLY_CONFIGURED:
        return "CCC Improperly Configured";
    case BT_ERROR_ALREADY_IN_PROGRESS:
        return "Procedure Already in Progress";
    case BT_ERROR_OUT_OF_RANGE:
        return "Out of Range";
    default:
        return "Unknown error type";
    }
}

static void read_cb(bool success, uint8_t att_ecode, const uint8_t *value, uint16_t length,
                    void *user_data)
{
    BTMG_DEBUG("enter");

    gattc_read_cb_para_t para;
    para.success = success;
    para.att_ecode = att_ecode;
    para.value = value;
    para.length = length;
    para.user_data = user_data;

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_read_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_read_cb(&para);
    }
    snfy_ready(gattc_snfy_handle, NULL);
}

static void write_cb(bool success, uint8_t att_ecode, void *user_data)
{
    BTMG_DEBUG("enter");

    gattc_write_cb_para_t para;
    para.success = success;
    para.att_ecode = att_ecode;
    para.user_data = user_data;
    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_write_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_write_cb(&para);
    }
    snfy_ready(gattc_snfy_handle, NULL);
}

static void write_long_cb(bool success, bool reliable_error, uint8_t att_ecode, void *user_data)
{
    BTMG_DEBUG("enter");

    gattc_write_long_cb_para_t para;

    para.success = success;
    para.reliable_error = reliable_error;
    para.att_ecode = att_ecode;
    para.user_data = user_data;
    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_write_long_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_write_long_cb(&para);
    }
    // snfy_ready(gattc_snfy_handle, NULL);
}

static void notify_indicate_cb(uint16_t value_handle, const uint8_t *value, uint16_t length, void *user_data)
{
    BTMG_DEBUG("enter");

    gattc_notify_indicate_cb_para_t para;

    para.value_handle = value_handle;
    para.value = value;
    para.length = length;

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_notify_indicate_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_notify_indicate_cb(&para);
    }
}

static void register_notify_indicate_cb(uint16_t att_ecode, void *user_data)
{
    BTMG_DEBUG("enter");

    if (att_ecode) {
        BTMG_ERROR("Failed to register notify/indicate handler "
                   "- error code: 0x%02x\n",
                   att_ecode);
        return;
    }

    BTMG_INFO("Registered notify handler!");
}

int bt_gattc_init(void **call_args, void **cb_args)
{
    int ret = 0;
    bt_conn_gattc_init();
    ret = bt_bluez_mainloop_init();
    RETURN_INT(ret);
}

int bt_gattc_deinit(void **call_args, void **cb_args)
{
    int ret = 0;
    bt_conn_gattc_deinit();
    ret = bt_bluez_mainloop_deinit();
    RETURN_INT(ret);
}

int bt_gattc_register_notify_indicate(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    int id = -1;
    gattc_reg_notify_indicate_args_t *r = (gattc_reg_notify_indicate_args_t *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(r->conn_id);

    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }

    id = bt_gatt_client_register_notify(cli->gattc, r->char_handle, register_notify_indicate_cb, notify_indicate_cb,
                                        NULL, NULL);
    bt_conn_unref(cli);
    BTMG_INFO("Registering notify/indicate handler with id: %u", id);
    RETURN_INT(id);
}

int bt_gattc_unregister_notify_indicate(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    gattc_unreg_notify_indicate_args_t *u = (gattc_unreg_notify_indicate_args_t *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(u->conn_id);
    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    if (!bt_gatt_client_unregister_notify(cli->gattc, u->id)) {
        BTMG_ERROR("Failed to unregister notify handler with id: %u", u->id);
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    bt_conn_unref(cli);
    RETURN_INT(0);
}

int bt_gattc_set_security(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    gattc_set_sec_args_t *sec_args = (gattc_set_sec_args_t *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(sec_args->conn_id);
    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_set_security(cli->gattc, sec_args->sec_level)) {
        BTMG_ERROR("Could not set sec level");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    } else {
        BTMG_DEBUG("Setting security level %d success", sec_args->sec_level);
    }

    bt_conn_unref(cli);
    RETURN_INT(0);
}

int bt_gattc_get_security(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    int level = -1;
    bt_conn_t *cli = (bt_conn_t *)call_args[0]; //todo
    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
    }
    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    level = bt_gatt_client_get_security(cli->gattc);
    if (level < 0)
        BTMG_ERROR("Could not set sec level");
    else
        BTMG_DEBUG("Security level: %u", level);

    RETURN_INT(level);
}

// extern struct bt_hci *bt_hci_handle;
int bt_gattc_connect(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    gattc_cn_args_t *cn_args = (gattc_cn_args_t *)call_args[0];
    int ret = 0;
    bdaddr_t dst_addr;
    gattc_conn_cb_para_t para;

    if (str2ba(cn_args->addr, &dst_addr) < 0) {
        BTMG_ERROR("Invalid remote address:%s", cn_args->addr);
        RETURN_INT(0);
    }
    ret = bt_conn_gattc_connect(0, dst_addr, cn_args->dst_type, cn_args->mtu,
                                cn_args->sec);
    if (ret) { // connect failed
        para.success = false;
        para.att_ecode = 0xff;
        para.conn_id = 0;
        para.user_data = NULL;
        if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_conn_cb) {
            btmg_cb_p->btmg_gatt_client_cb.gattc_conn_cb(&para);
        }
        RETURN_INT(-1);
    }
    RETURN_INT(0);
}

int bt_gattc_disconnect(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    gattc_dcn_args_t *dcn_args = (gattc_dcn_args_t *)call_args[0];
    int ret = 0;
    bdaddr_t dst_addr;
    gattc_disconn_cb_para_t para;

    if (str2ba(dcn_args->addr, &dst_addr) < 0) {
        BTMG_ERROR("Invalid remote address:%s", dcn_args->addr);
        RETURN_INT(-1);
    }

    ret = bt_conn_gattc_disconnect(0, dst_addr, 0xFF);
    if (ret) {
        BTMG_INFO("disconnect failed");
    }

    para.reason = LOCAL_HOST_TERMINATED;
    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_disconn_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_disconn_cb(&para); //todo para
    }
    RETURN_INT(0);
}

static void gattc_foreach_service_cb(struct gatt_db_attribute *attr, void *user_data)
{
    bt_conn_t *cli = (bt_conn_t *)user_data;
    uint16_t start, end;
    bool primary;
    bt_uuid_t uuid;

    if (!gatt_db_attribute_get_service_data(attr, &start, &end, &primary, &uuid))
        return;

    gattc_dis_service_cb_para_t para;
    para.conn_id = bt_conn_conn_handle_to_conn_id(cli);
    para.start_handle = start;
    para.end_handle = end;
    para.primary = primary;
    para.attr = (void *)attr;

    memcpy(&para.uuid, &uuid, sizeof(btmg_uuid_t));
    char uuid_str[MAX_LEN_UUID_STR];
    bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));

    BTMG_DEBUG("start handle:%04x, end handle:%04x, uuid:%s\n", start, end, uuid_str);
    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_dis_service_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_dis_service_cb(&para);
    }
}

int bt_gattc_discover_all_services(void **call_args, void **cb_args)
{
    gattc_dis_all_svs_args_t *s = (gattc_dis_all_svs_args_t *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(s->conn_id);

    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    BTMG_DEBUG("conn_id = %d,start handle:%04x,end handle:%04x\n", s->conn_id, s->start_handle, s->end_handle);
    gatt_db_foreach_service_in_range(cli->dbc, NULL, gattc_foreach_service_cb, cli, s->start_handle,
                                     s->end_handle);

    bt_conn_unref(cli);
    RETURN_INT(0);
}

int bt_gattc_discover_primary_services_by_uuid(void **call_args, void **cb_args)
{
    gattc_dis_svs_by_uuid_args_t *s = (gattc_dis_svs_by_uuid_args_t *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(s->conn_id);
    bt_uuid_t tmp, _uuid;

    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }

    if (bt_string_to_uuid(&tmp, s->uuid) < 0) {
        BTMG_ERROR("Invalid UUID: %s\n", s->uuid);
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }

    bt_uuid_to_uuid128(&tmp, &_uuid);

    BTMG_DEBUG("start handle:%04x,end handle:%04x,uuid:%s\n", s->start_handle, s->end_handle,
               s->uuid);
    gatt_db_foreach_service_in_range(cli->dbc, &_uuid, gattc_foreach_service_cb, cli,
                                     s->start_handle, s->end_handle);

    bt_conn_unref(cli);
    RETURN_INT(0);
}

static void gattc_discover_service_all_char_cb(struct gatt_db_attribute *attr, void *user_data)
{
    BTMG_DEBUG("enter");
    bt_conn_t *cli = (bt_conn_t *)user_data;

    uint16_t handle, value_handle;
    uint8_t properties;
    uint16_t ext_prop;
    bt_uuid_t uuid;

    if (!gatt_db_attribute_get_char_data(attr, &handle, &value_handle, &properties, &ext_prop,
                                         &uuid))
        return;
    gattc_dis_char_cb_para_t para;
    para.conn_id = bt_conn_conn_handle_to_conn_id(cli);
    para.start_handle = handle;
    para.value_handle = value_handle;
    para.properties = properties;
    para.ext_prop = ext_prop;
    para.attr = (void *)attr;

    memcpy(&para.uuid, &uuid, sizeof(btmg_uuid_t));

    char uuid_str[MAX_LEN_UUID_STR];
    bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
    BTMG_DEBUG("start_handle:%04x,value_handle:%04x,properties:%04x,ext_prop:%04x,uuid:%s\n",
               handle, value_handle, properties, ext_prop, uuid_str);

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_dis_char_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_dis_char_cb(&para);
    }
}

int bt_gattc_discover_service_all_char(void **call_args, void **cb_args)
{
    gattc_dis_svc_all_char_args_t *s = (gattc_dis_svc_all_char_args_t *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(s->conn_id);
    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    struct gatt_db_attribute *attr = (struct gatt_db_attribute *)s->svc_handle;

    if (attr == NULL) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }

    gatt_db_service_foreach_char(attr, gattc_discover_service_all_char_cb, cli);

    bt_conn_unref(cli);
    RETURN_INT(0);
}

static void gattc_discover_char_all_descriptor_cb(struct gatt_db_attribute *attr, void *user_data)
{
    gattc_dis_desc_cb_para_t para;

    para.handle = gatt_db_attribute_get_handle(attr);
    memcpy(&para.uuid, gatt_db_attribute_get_type(attr), sizeof(btmg_uuid_t));

    char uuid_str[MAX_LEN_UUID_STR];
    bt_uuid_to_string(gatt_db_attribute_get_type(attr), uuid_str, sizeof(uuid_str));
    BTMG_DEBUG("_handle:%04x,uuid:%s\n", para.handle, uuid_str);

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_dis_desc_cb) {
        btmg_cb_p->btmg_gatt_client_cb.gattc_dis_desc_cb(&para);
    }
}

int bt_gattc_discover_char_all_descriptor(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    gattc_dis_char_all_disc_args_t *s = (gattc_dis_char_all_disc_args_t *)call_args[0];
    struct gatt_db_attribute *attr = (struct gatt_db_attribute *)s->char_handle;

    if (attr == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }
    gatt_db_service_foreach_desc(attr, gattc_discover_char_all_descriptor_cb, NULL);

    RETURN_INT(0);
}

int bt_gattc_get_mtu(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    int mtu = -1;
    int conn_id = *(int *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(conn_id);
    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }
    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    mtu = bt_gatt_client_get_mtu(cli->gattc);
    RETURN_INT(mtu);
}

int bt_gattc_read_request(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    gattc_read_req_args_t *r = (gattc_read_req_args_t *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(r->conn_id);
    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    gattc_snfy_handle = snfy_new();
    if (!bt_gatt_client_read_value(cli->gattc, r->char_handle, read_cb, NULL, NULL)) {
        BTMG_ERROR("Failed to initiate read value procedure");
        snfy_free(gattc_snfy_handle);
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    snfy_await(gattc_snfy_handle);

    bt_conn_unref(cli);
    RETURN_INT(0);
}

int bt_gattc_write_long_value_request(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");
    gattc_write_long_req_args_t *w = (gattc_write_long_req_args_t *)call_args[0];

    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(w->conn_id);
    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }

    // gattc_snfy_handle = snfy_new();
    if (!bt_gatt_client_write_long_value(cli->gattc, w->reliable_writes,
        w->char_handle, w->offset, w->value, w->len, write_long_cb, NULL, NULL)) {
        BTMG_ERROR("Failed to initiate long write procedure");
        // snfy_free(gattc_snfy_handle);
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    // snfy_await(gattc_snfy_handle);
    bt_conn_unref(cli);
    RETURN_INT(0);
}

int bt_gattc_read_long_request(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    gattc_read_long_req_args_t *r = (gattc_read_long_req_args_t *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(r->conn_id);
    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    gattc_snfy_handle = snfy_new();
    if (!bt_gatt_client_read_long_value(cli->gattc, r->char_handle, r->offset, read_cb, NULL,
                                        NULL)) {
        BTMG_ERROR("Failed to initiate read loog value procedure");
        snfy_free(gattc_snfy_handle);
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    snfy_await(gattc_snfy_handle);

    bt_conn_unref(cli);
    RETURN_INT(0);
}

int bt_gattc_write_command(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    gattc_write_cmd_args_t *w = (gattc_write_cmd_args_t *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(w->conn_id);
    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    // gattc_snfy_handle = snfy_new();
    if (!bt_gatt_client_write_without_response(cli->gattc, w->char_handle, w->signed_write,
                                               w->value, w->len)) {
        BTMG_ERROR("Failed to initiate write without response "
                   "procedure");
        // snfy_free(gattc_snfy_handle);
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    // snfy_await(gattc_snfy_handle);
    bt_conn_unref(cli);
    RETURN_INT(0);
}

int bt_gattc_write_request(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    gattc_write_req_args_t *w = (gattc_write_req_args_t *)call_args[0];
    bt_conn_t *cli = bt_conn_conn_id_to_conn_handle(w->conn_id);
    if (cli == NULL) {
        BTMG_ERROR("GATT client not initialized");
        RETURN_INT(-1);
    }

    if (!bt_gatt_client_is_ready(cli->gattc)) {
        BTMG_ERROR("GATT client not initialized");
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }

    gattc_snfy_handle = snfy_new();
    if (!bt_gatt_client_write_value(cli->gattc, w->char_handle, w->value, w->len, write_cb, NULL,
                                    NULL)) {
        printf("Failed to initiate write procedure\n");
        snfy_free(gattc_snfy_handle);
        bt_conn_unref(cli);
        RETURN_INT(-1);
    }
    snfy_await(gattc_snfy_handle);

    bt_conn_unref(cli);
    RETURN_INT(0);
}

static void gattc_get_connected_list_handler(void *data, void *user_data)
{
    bt_conn_t *cli = data;
    if (cli) {
        char addr_str[18];
        ba2str(&(cli->conn.peer_bdaddr), addr_str);

        BTMG_DEBUG("[Device] handle:0x%04x, role:%d, peeraddr:%s(%s), status=%d,\n\t"
                   "conn.interval=%d, cli->conn.latency=%d, supervision_timeout=%d",
                   cli->conn.handle, cli->conn.role, addr_str,
                   cli->conn.peer_bdaddr_type ? "random" : "public", cli->conn.status,
                   cli->conn.interval, cli->conn.latency, cli->conn.supervision_timeout);

        gattc_connected_list_cb_para_t para;
        para.conn_id = bt_conn_conn_handle_to_conn_id(cli);
        para.addr = (void *)(addr_str);
        para.addr_type = cli->conn.peer_bdaddr_type ? BTMG_LE_RANDOM_ADDRESS : BTMG_LE_PUBLIC_ADDRESS;
        // para.user_data = user_data;

        if (btmg_cb_p && btmg_cb_p->btmg_gatt_client_cb.gattc_connected_list_cb) {
            btmg_cb_p->btmg_gatt_client_cb.gattc_connected_list_cb(&para);
        }
    }
}

int bt_gattc_get_connected_list(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");

    bt_conn_foreach(0, gattc_get_connected_list_handler, NULL);

    RETURN_INT(0);
}

act_func_t gatt_client_action_table[] = {
    [GATTC_INIT]                  = { bt_gattc_init,                              "init" },
    [GATTC_DEINIT]                = { bt_gattc_deinit,                            "deinit" },
    [GATTC_CONNECT]               = { bt_gattc_connect,                           "connect" },
    [GATTC_DISCONNECT]            = { bt_gattc_disconnect,                        "disconnect" },
    [GATTC_SET_SEC]               = { bt_gattc_set_security,                      "set_sec" },
    [GATTC_GET_SEC]               = { bt_gattc_get_security,                      "get_sec" },
    [GATTC_REG_NOTIFY_INDICATE]   = { bt_gattc_register_notify_indicate,          "reg_notify_indicate" },
    [GATTC_UNREG_NOTIFY_INDICATE] = { bt_gattc_unregister_notify_indicate,        "unreg_notify_indicate" },
    [GATTC_WRITE_REQ]             = { bt_gattc_write_request,                     "write_req" },
    [GATTC_WRITE_CMD]             = { bt_gattc_write_command,                     "write_cmd" },
    [GATTC_READ_REQ]              = { bt_gattc_read_request,                      "read_req" },
    [GATTC_WRITE_LONG_REQ]        = { bt_gattc_write_long_value_request,          "write_long_req"},
    [GATTC_READ_LONG_REQ]         = { bt_gattc_read_long_request,                 "read_long_req" },
    [GATTC_DIS_ALL_SVS]           = { bt_gattc_discover_all_services,             "dis_all_svc" },
    [GATTC_DIS_SVS_BY_UUID]       = { bt_gattc_discover_primary_services_by_uuid, "dis_all_svc_by_uuid" },
    [GATTC_DIS_SVC_ALL_CHAR]      = { bt_gattc_discover_service_all_char,         "dis_all_svc_char" },
    [GATTC_DIS_CHAR_ALL_DESC]     = { bt_gattc_discover_char_all_descriptor,      "dis_all_desc" },
    [GATTC_GET_MTU]               = { bt_gattc_get_mtu,                           "get_mtu" },
    [GATTC_GET_CONNECTED_LIST]    = { bt_gattc_get_connected_list,                "get_conn_list" }
};
