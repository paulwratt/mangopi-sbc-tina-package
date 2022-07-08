#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <gio/gunixfdlist.h>
#include "bt_manager.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "bt_log.h"
#include "bt_alarm.h"
#include "bt_adapter.h"

#include "bt_device.h"
#include "bt_bluez.h"
#include "bt_rfcomm.h"
#include "bt_bluez_signals.h"
#include "common.h"

static guint interface_subscription_add_id = -1;
static guint interface_subscription_remove_id = -1;
static guint adapter_subscription_id = -1;
static guint device_subscription_id = -1;
static guint tansport_subscription_id = -1;
static guint mediaplayer_subscription_id = -1;
static guint mediacontrol_subscription_id = -1;
static guint mediatransport_subscription_id = -1;

typedef enum {
    BT_EVENT_CONNECT = 1,
    BT_EVENT_BOND,
} bt_event_t;

static media_change_t media_change_cb = NULL;
unsigned int music_duration = 0;

void bluez_register_media_change_cb(media_change_t cb)
{
    media_change_cb = cb;
}

void bluez_unregister_media_change_cb(void)
{
    media_change_cb = NULL;
}
static void bluez_device_changed_callback(btmg_bt_device_t *device, bool add)
{
    if (add) {
        if (device->remote_name == NULL)
            return;
        if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_device_add_cb) {
            btmg_cb_p->btmg_gap_cb.gap_device_add_cb(device);
        }
    } else {
        if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_device_remove_cb) {
            btmg_cb_p->btmg_gap_cb.gap_device_remove_cb(device);
        }
    }
}

static void bluez_signal_interfaces_added(GDBusConnection *conn, const gchar *sender,
                                          const gchar *path, const gchar *interface,
                                          const gchar *signal, GVariant *params, void *userdata)
{
    GVariantIter *interfaces = NULL;
    GVariant *properties = NULL;
    char *object;
    gchar *key;

    g_variant_get(params, "(&oa{sa{sv}})", &object, &interfaces);
    while (g_variant_iter_next(interfaces, "{&s@a{sv}}", &key, &properties)) {
        if (g_str_has_prefix(key, "org.bluez")) {
            BTMG_DUMP("add interfaces: %s", key);
        }
        if (strcmp(key, "org.bluez.Device1") == 0) {
            btmg_bt_device_t device;
            _get_device_properties(properties, &device);
            bluez_device_changed_callback(&device, true);
            bt_free_device(&device);
        }
        g_variant_unref(properties);
    } /*end of while*/

    g_variant_iter_free(interfaces);
}

static void bluez_signal_interfaces_removed(GDBusConnection *conn, const gchar *sender,
                                            const gchar *path, const gchar *interface_,
                                            const gchar *signal, GVariant *params, void *userdata)
{
    GVariantIter *interfaces;
    const char *object_path;
    const char *interface;
    const char *address;
    btmg_bt_device_t device;
    const char *addr;
    gchar *key;

    g_variant_get(params, "(&oas)", &object_path, &interfaces);
    memset(&device, 0, sizeof(btmg_bt_device_t));
    addr = btmg_path_to_addr(object_path);
    device.remote_address = (char *)addr;
    while (g_variant_iter_next(interfaces, "&s", &key)) {
        if (strcmp(key, "org.bluez.Device1") == 0) {
            bluez_device_changed_callback(&device, false);
        }
    }
    g_variant_iter_free(interfaces);
}

void bluez_app_callback_handle(btmg_bt_device_t *device, bt_event_t evt)
{
    dev_node_t *dev_node = NULL;

    if (device->r_class == 0) {
        return;
    }
    switch (evt) {
    case BT_EVENT_BOND:
        BTMG_DEBUG("remote device:%s, pair:%d", device->remote_address, device->paired);
        if (device->paired == true) {
            if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_bond_state_cb)
                btmg_cb_p->btmg_gap_cb.gap_bond_state_cb(BTMG_BOND_STATE_BONDED,
                                                         device->remote_address);
        } else {
            if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_bond_state_cb)
                btmg_cb_p->btmg_gap_cb.gap_bond_state_cb(BTMG_BOND_STATE_UNBONDED,
                                                         device->remote_address);
        }
        break;
    case BT_EVENT_CONNECT:
        if (device->remote_name == NULL) {
            BTMG_ERROR("remote device name should not be null");
            break;
        }
        BTMG_DEBUG("remote device:%s, connected:%d", device->remote_address, device->connected);

        if (device->connected == false) {
            dev_node = btmg_dev_list_find_device(connected_devices, device->remote_address);
            if (dev_node != NULL) {
                if (dev_node->profile & BTMG_REMOTE_DEVICE_A2DP) {
                    if (bt_pro_info->is_a2dp_sink_enabled) {
                        BTMG_DEBUG("a2dp sink disconnected,device:%s", device->remote_address);
                        if (btmg_cb_p && btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb)
                            btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb(
                                    device->remote_address, BTMG_A2DP_SINK_DISCONNECTED);
                    }
                    if (bt_pro_info->is_a2dp_source_enabled) {
                        BTMG_DEBUG("a2dp source disconnected,device:%s", device->remote_address);
                        if (btmg_cb_p && btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb)
                            btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb(
                                    device->remote_address, BTMG_A2DP_SOURCE_DISCONNECTED);
                    }
                    bt_device_set_state(BTMG_DEVICE_STATE_DISCONNECTED);
                }
                if (dev_node->profile & BTMG_REMOTE_DEVICE_SPP) {
                    if (bt_pro_info->is_sppc_enabled) {
                        if (!is_spp_client_run())
                            break;
                        spp_client_stop_recv_thread();
                        BTMG_DEBUG("spp client disconnected,device:%s", device->remote_address);
                        if (btmg_cb_p && btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb)
                            btmg_cb_p->btmg_spp_client_cb.spp_client_connection_state_cb(
                                    device->remote_address, BTMG_SPP_CLIENT_DISCONNECTED);
                    }
                    if (bt_pro_info->is_spps_enabled) {
                        if (!is_spp_service_run())
                            break;
                        spp_service_stop_recv_thread();
                        BTMG_DEBUG("spp service disconnected,device:%s", device->remote_address);
                        if (btmg_cb_p && btmg_cb_p->btmg_spp_server_cb.spp_server_connection_state_cb) {
                            btmg_cb_p->btmg_spp_server_cb.spp_server_connection_state_cb(
                                    device->remote_address, BTMG_SPP_SERVER_DISCONNECTED);
                        }
                    }
                }
                BTMG_DEBUG("remove device %s from connected_devices", device->remote_address);
                btmg_dev_list_remove_device(connected_devices, device->remote_address);
            }
        }
        break;
    }
}

void bluez_signals_connection_cb(const gchar *path, bt_event_t evt, GVariant *val)
{
    GVariant *v1, *v2;
    bool is_connected = false;
    btmg_bt_device_t device;
    dev_node_t *dev_node = NULL;

    if (!(evt & (BT_EVENT_BOND | BT_EVENT_CONNECT)))
        return;

    if (_get_all_device_properties(path, &v1) == 0) {
        v2 = g_variant_get_child_value(v1, 0);
        _get_device_properties(v2, &device);
        bluez_app_callback_handle(&device, evt);
        bt_free_device(&device);
        g_variant_unref(v1);
        g_variant_unref(v2);
    } else {
        BTMG_DEBUG("interface is removed, cannot get device properties");
        if (evt == BT_EVENT_BOND) {
            BTMG_DEBUG("evt:BT_EVENT_BOND, ignore");
            return;
        }
        if (evt == BT_EVENT_CONNECT) {
            bt_malloc_device(&device);
            device.connected = g_variant_get_boolean(val);
            strcpy(device.remote_address, btmg_path_to_addr(path));
            BTMG_DEBUG("remote device address: %s", device.remote_address);
            dev_node = btmg_dev_list_find_device(connected_devices, device.remote_address);
            if (dev_node != NULL && dev_node->dev_name != NULL) {
                memcpy(device.remote_name, dev_node->dev_name, sizeof(dev_node->dev_name));
            }
            bluez_app_callback_handle(&device, evt);
            bt_free_device(&device);
        }
    }
}

void bluez_signals_rssi_cb(const gchar *path, GVariant *val)
{
    int rssi;
    rssi = g_variant_get_int16(val);
    if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_update_rssi_cb) {
        btmg_cb_p->btmg_gap_cb.gap_update_rssi_cb(btmg_path_to_addr(path), rssi);
    }
}

void btmg_signal_adapter_properties_changed(GDBusConnection *conn, const gchar *sender,
                                            const gchar *path, const gchar *interface,
                                            const gchar *signal, GVariant *params, void *userdata)
{
    GVariantIter *properties = NULL;
    GVariantIter *unknown = NULL;
    char *iface = NULL;
    char *key = NULL;
    char *uuid = NULL;
    GVariant *value = NULL;
    const gchar *type = (gchar *)g_variant_get_type_string(params);

    BTMG_DUMP("\n \
		signal:%s\n \
		signature:%s\n \
		object path:%s",
              signal, type, path);

    if (strcmp(type, "(sa{sv}as)") != 0) {
        BTMG_DEBUG("type is different: %s", type);
    } else {
        g_variant_get(params, "(&sa{sv}as)", &iface, &properties, &unknown);
        while (g_variant_iter_next(properties, "{&sv}", &key, &value)) {
            BTMG_DEBUG("property: %s", key);
            if (strcmp("Discovering", key) == 0) {
                if (g_variant_get_boolean(value) == TRUE) {
                    if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_scan_status_cb)
                        btmg_cb_p->btmg_gap_cb.gap_scan_status_cb(BTMG_SCAN_STARTED);
                    BTMG_INFO("Scan Started");
                } else {
                    if (btmg_cb_p && btmg_cb_p->btmg_gap_cb.gap_scan_status_cb)
                        btmg_cb_p->btmg_gap_cb.gap_scan_status_cb(BTMG_SCAN_STOPPED);
                    BTMG_INFO("Scan Stopped");
                }
            }
            g_variant_unref(value);
        }
    }
}

static void _device_properties_changed(const gchar *path, GVariant *properties)
{
    GVariant *val;
    GVariantIter *iter;
    gchar *key;

    g_variant_get(properties, "a{sv}", &iter);
    while (g_variant_iter_loop(iter, "{&sv}", &key, &val)) {
        if (g_strcmp0(key, "Connected") == 0) {
            BTMG_DEBUG("property: %s", key);
            bluez_signals_connection_cb(path, BT_EVENT_CONNECT, val);
        } else if (g_strcmp0(key, "Paired") == 0) {
            BTMG_DEBUG("property: %s", key);
            bluez_signals_connection_cb(path, BT_EVENT_BOND, val);
        } else if (g_strcmp0(key, "RSSI") == 0) {
            bluez_signals_rssi_cb(path, val);
        }
    }

    g_variant_iter_free(iter);
}

static void bluez_signal_device_properties_changed(GDBusConnection *conn, const gchar *sender,
                                                   const gchar *path, const gchar *interface_name,
                                                   const gchar *signal, GVariant *params,
                                                   void *userdata)
{
    GVariant *properties;
    gchar *interface;

    g_variant_get(params, "(&s@a{sv}@as)", &interface, &properties, NULL);
    if (g_str_has_prefix(path, "/org/bluez")) {
        _print_variant(params);
        BTMG_DUMP("path:%s", path);
        if (g_strcmp0("org.bluez.Device1", interface) == 0) {
            _device_properties_changed(path, properties);
        }
    }
    g_variant_unref(properties);
}

static void _mediaplayer_track(GVariant *dictionary, const gchar *path)
{
    gchar *key;
    gsize len = 0;
    GVariantIter iter;
    GVariant *value;
    unsigned int duration = -1;
    static btmg_track_info_t track_info;

    g_variant_iter_init(&iter, dictionary);
    while (g_variant_iter_next(&iter, "{&sv}", &key, &value)) {
        if (g_strcmp0(key, "Title") == 0) {
            strcpy(track_info.title, g_variant_get_string(value, &len));
        } else if (g_strcmp0(key, "Artist") == 0) {
            strcpy(track_info.artist, g_variant_get_string(value, &len));
        } else if (g_strcmp0(key, "Album") == 0) {
            strcpy(track_info.album, g_variant_get_string(value, &len));
        } else if (g_strcmp0(key, "Genre") == 0) {
            strcpy(track_info.genre, g_variant_get_string(value, &len));
        } else if (g_strcmp0(key, "NumberOfTracks") == 0) {
            sprintf(track_info.num_tracks, "%u", g_variant_get_uint32(value));
        } else if (g_strcmp0(key, "TrackNumber") == 0) {
            sprintf(track_info.track_num, "%u", g_variant_get_uint32(value));
        } else if (g_strcmp0(key, "Duration") == 0) {
            duration = g_variant_get_uint32(value);
            music_duration = duration;
            sprintf(track_info.duration, "%u", duration);
        }
        g_variant_unref(value);
    }

    if (btmg_cb_p && btmg_cb_p->btmg_avrcp_cb.avrcp_track_changed_cb)
        btmg_cb_p->btmg_avrcp_cb.avrcp_track_changed_cb(btmg_path_to_addr(path), track_info);
}

static void _mediaplayer_position(GVariant *value, const gchar *path)
{
    uint32_t position = 0;

    position = g_variant_get_uint32(value);
    if (btmg_cb_p && btmg_cb_p->btmg_avrcp_cb.avrcp_play_position_cb)
        btmg_cb_p->btmg_avrcp_cb.avrcp_play_position_cb(btmg_path_to_addr(path), music_duration,
                                                        position);
}

static void _mediaplayer_status(GVariant *value, const gchar *path)
{
    gsize len;
    char player_status[18];

    strcpy(player_status, g_variant_get_string(value, &len));
    if (strcmp(player_status, "playing") == 0) {
        if (btmg_cb_p && btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb) {
            btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb(btmg_path_to_addr(path),
                                                         BTMG_AVRCP_PLAYSTATE_PLAYING);
            if (bt_pro_info->is_a2dp_sink_enabled) {
                if (btmg_cb_p && btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_audio_state_cb)
                    btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_audio_state_cb(
                            btmg_path_to_addr(path), BTMG_A2DP_SINK_AUDIO_STARTED);
            }
        }
        if (media_change_cb) {
            media_change_cb(BT_MEDIA_PLAYING);
        }
    } else if (strcmp(player_status, "stopped") == 0) {
        if (btmg_cb_p && btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb) {
            btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb(btmg_path_to_addr(path),
                                                         BTMG_AVRCP_PLAYSTATE_STOPPED);
            if (bt_pro_info->is_a2dp_sink_enabled) {
                if (btmg_cb_p && btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_audio_state_cb)
                    btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_audio_state_cb(
                            btmg_path_to_addr(path), BTMG_A2DP_SINK_AUDIO_STOPPED);
            }
        }
        if (media_change_cb) {
            media_change_cb(BT_MEDIA_STOP);
        }
    } else if (strcmp(player_status, "paused") == 0) {
        if (btmg_cb_p && btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb) {
            btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb(btmg_path_to_addr(path),
                                                         BTMG_AVRCP_PLAYSTATE_PAUSED);
            if (bt_pro_info->is_a2dp_sink_enabled) {
                if (btmg_cb_p && btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_audio_state_cb)
                    btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_audio_state_cb(
                            btmg_path_to_addr(path), BTMG_A2DP_SINK_AUDIO_SUSPENDED);
            }
        }
        if (media_change_cb) {
            media_change_cb(BT_MEDIA_PAUSE);
        }
    } else if (strcmp(player_status, "forward-seek") == 0) {
        if (btmg_cb_p && btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb)
            btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb(btmg_path_to_addr(path),
                                                         BTMG_AVRCP_PLAYSTATE_FWD_SEEK);
    } else if (strcmp(player_status, "reverse-seek") == 0) {
        if (btmg_cb_p && btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb)
            btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb(btmg_path_to_addr(path),
                                                         BTMG_AVRCP_PLAYSTATE_REV_SEEK);
    } else if (strcmp(player_status, "error") == 0) {
        if (btmg_cb_p && btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb)
            btmg_cb_p->btmg_avrcp_cb.avrcp_play_state_cb(btmg_path_to_addr(path),
                                                         BTMG_AVRCP_PLAYSTATE_ERROR);
    }
}

static void bluez_signal_mediacontrol_properties_changed(GDBusConnection *conn, const gchar *sender,
                                                         const gchar *path, const gchar *interface,
                                                         const gchar *signal, GVariant *params,
                                                         void *userdata)
{
    int len = 0;
    GVariantIter *properties = NULL;
    GVariantIter *unknown = NULL;
    const char *iface = NULL;
    gchar *key = NULL;
    GVariant *value = NULL;
    dev_node_t *dev_node = NULL;
    bool is_mediacontrol_connected = false;
    const char *remote_address;
    char remote_name[256] = { 0 };
    const gchar *type = g_variant_get_type_string(params);

    if (strcmp(type, "(sa{sv}as)") != 0) {
        BTMG_ERROR("type is different: %s", type);
        goto fail;
    }

    remote_address = btmg_path_to_addr(path);
    g_variant_get(params, "(&sa{sv}as)", &iface, &properties, &unknown);
    while (g_variant_iter_next(properties, "{&sv}", &key, &value)) {
        if (!strcmp(key, "Connected")) {
            is_mediacontrol_connected = g_variant_get_boolean(value);
            BTMG_DEBUG("mediacontrol connected:%d", is_mediacontrol_connected);
        }
        g_variant_unref(value);
    }

    if (is_mediacontrol_connected) {
        bt_device_get_name(remote_address, remote_name);
        dev_node = btmg_dev_list_find_device(connected_devices, remote_address);
        if (dev_node == NULL) {
            btmg_dev_list_add_device(connected_devices, remote_name, remote_address,
                                     BTMG_REMOTE_DEVICE_A2DP);
            BTMG_DEBUG("add device %s into connected_devices", remote_address);
        }

        if (bt_pro_info->is_a2dp_sink_enabled) {
            BTMG_DEBUG("a2dp sink connected,device:%s", remote_address);
            if (btmg_cb_p && btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb) {
                btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb(
                        remote_address, BTMG_A2DP_SINK_CONNECTED);
            }
        }
        if (bt_pro_info->is_a2dp_source_enabled) {
            if (btmg_cb_p && btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb) {
                BTMG_DEBUG("a2dp source connected,device:%s", remote_address);
                btmg_cb_p->btmg_a2dp_source_cb.a2dp_source_connection_state_cb(
                        remote_address, BTMG_A2DP_SOURCE_CONNECTED);
            }
        }
        bt_device_set_state(BTMG_DEVICE_STATE_CONNECTED);
    }

fail:
    if (properties != NULL)
        g_variant_iter_free(properties);
    if (unknown != NULL)
        g_variant_iter_free(unknown);
}

static void bluez_signal_mediatransport_properties_changed(GDBusConnection *conn,
                                                           const gchar *sender, const gchar *path,
                                                           const gchar *interface,
                                                           const gchar *signal, GVariant *params,
                                                           void *userdata)
{
    int len = 0;
    GVariantIter *properties = NULL;
    GVariantIter *unknown = NULL;
    const char *iface = NULL;
    gchar *key = NULL;
    GVariant *value = NULL;
    dev_node_t *dev_node = NULL;

    const char *remote_address;
    char remote_name[256] = { 0 };
    const gchar *type = g_variant_get_type_string(params);

    if (strcmp(type, "(sa{sv}as)") != 0) {
        BTMG_ERROR("type is different: %s", type);
        return;
    }

    remote_address = btmg_path_to_addr(path);
    g_variant_get(params, "(&sa{sv}as)", &iface, &properties, &unknown);
    while (g_variant_iter_next(properties, "{&sv}", &key, &value)) {
        if (!strcmp(key, "Volume")) {
            uint16_t volumes = g_variant_get_uint16(value);
            BTMG_INFO("Volume is :%d", volumes);
        }
        g_variant_unref(value);
    }
}

static void bluez_signal_mediaplayer_properties_changed(GDBusConnection *conn, const gchar *sender,
                                                        const gchar *path, const gchar *interface,
                                                        const gchar *signal, GVariant *params,
                                                        void *userdata)
{
    GVariantIter *properties = NULL;
    GVariantIter *unknown = NULL;
    const char *iface = NULL;
    gchar *key = NULL;
    GVariant *value = NULL;
    int len = 0;
    const gchar *type = g_variant_get_type_string(params);

    BTMG_DUMP("\n \
		sender: %s\n \
		interface: %s\n \
		signal: %s\n \
		type: %s\n \
		path: %s\n",
              sender, interface, signal, type, path);

    if (strcmp(type, "(sa{sv}as)") != 0) {
        BTMG_ERROR("type is different: %s", type);
        goto fail;
    }

    g_variant_get(params, "(&sa{sv}as)", &iface, &properties, &unknown);
    while (g_variant_iter_next(properties, "{&sv}", &key, &value)) {
        BTMG_DEBUG("property: %s", key);
        if (g_strcmp0(key, "Track") == 0) {
            _mediaplayer_track(value, path);
        } else if (g_strcmp0(key, "Position") == 0) {
            _mediaplayer_position(value, path);
        } else if (g_strcmp0(key, "Status") == 0) {
            _mediaplayer_status(value, path);
        }
        g_variant_unref(value);
    }
fail:
    if (properties != NULL)
        g_variant_iter_free(properties);
    if (unknown != NULL)
        g_variant_iter_free(unknown);
}

void bluez_subscribe_signals(void)
{
    GDBusConnection *conn = bluez_mg.dbus;

    interface_subscription_add_id = g_dbus_connection_signal_subscribe(
            conn, "org.bluez", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", NULL, NULL,
            G_DBUS_SIGNAL_FLAGS_NONE, bluez_signal_interfaces_added, NULL, NULL);
    interface_subscription_remove_id = g_dbus_connection_signal_subscribe(
            conn, "org.bluez", "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", NULL,
            NULL, G_DBUS_SIGNAL_FLAGS_NONE, bluez_signal_interfaces_removed, NULL, NULL);
    adapter_subscription_id =
            g_dbus_connection_signal_subscribe(conn, "org.bluez", "org.freedesktop.DBus.Properties",
                                               "PropertiesChanged", NULL, "org.bluez.Adapter1",
                                               G_DBUS_SIGNAL_FLAGS_NONE,
                                               btmg_signal_adapter_properties_changed, NULL, NULL);
    device_subscription_id =
            g_dbus_connection_signal_subscribe(conn, "org.bluez", "org.freedesktop.DBus.Properties",
                                               "PropertiesChanged", NULL, "org.bluez.Device1",
                                               G_DBUS_SIGNAL_FLAGS_NONE,
                                               bluez_signal_device_properties_changed, NULL, NULL);
    mediaplayer_subscription_id = g_dbus_connection_signal_subscribe(
            conn, "org.bluez", "org.freedesktop.DBus.Properties", "PropertiesChanged", NULL,
            "org.bluez.MediaPlayer1", G_DBUS_SIGNAL_FLAGS_NONE,
            bluez_signal_mediaplayer_properties_changed, NULL, NULL);
    mediacontrol_subscription_id = g_dbus_connection_signal_subscribe(
            conn, "org.bluez", "org.freedesktop.DBus.Properties", "PropertiesChanged", NULL,
            "org.bluez.MediaControl1", G_DBUS_SIGNAL_FLAGS_NONE,
            bluez_signal_mediacontrol_properties_changed, NULL, NULL);
    mediatransport_subscription_id = g_dbus_connection_signal_subscribe(
            conn, "org.bluez", "org.freedesktop.DBus.Properties", "PropertiesChanged", NULL,
            "org.bluez.MediaTransport1", G_DBUS_SIGNAL_FLAGS_NONE,
            bluez_signal_mediatransport_properties_changed, NULL, NULL);
}

void bluez_unsubscribe_signals(void)
{
    GDBusConnection *conn = bluez_mg.dbus;

    g_dbus_connection_signal_unsubscribe(conn, interface_subscription_add_id);
    g_dbus_connection_signal_unsubscribe(conn, interface_subscription_remove_id);
    g_dbus_connection_signal_unsubscribe(conn, adapter_subscription_id);
    g_dbus_connection_signal_unsubscribe(conn, device_subscription_id);
    g_dbus_connection_signal_unsubscribe(conn, mediaplayer_subscription_id);
    g_dbus_connection_signal_unsubscribe(conn, mediacontrol_subscription_id);
    g_dbus_connection_signal_unsubscribe(conn, mediatransport_subscription_id);
}
