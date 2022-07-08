#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <gio/gio.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include "bt_log.h"
#include "bt_dev_list.h"
#include "bt_semaphore.h"
#include "bt_adapter.h"
#include "common.h"
#include "bt_le_hci.h"
#include "bt_bluez_signals.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <src/shared/bt.h>
#include <src/shared/hci.h>

#include "bt_gatt_inner.h"

#define LE_LINK 0x80
#define BLE_MAX_ADV_SIZE 31
static struct bt_hci *le_scan_device = NULL;

#ifndef HCIDEVUP
#define HCIDEVUP          _IOW('H', 201, int)
#endif // HCIDEVUP
#ifndef HCIDEVDOWN
#define HCIDEVDOWN        _IOW('H', 202, int)
#endif // HCIDEVDOWN

static int tiny_hciconfig(int hdev, char *opt)
{
    int up_or_down = 0;
    int ctl = 0;
    hdev = 0;
    if (!strcmp(opt, "up")) {
        up_or_down = 1;
    } else if (!strcmp(opt, "down")) {
        up_or_down = 0;
    } else {
        return -1;
    }
    if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0) {
        perror("Can't open HCI socket.");
        return -2;
    }
    /* Start HCI device */
    if (ioctl(ctl, (up_or_down ? (HCIDEVUP) : (HCIDEVDOWN)), hdev) < 0) {
        if (errno == EALREADY) {
            BTMG_INFO("Already %s", up_or_down ? "up" : "down");
            return 0;
        }
        fprintf(stderr, "Can't init device hci%d: %s (%d)\n",
                        hdev, strerror(errno), errno);
        return -3;
    }
    close(ctl);
    return 0;
}

int bt_le_set_advertising_params(btmg_le_advertising_parameters_t *adv_param)
{
    int dd, ret, status;
    int hdev = 0;
    char *opt = NULL;
    struct hci_request rq;
    le_set_advertising_parameters_cp adv_params_cp;
    le_set_advertise_enable_cp advertise_cp;

    if (adv_param == NULL) {
        BTMG_ERROR("Invalid advertising parameters!");
        return -1;
    }
    memset(&adv_params_cp, 0, sizeof(adv_params_cp));

    if (adv_param->adv_type >= BTMG_LE_ADV_TYPE_MAX) {
        BTMG_ERROR("Invalid advertising type!");
        return -1;
    } else {
        adv_params_cp.advtype = adv_param->adv_type;
    }

    if (adv_param->adv_type != BTMG_LE_ADV_DIRECT_HIGH_IND &&
        (adv_param->min_interval < 0x0020 || adv_param->min_interval > 0x4000 ||
         adv_param->max_interval < 0x0020 || adv_param->max_interval > 0x4000 ||
         adv_param->max_interval < adv_param->min_interval)) {
        BTMG_ERROR("Invalid advertising interval range");
        return -1;
    } else if (adv_param->adv_type != BTMG_LE_ADV_DIRECT_HIGH_IND) {
        adv_params_cp.min_interval = htobs(adv_param->min_interval);
        adv_params_cp.max_interval = htobs(adv_param->max_interval); /*range from 0x0020 to 0x4000*/
    }

    if (adv_param->own_addr_type >= 0x04) {
        BTMG_ERROR("Invalid own addr type!");
        return -1;
    } else {
        switch (adv_param->own_addr_type) {
        case BTMG_LE_PUBLIC_ADDRESS:
            adv_params_cp.own_bdaddr_type = 0;
            break;
        case BTMG_LE_RANDOM_ADDRESS:
            adv_params_cp.own_bdaddr_type = 1;
            break;
        default:
            break;
        }
    }

    if (adv_param->adv_type == BTMG_LE_ADV_DIRECT_HIGH_IND ||
        adv_param->adv_type == BTMG_LE_ADV_DIRECT_LOW_IND) {
        if (adv_param->peer_addr_type >= 0x02) {
            BTMG_ERROR("Invalid peer addr type!");
            return -1;
        }
        str2ba(adv_param->peer_addr, &adv_params_cp.direct_bdaddr);
        adv_params_cp.direct_bdaddr_type = adv_param->peer_addr_type;
    } else {
        /* advertising filter policy shall be ignored for directed advertising*/
        if (adv_param->filter >= BTMG_LE_FILTER_POLICY_MAX) {
            BTMG_ERROR("Invalid advertising filter policy!");
            return -1;
        } else {
            adv_params_cp.filter = adv_param->filter;
        }
    }

    if (adv_param->chan_map > BTMG_LE_ADV_CHANNEL_ALL ||
        adv_param->chan_map == BTMG_LE_ADV_CHANNEL_NONE) {
        BTMG_ERROR("Invalid advertising channel map:%d!", adv_param->chan_map);
        return -1;
    } else {
        BTMG_INFO("advertising channel map: 0x%x", adv_param->chan_map);
        adv_params_cp.chan_map =
                adv_param->chan_map; /*bit0:channel 37, bit1: channel 38, bit2: channel39*/
    }

    dd = hci_open_dev(hdev);
    if (dd < 0) {
        BTMG_ERROR("Could not open device");
        return -1;
    }
    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_LE_CTL;
    rq.ocf = OCF_LE_SET_ADVERTISING_PARAMETERS;
    rq.cparam = &adv_params_cp;
    rq.clen = LE_SET_ADVERTISING_PARAMETERS_CP_SIZE;
    rq.rparam = &status;
    rq.rlen = 1;

    ret = hci_send_req(dd, &rq, 1000);

    if (ret < 0) {
        BTMG_ERROR("cannot send advertising param, ret: %d, status: %d", ret, status);
        goto done;
    } else if (status == 0x11) {
        BTMG_ERROR("unsupported advertising interval range by the controller!");
        goto done;
    }

    hci_close_dev(dd);
    return 0;

done:
    hci_close_dev(dd);
    return -1;
}

int bt_le_set_advertising_data(btmg_adv_data_t *adv_data)
{
    int dd;
    uint8_t manuf_len;
    uint16_t ogf, ocf;
    int index;
    le_set_advertising_data_cp advdata;

    dd = hci_open_dev(0);
    if (dd < 0) {
        BTMG_DEBUG("Could not open device");
        return -1;
    }
    memset(&advdata, 0, sizeof(le_set_advertising_data_cp));
    advdata.length = adv_data->data_len;
    memcpy(advdata.data, adv_data->data, adv_data->data_len);

    ogf = OGF_LE_CTL;
    ocf = OCF_LE_SET_ADVERTISING_DATA;
    if (hci_send_cmd(dd, ogf, ocf, 32, (void *)&advdata) < 0) {
        BTMG_INFO("Send failed");
        return -2;
    }

    hci_close_dev(dd);
    return 0;
}

int bt_le_set_scan_rsp_data(btmg_scan_rsp_data_t *rsp_data)
{
    int dd;
    uint8_t manuf_len;
    uint16_t ogf, ocf;
    int index;
    char rspdata[32] = { 0 };

    dd = hci_open_dev(0);
    if (dd < 0) {
        BTMG_DEBUG("Could not open device");
        return -1;
    }

    rspdata[0] = rsp_data->data_len;
    memcpy(&rspdata[1], rsp_data->data, rsp_data->data_len);

    ogf = OGF_LE_CTL;
    ocf = OCF_LE_SET_SCAN_RESPONSE_DATA;
    if (hci_send_cmd(dd, ogf, ocf, 32, rspdata) < 0) {
        BTMG_INFO("Send failed");
        return -2;
    }

    hci_close_dev(dd);
    return 0;
}

int bt_le_set_random_address(void)
{
    ssize_t len;
    int urandom_fd;
    int ret = -1;

    struct hci_request rq;
    le_set_random_address_cp cp;
    uint8_t status;
    int dd, err;

    char addr[6];

    urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd < 0) {
        fprintf(stderr, "Failed to open /dev/urandom device\n");
        return ret;
    }

    len = read(urandom_fd, addr, sizeof(addr));
    if (len < 0 || len != sizeof(addr)) {
        fprintf(stderr, "Failed to read random data\n");
        goto done;
    }

    /* Clear top most significant bits */
    addr[5] &= 0x3f;

    dd = hci_open_dev(0);
    if (dd < 0) {
        fprintf(stderr, "Could not open device: %s(%d)\n", strerror(dd), dd);
        goto done;
    }

    memset(&cp, 0, sizeof(cp));

    memcpy(cp.bdaddr.b, addr, 6);

    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_LE_CTL;
    rq.ocf = OCF_LE_SET_RANDOM_ADDRESS;
    rq.cparam = &cp;
    rq.clen = LE_SET_RANDOM_ADDRESS_CP_SIZE;
    rq.rparam = &status;
    rq.rlen = 1;

    ret = hci_send_req(dd, &rq, 1000);
    if (status || ret < 0) {
        fprintf(stderr,
                "Can't set random address for hci0: "
                "%s (%d)\n",
                strerror(ret), ret);
    }

    hci_close_dev(dd);
    BTMG_INFO("*************************************************");
    BTMG_INFO("[RandomAddress %02X:%02X:%02X:%02X:%02X:%02X ]",addr[5],addr[4],addr[3],addr[2],addr[1],addr[0]);
    BTMG_INFO("*************************************************");
done:
    close(urandom_fd);
    return ret;
}

int bt_le_advertising_enable(bool enable)
{
    struct hci_request rq;
    le_set_advertising_parameters_cp adv_params_cp;
    le_set_advertise_enable_cp advertise_cp;
    uint8_t status;
    int dd, ret;
    int hdev = 0;
    char *opt = NULL;

    BTMG_DEBUG("ADV:%s\n", enable ? "ENABLE" : "DISABLE");

    dd = hci_open_dev(hdev);

    if (dd < 0) {
        BTMG_ERROR("Could not open device");
        return -1;
    }

    memset(&advertise_cp, 0, sizeof(advertise_cp));
    advertise_cp.enable = (uint8_t)enable;

    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_LE_CTL;
    rq.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
    rq.cparam = &advertise_cp;
    rq.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
    rq.rparam = &status;
    rq.rlen = 1;

    ret = hci_send_req(dd, &rq, 1000);
    if (ret < 0) {
        BTMG_ERROR("cannot send enable advertising");
        goto done;
    } else {
        if (status == 0)
            BTMG_ERROR("set adv enable:%d", enable);
        else if (status == 12)
            BTMG_DEBUG("%s\n", enable ? "likely already advertising..." :
                                        "likely already disable advertise...");
        else
            BTMG_ERROR("UnExpected status");
    }

    hci_close_dev(dd);
    return 0;

done:
    hci_close_dev(dd);
    return -1;
}

int find_ledev_conn_handle(int dev_id, uint8_t *remote_addr)
{
    struct hci_conn_list_req *cl;
    struct hci_conn_info *ci;
    int dd, i;

    dd = hci_open_dev(dev_id);
    if (dd < 0) {
        BTMG_ERROR("Could not open device");
        return -1;
    }

    if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
        BTMG_ERROR("Can't allocate memory");
        hci_close_dev(dd);
        return -1;
    }

    cl->dev_id = dev_id;
    cl->conn_num = 10;
    ci = cl->conn_info;

    if (ioctl(dd, HCIGETCONNLIST, (void *)cl)) {
        BTMG_ERROR("Can't get connection list");
        free(cl);
        hci_close_dev(dd);
        return -1;
    }

    for (i = 0; i < cl->conn_num; i++, ci++) {
        char addr[18];
        char *str;
        ba2str(&ci->bdaddr, addr);
        str = hci_lmtostr(ci->link_mode);
        BTMG_DEBUG("\t%s %x %s handle %d state %d lm %s\n", ci->out ? "<" : ">", ci->type, addr,
                   ci->handle, ci->state, str);
        if (ci->type == LE_LINK) {
            if (!strcmp(addr, remote_addr)) {
                bt_free(str);
                hci_close_dev(dd);
                BTMG_DEBUG("find ledev:%s handle is:%d", remote_addr, ci->handle);
                return ci->handle;
            }
        }
        bt_free(str);
    }

    free(cl);
    hci_close_dev(dd);

    return -1;
}

static le_scan_status_t le_scan_status = {
    .state = false,
};

le_scan_status_t *bt_le_get_scan_status(void)
{
    return &le_scan_status;
}

int bt_le_scan_start(void **call_args, void **cb_args)
{
    bt_le_scan_start_args_t *start_args = (bt_le_scan_start_args_t *)call_args[0];
    uint16_t interval = htobs(start_args->scan_param->scan_interval);
    uint16_t window = htobs(start_args->scan_param->scan_window);
    uint8_t filter_policy = start_args->scan_param->filter_policy;
    uint8_t filter_dup = start_args->scan_param->filter_duplicate;
    uint8_t scan_type = start_args->scan_param->scan_type;
    uint8_t own_type = start_args->scan_param->own_addr_type;
    int dd;

    dd = hci_open_dev(0);
    if (dd < 0) {
        BTMG_ERROR("Could not open device");
        return -1;
    }

    if (hci_le_set_scan_enable(dd, 0x00, filter_dup, 10000) != 0) {
        BTMG_ERROR("Enable scan failed");
        hci_close_dev(dd);
        return -1;
    }

    if (hci_le_set_scan_parameters(dd, scan_type, interval, window, own_type,
                                   filter_policy, 10000) != 0) {
        BTMG_ERROR("Set scan parameters failed");
        hci_close_dev(dd);
        return -1;
    }

    le_scan_status.state = true;
    memcpy(&le_scan_status.scan_param, start_args->scan_param, sizeof(btmg_le_scan_param_t));

    if (hci_le_set_scan_enable(dd, 0x01, filter_dup, 10000) != 0) {
        BTMG_ERROR("Enable scan failed");
        hci_close_dev(dd);
        return -1;
    }
    hci_close_dev(dd);

    return 0;
}

int bt_le_scan_stop(void **call_args, void **cb_args)
{
    int dd;

    dd = hci_open_dev(0);
    if (dd < 0) {
        BTMG_ERROR("Device open failed");
        return -1;
    }

    if (hci_le_set_scan_enable(dd, 0x00, 0x00, 10000) < 0) {
        BTMG_ERROR("scan disable failed");
        hci_close_dev(dd);
        return -1;
    }

    hci_close_dev(dd);

    return 0;
}

int bt_le_set_scan_parameters(void **call_args, void **cb_args)
{
    bt_le_set_scan_para_args_t *args = (bt_le_set_scan_para_args_t *)call_args[0];
    int dd = -1;
    uint16_t interval = htobs(args->scan_param->scan_interval);
    uint16_t window = htobs(args->scan_param->scan_window);
    uint8_t filter_policy = args->scan_param->filter_policy;
    uint8_t scan_type = args->scan_param->scan_type;
    btmg_le_addr_type_t own_type = args->scan_param->own_addr_type;

    dd = hci_open_dev(0);
    if (dd < 0) {
        BTMG_ERROR("Could not open device");
        return -1;
    }
    if (hci_le_set_scan_parameters(dd, scan_type, interval, window, own_type,
                                   filter_policy, 5000) < 0) {
        BTMG_ERROR("Set scan parameters failed");
        hci_close_dev(dd);
        return -1;
    }
    hci_close_dev(dd);

    return 0;
}

int bt_le_update_conn_params(void **call_args, void **cb_args)
{
    bt_le_update_cn_para_args_t *args = (bt_le_update_cn_para_args_t *)call_args[0];
    int dd = -1, i;
    uint16_t handle, conn_handle, min_interval, max_interval, latency, supervision_timeout;
    struct hci_conn_list_req *cl;
    struct hci_conn_info *ci;

    dd = hci_open_dev(0);
    if (dd < 0) {
        BTMG_ERROR("Could not open device");
        return -1;
    }

    if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
        BTMG_ERROR("Can't allocate memory for cl");
    }

    cl->dev_id = 0;
    cl->conn_num = 10;
    ci = cl->conn_info;

    if (ioctl(dd, HCIGETCONNLIST, (void *)cl)) {
        BTMG_ERROR("Can't get connection list");
        free(cl);
        hci_close_dev(dd);
        return -1;
    }

    for (i = 0; i < cl->conn_num; i++, ci++) {
        char addr[18];
        char *str;
        ba2str(&ci->bdaddr, addr);
        str = hci_lmtostr(ci->link_mode);
        BTMG_INFO("\t%s %x %s handle %d state %d lm %s\n", ci->out ? "<" : ">", ci->type, addr,
                  ci->handle, ci->state, str);
        if (ci->type == LE_LINK) {
            conn_handle = ci->handle;
            bt_free(str);
            break;
        }
        bt_free(str);
    }

    handle = htobs(conn_handle);
    min_interval = htobs(args->conn_params->min_conn_interval);
    max_interval = htobs(args->conn_params->max_conn_interval);
    latency = htobs(args->conn_params->slave_latency);
    supervision_timeout = htobs(args->conn_params->conn_sup_timeout);

    if (hci_le_conn_update(dd, handle, min_interval, max_interval, latency, supervision_timeout,
                           5000) < 0) {
        BTMG_ERROR("Could not change connection params");
        hci_close_dev(dd);
        free(cl);
        return -1;
    }
    free(cl);
    hci_close_dev(dd);

    return 0;
}

static void le_scan_report(const uint8_t *data, uint8_t size, void *user_data)
{
    const struct bt_hci_evt_le_adv_report *evt = (void *)data;
    btmg_le_scan_report_t adv_report;
    int8_t *rssi;
    uint8_t evt_len = 0, num_reports = 0, i, j;

    num_reports = evt->num_reports;

    for (i = 0; i < num_reports; i++) {
        rssi = (int8_t *)(evt->data + evt->data_len);

        memcpy(adv_report.peer_addr, evt->addr, 6);
        adv_report.addr_type =
                (evt->addr_type == 0x00) ? BTMG_LE_PUBLIC_ADDRESS : BTMG_LE_RANDOM_ADDRESS;
        if (evt->event_type == 0x04)
            adv_report.adv_type = LE_SCAN_RSP_DATA;
        else
            adv_report.adv_type = LE_ADV_DATA;
        adv_report.rssi = *rssi;
        if (evt->data_len <= BLE_MAX_ADV_SIZE) {
            memcpy(adv_report.report.data, evt->data, evt->data_len);
            adv_report.report.data_len = evt->data_len;
        } else {
            memcpy(adv_report.report.data, evt->data, BLE_MAX_ADV_SIZE);
            adv_report.report.data_len = BLE_MAX_ADV_SIZE;
        }

        char name[31] = { 0 };
        int type;
        int j = 0;
        for (;;) {
            type = adv_report.report.data[j + 1];
            if (type == 0x09) { //complete local name.
                memcpy(name, &adv_report.report.data[j + 2], adv_report.report.data[j] - 1);
                name[adv_report.report.data[j] - 1] = '\0';
                BTMG_DEBUG("rssi:%d,scan report name:%s", adv_report.rssi, name);
            }
            j = j + adv_report.report.data[j] + 1;
            if (j >= adv_report.report.data_len)
                break;
        }

        if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_le_scan_report_cb) {
            btmg_cb_p->btmg_gap_cb.gap_le_scan_report_cb(&adv_report);
        }
        evt_len = sizeof(*evt) + evt->data_len + 1;

        if (size > evt_len) {
            data += evt_len - 1;
            size -= evt_len - 1;
            evt = (void *)data;
        } else {
            break;
        }
    }
}

static void le_conn_complete(const uint8_t *data, uint8_t size, void *user_data)
{
    const struct bt_hci_evt_le_conn_complete *evt = (void *)data;

    BTMG_INFO("mac: %02X:%02X:%02X:%02X:%02X:%02X, addr_type: %d, role: %d, "
              "clock_accurary: %d",
              evt->peer_addr[5], evt->peer_addr[4], evt->peer_addr[3], evt->peer_addr[2],
              evt->peer_addr[1], evt->peer_addr[0], evt->peer_addr_type, evt->role,
              evt->clock_accuracy);
    BTMG_INFO("status: %d, handle: %04x, interval: %04x, latency: %04x, "
              "supv_timeout: %04x",
              evt->status, evt->handle, evt->interval, evt->latency, evt->supv_timeout);
}

static void le_conn_params_update(const uint8_t *data, uint8_t size, void *user_data)
{
    const struct bt_hci_evt_le_conn_update_complete *evt = (void *)data;

    BTMG_INFO("status: %d, handle: %04x, interval: %04x, latency: %04x, "
              "supv_timeout: %04x",
              evt->status, evt->handle, evt->interval, evt->latency, evt->supv_timeout);
}

static void le_conn_data_length_change(const uint8_t *data, uint8_t size, void *user_data)
{
    const struct bt_hci_evt_le_data_length_change *evt = (void *)data;

    BTMG_INFO("handle: %04x, tx_len: %d, tx_time: %d, rx_len: %d, rx_time: %d", evt->handle,
              evt->max_tx_len, evt->max_tx_time, evt->max_rx_len, evt->max_rx_time);
}

static void le_meta_event(const void *data, uint8_t size, void *user_data)
{
    uint8_t evt_code = ((const uint8_t *)data)[0];

    switch (evt_code) {
    case BT_HCI_EVT_LE_CONN_COMPLETE:
        BTMG_DEBUG("connect complete");
        le_conn_complete((const uint8_t *)data + 1, size - 1, user_data);
        break;
    case BT_HCI_EVT_LE_ADV_REPORT:
        le_scan_report((const uint8_t *)data + 1, size - 1, user_data);
        break;
    case BT_HCI_EVT_LE_CONN_UPDATE_COMPLETE:
        BTMG_DEBUG("connect update params complete");
        le_conn_params_update((const uint8_t *)data + 1, size - 1, user_data);
        break;
    case BT_HCI_EVT_LE_DATA_LENGTH_CHANGE:
        BTMG_DEBUG("connect mtu exchanged\n");
        le_conn_data_length_change((const uint8_t *)data + 1, size - 1, user_data);
        break;
    default:
        BTMG_DEBUG("le meta event, event code 0x%02x", evt_code);
    }
}

static struct bt_hci *le_scan_report_start(int dev_id)
{
    struct bt_hci *device = NULL;
    device = bt_hci_new_raw_device(dev_id);

    if (device == NULL) {
        BTMG_ERROR("Failed to open HCI raw device");
        return NULL;
    }

    bt_hci_register(device, BT_HCI_EVT_LE_META_EVENT, le_meta_event, NULL, NULL);

    return device;
}

static int le_scan_report_stop(int dev_id, struct bt_hci *dev)
{
    BTMG_DEBUG("enter");

    if (dev == NULL) {
        BTMG_ERROR("hci dev is null");
        return -1;
    }
    bt_hci_unref(dev);

    return 0;
}

int bt_le_scan_init(int dev_id)
{
    BTMG_DEBUG("enter");

    le_scan_device = le_scan_report_start(0);
    if (le_scan_device == NULL) {
        BTMG_ERROR("le scan device is null");
        return -1;
    } else {
        return 0;
    }
}

int bt_le_scan_deinit(int dev_id)
{
    BTMG_DEBUG("enter");

    le_scan_report_stop(0, le_scan_device);
    le_scan_device = NULL;

    return 0;
}

static int find_conn(int s, int dev_id, long arg)
{
    struct hci_conn_list_req *cl;
    struct hci_conn_info *ci;
    evt_le_connection_complete *conn = (evt_le_connection_complete *)arg;
    int i;

    cl = malloc(10 * sizeof(*ci) + sizeof(*cl));
    if (!cl) {
        BTMG_ERROR("Can't allocate memory");
        return -1;
    }
    cl->dev_id = dev_id;
    cl->conn_num = 10;
    ci = cl->conn_info;

    if (ioctl(s, HCIGETCONNLIST, (void *)cl)) {
        BTMG_ERROR("Can't get connection list");
        free(cl);
        return -2;
    }

    for (i = 0; i < cl->conn_num; i++, ci++) {
        if (bacmp(&conn->peer_bdaddr, &ci->bdaddr) == 0) {
            conn->handle = ci->handle;
            conn->role = (ci->link_mode & HCI_LM_MASTER) ? 1 : 0;
            free(cl);
            return 0;
        }
    }

    free(cl);
    return -3;
}

int bt_le_gap_find_conn(evt_le_connection_complete *conn)
{
    conn->handle = BT_LE_GAP_HANDLE_INVALID;
    hci_for_each_dev(HCI_UP, find_conn, (intptr_t)conn);
    if (conn->handle == BT_LE_GAP_HANDLE_INVALID) {
        BTMG_ERROR("not found conn");
        return -1;
    }
    BTMG_INFO("conn handle:%04x", conn->handle);
    return 0;
}

int bt_le_disconnect(int dev_id, uint16_t handle, uint8_t reason)
{
    int err, opt, dd;
    uint16_t _handle;
    uint8_t _reason;

    if (dev_id < 0) {
        dev_id = hci_get_route(NULL);
    }

    dd = hci_open_dev(dev_id);
    if (dd < 0) {
        BTMG_ERROR("Could not open device");
        return -1;
    }

    _handle = handle;
    _reason = reason ? reason : HCI_OE_USER_ENDED_CONNECTION;

    err = hci_disconnect(dd, _handle, _reason, 4000);
    if (err < 0) {
        BTMG_ERROR("Could not disconnect");
        hci_close_dev(dd);
        return -1;
    }

    hci_close_dev(dd);
    return 0;
}

act_func_t le_gap_action_table[] = {
    [LE_SCAN_START] = { bt_le_scan_start, "le_scan_start" },
    [LE_SCAN_STOP] = { bt_le_scan_stop, "le_scan_stop" },
    [LE_SET_SCAN_PARA] = { bt_le_set_scan_parameters, "le_scan_para" },
    [LE_UPDATE_CN_PARA] = { bt_le_update_conn_params, "le_update_cn_para" },
};
