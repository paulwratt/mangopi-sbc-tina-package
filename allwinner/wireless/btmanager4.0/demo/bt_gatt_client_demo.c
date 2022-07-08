#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <bt_manager.h>
#include "bt_log.h"
#include "bt_dev_list.h"
#include "bt_test.h"

int gattc_cn_handle = 0;
static char gattc_select_addr[18];

#define CMD_NOTIFY_INDICATE_ID_MAX 20
static int notify_indicate_id_save[CMD_NOTIFY_INDICATE_ID_MAX + 1];

static void bt_test_gattc_scan_report_cb(btmg_le_scan_report_t *data)
{
    int j = 0;
    int type;
    char name[31] = { 0 };
    char addrstr[64] = { 0 };

    for (;;) {
        type = data->report.data[j + 1];
        //complete local name.
        if (type == 0x09) {
            memcpy(name, &data->report.data[j + 2], data->report.data[j] - 1);
            name[data->report.data[j] - 1] = '\0';
        }
        j = j + data->report.data[j] + 1;
        if (j >= data->report.data_len)
            break;
    }

    snprintf(addrstr, 64, "%02X:%02X:%02X:%02X:%02X:%02X (%s)", data->peer_addr[5],
             data->peer_addr[4], data->peer_addr[3], data->peer_addr[2], data->peer_addr[1],
             data->peer_addr[0],
             (data->addr_type == BTMG_LE_PUBLIC_ADDRESS) ?
                     ("public") :
                     ((data->addr_type == BTMG_LE_RANDOM_ADDRESS) ? ("random") : ("error")));

    printf("[devices]: %s, adv type:%d, rssi:%d %s\n", addrstr, data->adv_type, data->rssi, name);
}

static void bt_test_gattc_notify_indicate_cb(gattc_notify_indicate_cb_para_t *data)
{
    int i;

    printf("\n\tHandle Value Notify/Indicate: 0x%04x - ", data->value_handle);

    if (data->length == 0) {
        printf("(0 bytes)\n");
        return;
    }

    printf("(%u bytes): ", data->length);

    for (i = 0; i < data->length; i++)
        printf("%02x ", data->value[i]);

    printf("\n");
}

static void bt_test_gattc_write_long_cb(gattc_write_long_cb_para_t *data)
{
    if (data->success) {
        printf("Write successful\n");
    } else if (data->reliable_error) {
        printf("Reliable write not verified\n");
    } else {
        printf("\nWrite failed: %s (0x%02x)\n",
               bt_manager_gatt_client_ecode_to_string(data->att_ecode), data->att_ecode);
    }
}

static void bt_test_gattc_write_cb(gattc_write_cb_para_t *data)
{
    if (data->success) {
        printf("\nWrite successful\n");
    } else {
        printf("\nWrite failed: %s (0x%02x)\n",
               bt_manager_gatt_client_ecode_to_string(data->att_ecode), data->att_ecode);
    }
}

static void bt_test_gattc_read_cb(gattc_read_cb_para_t *data)
{
    int i;

    if (!data->success) {
        printf("\nRead request failed: %s (0x%02x)\n",
               bt_manager_gatt_client_ecode_to_string(data->att_ecode), data->att_ecode);
        return;
    }

    printf("\nRead value");

    if (data->length == 0) {
        printf(": 0 bytes\n");
        return;
    }

    printf(" (%u bytes): ", data->length);

    for (i = 0; i < data->length; i++)
        printf("%02x ", data->value[i]);

    printf("\n");
}

static void bt_test_gattc_conn_cb(gattc_conn_cb_para_t *data)
{
    if (!data->success) {
        BTMG_INFO("gattc connect failed, error code: 0x%02x", data->att_ecode);
        return;
    }
    gattc_cn_handle = data->conn_id;
    BTMG_INFO("gattc connect completed, conn_id = 0x%x", gattc_cn_handle);;
}

static void bt_test_gattc_disconn_cb(gattc_disconn_cb_para_t *data)
{
    BTMG_INFO("Device disconnected");

    switch (data->reason) {
    case LOCAL_HOST_TERMINATED:
        BTMG_INFO("reason: LOCAL_HOST_TERMINATED");
        break;
    case CONNECTION_TIMEOUT:
        BTMG_INFO("reason: CONNECTION_TIMEOUT");
        break;
    case REMOTE_USER_TERMINATED:
        BTMG_INFO("reason: REMOTE_USER_TERMINATED");
        break;
    default:
        BTMG_INFO("reason: UNKNOWN_OTHER_ERROR");
    }
}

static void bt_test_gattc_service_changed_cb(gattc_service_changed_cb_para_t *data)
{
    BTMG_INFO("\nService Changed handled - start: 0x%04x end: 0x%04x\n", data->start_handle,
           data->end_handle);
}

static void print_uuid(const btmg_uuid_t *uuid)
{
    char uuid_str[37];
    btmg_uuid_t uuid128;

    bt_manager_uuid_to_uuid128(uuid, &uuid128);
    bt_manager_uuid_to_string(&uuid128, uuid_str, sizeof(uuid_str));

    printf("%s\n", uuid_str);
}

static void bt_test_gattc_dis_service_cb(gattc_dis_service_cb_para_t *data)
{
    char buf[37];
    btmg_uuid_t uuid128;

    bt_manager_uuid_to_uuid128(&data->uuid, &uuid128);
    bt_manager_uuid_to_string(&uuid128, buf, sizeof(buf));

    printf("-------|--------------------------------------------\n");
    printf("0x%04x | %s:\n", data->start_handle,
           !strcmp(buf, "00001800-0000-1000-8000-00805f9b34fb") ?
                   "Generic Access" :
                   (!strcmp(buf, "00001801-0000-1000-8000-00805f9b34fb") ? "Generic Attribute" :
                                                                           "Unknow Service"));
    printf("       | UUID: %s\n", buf);
    printf("       | %s:\n", data->primary ? "PRIMARY SERVICE" : "SECONDARY SERVICE");
    printf("       |\n");
    bt_manager_gatt_client_discover_service_all_char(data->conn_id, data->attr);
}

static void bt_test_gattc_dis_char_cb(gattc_dis_char_cb_para_t *data)
{
    char buf[128];
    btmg_uuid_t uuid128;

    snprintf(buf, sizeof(buf), "0x%04x |     Unknow Characteristic          :%s %s %s %s",
             data->start_handle, (data->properties & BT_GATT_CHAR_PROPERTY_READ ? "R" : ""),
             (((data->properties & BT_GATT_CHAR_PROPERTY_WRITE) ||
               (data->properties & BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE) ||
               (data->properties & BT_GATT_CHAR_PROPERTY_SIGNED_WRITE)) ?
                      "W" :
                      ""),
             (data->properties & BT_GATT_CHAR_PROPERTY_NOTIFY ? "N" : ""),
             (data->properties & BT_GATT_CHAR_PROPERTY_INDICATE ? "I" : ""));
    printf("%s\n", buf);
    bt_manager_uuid_to_uuid128(&data->uuid, &uuid128);
    bt_manager_uuid_to_string(&uuid128, buf, sizeof(buf));
    printf("       |     UUID:%s\n", buf);
    snprintf(buf, sizeof(buf), "       |     Properties:%s%s%s%s%s%s%s",
             (data->ext_prop ? "EXTENDED PROPS," : ""),
             (data->properties & BT_GATT_CHAR_PROPERTY_READ ? "READ," : ""),
             (data->properties & BT_GATT_CHAR_PROPERTY_WRITE ? "WRITE," : ""),
             (data->properties & BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE ? "WRITE NO RESPONSE," :
                                                                           ""),
             (data->properties & BT_GATT_CHAR_PROPERTY_NOTIFY ? "NOTIFY," : ""),
             (data->properties & BT_GATT_CHAR_PROPERTY_INDICATE ? "INDICATE," : ""),
             (data->properties & BT_GATT_CHAR_PROPERTY_SIGNED_WRITE ? "SIGNED WRITE," : ""));
    printf("%s\n", buf);
    printf("0x%04x |     Value:\n", data->value_handle);

    bt_manager_gatt_client_discover_char_all_descriptor(data->conn_id, data->attr);
}

static void bt_test_gattc_dis_desc_cb(gattc_dis_desc_cb_para_t *data)
{
    char buf[128];
    btmg_uuid_t uuid128;

    printf("       |     %s\n", (data->uuid.value.u16 == 0x2902) ?
                                        "Client Characteristic Configuration" :
                                        "Unknow Descripter");
    bt_manager_uuid_to_uuid128(&data->uuid, &uuid128);
    bt_manager_uuid_to_string(&uuid128, buf, sizeof(buf));
    printf("       |     UUID: %s\n", buf);
    printf("0x%04x |     Value:(0x)**-**\n", data->handle);
}

static void bt_test_gattc_get_conn_list_cb(gattc_connected_list_cb_para_t *data)
{
    if (strcmp((const char *)(data->addr), gattc_select_addr) == 0) {
        gattc_cn_handle = data->conn_id;
        printf("----------------[select]----------------\n");
    }
    printf("[device]: %d %s (%s)\n", data->conn_id, data->addr,
           data->addr_type == BTMG_LE_PUBLIC_ADDRESS ?
                   "public" :
                   (data->addr_type == BTMG_LE_RANDOM_ADDRESS ? "random" : "error"));

    return;
}

void bt_gatt_client_register_callback(btmg_callback_t *cb)
{
    cb->btmg_gatt_client_cb.gattc_conn_cb = bt_test_gattc_conn_cb;
    cb->btmg_gatt_client_cb.gattc_disconn_cb = bt_test_gattc_disconn_cb;
    cb->btmg_gatt_client_cb.gattc_read_cb = bt_test_gattc_read_cb;
    cb->btmg_gatt_client_cb.gattc_write_cb = bt_test_gattc_write_cb;
    cb->btmg_gatt_client_cb.gattc_write_long_cb = bt_test_gattc_write_long_cb;
    cb->btmg_gatt_client_cb.gattc_service_changed_cb = bt_test_gattc_service_changed_cb;
    cb->btmg_gatt_client_cb.gattc_notify_indicate_cb = bt_test_gattc_notify_indicate_cb;
    cb->btmg_gatt_client_cb.gattc_dis_service_cb = bt_test_gattc_dis_service_cb;
    cb->btmg_gatt_client_cb.gattc_dis_char_cb = bt_test_gattc_dis_char_cb;
    cb->btmg_gatt_client_cb.gattc_dis_desc_cb = bt_test_gattc_dis_desc_cb;
    cb->btmg_gatt_client_cb.gattc_connected_list_cb = bt_test_gattc_get_conn_list_cb;

    cb->btmg_gap_cb.gap_le_scan_report_cb = bt_test_gattc_scan_report_cb;
}

int bt_gatt_client_init()
{
    bt_manager_gatt_client_init();
    return 0;
}

int bt_gatt_client_deinit()
{
    bt_manager_gatt_client_deinit();
    return 0;
}

static void cmd_gatt_client_connect(int argc, char *args[])
{
    int i = argc;
    btmg_le_addr_type_t addr_type = BTMG_LE_RANDOM_ADDRESS;

    for (int i = 0; i < argc; i++) {
        BTMG_ERROR("%d: argv = %s", argc, args[i]);
    }
    if (argc < 1) {
        goto end;
    }

    if (strlen(args[0]) < 17) {
        goto end;
    }
    if (argc == 2) {
        if (strcmp(args[1], "public") == 0) {
            BTMG_INFO("addr_type:public");
            addr_type = BTMG_LE_PUBLIC_ADDRESS;
        } else if (strcmp(args[1], "random") == 0) {
            BTMG_INFO("addr_type:random");
            addr_type = BTMG_LE_RANDOM_ADDRESS;
        } else {
            goto end;
        }
    }

    bt_manager_gatt_client_connect((uint8_t *)args[0], addr_type, 517, BTMG_SECURITY_LOW);

    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
}

static void cmd_gatt_client_disconnect(int argc, char *args[])
{
    if (argc < 1) {
        goto end;
    }

    if (strlen(args[0]) < 17) {
        goto end;
    }

    bt_manager_gatt_client_disconnect((uint8_t *)args[0]);

    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
}

static void cmd_gatt_client_connections(int argc, char *args[])
{
    memset(gattc_select_addr, 0, 18);
    bt_manager_gatt_client_get_conn_list();
    return;
}

static void cmd_gatt_client_select(int argc, char *args[])
{
    if (argc < 1) {
        goto end;
    }
    if (strlen(args[0]) < 17) {
        goto end;
    }
    memcpy(gattc_select_addr, (uint8_t *)args[0], 18);
    bt_manager_gatt_client_get_conn_list();
    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
    return;
}

#define CHAR_SIZE_MAX 512
static void cmd_gatt_client_write_request(int argc, char *argv[])
{
    int len = 0;
    int err;
    uint16_t handle, offset;
    static uint8_t gatt_write_buf[CHAR_SIZE_MAX];

    if (argc < 3) {
        goto end;
    }

    handle = strtoul(argv[0], NULL, 16);
    offset = strtoul(argv[1], NULL, 16);

    if (argc == 4 && !strcmp(argv[3], "string")) {
        len = MIN(strlen(argv[2]), sizeof(gatt_write_buf));
        strncpy((char *)gatt_write_buf, argv[2], len);
    } else {
        len = hex2bin(argv[2], strlen(argv[2]), gatt_write_buf, sizeof(gatt_write_buf));
        if (len == 0) {
            BTMG_ERROR("No data set");
            return;
        }
    }

    bt_manager_gatt_client_write_request(gattc_cn_handle, handle, gatt_write_buf, len);

    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
}

static void cmd_gatt_client_write_command(int argc, char *argv[])
{
    uint16_t handle;
    int repeat = 0;
    int len = 0, sign = false;
    static uint8_t gatt_write_buf[CHAR_SIZE_MAX];

    if (argc < 3) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    handle = strtoul(argv[0], NULL, 16);
    sign = (bool)strtoul(argv[1], NULL, 16);

    if (argc >= 4 && !strcmp(argv[3], "string")) {
        len = MIN(strlen(argv[2]), sizeof(gatt_write_buf));
        strncpy((char *)gatt_write_buf, argv[2], len);
    } else {
        len = hex2bin(argv[2], strlen(argv[2]), gatt_write_buf, sizeof(gatt_write_buf));
        if (len == 0) {
            BTMG_ERROR("No data set");
            return;
        }
    }
    if (argc > 4) {
        repeat = strtoul(argv[4], NULL, 16);
    }
    if (!repeat) {
        repeat = 1;
    }

    while (repeat--) {
        bt_manager_gatt_client_write_command(gattc_cn_handle, handle, sign, gatt_write_buf, len);
    }

    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
}

static void cmd_gatt_client_read_request(int argc, char *args[])
{
    uint16_t handle;
    char *endptr = NULL;

    if (argc != 1) {
        goto end;
    }

    handle = strtol(args[0], &endptr, 0);

    bt_manager_gatt_client_read_request(gattc_cn_handle, handle);
    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
}

static void cmd_gatt_client_write_long_request(int argc, char *args[])
{
    uint16_t handle;
    int reliable_writes = 0;
    uint8_t long_data[2048];
    int mtu = 0;

    if (argc < 1) {
        goto end;
    }
    for (int i = 0; i < 2048; i++) {
        long_data[i] = i;
    }
    handle = strtol(args[0], NULL, 0);
    if (argc == 2) {
        reliable_writes = 1;
    }
    mtu = bt_manager_gatt_client_get_mtu(gattc_cn_handle);
    if (mtu >= 23 && mtu <= 517) {
        for (int i = 0; i < 2048 / mtu; i++) {
            BTMG_ERROR("write long request: i= %d, mtu = %d, reliable=%s", i, mtu,reliable_writes?"Y":"N");
            bt_manager_gatt_client_write_long_request(gattc_cn_handle,
            (bool)reliable_writes, handle, i * mtu, long_data, mtu);
        }
    }
    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
}

static void cmd_gatt_client_dis_all_svcs(int argc, char *args[])
{
    btmg_log_level_t debug_level = bt_manager_get_loglevel();
    bt_manager_set_loglevel(BTMG_LOG_LEVEL_INFO);
    printf("----------------------------------------------------\nhandle | \n");
    bt_manager_gatt_client_discover_all_services(gattc_cn_handle, 0x0001, 0xffff);
    printf("---------------------------------------------------\n");
    bt_manager_set_loglevel(debug_level);

    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
}

static void cmd_gatt_client_dis_svcs_by_uuid(int argc, char *args[])
{
    if (argc != 1) {
        goto end;
    }

    if (strlen(args[0]) < 6) {
        goto end;
    }
    bt_manager_gatt_client_discover_services_by_uuid(gattc_cn_handle, args[0], 0x0001,
                                                             0xffff);
    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
}

static void cmd_ble_scan(int argc, char *argv[])
{
    int i = 0;
    int param_change = 0;
    int scan_on = 0;
    static btmg_le_scan_param_t scan_param = {
        .scan_type = LE_SCAN_TYPE_ACTIVE,
        .scan_interval = 0x0640,
        .scan_window = 0x0060,
        .filter_duplicate = LE_SCAN_DUPLICATE_DISABLE,
        .own_addr_type = BTMG_LE_PUBLIC_ADDRESS,
        .filter_policy = LE_SCAN_FILTER_POLICY_ALLOW_ALL,
    };
    printf("argc =  %d argv[0]%s", argc, argv[0]);
    while (i < argc) {
        if (!strcmp(argv[i], "on") || !strcmp(argv[i], "1")) {
            scan_on = 1;
        } else if (!strcmp(argv[i], "off") || !strcmp(argv[i], "0")) {
            bt_manager_le_scan_stop();
            printf("stop scan\n");
            scan_on = 0;
            return;
        } else if (!strcmp(argv[i], "passive")) {
            scan_param.scan_type = LE_SCAN_TYPE_PASSIVE;
            param_change++;
            scan_on = 1;
        } else if (!strcmp(argv[i], "dups")) {
            param_change++;
            scan_param.filter_duplicate = LE_SCAN_DUPLICATE_DISABLE;
        } else if (!strcmp(argv[i], "nodups")) {
            param_change++;
            scan_param.filter_duplicate = LE_SCAN_DUPLICATE_ENABLE;
        } else if (!strncmp(argv[i], "int=0x", 4)) {
            uint32_t num;
            int cnt = sscanf(argv[i] + 4, "%x", &num);
            if (cnt != 1) {
                printf("invalid param %s", argv[i] + 4);
                return;
            }
            scan_param.scan_interval = num;
            param_change++;
            printf("scan interval 0x%x\n", scan_param.scan_interval);
        } else if (!strncmp(argv[i], "win=0x", 4)) {
            uint32_t num;
            int cnt = sscanf(argv[i] + 4, "%x", &num);
            if (cnt != 1) {
                printf("invalid param %s\n", argv[i] + 4);
                return;
            }
            scan_param.scan_window = num;
            param_change++;
            printf("scan window 0x%x\n", scan_param.scan_window);
        } else if (!strcmp(argv[i], "wl")) {
            scan_param.filter_policy = LE_SCAN_FILTER_POLICY_ONLY_WLIST;
            printf("filter whitelist %s\n", argv[i]);
            return;
        } else {
            printf("invalid param %s\n", argv[i]);
            return;
        }
        i++;
    }
    if (param_change) {
        bt_manager_le_set_scan_parameters(&scan_param);
    }

    if (argc == 0) {
        bt_manager_le_scan_stop();
        printf("invalid argc %d\n", argc);
        return;
    }
    if (scan_on) {
        bt_manager_le_scan_start(&scan_param);
        printf("scan start\n");
    }
    return;
}

static void cmd_gatt_client_get_mtu(int argc, char *args[])
{
    int mtu;
    mtu = bt_manager_gatt_client_get_mtu(gattc_cn_handle);

    if (mtu > 0) {
        printf("[MTU]:%d\n", mtu);
    } else {
        printf("get mtu failed.\n");
    }
    return;
}

static void cmd_gatt_client_register_notify_indicate(int argc, char *args[])
{
    uint16_t handle;
    int notify_indicate_id;
    int i;
    char *endptr = NULL;
    if (argc != 1) {
        goto end;
    }
    handle = strtol(args[0], &endptr, 0);
    notify_indicate_id = bt_manager_gatt_client_register_notify_indicate(gattc_cn_handle, handle);
    if (notify_indicate_id == 0) {
        printf("register err\n");
    } else {
        for (i = 0; i < CMD_NOTIFY_INDICATE_ID_MAX && notify_indicate_id_save[i]; i++) {
            // do nothing
        }
        notify_indicate_id_save[i] = notify_indicate_id;
        printf("register with id = %d\n", notify_indicate_id);
    }
    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
    return;
}

static void cmd_gatt_client_unregister_notify_indicate(int argc, char *args[])
{
    int i;
    for (i = 0; i < CMD_NOTIFY_INDICATE_ID_MAX && notify_indicate_id_save[i]; i++) {
        bt_manager_gatt_client_unregister_notify_indicate(gattc_cn_handle, notify_indicate_id_save[i]);
        notify_indicate_id_save[i] = 0;
    }
    return;
}

static void cmd_ble_set_scan_parameters(int argc, char *args[])
{
    static btmg_le_scan_param_t scan_param = {
        .scan_type = LE_SCAN_TYPE_PASSIVE,
        .scan_interval = 0x0010,
        .scan_window = 0x0005,
        .filter_duplicate = LE_SCAN_DUPLICATE_DISABLE,
        .own_addr_type = BTMG_LE_PUBLIC_ADDRESS,
        .filter_policy = LE_SCAN_FILTER_POLICY_ALLOW_ALL,
    };

    bt_manager_le_set_scan_parameters(&scan_param);

    return;
}

static void cmd_ble_update_conn_params(int argc, char *args[])
{
    btmg_le_conn_param_t conn_params;
    if (argc < 4) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    conn_params.min_conn_interval = strtoul(args[0], NULL, 16);
    conn_params.max_conn_interval = strtoul(args[1], NULL, 16);
    conn_params.slave_latency = strtoul(args[2], NULL, 16);
    conn_params.conn_sup_timeout = strtoul(args[3], NULL, 16);
    bt_manager_le_update_conn_params(&conn_params);
    printf("conn update update.");
    return;
}

static void cmd_gattc_init(int argc, char *args[])
{
    bt_manager_gatt_client_init();
    return;
}

static void cmd_gattc_deint(int argc, char *args[])
{
    bt_manager_gatt_client_deinit();
    return;
}

void cmd_gatt_testcase_client(int argc, char *args[]);

cmd_tbl_t bt_gattc_cmd_table[] = {
    { "gattc_init", NULL, cmd_gattc_init, "gattc_init [none]: gatt client init" },
    { "gattc_deinit", NULL, cmd_gattc_deint, "gattc_deinit [none]: gatt client deinit" },
    { "ble_scan", NULL, cmd_ble_scan,
      "ble_scan [on, passive, off] [int=0x<hex>] [win=0x<hex>] [wl] [dups/nodups]" },
    { "ble_connect", NULL, cmd_gatt_client_connect,
      "ble_connect [mac] [public random]: gatt client method to connect" },
    { "ble_disconnect", NULL, cmd_gatt_client_disconnect,
      "ble_disconnect [mac]: gatt client method to disconnect" },
    { "ble_conn_update", NULL, cmd_ble_update_conn_params,
      "ble_conn_update [min][max][latency][timeout]" },
    { "ble_connections", NULL, cmd_gatt_client_connections, "ble_connections" },
    { "ble_select", NULL, cmd_gatt_client_select, "ble_select [mac]" },
    { "gatt_write", NULL, cmd_gatt_client_write_request,
      "gatt_write <handle> <offset=0> <data> [string]: gatt write request" },
    { "gatt_write_cmd", NULL, cmd_gatt_client_write_command,
      "gatt_write_cmd <handle> <sign=0> <data> [string] [repeat]: gatt write command without response" },
    { "gatt_read", NULL, cmd_gatt_client_read_request, "gatt_read <handle>:gatt read request" },
    { "gatt_write_long", NULL, cmd_gatt_client_write_long_request, "gatt_read <handle> [reliable]:gatt write long request" },
    { "gatt_discover", NULL, cmd_gatt_client_dis_all_svcs,
      "gatt_discover: discovery all services" },
    { "gatt_discover_uuid", NULL, cmd_gatt_client_dis_svcs_by_uuid,
      "gatt_discover_uuid [uuid]: discovery services by uuid" },
    { "get_mtu", NULL, cmd_gatt_client_get_mtu, "get_mtu: get MTU" },
    { "register_notify_indicate", NULL, cmd_gatt_client_register_notify_indicate,
      "register_notify [value handle]:gatt register notify callback" },
    { "unregister_notify_indicate", NULL, cmd_gatt_client_unregister_notify_indicate,
      "unregister_notify_indicate: gatt unregister all notify callback" },

    { "gatt_testcase_client", NULL, cmd_gatt_testcase_client, "gatt_testcase_client [testid]" },
    // {"gatt_stress_test_client", NULL, cmd_gatt_stress_test_client,    "gatt_stress_test_client [testid] [test times]"},
    { NULL, NULL, NULL, NULL },
};
