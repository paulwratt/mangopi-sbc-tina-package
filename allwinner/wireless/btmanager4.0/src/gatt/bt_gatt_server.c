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
#include "bt_le_hci.h"

struct service_data {
    struct gatt_db_attribute *match;
    bool found;
    uint16_t handle;
};

void print_uuid(const bt_uuid_t *uuid)
{
    char uuid_str[MAX_LEN_UUID_STR];
    bt_uuid_t uuid128;

    bt_uuid_to_uuid128(uuid, &uuid128);
    bt_uuid_to_string(&uuid128, uuid_str, sizeof(uuid_str));

    BTMG_DEBUG("%s", uuid_str);
}

static int bt_gatts_init(void **call_args, void **cb_args)
{
    int ret = 0;
    int bt_conn_gatts_init();
    ret = bt_conn_gatts_init();
    RETURN_INT(ret);
}

static int bt_gatts_deinit(void **call_args, void **cb_args)
{
    int ret = 0;
    int bt_conn_gatts_deinit();
    ret = bt_conn_gatts_deinit();
    RETURN_INT(ret);
}

static int bt_gatts_add_service(void **call_args, void **cb_args)
{
    struct gatt_db_attribute *service;
    bt_uuid_t uuid;

    gatts_add_svc_t *s = (gatts_add_svc_t *)call_args[0];

    BTMG_DEBUG("enter");
    BTMG_DEBUG("service uuid: %s", s->uuid);

    bt_string_to_uuid(&uuid, s->uuid);
    print_uuid(&uuid);
    BTMG_DEBUG("handle_num = %d, primary: %s", s->number, s->primary ? "yes" : "no");

    service = gatt_db_add_service(gatt_server->dbs, &uuid, s->primary, s->number);
    if (!service) {
        BTMG_ERROR("add service failed.");
        RETURN_INT(-1);
    }

    uint16_t start_handle;

    gatt_db_attribute_get_service_handles(service, &start_handle, NULL);

    gatts_add_svc_msg_t server_msg;

    server_msg.svc_handle = start_handle;
    server_msg.handle_num = s->number;

    gatt_db_service_set_active(service, true);

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_server_cb.gatts_add_svc_cb) {
        btmg_cb_p->btmg_gatt_server_cb.gatts_add_svc_cb(&server_msg);
    }

    RETURN_INT(0);
}

static void attrib_char_read_cb(struct gatt_db_attribute *attrib, unsigned int id, uint16_t offset,
                                uint8_t opcode, struct bt_att *att, void *user_data)
{
    uint16_t attr_handle = gatt_db_attribute_get_handle(attrib);
    bool is_long_read;
    uint32_t trans_id;

    BTMG_DEBUG("enter");
    gatt_server->id = id;
    gatt_server->in_read_req = true;
    gatt_server->curr_opcode = opcode;

    trans_id = gatt_server->trans_id++;
    gatt_server->cur_trans_id = trans_id;

    if (opcode == BT_ATT_OP_READ_REQ)
        is_long_read = false;
    else if (opcode == BT_ATT_OP_READ_BLOB_REQ)
        is_long_read = true;
    else {
        is_long_read = false;
        BTMG_DEBUG("Unexpected opcode for read req");
    }

    BTMG_DEBUG("read_cb called");

    gatts_char_read_req_t ChrMsg;
    ChrMsg.attr_handle = attr_handle;
    ChrMsg.is_blob_req = is_long_read;
    ChrMsg.offset = offset;
    ChrMsg.trans_id = id;
    if (btmg_cb_p && btmg_cb_p->btmg_gatt_server_cb.gatts_char_read_req_cb) {
        btmg_cb_p->btmg_gatt_server_cb.gatts_char_read_req_cb(&ChrMsg);
    }
}

static void attrib_char_write_cb(struct gatt_db_attribute *attrib, unsigned int id, uint16_t offset,
                                 const uint8_t *value, size_t len, uint8_t opcode,
                                 struct bt_att *att, void *user_data)
{
    uint16_t attr_handle;
    bool need_rsp;
    uint32_t trans_id;

    BTMG_DEBUG("enter");

    gatt_server->id = id;
    gatt_server->in_read_req = false;
    gatt_server->curr_opcode = opcode;

    attr_handle = gatt_db_attribute_get_handle(attrib);

    trans_id = gatt_server->trans_id++;
    gatt_server->cur_trans_id = trans_id;

    if (opcode == BT_ATT_OP_WRITE_REQ)
        need_rsp = true;
    else if (opcode == BT_ATT_OP_WRITE_CMD)
        need_rsp = false;
    else
        BTMG_DEBUG("Unexpected opcode %d", opcode);

    BTMG_DEBUG("write_cb called");

    if (opcode == BT_ATT_OP_WRITE_CMD)
        gatt_db_attribute_write_result(attrib, id, 0);

    gatts_char_write_req_t ChrWriteMsg;
    memcpy(ChrWriteMsg.value, value, len);
    ChrWriteMsg.value_len = len;
    ChrWriteMsg.attr_handle = attr_handle;
    ChrWriteMsg.trans_id = id;
    ChrWriteMsg.offset = offset;
    ChrWriteMsg.need_rsp = need_rsp;
    if (btmg_cb_p && btmg_cb_p->btmg_gatt_server_cb.gatts_char_write_req_cb) {
        btmg_cb_p->btmg_gatt_server_cb.gatts_char_write_req_cb(&ChrWriteMsg);
    }
}

static void attrib_desc_read_cb(struct gatt_db_attribute *attrib, unsigned int id, uint16_t offset,
                                uint8_t opcode, struct bt_att *att, void *user_data)
{
    uint16_t attr_handle = gatt_db_attribute_get_handle(attrib);
    bool is_long_read;
    uint32_t trans_id;

    BTMG_DEBUG("enter");
    gatt_server->id = id;
    gatt_server->in_read_req = true;
    gatt_server->curr_opcode = opcode;

    trans_id = gatt_server->trans_id++;
    gatt_server->cur_trans_id = trans_id;

    if (opcode == BT_ATT_OP_READ_REQ)
        is_long_read = false;
    else if (opcode == BT_ATT_OP_READ_BLOB_REQ)
        is_long_read = true;
    else {
        is_long_read = false;
        BTMG_DEBUG("Unexpected opcode for read req");
    }

    BTMG_DEBUG("desc read_cb called");

    gatts_desc_read_req_t DescMsg;
    DescMsg.attr_handle = attr_handle;
    DescMsg.is_blob_req = is_long_read;
    DescMsg.offset = offset;
    DescMsg.trans_id = id;
    if (btmg_cb_p && btmg_cb_p->btmg_gatt_server_cb.gatts_desc_read_req_cb) {
        btmg_cb_p->btmg_gatt_server_cb.gatts_desc_read_req_cb(&DescMsg);
    }
}

static void attrib_desc_write_cb(struct gatt_db_attribute *attrib, unsigned int id, uint16_t offset,
                                 const uint8_t *value, size_t len, uint8_t opcode,
                                 struct bt_att *att, void *user_data)
{
    uint16_t attr_handle;
    bool need_rsp;
    uint32_t trans_id;

    BTMG_DEBUG("enter");

    gatt_server->id = id;
    gatt_server->in_read_req = false;
    gatt_server->curr_opcode = opcode;

    attr_handle = gatt_db_attribute_get_handle(attrib);

    trans_id = gatt_server->trans_id++;
    gatt_server->cur_trans_id = trans_id;

    if (opcode == BT_ATT_OP_WRITE_REQ)
        need_rsp = true;
    else if (opcode == BT_ATT_OP_WRITE_CMD)
        need_rsp = false;
    else
        BTMG_DEBUG("Unexpected opcode %d", opcode);

    BTMG_DEBUG("desc write_cb called");

    gatts_desc_write_req_t DescWriteMsg;
    memcpy(DescWriteMsg.value, value, len);
    DescWriteMsg.value_len = len;
    DescWriteMsg.attr_handle = attr_handle;
    DescWriteMsg.trans_id = id;
    DescWriteMsg.offset = offset;
    DescWriteMsg.need_rsp = need_rsp;

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_server_cb.gatts_desc_write_req_cb) {
        btmg_cb_p->btmg_gatt_server_cb.gatts_desc_write_req_cb(&DescWriteMsg);
    }
}

static void find_matching_service(struct gatt_db_attribute *service, void *user_data)
{
    struct service_data *service_data = user_data;
    uint16_t start_handle;

    BTMG_DEBUG("enter");
    if (service_data->found)
        return;

    gatt_db_attribute_get_service_handles(service, &start_handle, NULL);

    if (service_data->handle == start_handle) {
        service_data->found = true;
        service_data->match = service;
    }
}

static struct gatt_db_attribute *find_service_by_handle(uint16_t handle, struct gatt_db *db)
{
    struct service_data service_data;

    BTMG_DEBUG("enter");

    service_data.handle = handle;
    service_data.found = false;

    gatt_db_foreach_service(db, NULL, find_matching_service, &service_data);

    if (service_data.found)
        return service_data.match;
    return NULL;
}

static int bt_gatts_add_char(void **call_args, void **cb_args)
{
    struct gatt_db_attribute *service;
    struct gatt_db_attribute *character;
    bt_uuid_t uuid;

    BTMG_DEBUG("enter");

    gatts_add_char_t *c = (gatts_add_char_t *)call_args[0];
    bt_string_to_uuid(&uuid, c->uuid);
    print_uuid(&uuid);

    BTMG_DEBUG("server handle : %d , prop: %d ,perm :%d", c->svc_handle, c->properties,
               c->permissions);

    service = find_service_by_handle(c->svc_handle, gatt_server->dbs);

    if (!service) {
        BTMG_DEBUG("no service found by %d", c->svc_handle);
        RETURN_INT(-1);
    }

    character = gatt_db_service_add_characteristic(service, &uuid, c->permissions, c->properties,
                                                   attrib_char_read_cb, attrib_char_write_cb,
                                                   gatt_server);

    uint16_t char_handle = gatt_db_attribute_get_handle(character);

    BTMG_DEBUG("Permission of characteristic at handle %u is 0x%08x", char_handle, c->permissions);

    gatts_add_char_msg_t AddChrMsg;

    AddChrMsg.char_handle = char_handle;

    AddChrMsg.uuid = c->uuid;

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_server_cb.gatts_add_char_cb) {
        btmg_cb_p->btmg_gatt_server_cb.gatts_add_char_cb(&AddChrMsg);
    }
    RETURN_INT(0);
}

static int bt_gatts_add_desc(void **call_args, void **cb_args)
{
    struct gatt_db_attribute *service;
    struct gatt_db_attribute *desc;
    bt_uuid_t uuid;

    BTMG_DEBUG("enter");

    gatts_add_desc_t *d = (gatts_add_desc_t *)call_args[0];
    bt_string_to_uuid(&uuid, d->uuid);
    print_uuid(&uuid);

    BTMG_DEBUG("server handle : %d ,perm :%d", d->svc_handle, d->permissions);

    service = find_service_by_handle(d->svc_handle, gatt_server->dbs);
    if (!service) {
        BTMG_DEBUG("No service found by %d", d->svc_handle);
        RETURN_INT(-1);
    }

    desc = gatt_db_service_add_descriptor(service, &uuid, d->permissions, attrib_desc_read_cb,
                                          attrib_desc_write_cb, gatt_server);

    uint16_t desc_handle = gatt_db_attribute_get_handle(desc);

    BTMG_DEBUG("Permission of desc at handle %u is %08x", desc_handle, d->permissions);
    gatts_add_desc_msg_t AddDescMsg;

    AddDescMsg.desc_handle = desc_handle;

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_server_cb.gatts_add_desc_cb) {
        btmg_cb_p->btmg_gatt_server_cb.gatts_add_desc_cb(&AddDescMsg);
    }

    RETURN_INT(0);
}

static int bt_gatts_start_service(void **call_args, void **cb_args)
{
	struct gatt_db_attribute *service;
    gatts_star_svc_t *s = (gatts_star_svc_t *)call_args[0];

    BTMG_DEBUG("server handle : %d", s->svc_handle);
    service = find_service_by_handle(s->svc_handle, gatt_server->dbs);
    if (!service) {
        BTMG_DEBUG("no service found by %d", s->svc_handle);
        RETURN_INT(-1);
    }
    gatt_db_service_set_active(service, false);


    RETURN_INT(0);
}

static int bt_gatts_stop_service(void **call_args, void **cb_args)
{
    struct gatt_db_attribute *service;

    BTMG_DEBUG("enter");

    gatts_stop_svc_t *s = (gatts_stop_svc_t *)call_args[0];
    service = find_service_by_handle(s->svc_handle, gatt_server->dbs);
    if (!service) {
        BTMG_DEBUG("no service found by %d", s->svc_handle);
        RETURN_INT(-1);
    }
    gatt_db_service_set_active(service, false);

    RETURN_INT(0);
}

static int bt_gatts_get_mtu(void **call_args, void **cb_args)
{
    int mtu;

    if ((!gatt_server) || (!gatt_server->att)) {
        BTMG_ERROR("Failed to initialze ATT transport layer");
        RETURN_INT(0);
    }
    mtu = (int)bt_att_get_mtu(gatt_server->att);

    memcpy(cb_args[0], (void *)&mtu, sizeof(int));

    RETURN_INT(mtu);
}

static int bt_gatts_send_write_rsp(void **call_args, void **cb_args)
{
    bool ret = false;
    struct gatt_db_attribute *attrib;

    gatts_write_rsp_t *s = (gatts_write_rsp_t *)call_args[0];

    attrib = gatt_db_get_attribute(gatt_server->dbs, s->attr_handle);
    if (!attrib) {
        BTMG_ERROR("couldn't find attribute for handle: %d", s->attr_handle);
        RETURN_INT(-1);
    }

    ret = gatt_db_attribute_write_result(attrib, s->trans_id, s->state);

    if (ret) {
        RETURN_INT(0);
    } else {
        BTMG_ERROR("write rsp failed for trans_id: %d", s->trans_id);
        RETURN_INT(-1);
    }
}

static int bt_gatts_send_rsp(void **call_args, void **cb_args)
{
    uint8_t opcode;
    struct gatt_db_attribute *attrib;

    BTMG_DEBUG("enter");
    gatts_send_read_rsp_t *s = (gatts_send_read_rsp_t *)call_args[0];

    BTMG_DEBUG("send response for handle: %u,response status: %u", s->attr_handle, s->status);

    opcode = gatt_server->curr_opcode + 1;
    attrib = gatt_db_get_attribute(gatt_server->dbs, s->attr_handle);

    if (opcode == BT_ATT_OP_READ_RSP) {
        BTMG_DEBUG("read response");
        gatt_db_attribute_read_result(attrib, s->trans_id, 0, (uint8_t *)s->value, s->value_len);
    } else if (opcode == BT_ATT_OP_READ_BLOB_RSP) {
        BTMG_DEBUG("read blob response");
        gatt_db_attribute_read_result(attrib, s->trans_id, 0, (uint8_t *)s->value, s->value_len);
    } else if (opcode == BT_ATT_OP_WRITE_RSP) {
        BTMG_DEBUG("write response");
        gatt_db_attribute_write_result(attrib, s->trans_id, 0);

    } else {
        BTMG_DEBUG("Unexpected opcode for response pdu %u", opcode);
    }

    RETURN_INT(0);
}

static void indication_confirm_cb(void *user_data)
{
    BTMG_DEBUG("Received indication confirmation");

    if (btmg_cb_p && btmg_cb_p->btmg_gatt_server_cb.gatts_send_indication_cb) {
        btmg_cb_p->btmg_gatt_server_cb.gatts_send_indication_cb(user_data);
    }
}

static int bt_gatts_send_notify(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");
    gatts_notify_data_t *s = (gatts_notify_data_t *)call_args[0];

    if (!bt_gatt_server_send_notification(gatt_server->gatts, s->attr_handle, (uint8_t *)s->value,
                                          s->value_len, false))
        BTMG_ERROR("Failed to send notification");

    RETURN_INT(0);
}

static int bt_gatts_send_ind(void **call_args, void **cb_args)
{
    BTMG_DEBUG("enter");
    gatts_indication_data_t *s = (gatts_indication_data_t *)call_args[0];

    if (!bt_gatt_server_send_indication(gatt_server->gatts, s->attr_handle, (uint8_t *)s->value,
                                        s->value_len, indication_confirm_cb, NULL, NULL))
        BTMG_ERROR("Failed to send indication");

    RETURN_INT(0);
}

static int bt_gatts_remove_service(void **call_args, void **cb_args)
{
    struct gatt_db_attribute *service;

    BTMG_DEBUG("enter");
    gatts_remove_svc_t *s = (gatts_remove_svc_t *)call_args[0];

    service = find_service_by_handle(s->svc_handle, gatt_server->dbs);
    if (!service) {
        BTMG_DEBUG("No service found by %d", s->svc_handle);
        RETURN_INT(-1);
    }

    gatt_db_remove_service(gatt_server->dbs, service);

    RETURN_INT(0);
}

static int disconnect_all_by_hci(int s, int dev_id, long arg)
{
    struct hci_conn_list_req *cl;
    struct hci_conn_info *ci;
    int id = arg;
    int i;

    if (id != -1 && dev_id != id)
        return 0;

    if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
        BTMG_ERROR("Can't allocate memory");
        return 0;
    }
    cl->dev_id = dev_id;
    cl->conn_num = 10;
    ci = cl->conn_info;

    if (ioctl(s, HCIGETCONNLIST, (void *)cl)) {
        free(cl);
        BTMG_ERROR("Can't get connection list");
        return -1;
    }

    for (i = 0; i < cl->conn_num; i++, ci++) {
        char addr[18];
        char *str;
        ba2str(&ci->bdaddr, addr);
        str = hci_lmtostr(ci->link_mode);
        BTMG_DEBUG("\t%s %x %s handle %d state %d lm %s", ci->out ? "<" : ">", ci->type, addr,
                   ci->handle, ci->state, str);
        if (ci->type == LE_LINK) {
            bt_le_disconnect(0, ci->handle, HCI_OE_USER_ENDED_CONNECTION);
        }
        bt_free(str);
    }

    free(cl);
    return 0;
}

static int bt_gatts_disconnect(void **call_args, void **cb_args)
{
    hci_for_each_dev(HCI_UP, disconnect_all_by_hci, 0);
    RETURN_INT(0);
}

act_func_t gatts_table[] = {
    [GATTS_ADD_SVC]        = { bt_gatts_add_service,    "gatts_add_svc" },
    [GATTS_ADD_CHAR]       = { bt_gatts_add_char,       "gatts_add_char" },
    [GATTS_ADD_DESC]       = { bt_gatts_add_desc,       "gatts_add_desc" },
    [GATTS_START_SVC]      = { bt_gatts_start_service,  "gatts_start_svc" },
    [GATTS_STOP_SVC]       = { bt_gatts_stop_service,   "gatts_stop_svc" },
    [GATTS_REMOVE_SVC]     = { bt_gatts_remove_service, "gatts_remove_svc" },
    [GATTS_GET_MTU]        = { bt_gatts_get_mtu,        "gatts_get_mtu" },
    [GATTS_SEND_WRITE_RSP] = { bt_gatts_send_write_rsp, "gatts_send_write_rsp" },
    [GATTS_SEND_RSP]       = { bt_gatts_send_rsp,       "gatts_send_rsp" },
    [GATTS_SEND_NOTIFY]    = { bt_gatts_send_notify,    "gatts_send_notify" },
    [GATTS_SEND_IND]       = { bt_gatts_send_ind,       "gatts_send_ind" },
    [GATTS_DISCONNECT]     = { bt_gatts_disconnect,     "gatts_disconnect" },
    [GATTS_INIT]           = { bt_gatts_init,           "gatts_init" },
    [GATTS_DEINIT]         = { bt_gatts_deinit,         "gatts_deinit" },
};
