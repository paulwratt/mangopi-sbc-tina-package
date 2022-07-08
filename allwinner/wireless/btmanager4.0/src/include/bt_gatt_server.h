#ifndef __BT_GATT_SERVER_H__
#define __BT_GATT_SERVER_H__

#include <api_action.h>
#define AG_MAX_BDADDR_LEN 18

typedef enum {
    GATTS_ADD_SVC,
    GATTS_ADD_CHAR,
    GATTS_ADD_DESC,
    GATTS_START_SVC,
    GATTS_STOP_SVC,
    GATTS_REMOVE_SVC,
    GATTS_GET_MTU,
    GATTS_SEND_WRITE_RSP,
    GATTS_SEND_RSP,
    GATTS_SEND_NOTIFY,
    GATTS_SEND_IND,
    GATTS_DISCONNECT,
    GATTS_INIT,
    GATTS_DEINIT,
} gatts_call_t;

extern act_func_t gatts_table[];

#endif
