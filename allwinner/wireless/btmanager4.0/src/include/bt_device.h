#ifndef __BTMG_DEVICE_H
#define __BTMG_DEVCIE_H

#include "bt_manager.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

btmg_device_state bt_device_get_state(void);
void bt_device_set_state(btmg_device_state state);
int bt_device_connect(const char *addr);
int bt_device_disconnect(const char *addr);
int bt_device_pair(const char *addr);
int bt_device_get_name(const char *addr, char *name);
void bt_device_get_address(const char *path, char **address);
bool bt_device_is_paired(const char *addr);
bool bt_device_is_connected(const char *addr);
int bt_remove_device(const char *addr, bool is_paired);
int bt_remove_unpaired_devices(void);
int bt_malloc_device(btmg_bt_device_t *device);
int bt_free_device(btmg_bt_device_t *device);
int bt_free_devices(btmg_bt_device_t *device_list, int count);
int bt_get_paired_devices(btmg_bt_device_t **dev_list, int *count);
int bt_free_paired_devices(btmg_bt_device_t *dev_list, int count);
#ifdef __cplusplus
}; /*extern "C"*/
#endif

#endif
