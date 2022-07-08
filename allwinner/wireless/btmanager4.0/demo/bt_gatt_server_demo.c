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

#include "bt_dev_list.h"
#include "bt_log.h"
#include "bt_test.h"

#define TEST_SERVICE_UUID ((char *)"1112")
#define TEST_CHAR_UUID1 ((char *)"2223")
#define TEST_CHAR_UUID2 ((char *)"3334")
#define DEV_NAME_CHAR_UUID2 ((char *)"5555")
#define CCCD_UUID ((char *)"2902")

#define TEST_DESC_UUID ((char *)"00006666-0000-1000-8000-00805F9B34FB")

#define NORDIC_UART_SERVICE_UUID ((char *)"6e400001-b5a3-f393-e0a9-e50e24dcca9e")
#define NORDIC_UART_CHAR_RX_UUID ((char *)"6e400002-b5a3-f393-e0a9-e50e24dcca9e")
#define NORDIC_UART_CHAR_TX_UUID ((char *)"6e400003-b5a3-f393-e0a9-e50e24dcca9e")

static uint16_t service_handle;
static int is_gatt_testcase_server = 0;
char device_name[28] = "aw-ble-test-007";

static void bt_test_gatt_connection_cb(char *addr, gatts_connection_event_t event, int err)
{
    if (event == BT_GATT_CONNECTION) {
        printf("gatt server Connected: %s.\n", addr);
    } else if (event == BT_GATT_DISCONNECT) {
        printf("gatt server Disconnected: %s (reason UNKONW)", addr);
    } else {
        printf("gatt server event unkown.\n");
    }
}

static void bt_test_gatt_add_service_cb(gatts_add_svc_msg_t *msg)
{
    if (msg != NULL) {
        service_handle = msg->svc_handle;
        printf("add service handle: %d, handle max number: %d\n",
                                service_handle, msg->handle_num);
    }
}

static int char_handle = 0;
static int dev_name_char_handle = 0;
static int notify_handle = 0;
static int indicate_handle = 0;

static void bt_test_gatts_add_char_cb(gatts_add_char_msg_t *msg)
{
    if (msg != NULL) {
        printf("add char,uuid: %s,chr handle is %d\n", msg->uuid, msg->char_handle);
        if (strcmp(msg->uuid, DEV_NAME_CHAR_UUID2) == 0) {
            dev_name_char_handle = msg->char_handle;
        } else if (strcmp(msg->uuid, NORDIC_UART_CHAR_TX_UUID) == 0) {
            notify_handle = msg->char_handle;
        } else if (strcmp(msg->uuid, TEST_CHAR_UUID1) == 0) {
            indicate_handle = msg->char_handle;
        } else {
            char_handle = msg->char_handle;
        }
    }
}

static void bt_test_gatt_add_desc_cb(gatts_add_desc_msg_t *msg)
{
    if (msg != NULL) {
        printf("desc handle is %d\n", msg->desc_handle);
    }
}

static void bt_test_gatt_char_read_request_cb(gatts_char_read_req_t *chr_read)
{
    char value[1];
    static unsigned char count = 0;
    char dev_name[] = "aw_ble_test_1149";

    printf("trans_id:%d,attr_handle:%d,offset:%d\n", chr_read->trans_id, chr_read->attr_handle,
           chr_read->offset);

    if (chr_read) {
        if (chr_read->attr_handle == dev_name_char_handle) {
            gatts_send_read_rsp_t data;
            data.trans_id = chr_read->trans_id;
            data.attr_handle = chr_read->attr_handle;
            data.status = 0x0b;
            data.auth_req = 0x00;
            value[0] = count;
            data.value = dev_name;
            data.value_len = strlen(dev_name);
            bt_manager_gatt_server_send_read_response(&data);
            return;
        }
        gatts_send_read_rsp_t data;
        data.trans_id = chr_read->trans_id;
        data.attr_handle = chr_read->attr_handle;
        data.status = 0x0b;
        data.auth_req = 0x00;
        value[0] = count;
        data.value = value;
        data.value_len = 1;
        bt_manager_gatt_server_send_read_response(&data);
        count++;
    }
}

static void bt_test_gatts_send_indication_cb(gatts_send_indication_t *send_ind)
{
    printf("enter_func %d\n", __LINE__);
}

static void bt_test_gatt_char_write_request_cb(gatts_char_write_req_t *char_write)
{
    printf("enter_func %d\n", __LINE__);
    static unsigned char data_value[1] = { 0 };
    int ret = 0;

    if (char_write) {
        printf("attr_handle: %d,tran_id: %d\n", char_write->attr_handle, char_write->trans_id);
        printf("Value:");
        bt_manager_hex_dump((char *)" ", 20, (unsigned char *)char_write->value,
                            char_write->value_len);
    }

    /*
    gatts_notify_rsp_t data;
    data.attr_handle = char_write->attr_handle;
    data.value = data_value;
    data.value_len = 1;

    data_value[0] ++ ;
    printf("send data value:%d\n",data_value[0]);
    bt_manager_gatt_send_notification(&data);
*/

    printf("char_write->need_rsp: %d\n", char_write->need_rsp);
    if (char_write->need_rsp) {
        gatts_write_rsp_t data;
        data.trans_id = char_write->trans_id;
        data.attr_handle = char_write->attr_handle;
        data.state = BT_GATT_SUCCESS;
        ret = bt_manager_gatt_server_send_write_response(&data);
        if (ret != 0)
            printf("send write response failed!\n");
        else
            printf("send write response success!\n");
    }
}

static void bt_test_gatt_desc_read_requset_cb(gatts_desc_read_req_t *desc_read)
{
    printf("enter\n");
    char value[1];
    static unsigned char count = 0;
    printf("enter\n");

    printf("trans_id:%d,attr_handle:%d,offset:%d\n", desc_read->trans_id, desc_read->attr_handle,
           desc_read->offset);

    if (desc_read) {
        gatts_send_read_rsp_t data;
        data.trans_id = desc_read->trans_id;
        data.attr_handle = desc_read->attr_handle;
        data.status = 0x0b;
        data.auth_req = 0x00;
        value[0] = count;
        data.value = value;
        data.value_len = 1;
        bt_manager_gatt_server_send_read_response(&data);
        count++;
    }
}

static void bt_test_gatt_desc_write_request_cb(gatts_desc_write_req_t *desc_write)
{
    int ret = 0;

    printf("enter\n");
    printf("desc_write->need_rsp: %d\n", desc_write->need_rsp);
    printf("desc_write->attr_handle: %d\n", desc_write->attr_handle);

    if (desc_write->need_rsp) {
        gatts_write_rsp_t data;
        data.trans_id = desc_write->trans_id;
        data.attr_handle = desc_write->attr_handle;
        data.state = BT_GATT_SUCCESS;
        ret = bt_manager_gatt_server_send_write_response(&data);
        if (ret != 0)
            printf("send write response failed!\n");
        else
            printf("send write response success!\n");
    }
}

static void set_adv_param(void)
{
    btmg_le_advertising_parameters_t adv_params;

    adv_params.min_interval = 0x0020;
    adv_params.max_interval = 0x01E0; /*range from 0x0020 to 0x4000*/
    adv_params.own_addr_type = BTMG_LE_RANDOM_ADDRESS;
    adv_params.adv_type = BTMG_LE_ADV_IND; /*ADV_IND*/
    adv_params.chan_map =
            BTMG_LE_ADV_CHANNEL_ALL; /*0x07, *bit0:channel 37, bit1: channel 38, bit2: channel39*/
    adv_params.filter = BTMG_LE_PROCESS_ALL_REQ;
    bt_manager_le_set_adv_param(&adv_params);
}

static int le_set_adv_data(const char *ble_name, uint16_t uuid)
{
    int dd;
    uint8_t manuf_len;
    int index;
    char advdata[31] = { 0 };
    char uuid_buf[5] = { 0 };
    int advdata_len = 0;

    index = 0;

    advdata[index] = 0x02; /* flag len */
    advdata[index + 1] = 0x01; /* type for flag */
    advdata[index + 2] = 0x1A; //0x05

    index += advdata[index] + 1;

    advdata[index] = strlen(ble_name) + 1; /* name len */
    advdata[index + 1] = 0x09; /* type for local name */
    int name_len;
    name_len = strlen(ble_name);
    strcpy(&(advdata[index + 2]), ble_name);
    index += advdata[index] + 1;

    advdata[index] = 0x03; /* uuid len */
    advdata[index + 1] = 0x03; /* type for complete list of 16-bit uuid */
    advdata[index + 2] = (char)(uuid & 0xFF);
    advdata[index + 3] = (char)((uuid >> 8) & 0xFF);
    index += advdata[index] + 1;

    btmg_adv_data_t adv;
    adv.data_len = index;
    memcpy(adv.data, advdata, 31);

    return bt_manager_le_set_adv_data(&adv);
}

static int le_set_scan_rsp_data()
{
    int index;
    char rsp_data[31] = { 0 };
    index = 0;

    for (int i = 0; i < 31; i++) {
        rsp_data[i] = i;
    }
    rsp_data[index] = 30; /* flag len */
    rsp_data[index + 1] = 0xFF; /* type for flag */
    rsp_data[index + 2] = 0xFF; //0x05

    index += rsp_data[index] + 1;

    // rsp_data[index] = strlen(ble_name) + 1;            /* name len */
    // rsp_data[index + 1] = 0x09;        /* type for local name */
    // int i, name_len;
    // name_len = strlen(ble_name);
    // strcpy(&(rsp_data[index + i + 2]), ble_name);
    // index += rsp_data[index] + 1;

    // rsp_data[index] = 0x03;            /* uuid len */
    // rsp_data[index + 1] = 0x03;        /* type for complete list of 16-bit uuid */
    // rsp_data[index + 2] = (char)(uuid&0xFF);
    // rsp_data[index + 3] = (char)((uuid>>8)&0xFF);
    // index += rsp_data[index] + 1;

    btmg_scan_rsp_data_t scan_rsp_data;
    scan_rsp_data.data_len = index;
    memcpy(scan_rsp_data.data, rsp_data, 31);

    return bt_manager_le_set_scan_rsp_data(&scan_rsp_data);
}

void bt_gatt_server_register_callback(btmg_callback_t *cb)
{
    cb->btmg_gatt_server_cb.gatts_add_svc_cb = bt_test_gatt_add_service_cb;
    cb->btmg_gatt_server_cb.gatts_add_char_cb = bt_test_gatts_add_char_cb;
    cb->btmg_gatt_server_cb.gatts_add_desc_cb = bt_test_gatt_add_desc_cb;
    cb->btmg_gatt_server_cb.gatts_connection_event_cb = bt_test_gatt_connection_cb;
    cb->btmg_gatt_server_cb.gatts_char_read_req_cb = bt_test_gatt_char_read_request_cb;
    cb->btmg_gatt_server_cb.gatts_char_write_req_cb = bt_test_gatt_char_write_request_cb;
    cb->btmg_gatt_server_cb.gatts_desc_read_req_cb = bt_test_gatt_desc_read_requset_cb;
    cb->btmg_gatt_server_cb.gatts_desc_write_req_cb = bt_test_gatt_desc_write_request_cb;
    cb->btmg_gatt_server_cb.gatts_send_indication_cb = bt_test_gatts_send_indication_cb;
}

static int bt_gatt_server_register_bt_test_svc()
{
    gatts_add_svc_t svc;
    gatts_add_char_t chr1;
    gatts_add_char_t chr2;
    gatts_add_char_t chr3;
    gatts_add_desc_t desc1;
    gatts_add_desc_t desc2;

    printf("add service,uuid:%s\n", TEST_SERVICE_UUID);
    svc.number = 10;
    svc.uuid = TEST_SERVICE_UUID;
    svc.primary = true;
    bt_manager_gatt_server_create_service(&svc);

    chr1.permissions = BT_GATT_PERM_READ | BT_GATT_PERM_WRITE;
    chr1.properties = BT_GATT_CHAR_PROPERTY_READ | BT_GATT_CHAR_PROPERTY_WRITE |
                      BT_GATT_CHAR_PROPERTY_INDICATE;
    chr1.svc_handle = service_handle;
    chr1.uuid = TEST_CHAR_UUID1;
    bt_manager_gatt_server_add_characteristic(&chr1);

    desc1.permissions = BT_GATT_PERM_READ | BT_GATT_PERM_WRITE;
    desc1.uuid = CCCD_UUID;
    desc1.svc_handle = service_handle;
    bt_manager_gatt_server_add_descriptor(&desc1);

    chr2.permissions = BT_GATT_PERM_READ | BT_GATT_PERM_WRITE;
    chr2.properties =
            BT_GATT_CHAR_PROPERTY_READ | BT_GATT_CHAR_PROPERTY_WRITE | BT_GATT_CHAR_PROPERTY_NOTIFY;
    chr2.svc_handle = service_handle;
    chr2.uuid = TEST_CHAR_UUID2;
    bt_manager_gatt_server_add_characteristic(&chr2);

    desc2.permissions = BT_GATT_PERM_READ | BT_GATT_PERM_WRITE;
    desc2.uuid = CCCD_UUID;
    desc2.svc_handle = service_handle;
    bt_manager_gatt_server_add_descriptor(&desc2);

    chr3.permissions = BT_GATT_PERM_READ;
    chr3.properties = BT_GATT_CHAR_PROPERTY_READ;
    chr3.svc_handle = service_handle;
    chr3.uuid = DEV_NAME_CHAR_UUID2;
    bt_manager_gatt_server_add_characteristic(&chr3);

    return 0;
}

static int bt_gatt_server_register_ble_uart_svc()
{
    gatts_add_svc_t svc;
    gatts_add_char_t chr_rx;
    gatts_add_char_t chr_tx;
    gatts_add_char_t chr3;
    gatts_add_desc_t desc;
    gatts_star_svc_t start_svc;

    printf("add service,uuid:%s\n", NORDIC_UART_SERVICE_UUID);
    svc.number = 10;
    svc.uuid = NORDIC_UART_SERVICE_UUID;
    svc.primary = true;
    bt_manager_gatt_server_create_service(&svc);

    chr_rx.permissions = BT_GATT_PERM_WRITE;
    chr_rx.properties = BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE | BT_GATT_CHAR_PROPERTY_WRITE;
    chr_rx.svc_handle = service_handle;
    chr_rx.uuid = NORDIC_UART_CHAR_RX_UUID;
    bt_manager_gatt_server_add_characteristic(&chr_rx);

    chr_tx.permissions = BT_GATT_PERM_READ;
    chr_tx.properties = BT_GATT_CHAR_PROPERTY_NOTIFY;
    chr_tx.svc_handle = service_handle;
    chr_tx.uuid = NORDIC_UART_CHAR_TX_UUID;
    bt_manager_gatt_server_add_characteristic(&chr_tx);

    desc.permissions = BT_GATT_PERM_READ | BT_GATT_PERM_WRITE;
    desc.uuid = CCCD_UUID;
    desc.svc_handle = service_handle;
    bt_manager_gatt_server_add_descriptor(&desc);

    return 0;
}

int bt_gatt_server_init()
{
    bt_manager_gatt_server_init();

    bt_gatt_server_register_bt_test_svc();
    bt_gatt_server_register_ble_uart_svc();

    set_adv_param();
    bt_manager_le_set_random_address();
    le_set_adv_data(device_name, 0);
    bt_manager_le_enable_adv(true);
    return 0;
}

int bt_gatt_server_deinit()
{
    bt_manager_gatt_server_deinit();
    return 0;
}

static int bt_test_send_indication(int attr_handle, char *data, int len)
{
    char default_data[] = "hello";
    gatts_indication_data_t indication_data;
    indication_data.attr_handle = attr_handle;
    indication_data.value = data;
    indication_data.value_len = len;

    if (data == NULL || len == 0) {
        indication_data.value = default_data;
        indication_data.value_len = strlen(default_data);
    }
    bt_manager_gatt_server_send_indication(&indication_data);
    return 0;
}

static int bt_test_send_notify(int attr_handle, char *data, int len)
{
    char default_data[] = "hello";

    gatts_notify_data_t notify_data;
    notify_data.attr_handle = attr_handle;
    notify_data.value = data;
    notify_data.value_len = len;

    if (data == NULL || len == 0) {
        notify_data.value = default_data;
        notify_data.value_len = strlen(default_data);
    }
    bt_manager_gatt_server_send_notify(&notify_data);
    return 0;
}

static void cmd_gatts_init(int argc, char *args[])
{
    is_gatt_testcase_server = 0;
    bt_gatt_server_init();
    return;
}

static void cmd_gatts_deinit(int argc, char *args[])
{
    bt_gatt_server_deinit();
    return;
}

static void cmd_ble_name(int argc, char *args[])
{
    if (argc > 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    if (argc == 1) {
		if (strlen(args[0]) > 27) {
        	BTMG_INFO("name = [%s] exceeds limit", device_name);
			return;
		}
        strcpy(device_name, args[0]);
        BTMG_INFO("set name = %s", device_name);
        bt_manager_le_enable_adv(false);
        // le_set_scan_data();
        le_set_adv_data(device_name, 0);
        bt_manager_le_enable_adv(true);
    }
    if (argc == 0) {
        BTMG_INFO("get name = %s", device_name);
    }
    return;
}

static void cmd_ble_adv(int argc, char *args[])
{
    int value = 0;

    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    value = atoi(args[0]);
    BTMG_INFO("Set ble adv enable :%d", value);
    if (value || strcmp(args[0], "on") == 0) {
        bt_manager_le_enable_adv(true);
    } else {
        bt_manager_le_enable_adv(false);
    }
}

static void cmd_gatt_indicate(int argc, char *args[])
{
    static char data[512] = { 0 };
    int attr_handle = indicate_handle;
    int len = argc - 1;

    if (argc < 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    if (argc > 2) {
        attr_handle = atoi(args[0]);
    }
    if (attr_handle == 0) {
        attr_handle = indicate_handle;
    }

    for (int i = 0; i < len; i++) {
        data[i] = (char)strtoul(args[i + 1], NULL, 16);
    }
    if (len == 0) {
        len = 1;
    }
    bt_test_send_indication(attr_handle, data, len);
    data[0]++;
}

static void cmd_gatt_notify(int argc, char *args[])
{
    static char data[512] = { 0 };
    int attr_handle = notify_handle;
    int len = argc - 1;

    if (argc < 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    if (argc > 2) {
        attr_handle = atoi(args[0]);
    }
    if (attr_handle == 0) {
        attr_handle = notify_handle;
    }

    for (int i = 0; i < len; i++) {
        data[i] = (char)strtoul(args[i + 1], NULL, 16);
    }
    if (len == 0) {
        len = 1;
    }
    bt_test_send_notify(attr_handle, data, len);
    data[0]++;
}

static void cmd_ble_disconnect(int argc, char *args[])
{
    BTMG_DEBUG("disconnect all");
    bt_manager_le_disconnect();
}

static void cmd_gatt_testcase_server(int argc, char *args[])
{
    int id, ret;
    char adv_name[20];
    gatts_star_svc_t start_svc;
    BTMG_DEBUG("enter");
    if (argc <= 0) {
        BTMG_ERROR("error see help");
        return;
    }
    id = atoi(args[0]);
    snprintf(adv_name, sizeof(adv_name) - 1, "AW_BLE_4.0_%04d", id);
    bt_manager_le_enable_adv(false);
    bt_gatt_server_deinit();

    bt_manager_gatt_server_init();
    bt_gatt_server_register_bt_test_svc();
    bt_gatt_server_register_ble_uart_svc();
    bt_manager_gatt_server_start_service(&start_svc);

    set_adv_param();
    bt_manager_le_set_random_address();

    // set advertise and scan rsp
    le_set_adv_data(adv_name, (uint16_t)(1));
    le_set_scan_rsp_data();
    // le_set_scan_data();
    bt_manager_le_enable_adv(true);
    is_gatt_testcase_server = 1;
    return;
}

static void cmd_gatt_stress_test_server(int argc, char *args[])
{
    BTMG_DEBUG("enter");
    return;
}

cmd_tbl_t bt_gatts_cmd_table[] = {
    { "gatts_init", NULL, cmd_gatts_init, "gatts_init: gatt server init" },
    { "gatts_deinit", NULL, cmd_gatts_deinit, "gatts_deinit:gatt server deinit" },
    { "ble_name", NULL, cmd_ble_name, "ble name [name/none] to set name or get name" },
    { "ble_advertise", NULL, cmd_ble_adv,
      "ble_advertise [0/1]/[off/on]: Disable/Enable ble advertising" },
    { "ble_disconnect_all", NULL, cmd_ble_disconnect,
      "ble_disconnect: disconnect all le connections" },
    { "gatt_indicate", NULL, cmd_gatt_indicate, "gatt_indicate <char_handle=0> [value]" },
    { "gatt_notify", NULL, cmd_gatt_notify, "gatt_notify <char_handle=0> [value]" },
    { "gatt_testcase_server", NULL, cmd_gatt_testcase_server, "gatt_testcase_server [testid]" },
    // {"gatt_stress_test_server", NULL, cmd_gatt_stress_test_server,   "gatt_stress_test_server [testid] [test times]"},
    { NULL, NULL, NULL, NULL },
};
