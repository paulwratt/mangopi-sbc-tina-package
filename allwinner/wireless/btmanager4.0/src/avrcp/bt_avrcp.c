#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <unistd.h>
#include <gio/gio.h>
#include "common.h"
#include "bt_log.h"
#include "bt_bluez.h"

static int _avrcp_control(const char *action)
{
    int ret = BT_OK;
    char device_address[18] = { 0 };
    dev_node_t *dev_node = NULL;

    GError *err = NULL;
    GVariant *result = NULL;
    GVariant *v = NULL;
    char *dev_path = NULL;
    gchar *play_path = NULL;

    BTMG_INFO("avrcp action:%s", action);

    dev_node = connected_devices->head;
    while (dev_node != NULL) {
        if (dev_node->profile & BTMG_REMOTE_DEVICE_A2DP) {
            memcpy(device_address, dev_node->dev_addr, sizeof(device_address));
            break;
        }
        dev_node = dev_node->next;
    }
    dev_path = btmg_addr_to_path(device_address);
    if (dev_path == NULL) {
        BTMG_ERROR("Can't get connected device");
        return BT_ERROR_A2DP_DEVICE_NOT_CONNECTED;
    }
    /* /org/bluez/hci0/dev_48_A1_95_5C_82_B3 */
    dev_path[37] = '\0';
    BTMG_DEBUG("device path %s", dev_path);
    result = g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", dev_path,
                                         "org.freedesktop.DBus.Properties", "Get",
                                         g_variant_new("(ss)", "org.bluez.MediaControl1", "Player"),
                                         G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, 5000, NULL,
                                         &err);
    ret = g_dbus_call_sync_check_error(err);
    if (ret < 0) {
        BTMG_ERROR("get player fail");
        return ret;
    }

    g_variant_get(result, "(v)", &v);
    g_variant_unref(result);
    g_variant_get(v, "o", &play_path);
    g_variant_unref(v);

    BTMG_DEBUG("device player path %s", play_path);
    result = g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", play_path,
                                         "org.bluez.MediaPlayer1", action, NULL, NULL,
                                         G_DBUS_CALL_FLAGS_NONE, 5000, NULL, &err);
    ret = g_dbus_call_sync_check_error(err);
    if (ret < 0) {
        BTMG_ERROR("call action: %s fail", action);
        goto final;
    }

final:

    if (result != NULL)
        g_variant_unref(result);

    return ret;
}

int bluez_avrcp_play()
{
    return _avrcp_control("Play");
}

int bluez_avrcp_pause()
{
    return _avrcp_control("Pause");
}

int bluez_avrcp_next()
{
    return _avrcp_control("Next");
}

int bluez_avrcp_previous()
{
    return _avrcp_control("Previous");
}

int bluez_avrcp_fastforward()
{
    return _avrcp_control("FastForward");
}

int bluez_avrcp_rewind()
{
    return _avrcp_control("Rewind");
}
