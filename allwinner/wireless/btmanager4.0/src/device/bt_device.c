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
#include "platform.h"
#include "bt_device.h"
#include "bt_bluez.h"
#include "common.h"

btmg_device_state bt_dev_state;

btmg_device_state bt_device_get_state(void)
{
	return bt_dev_state;
}

void bt_device_set_state(btmg_device_state state)
{
	bt_dev_state = state;
}


void bt_device_pair_callback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;

    if (user_data == NULL) {
        BTMG_DEBUG("device pair call invalid");
        return;
    }

    g_dbus_connection_call_finish(bluez_mg.dbus, res, &error);

    bt_device_set_state(BTMG_DEVICE_STATE_PAIRED);
    if (error != NULL) {
        bt_device_set_state(BTMG_DEVICE_STATE_IDLE);
        BTMG_ERROR("Pair remote device failed :%s\n", error->message);
        if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_bond_state_cb)
            btmg_cb_p->btmg_gap_cb.gap_bond_state_cb(BTMG_BOND_STATE_BOND_FAILED, user_data);
        g_clear_error(&error);
    }
}

int bt_device_connect(const char *addr)
{
    gchar *path;
    GError *error = NULL;
    GVariant *result = NULL;
    int ret = BT_OK;

    BTMG_DEBUG("enter");

    if (bt_device_get_state() == BTMG_DEVICE_STATE_CONNECTING) {
        BTMG_ERROR("Connecting ...");
        return BT_ERROR_IN_PROCESS;
    }

    if (bt_device_get_state() == BTMG_DEVICE_STATE_CONNECTED) {
        BTMG_INFO("A device is currently connected");
        return BT_OK;
    }

    _get_object_path(addr, &path);

    if (path == NULL) {
        BTMG_ERROR("device not found,please check again");
        return BT_ERROR_DEVICE_NOT_FOUND;
    }

    if (bt_device_get_state() == BTMG_DEVICE_STATE_IDLE && _is_connected(path)) {
        BTMG_ERROR("device: %s is connected", addr);
        bt_device_set_state(BTMG_DEVICE_STATE_CONNECTED);
        g_free(path);
        return BT_OK;
    }

    if (bt_pro_info->is_a2dp_sink_enabled) {
        if (btmg_cb_p && btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb)
            btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb(addr,
                                                                       BTMG_A2DP_SINK_CONNECTING);
    }

    if (bt_pro_info->is_a2dp_source_enabled) {
        if (btmg_cb_p && btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb)
            btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb(
                    addr, BTMG_A2DP_SOURCE_CONNECTING);
    }

    bt_device_set_state(BTMG_DEVICE_STATE_CONNECTING);

    result = g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", path, "org.bluez.Device1",
                                         "Connect", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, 15000, NULL,
                                         &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        if (bt_pro_info->is_a2dp_sink_enabled) {
            if (btmg_cb_p && btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb)
                btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb(
                        addr, BTMG_A2DP_SINK_CONNECT_FAILED);
        }
        if (bt_pro_info->is_a2dp_source_enabled) {
            if (btmg_cb_p && btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb)
                btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb(
                        addr, BTMG_A2DP_SOURCE_CONNECT_FAILED);
        }
        bt_device_set_state(BTMG_DEVICE_STATE_IDLE);
        goto exit;
    }
    bt_device_set_state(BTMG_DEVICE_STATE_CONNECTED);
    g_variant_unref(result);

exit:
    g_free(path);

    return ret;
}

int bt_device_disconnect(const char *addr)
{
    GVariant *result = NULL;
    GError *error = NULL;
    gchar *path;
    int ret = BT_OK;

    BTMG_DEBUG("enter");

    if (bt_device_get_state() == BTMG_DEVICE_STATE_DISCONNECTING) {
        BTMG_ERROR("Disconnecting ...");
        return BT_ERROR_IN_PROCESS;
    }

    _get_object_path(addr, &path);

    if (path == NULL) {
        BTMG_ERROR("device not found,please check again");
        return BT_ERROR_DEVICE_NOT_FOUND;
    }

    if (_is_connected(path) == false) {
        BTMG_ERROR("device:[%s] is not currently connected, no need to disconnect.", addr);
        return BT_OK;
    }

    if (bt_pro_info->is_a2dp_sink_enabled) {
        if (btmg_cb_p && btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb)
            btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb(
                    addr, BTMG_A2DP_SINK_DISCONNECTING);
    }
    if (bt_pro_info->is_a2dp_source_enabled) {
        if (btmg_cb_p && btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb)
            btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb(
                    addr, BTMG_A2DP_SOURCE_DISCONNECTING);
    }

    bt_device_set_state(BTMG_DEVICE_STATE_DISCONNECTING);

    result = g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", path, "org.bluez.Device1",
                                         "Disconnect", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, 5000,
                                         NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        if (bt_pro_info->is_a2dp_sink_enabled) {
            if (btmg_cb_p && btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb)
                btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb(
                        addr, BTMG_A2DP_SINK_DISCONNEC_FAILED);
        }
        if (bt_pro_info->is_a2dp_source_enabled) {
            if (btmg_cb_p && btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb)
                btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb(
                        addr, BTMG_A2DP_SOURCE_DISCONNEC_FAILED);
        }
        goto exit;
    }

    bt_device_set_state(BTMG_DEVICE_STATE_IDLE);
    g_variant_unref(result);

exit:
    g_free(path);

    return ret;
}

int bt_device_pair(const char *addr)
{
    gchar *path;

    BTMG_DEBUG("enter");

    if (bt_device_get_state() == BTMG_DEVICE_STATE_PAIRING) {
        BTMG_ERROR("Pairing ...");
        return BT_ERROR_IN_PROCESS;
    }

    _get_object_path(addr, &path);

    if (path == NULL) {
        BTMG_ERROR("device not found,please check again");
        return BT_ERROR_DEVICE_NOT_FOUND;
    }

    if (_is_paired(path)) {
        BTMG_DEBUG("this device is paired");
        return BT_OK;
    }

    if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_bond_state_cb)
        btmg_cb_p->btmg_gap_cb.gap_bond_state_cb(BTMG_BOND_STATE_BONDING, addr);

    g_dbus_connection_call(bluez_mg.dbus, "org.bluez", path, "org.bluez.Device1", "Pair", NULL,
                           NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, bt_device_pair_callback,
                           (char *)addr);

    bt_device_set_state(BTMG_DEVICE_STATE_PAIRING);

    g_free(path);

    return BT_OK;
}

int bt_device_get_name(const char *addr, char *name)
{
    gchar *path;
    GVariant *remote_name = NULL;
    const gchar *g_name = NULL;

    _get_object_path(addr, &path);

    if (path == NULL) {
        BTMG_ERROR("device not found,please check again");
        return BT_ERROR_DEVICE_NOT_FOUND;
    }
    remote_name =
            g_dbus_get_property(bluez_mg.dbus, "org.bluez", path, "org.bluez.Device1", "Name");

    if (remote_name != NULL) {
        g_name = g_variant_get_string(remote_name, NULL);
        strcpy(name, g_name);
        g_variant_unref(remote_name);
    } else {
        BTMG_ERROR("get remote name fail");
        return BT_ERROR;
    }
    BTMG_DEBUG("get remote name:%s", name);

    return BT_OK;
}

void bt_device_get_address(const char *path, char **address)
{
    GVariant *addr = NULL;

    addr = g_dbus_get_property(bluez_mg.dbus, "org.bluez", path, "org.bluez.Device1", "Address");

    if (addr != NULL) {
        g_variant_get(addr, "s", address);
        g_variant_unref(addr);
    } else {
        BTMG_ERROR("get remote name fail");
    }
}

bool bt_device_is_paired(const char *addr)
{
    gchar *path;
    bool paired = false;

    _get_object_path(addr, &path);

    if (path == NULL) {
        BTMG_ERROR("device not found,please check again");
        return false;
    }

    paired = _is_paired(path) ? true : false;

    g_free(path);

    return paired;
}

bool bt_device_is_connected(const char *addr)
{
    gchar *path;
    bool connected = false;

    _get_object_path(addr, &path);

    if (path == NULL)
        return connected;

    connected = _is_connected(path) ? true : false;

    g_free(path);

    return connected;
}

int bt_remove_device(const char *addr, bool is_paired)
{
    GVariant *result = NULL;
    GError *error = NULL;
    gchar *path;
    int ret = BT_OK;

    BTMG_DEBUG("enter");

    _get_object_path(addr, &path);

    if (path == NULL) {
        BTMG_ERROR("device not found,please check again");
        return BT_ERROR_DEVICE_NOT_FOUND;
    }

    result = g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", "/org/bluez/hci0",
                                         "org.bluez.Adapter1", "RemoveDevice",
                                         g_variant_new("(o)", path), NULL, G_DBUS_CALL_FLAGS_NONE,
                                         -1, NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        BTMG_ERROR("remove device fail");
        goto exit;
    }

    if (is_paired) {
        if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_bond_state_cb)
            btmg_cb_p->btmg_gap_cb.gap_bond_state_cb(BTMG_BOND_STATE_UNBONDED, addr);

    }
    g_variant_unref(result);

exit:
    g_free(path);

    return ret;
}

int bt_remove_unpaired_devices(void)
{
    btmg_bt_device_t *device_list = NULL;
    int count = 0, i = 0, ret = 0;

    BTMG_DEBUG("enter");

    ret = _get_devices(BTMG_DEVICE_TYPE_ANY, &device_list, &count);
    if (ret != 0) {
        BTMG_ERROR("get devices fail");
        return BT_ERROR_DEVICE_NOT_FOUND;
    }
    if (count == 0) {
        BTMG_DEBUG("bluez scan cache is empty, no need to remove");
        return BT_OK;
    }
    for (i = 0; i < count; i++) {
        if (!bt_device_is_paired(device_list[i].remote_address)) {
            BTMG_DEBUG("bt_remove_device %s", device_list[i].remote_address);
            bt_remove_device(device_list[i].remote_address, false);
        }
    }
    bt_free_devices(device_list, count);

    return BT_OK;
}

int bt_malloc_device(btmg_bt_device_t *device)
{
    int address_len = 18;
    int name_max_len = 256;
    int icon_max_len = 128;

    if (device == NULL)
        return BT_ERROR_INVALID_ARGS;

    device->remote_address = (char *)malloc(sizeof(char) * address_len);
    device->remote_name = (char *)malloc(sizeof(char) * name_max_len);
    device->icon = (char *)malloc(sizeof(char) * icon_max_len);

    return BT_OK;
}

int bt_free_device(btmg_bt_device_t *device)
{
    if (device == NULL)
        return BT_ERROR_INVALID_ARGS;

    if (device->remote_address != NULL)
        g_free(device->remote_address);
    if (device->remote_name != NULL)
        g_free(device->remote_name);
    if (device->icon != NULL)
        g_free(device->icon);

    return BT_OK;
}

int bt_free_devices(btmg_bt_device_t *device_list, int count)
{
    int i = 0;

    if (device_list == NULL)
        return BT_ERROR_INVALID_ARGS;

    for (i = 0; i < count; i++) {
        if (device_list[i].remote_address != NULL)
            g_free(device_list[i].remote_address);
        if (device_list[i].remote_name != NULL)
            g_free(device_list[i].remote_name);
        if (device_list[i].icon != NULL)
            g_free(device_list[i].icon);
    }

    return BT_OK;
}

int bt_free_paired_devices(btmg_bt_device_t *dev_list, int count)
{
    BTMG_DEBUG("enter");

    return bt_free_devices(dev_list, count);
}

int bt_get_paired_devices(btmg_bt_device_t **dev_list, int *count)
{
    BTMG_DEBUG("enter");

    return _get_devices(BTMG_DEVICE_TYPE_PAIRED, dev_list, count);
}
