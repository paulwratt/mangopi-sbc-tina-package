#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "bt_log.h"
#include "bt_manager.h"
#include "bt_adapter.h"
#include "bt_device.h"
#include "bt_bluez.h"
#include "common.h"
#include "platform.h"

btmg_adapter_state_t bt_adapter_get_power_state(void)
{
    bool powered = false;
    GVariant *power_state = NULL;

    power_state = g_dbus_get_property(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0",
                                      "org.bluez.Adapter1", "Powered");

    if (power_state != NULL) {
        powered = g_variant_get_boolean(power_state);
        g_variant_unref(power_state);
    } else {
        BTMG_ERROR("get power state fail");
        return BT_ERROR;
    }

    if (powered == true) {
        BTMG_DEBUG("bt adapter power on");
        return BTMG_ADAPTER_ON;
    } else {
        BTMG_DEBUG("bt adapter power off");
        return BTMG_ADAPTER_OFF;
    }
}

int bt_adapter_set_power(bool power)
{
    bool result = false;

    BTMG_DEBUG("set power state:%d", power);

    result = g_dbus_set_property(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0",
                                 "org.bluez.Adapter1", "Powered", g_variant_new_boolean(power));

    if (!result) {
        BTMG_ERROR("set adapter power fail");
        return BT_ERROR;
    }

    return BT_OK;
}

int bt_adapter_set_alias(const char *alias)
{
    bool result = false;

    if (alias == NULL) {
        BTMG_DEBUG("alias is NULL");
        return BT_ERROR_INVALID_ARGS;
    }

    BTMG_DEBUG("set alias:%s", alias);
    result = g_dbus_set_property(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0",
                                 "org.bluez.Adapter1", "Alias", g_variant_new_string(alias));

    if (!result) {
        BTMG_ERROR("set adapter alias fail");
        return BT_ERROR;
    }

    return BT_OK;
}

int bt_adapter_set_discoverable(bool discoverable)
{
    bool result = false;

    BTMG_DEBUG("set discoverable:%d", discoverable);
    result =
            g_dbus_set_property(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
                                "Discoverable", g_variant_new_boolean(discoverable));

    if (!result) {
        BTMG_ERROR("set discoverable fail");
        return BT_ERROR;
    }

    return BT_OK;
}

int bt_adapter_set_pairable(bool pairable)
{
    bool result = false;

    BTMG_DEBUG("set pairable:%d", pairable);

    result = g_dbus_set_property(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0",
                                 "org.bluez.Adapter1", "Pairable", g_variant_new_boolean(pairable));

    if (!result) {
        BTMG_ERROR("set pairable fail");
        return BT_ERROR;
    }

    return BT_OK;
}

int bt_adapter_set_pairableTimeout(unsigned int timeout)
{
    bool result = false;

    BTMG_DEBUG("set pairableTimeout:%d", timeout);

    result =
            g_dbus_set_property(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
                                "PairableTimeout", g_variant_new_uint32(timeout));

    if (!result) {
        BTMG_ERROR("set pairableTimeout fail");
        return BT_ERROR;
    }

    return BT_OK;
}

int bt_adapter_get_info(struct bt_adapter *adapter)
{
    GVariant *result;
    GError *error = NULL;
    int ret = BT_OK;

    BTMG_DEBUG("enter");

    if (adapter == NULL) {
        BTMG_DEBUG("adapter is null");
        return BT_ERROR_INVALID_ARGS;
    }

    result = g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0",
                                         "org.freedesktop.DBus.Properties", "GetAll",
                                         g_variant_new("(s)", "org.bluez.Adapter1"), NULL,
                                         G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        BTMG_DEBUG("get adapter info fail");
        goto exit;
    }

    _get_adapter_properties(result, adapter);
    g_variant_unref(result);

exit:

    return ret;
}

int bt_adapter_get_name(char *name)
{
    GVariant *alias = NULL;
    const gchar *g_name;

    alias = g_dbus_get_property(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
                                "Alias");

    if (alias != NULL) {
        g_name = g_variant_get_string(alias, NULL);
        strcpy(name, g_name);
        g_variant_unref(alias);
    } else {
        BTMG_ERROR("get name fail");
        return BT_ERROR;
    }
    BTMG_DEBUG("get name:%s", name);

    return BT_OK;
}

int bt_adapter_get_address(char *address)
{
    GVariant *addr = NULL;
    const gchar *g_addr;
    gsize *g_length;

    addr = g_dbus_get_property(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
                               "Address");

    if (addr != NULL) {
        g_addr = g_variant_get_string(addr, g_length);
        strcpy(address, (const char *)g_addr);
        g_variant_unref(addr);
    } else {
        BTMG_ERROR("get address fail");
        return BT_ERROR;
    }
    BTMG_DEBUG("get address: %s", address);

    return BT_OK;
}

int bt_adapter_scan_filter(btmg_scan_filter_t *filter)
{
    GError *error = NULL;
    GVariantBuilder *b1, *b2;
    GVariant *scan_filter;
    gchar *type;
    guint i;
    int ret = BT_OK;

    if (filter != NULL) {
        b1 = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
        b2 = g_variant_builder_new(G_VARIANT_TYPE("as"));
        if (filter->type == 1)
            type = "bredr";
        else if (filter->type == 2)
            type = "le";
        else
            type = "auto";

        BTMG_DEBUG("scan filter type: %s", type);
        g_variant_builder_add(b1, "{sv}", "Transport", g_variant_new_string(type));

        if (filter->uuid_num < 1) {
            filter->uuid_num = 0;
            filter->uuid_list = NULL;
        }

        for (i = 0; i < filter->uuid_num; i++) {
            g_variant_builder_add(b1, "s", filter->uuid_list[i]);
            BTMG_DEBUG("uuid: %s", filter->uuid_list[i]);
        }
        g_variant_builder_add(b1, "{sv}", "UUIDs", g_variant_new("as", b2));
        g_variant_builder_add(b1, "{sv}", "RSSI", g_variant_new_int16(filter->rssi));

        BTMG_DEBUG("filter rssi: %d", filter->rssi);

        scan_filter = g_variant_new("(a{sv})", b1);
        g_variant_builder_unref(b1);
        g_variant_builder_unref(b2);
    } else {
        scan_filter = g_variant_new("(a{sv})", NULL);
    }

    g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
                                "SetDiscoveryFilter", scan_filter, NULL, G_DBUS_CALL_FLAGS_NONE, -1,
                                NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        BTMG_ERROR("set scan filter fail");
        goto exit;
    }

exit:

    return ret;
}

int bt_adapter_start_scan(void)
{
    GVariant *result = NULL;
    GError *error = NULL;
    int ret = BT_OK;

    BTMG_DEBUG("enter");

    bt_remove_unpaired_devices();
    result = g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0",
                                         "org.bluez.Adapter1", "StartDiscovery", NULL, NULL,
                                         G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        BTMG_ERROR("start scan fail");
        goto exit;
    }

    g_variant_unref(result);

exit:

    return ret;
}

int bt_adapter_stop_scan(void)
{
    GVariant *result = NULL;
    GError *error = NULL;
    int ret = BT_OK;

    BTMG_DEBUG("enter");

    result = g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0",
                                         "org.bluez.Adapter1", "StopDiscovery", NULL, NULL,
                                         G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        BTMG_ERROR("stop scan fail");
        goto exit;
    }

    g_variant_unref(result);
exit:

    return ret;
}

bool bt_adapter_is_scanning(void)
{
    GVariant *scan_state = NULL;
    gboolean is_scanning;

    scan_state = g_dbus_get_property(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0",
                                     "org.bluez.Adapter1", "Discovering");

    if (scan_state != NULL) {
        is_scanning = g_variant_get_boolean(scan_state);
        g_variant_unref(scan_state);
    } else {
        BTMG_ERROR("get adapter scan state fail");
        return BT_ERROR;
    }

    return is_scanning;
}

int bt_adapter_free(struct bt_adapter *adapter)
{
    int i = 0;

    if (adapter == NULL)
        return BT_ERROR_INVALID_ARGS;

    if (adapter->uuid_length > 0) {
        for (i = 0; i < adapter->uuid_length; i++) {
            g_free(adapter->uuid_list[i].uuid);
        }
        g_free(adapter->uuid_list);
    }
    g_free(adapter->address);
    g_free(adapter->name);
    g_free(adapter->alias);

    return BT_OK;
}

int bt_adapter_set_scan_mode(char *opt)
{
    struct hci_dev_req dr;
    int ctl;
    int hdev = 0;

    if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0) {
        BTMG_ERROR("Can't open HCI socket.");
        return BT_ERROR;
    }

    dr.dev_id = hdev;
    if (!strcmp(opt, "iscan"))
        dr.dev_opt = SCAN_INQUIRY;
    else if (!strcmp(opt, "pscan"))
        dr.dev_opt = SCAN_PAGE;
    else if (!strcmp(opt, "piscan"))
        dr.dev_opt = SCAN_PAGE | SCAN_INQUIRY;
    else if (!strcmp(opt, "nopiscan"))
        dr.dev_opt = SCAN_DISABLED;

    if (ioctl(ctl, HCISETSCAN, (unsigned long)&dr) < 0) {
        BTMG_ERROR("Can't set scan mode, err: %s", strerror(errno));
        close(ctl);
        return BT_ERROR;
    }
    close(ctl);

    return BT_OK;
}

int bt_adapter_set_page_timeout(int timeout)
{
    struct hci_request rq;
    write_page_timeout_cp cp;
    int hdev = 0;
    int s;

    if ((s = hci_open_dev(hdev)) < 0) {
        BTMG_ERROR("Can't open hci device");
        return BT_ERROR;
    }
    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_HOST_CTL;
    rq.ocf = OCF_WRITE_PAGE_TIMEOUT;
    rq.cparam = &cp;
    rq.clen = WRITE_PAGE_TIMEOUT_CP_SIZE;
    cp.timeout = htobs((uint16_t)timeout);

    if (timeout < 0x01 || timeout > 0xFFFF) {
        BTMG_ERROR("page timeout out of range!\n");
        return BT_ERROR;
    }
    if (hci_send_req(s, &rq, 2000) < 0) {
        BTMG_ERROR("Can't set page timeout, err: %s", strerror(errno));
        hci_close_dev(s);
        return BT_ERROR;
    }
    hci_close_dev(s);

    return BT_OK;
}
