#include <stdarg.h>
#include <syslog.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdbool.h>

#include "bt_log.h"
#include "bt_manager.h"

int btmg_ex_debug_mask = 0;

int btmg_debug_level = 2;
int btmg_debug_show_keys;
int btmg_debug_timestamp;

const bt_error_info_string bt_error_info[] = {
    { BT_OK, "OK" },
    { BT_ERROR, "ERROR" },
    { BT_ERROR_ADATER_NOT_ON, "Bt adater hasn't turn on yet" },
    { BT_ERROR_NO_MEMORY, "Memory allocation failed" },
    { BT_ERROR_INVALID_ARGS, "Invalid parameters" },
    { BT_ERROR_IN_PROCESS, "Currently processing" },
    { BT_ERROR_TIME_OUT, "Timeout" },
    { BT_ERROR_NULL_VARIABLE, "Variable is NULL" },
    { BT_ERROR_DEVICE_NOT_FOUND, "Device was not found in the cache" },
    { BT_ERROR_A2DP_DEVICE_NOT_CONNECTED, "No a2dp device connected" },
    { BT_ERROR_A2DP_PCM_OPEN_FAIL, "PCM open fail" },
    { BT_ERROR_A2DP_SRC_STREAM_NOT_START, "A2dp src stream has not started yet" },
    { BT_ERROR_A2DP_SRC_NOT_INIT, "A2dp src stream has not init yet" },
    { BT_ERROR_A2DP_SINK_NOT_INIT, "A2dp sink stream has not init yet" },
    { BT_ERROR_BLUEZ_HOST_IS_DOWN, "Remote BT device may not be turned on" },
    { BT_ERROR_BLUEZ_OPERATION_IN_PROGRESS, "BLUEZ stack is already in progress" },
    { BT_ERROR_BLUEZ_RESOURCE_UNAVAILABLE, "BLUEZ resources temporarily unavailable" }
};

int btmg_set_debug_level(int level)
{
    btmg_debug_level = level;
    return log_set_debug_level((log_level_t)level);
}

int btmg_get_debug_level()
{
    return log_get_debug_level();
}

int btmg_set_ex_debug_mask(int mask)
{
    if (mask >= EX_DBG_MASK_MAX) {
        BTMG_ERROR("mask is Invalid.");
        return BT_ERROR_INVALID_ARGS;
    }
    btmg_ex_debug_mask = mask;

    return BT_OK;
}

int btmg_get_ex_debug_mask(void)
{
    return btmg_ex_debug_mask;
}

const char *btmg_get_error_info(int error)
{
    int i, size;
    const bt_error_info_string *error_info_list;
    error_info_list = bt_error_info;
    size = sizeof(bt_error_info) / sizeof(bt_error_info_string);

    for (i = 0; i < size; i++) {
        if (error_info_list[i].error == error) {
            return (const char *)error_info_list[i].info;
        }
    }

    return "null";
}

int btmg_parse_ex_debug_mask(char *key, char *op, char *val)
{
    int mask = btmg_get_ex_debug_mask();
    int mask_old = mask;
    int mask_value_new = strtoul(val, NULL, 0);
    if (mask_value_new && !strcmp(key,"mask")) {
        if (!strcmp(op,"+=")) {
            mask |= mask_value_new;
        } else if (!strcmp(op,"-=")) {
            mask &= ~mask_value_new;
        } else if (!strcmp(op,"=")) {
            mask = mask_value_new;
        }
        btmg_set_ex_debug_mask(mask);
        BTMG_DEBUG("ex_debug_mask old = 0x%04x", mask_old);
        BTMG_DEBUG("ex_debug_mask new = 0x%04x", mask);
        return 0;
    }

    if (!strcmp(op,"+=")) {
        if (!strcmp(key,"mask")) {
            if (!strcmp(val,"all")) {
                mask |= EX_DBG_MASK_MAX-1;
            }
        }
    } else if (!strcmp(op,"-=")) {
        if (!strcmp(key,"mask")) {
            if (!strcmp(val,"all")) {
                mask &= 0;
            }
        }
    } else if (!strcmp(op,"=")) {
        if (!strcmp(key,"mask")) {
            if (!strcmp(val,"all")) {
                mask |= EX_DBG_MASK_MAX-1;
            }
        }
    }

    btmg_set_ex_debug_mask(mask);
    BTMG_DEBUG("ex_debug_mask old = 0x%04x", mask_old);
    BTMG_DEBUG("ex_debug_mask new = 0x%04x", mask);
    return 0;
}