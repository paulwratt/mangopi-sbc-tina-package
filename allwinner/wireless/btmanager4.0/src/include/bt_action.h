#ifndef __BT_ACTION_H
#define __BT_ACTION_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#if __cplusplus
extern "C" {
#endif

#include <api_action.h>

typedef enum {
    BT_CHECK_STATE,
    BT_GET_LOGLEVEL,
    BT_SET_EX_DEBUG,
    BT_GET_EX_DEBUG,
    BT_ENABLE_PROFILE,
    BT_BTMG_ENABLE,
    BT_SET_DEFAULT_PROFILE,
    BT_SET_SCAN_MODE,
    BT_SET_SCAN_FILTER,
    BT_START_SCAN,
    BT_STOP_SCAN,
    BT_IS_SCAN,
    BT_PAIR,
    BT_UNPAIR,
    BT_GET_PAIRED_DEV,
    BT_FREE_PAIRED_DEV,
    BT_GET_ADATER_STATE,
    BT_GET_ADATER_NAME,
    BT_SET_ADATER_NAME,
    BT_GET_ADATER_ADDRESS,
    BT_GET_DEVICE_NAME,
    BT_DEV_CONNECT,
    BT_DEV_DISCONNECT,
    BT_IS_CONNECTED,
    BT_SET_PAGE_TO,
    BT_REMOVE_DEV,
    BT_A2DP_SRC_INIT,
    BT_A2DP_SRC_DEINIT,
    BT_A2DP_SRC_STREAM_START,
    BT_A2DP_SRC_STREAM_STOP,
    BT_A2DP_SRC_STREAM_SEND,
    BT_A2DP_SRC_IS_STREAM_START,
    BT_A2DP_SET_VOL,
    BT_A2DP_GET_VOL,
    BT_AVRCP_COMMAND,
    BT_SET_LINK_SUP_TO,
    BT_HFP_AT_ATA,
    BT_HFP_AT_CHUP,
    BT_HFP_AT_ATD,
    BT_HFP_AT_BLDN,
    BT_HFP_AT_BTRH,
    BT_HFP_AT_VTS,
    BT_HFP_AT_BCC,
    BT_HFP_AT_CNUM,
    BT_HFP_AT_VGS,
    BT_HFP_AT_VGM,
    BT_HFP_AT_CMD,
    BT_SPP_CLI_CONNECT,
    BT_SPP_CLI_SEND,
    BT_SPP_CLI_DISCONNECT,
    BT_SPP_SVC_ACCEPT,
    BT_SPP_SVC_SEND,
    BT_SPP_SVC_DISCONNECT,
} bt_call_t;

extern act_func_t bt_table[];

#if __cplusplus
}; // extern "C"
#endif

#endif
