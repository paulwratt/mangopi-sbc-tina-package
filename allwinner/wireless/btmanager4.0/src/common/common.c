
#include "common.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bt_bluez.h"
#include "bt_log.h"
#include "bt_adapter.h"
#include "bt_bluez_signals.h"
#include "bt_manager.h"

/**
 * Get a property of a given D-Bus interface.
 *
 * @param conn D-Bus connection handler.
 * @param name Valid D-Bus name or NULL.
 * @param path Valid D-Bus object path.
 * @param interface Interface with the given property.
 * @param property The property name.
 * @return On success this function returns variant containing property value.
 * Otherwise, NULL is returned. */
GVariant *g_dbus_get_property(GDBusConnection *conn, const char *name, const char *path,
                              const char *interface, const char *property)
{
    GDBusMessage *msg = NULL, *rep = NULL;
    GVariant *value = NULL;
    GError *err = NULL;

    msg = g_dbus_message_new_method_call(name, path, "org.freedesktop.DBus.Properties", "Get");

    g_dbus_message_set_body(msg, g_variant_new("(ss)", interface, property));

    if ((rep = g_dbus_connection_send_message_with_reply_sync(
                 conn, msg, G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err)) == NULL)
        goto fail;

    if (g_dbus_message_get_message_type(rep) == G_DBUS_MESSAGE_TYPE_ERROR) {
        g_dbus_message_to_gerror(rep, &err);
        BTMG_ERROR("error: %s", err->message);
        goto fail;
    }

    g_variant_get(g_dbus_message_get_body(rep), "(v)", &value);

fail:
    if (msg != NULL)
        g_object_unref(msg);
    if (rep != NULL)
        g_object_unref(rep);
    if (err != NULL) {
        g_error_free(err);
    }

    return value;
}

/**
 * Set a property of a given D-Bus interface.
 *
 * @param conn D-Bus connection handler.
 * @param name Valid D-Bus name or NULL.
 * @param path Valid D-Bus object path.
 * @param interface Interface with the given property.
 * @param property The property name.
 * @param value Variant containing property value.
 * @return On success this function returns TRUE. Otherwise, FALSE. */
gboolean g_dbus_set_property(GDBusConnection *conn, const char *name, const char *path,
                             const char *interface, const char *property, const GVariant *value)
{
    GDBusMessage *msg = NULL, *rep = NULL;
    GError *err = NULL;

    msg = g_dbus_message_new_method_call(name, path, "org.freedesktop.DBus.Properties", "Set");

    g_dbus_message_set_body(msg, g_variant_new("(ssv)", interface, property, value));

    if ((rep = g_dbus_connection_send_message_with_reply_sync(
                 conn, msg, G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err)) == NULL)
        goto fail;

    if (g_dbus_message_get_message_type(rep) == G_DBUS_MESSAGE_TYPE_ERROR) {
        g_dbus_message_to_gerror(rep, &err);
        goto fail;
    }

fail:
    if (msg != NULL)
        g_object_unref(msg);
    if (rep != NULL)
        g_object_unref(rep);
    if (err != NULL) {
        g_error_free(err);
        return FALSE;
    }

    return TRUE;
}

int g_dbus_call_sync_check_error(GError *err)
{
    int error = BT_ERROR;

    if (!err)
        return BT_OK;

    BTMG_ERROR("error code:%d, error:%s", err->code, err->message);
    if (!strcmp(err->message,
                "GDBus.Error:org.bluez.Error.InProgress: Operation already in progress"))
        error = BT_ERROR_BLUEZ_OPERATION_IN_PROGRESS;

    if (!strcmp(err->message,
                "GDBus.Error:org.bluez.Error.Failed: Resource temporarily unavailable"))
        error = BT_ERROR_BLUEZ_RESOURCE_UNAVAILABLE;

    if (!strcmp(err->message, "GDBus.Error:org.bluez.Error.Failed: Host is down"))
        error = BT_ERROR_BLUEZ_HOST_IS_DOWN;

    if (!strcmp(err->message,
                "GDBus.Error:org.freedesktop.DBus.Error.NoReply: Message recipient disconnected from message bus without replying"))
        error = BT_ERROR_BLUEZ_MES_RECIPIENT_DISCONNECTED_FROM_MES_BUS_WITHOUT_REPLY;

    g_error_free(err);

    return error;
}

int _get_managed_objects(GVariant **variant)
{
    GError *error = NULL;
    int ret = BT_OK;

    *variant =
            g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", "/",
                                        "org.freedesktop.DBus.ObjectManager", "GetManagedObjects",
                                        NULL, NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

    ret = g_dbus_call_sync_check_error(error);

    return ret;
}

void _get_object_path(const char *addr, char **path)
{
    GVariant *obj1, *ar1, *ar2;
    GVariantIter *iter1, *iter2;
    gchar *dev_path, *itf;
    btmg_bt_device_t device;
    *path = NULL;

    if (_get_managed_objects(&obj1) != 0)
        return;

    g_variant_get(obj1, "(a{oa{sa{sv}}})", &iter1);
    while (g_variant_iter_loop(iter1, "{&o@a{sa{sv}}}", &dev_path, &ar1)) {
        if (*path != NULL) {
            g_variant_unref(ar1);
            break;
        }
        g_variant_get(ar1, "a{sa{sv}}", &iter2);
        while (g_variant_iter_loop(iter2, "{&s@a{sv}}", &itf, &ar2)) {
            if (strcasecmp(itf, "org.bluez.Device1") != 0)
                continue;
            _get_device_properties(ar2, &device);
            if (strcasecmp(device.remote_address, addr) == 0) {
                *path = g_strdup(dev_path);
                g_variant_unref(ar2);
                break;
            }
        }
        g_variant_iter_free(iter2);
    }
    g_variant_iter_free(iter1);
    g_variant_unref(obj1);
}

void _get_adapter_properties(GVariant *p, struct bt_adapter *adapter)
{
    GVariant *v, *uuid;
    GVariantIter *iter;
    gchar *key = NULL;
    gint i = 0, uuid_len = 0;

    if (!adapter)
        return;

    memset(adapter, 0x00, sizeof(struct bt_adapter));

    g_variant_get(p, "(a{sv})", &iter);
    while (g_variant_iter_loop(iter, "{&sv}", &key, &v)) {
        if (g_strcmp0(key, "Address") == 0) {
            g_variant_get(v, "s", &adapter->address);
        } else if (g_strcmp0(key, "Name") == 0) {
            g_variant_get(v, "s", &adapter->name);
        } else if (g_strcmp0(key, "Alias") == 0) {
            g_variant_get(v, "s", &adapter->alias);
        } else if (g_strcmp0(key, "Discoverable") == 0) {
            g_variant_get(v, "b", &adapter->discoverable);
        } else if (g_strcmp0(key, "Discovering") == 0) {
            g_variant_get(v, "b", &adapter->discovering);
        } else if (g_strcmp0(key, "Pairable") == 0) {
            g_variant_get(v, "b", &adapter->pairable);
        } else if (g_strcmp0(key, "PairableTimeout") == 0) {
            g_variant_get(v, "u", &adapter->pair_timeout);
        } else if (g_strcmp0(key, "DiscoverableTimeout") == 0) {
            g_variant_get(v, "u", &adapter->discover_timeout);
        } else if (g_strcmp0(key, "UUIDs") == 0) {
            uuid_len = g_variant_n_children(v);
            adapter->uuid_length = uuid_len;
            if (uuid_len > 0) {
                adapter->uuid_list = (bt_dev_uuid_t *)malloc(sizeof(bt_dev_uuid_t) * uuid_len);
                for (i = 0; i < uuid_len; i++) {
                    uuid = g_variant_get_child_value(v, i);
                    g_variant_get(uuid, "s", &adapter->uuid_list[i].uuid);
                    g_variant_unref(uuid);
                }
            }
        }
    }
    g_variant_iter_free(iter);
}

void _get_device_properties(GVariant *prop_array, btmg_bt_device_t *device)
{
    GVariant *v;
    GVariantIter *iter;
    gchar *key = NULL;

    int16_t tmp_rssi = 0;

    if (device == NULL)
        return;

    memset(device, 0x00, sizeof(btmg_bt_device_t));

    g_variant_get(prop_array, "a{sv}", &iter);
    while (g_variant_iter_loop(iter, "{&sv}", &key, &v)) {
        if (g_strcmp0(key, "Address") == 0) {
            g_variant_get(v, "s", &device->remote_address);
        } else if (g_strcmp0(key, "Name") == 0) {
            g_variant_get(v, "s", &device->remote_name);
        } else if (g_strcmp0(key, "Class") == 0) {
            device->r_class = g_variant_get_uint32(v);
        } else if (g_strcmp0(key, "Paired") == 0) {
            device->paired = g_variant_get_boolean(v);
        } else if (g_strcmp0(key, "Trusted") == 0) {
            device->trusted = g_variant_get_boolean(v);
        } else if (g_strcmp0(key, "Blocked") == 0) {
            device->blocked = g_variant_get_boolean(v);
        } else if (g_strcmp0(key, "Connected") == 0) {
            device->connected = g_variant_get_boolean(v);
        } else if (g_strcmp0(key, "RSSI") == 0) {
            device->rssi = g_variant_get_int16(v);
        } else if (g_strcmp0(key, "Icon") == 0) {
            g_variant_get(v, "s", &device->icon);
        }
    }

    BTMG_DUMP("address:%s, name:%s, class:%d, paired:%d connected:%d, rssi:%d",
              device->remote_address, device->remote_name, device->r_class, device->paired,
              device->connected, device->rssi);

    g_variant_iter_free(iter);
}

int _get_all_device_properties(const gchar *path, GVariant **v)
{
    int ret = BT_OK;
    GError *error = NULL;

    *v = g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", path,
                                     "org.freedesktop.DBus.Properties", "GetAll",
                                     g_variant_new("(s)", "org.bluez.Device1"), NULL,
                                     G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        BTMG_ERROR("get all device properties fail");
        return ret;
    }

    return ret;
}

int _get_devices(btmg_device_state_type type, btmg_bt_device_t **dev_list, int *count)
{
    int ret = BT_OK;
    GVariant *objects = NULL, *prop_array = NULL;
    GVariant *path_array = NULL, *if_array = NULL, *path_item = NULL, *if_item = NULL;
    gsize path_count = 0, if_count = 0;
    gchar *path, *interface;
    gint cnt = 0;
    int i = 0, j = 0;
    btmg_bt_device_t *tmp_list = NULL;

    BTMG_DEBUG("enter");

    ret = _get_managed_objects(&objects);
    if (ret < 0)
        return ret;

    path_array = g_variant_get_child_value(objects, 0);
    path_count = g_variant_n_children(path_array);
    tmp_list = (btmg_bt_device_t *)malloc(sizeof(btmg_bt_device_t));
    if (tmp_list == NULL) {
        BTMG_ERROR("tmp_list malloc falied");
        return BT_ERROR_NO_MEMORY;
    }
    for (i = 0; i < path_count; i++) {
        path_item = g_variant_get_child_value(path_array, i);
        g_variant_get(path_item, "{&o@a{sa{sv}}}", &path, &if_array);
        if_count = g_variant_n_children(if_array);
        for (j = 0; j < if_count; j++) {
            if_item = g_variant_get_child_value(if_array, j);
            g_variant_get(if_item, "{&s@a{sv}}", &interface, &prop_array);
            if (strcasecmp(interface, "org.bluez.Device1") == 0) {
                switch (type) {
                case BTMG_DEVICE_TYPE_PAIRED:
                    if (_is_paired(path) == true) {
                        cnt++;
                        tmp_list = (btmg_bt_device_t *)realloc(tmp_list,
                                                               sizeof(btmg_bt_device_t) * cnt);
                        if (tmp_list == NULL) {
                            BTMG_ERROR("realloc tmp_list falied");
                            return BT_ERROR_NO_MEMORY;
                        }
                        _get_device_properties(prop_array, &tmp_list[cnt - 1]);
                    }
                    break;
                case BTMG_DEVICE_TYPE_CONNECTED:
                    if (_is_connected(path) == true) {
                        cnt++;
                        tmp_list = (btmg_bt_device_t *)realloc(tmp_list,
                                                               sizeof(btmg_bt_device_t) * cnt);
                        if (tmp_list == NULL) {
                            BTMG_ERROR("realloc tmp_list falied");
                            return BT_ERROR_NO_MEMORY;
                        }
                        _get_device_properties(prop_array, &tmp_list[cnt - 1]);
                    }
                    break;
                case BTMG_DEVICE_TYPE_ANY:
                    cnt++;
                    tmp_list =
                            (btmg_bt_device_t *)realloc(tmp_list, sizeof(btmg_bt_device_t) * cnt);
                    if (tmp_list == NULL) {
                        BTMG_ERROR("realloc tmp_list falied");
                        return BT_ERROR_NO_MEMORY;
                    }
                    _get_device_properties(prop_array, &tmp_list[cnt - 1]);
                    break;
                default:
                    break;
                }
            }
            g_variant_unref(if_item);
            g_variant_unref(prop_array);
        }
        g_variant_unref(path_item);
        g_variant_unref(if_array);
    }
    *count = cnt;
    *dev_list = tmp_list;

    g_variant_unref(path_array);
    g_variant_unref(objects);

    return ret;
}

int get_process_run_state(const char *process, int length)
{
    int bytes;
    char buf[10];
    char cmd[60];
    FILE *strea;

    if (length > 20) {
        BTMG_ERROR("process name is too long");
        return -1;
    }
    sprintf(cmd, "ps | grep %s | grep -v grep", process);
    strea = popen(cmd, "r");
    if (!strea)
        return -1;
    bytes = fread(buf, sizeof(char), sizeof(buf), strea);
    pclose(strea);
    if (bytes > 0) {
        BTMG_DEBUG("%s :process is running", process);
        return 0;
    } else {
        BTMG_DEBUG("%s :process is not running", process);
        return -1;
    }
}

int get_result_from_systemall(const char *cmd, char *result, int size)
{
    int fd[2];

    if (pipe(fd)) {
        BTMG_ERROR("pipe error!");
        return BT_ERROR;
    }

    int fd1 = dup(STDOUT_FILENO);
    int fd2 = dup2(fd[1], STDOUT_FILENO);
    system(cmd);
    read(fd[0], result, size - 1);
    result[strlen(result) - 1] = 0;
    dup2(fd1, fd2);

    return BT_OK;
}

int get_vol_from_bluealsa(const char *cmd, char *vol)
{
    char *p1;
    char *p2;
    char result[512] = { 0 };

    get_result_from_systemall(cmd, result, sizeof(result) / sizeof(result[0]));

    p1 = strchr(result, '[');
    p2 = strchr(result, '%');
    p1 += strlen("[");
    memcpy(vol, p1, p2 - p1);

    return BT_OK;
}

gboolean _is_paired(const char *device_path)
{
    GVariant *paired = NULL;
    gboolean is_paired = FALSE;

    paired = g_dbus_get_property(bluez_mg.dbus, "org.bluez", device_path, "org.bluez.Device1",
                                 "Paired");

    if (paired != NULL) {
        is_paired = g_variant_get_boolean(paired);
        g_variant_unref(paired);
    }

    return is_paired;
}

gboolean _is_connected(const char *device_path)
{
    GVariant *connected = NULL;
    gboolean is_connected = FALSE;

    connected = g_dbus_get_property(bluez_mg.dbus, "org.bluez", device_path, "org.bluez.Device1",
                                    "Connected");

    if (connected != NULL) {
        is_connected = g_variant_get_boolean(connected);
        g_variant_unref(connected);
    }

    return is_connected;
}

bool _is_a2dp_device_connected(void)
{
    bool connected = false;
    dev_node_t *dev_node = NULL;

    dev_node = connected_devices->head;
    while (dev_node != NULL) {
        if (dev_node->profile & BTMG_REMOTE_DEVICE_A2DP) {
            connected = true;
            break;
        }
        dev_node = dev_node->next;
    }

    return connected;
}

void _print_variant(GVariant *v)
{
    gchar *pretty;
    pretty = g_variant_print(v, TRUE);
    BTMG_DUMP("GVariant type: %s", g_variant_get_type_string(v));
    BTMG_DUMP("GVariant value: %s", pretty);
    g_free(pretty);
}

const char *btmg_path_to_addr(const char *path)
{
    int i = 0;
    char *ptr = NULL;
    static char addr[18];

    ptr = strstr(path, "dev_");
    if (ptr == NULL) {
        BTMG_ERROR("path is null");
        return NULL;
    } else {
        for (i = 0; i < 6; i++) {
            strncpy(addr + i * 3, ptr + 4 + i * 3, 2);
            *(addr + 2 + i * 3) = ':';
        }
        *(addr + 17) = '\0';
    }

    return addr;
}

char *btmg_addr_to_path(const char *addr)
{
    int i = -1;
    char *dup_str;
    static char path[38];

    memset(path, 0, sizeof(path));
    memcpy(path, "/org/bluez/hci0/dev_", 20);

    for (i = 0; i < 6; i++) {
        dup_str = strndup(addr + i * 3, 2);
        strcat(path, dup_str);
        if (i != 5)
            strcat(path, "_");
        free(dup_str);
    }

    return path;
}

void ms_sleep(unsigned long ms)
{
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    int err;
    do {
        err = select(0, NULL, NULL, NULL, &tv);
    } while (err < 0 && errno == EINTR);
}

char *string_left_cut(char *dst, char *src, int n)
{
    char *p = src;
    char *q = dst;
    int len = strlen(src);

    if (n > len)
        n = len;

    while (n--)
        *(q++) = *(p++);

    *(q++) = '\0';

    return dst;
}

bool btmg_string_is_bdaddr(const char *string)
{
    size_t len = strlen(string);

    if (string == NULL) {
        BTMG_DEBUG("string parameter is NULL");
        return false;
    }

    if (len != 17)
        return false;

    for (size_t i = 0; i < len; ++i) {
        // Every 3rd char must be ':'.
        if (((i + 1) % 3) == 0 && string[i] != ':')
            return false;
        // All other chars must be a hex digit.
        if (((i + 1) % 3) != 0 && !isxdigit(string[i]))
            return false;
    }

    return true;
}
