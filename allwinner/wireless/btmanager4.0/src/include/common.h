#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <time.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <gio/gio.h>

#include "bt_manager.h"
#include "bt_adapter.h"
#include "bt_action.h"
#include "bt_dev_list.h"

typedef struct {
    GDBusConnection *dbus;
} bluez_mg_t;

extern bluez_mg_t bluez_mg;

typedef enum {
    ACT_ID_GATTC = 0,
    ACT_ID_GATTS,
    ACT_ID_LE_GAP,
    ACT_ID_BT,
} act_idx_t;

typedef enum {
    BTMG_DEVICE_STATE_IDLE,
    BTMG_DEVICE_STATE_PAIRING,
    BTMG_DEVICE_STATE_PAIRED,
    BTMG_DEVICE_STATE_CONNECTING,
    BTMG_DEVICE_STATE_CONNECTED,
    BTMG_DEVICE_STATE_DISCONNECTING,
    BTMG_DEVICE_STATE_DISCONNECTED
} btmg_device_state;

typedef enum {
    BTMG_ERROR_HOST_IS_DOWN,
    BTMG_ERROR_BT_IS_CONNECTED,
} btmg_error_code_t;

typedef enum {
    BTMG_DEVICE_TYPE_ANY,
    BTMG_DEVICE_TYPE_PAIRED,
    BTMG_DEVICE_TYPE_CONNECTED
} btmg_device_state_type;

typedef enum {
    BTMG_REMOTE_DEVICE_A2DP = 1 << 1,
    BTMG_REMOTE_DEVICE_SPP = 1 << 2,
} remote_device_profile;

GVariant *g_dbus_get_property(GDBusConnection *conn, const char *name, const char *path,
                              const char *interface, const char *property);
gboolean g_dbus_set_property(GDBusConnection *conn, const char *name, const char *path,
                             const char *interface, const char *property, const GVariant *value);
int g_dbus_call_sync_check_error(GError *err);
int _get_managed_objects(GVariant **variant);
void _get_object_path(const char *addr, char **path);
void _get_adapter_properties(GVariant *p, struct bt_adapter *adapter);
void _get_device_properties(GVariant *prop_array, btmg_bt_device_t *device);
int _get_all_device_properties(const gchar *path, GVariant **v);
int _get_devices(btmg_device_state_type type, btmg_bt_device_t **dev_list, int *count);
int get_process_run_state(const char *process, int length);
int get_result_from_systemall(const char *cmd, char *result, int size);
int get_vol_from_bluealsa(const char *cmd, char *vol);
gboolean _is_paired(const char *device_path);
gboolean _is_connected(const char *device_path);
bool _is_a2dp_device_connected(void);
void _print_variant(GVariant *v);
const char *btmg_path_to_addr(const char *path);
char *btmg_addr_to_path(const char *addr);
void ms_sleep(unsigned long ms);
char *string_left_cut(char *dst, char *src, int n);
bool btmg_string_is_bdaddr(const char *string);

extern btmg_callback_t *btmg_cb_p;
extern btmg_profile_info_t *bt_pro_info;
extern dev_list_t *connected_devices;

#define RETURN_INT(ret)                                                                            \
    do {                                                                                           \
        int val = ret;                                                                             \
        memcpy(cb_args[0], (void *)(&val), sizeof(int));                                           \
        return val;                                                                                \
    } while (0)

#ifdef __cplusplus
};
#endif

#endif
