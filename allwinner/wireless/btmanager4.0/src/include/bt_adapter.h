#ifndef __BTMG_ADAPTER_H
#define __BTMG_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif
#include "bt_manager.h"

struct bt_adapter {
    char *address; /* The bluetooth device address. */
    bool discoverable; /* True if the device is disocverable; false otherwise. */
    char *name; /* The bluetooth system name. */
    bool pairable; /* True if the device is pairable; false otherwise */
    char *alias; /* The bluetooth friendly name. */
    bool discovering; /* Indicates that a device discovery procedure is active */
    unsigned int pair_timeout; /* The pairable timeout in seconds */
    unsigned int discover_timeout; /* The discoverable timeout in seconds */
    bt_dev_uuid_t *uuid_list; /* List of UUIDs of the available local services */
    int uuid_length; /* The size of  uuid_list */
};

btmg_adapter_state_t bt_adapter_get_power_state(void);
int bt_adapter_set_power(bool power);
int bt_adapter_set_alias(const char *alias);
int bt_adapter_set_discoverable(bool discoverable);
int bt_adapter_set_pairable(bool pairable);
int bt_adapter_set_pairableTimeout(unsigned int timeout);
int bt_adapter_get_info(struct bt_adapter *adapter);
int bt_adapter_get_name(char *name);
int bt_adapter_get_address(char *address);
int bt_adapter_scan_filter(btmg_scan_filter_t *filter);
int bt_adapter_start_scan(void);
int bt_adapter_stop_scan(void);
int bt_adapter_free(struct bt_adapter *adapter);
bool bt_adapter_is_scanning(void);
int bt_adapter_set_scan_mode(char *opt);
int bt_adapter_set_page_timeout(int timeout);

#ifdef __cplusplus
}; /*extern "C"*/
#endif

#endif
