#ifndef __AW_MESH_CONFIG_MODEL_H__
#define __AW_MESH_CONFIG_MODEL_H__
#include "AWDefine.h"
#include "AWmeshDefine.h"

//Configuration MMDL OPCODES
#define AW_MESH_ACCESS_MSG_CONFIG_BEACON_GET 0x8009
#define AW_MESH_ACCESS_MSG_CONFIG_BEACON_SET 0x800A
#define AW_MESH_ACCESS_MSG_CONFIG_BEACON_STATUS  0x800B
#define AW_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_GET   0x8008
#define AW_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_STATUS    0x02
#define AW_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_GET    0x800C
#define AW_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_SET    0x800D
#define AW_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_STATUS 0x800E
#define AW_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_GET 0x8012
#define AW_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_SET 0x8013
#define AW_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_STATUS  0x8014
#define AW_MESH_ACCESS_MSG_CONFIG_FRIEND_GET 0x800F
#define AW_MESH_ACCESS_MSG_CONFIG_FRIEND_SET 0x8010
#define AW_MESH_ACCESS_MSG_CONFIG_FRIEND_STATUS  0x8011
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_GET  0x8018
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_SET  0x03
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_STATUS   0x8019
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET  0x801A
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_ADD 0x801B
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE  0x801C
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL  0x801D
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE   0x801E
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_STATUS  0x801F
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD 0x8020
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE  0x8021
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_OVERWRITE   0x8022
#define AW_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_GET   0x8023
#define AW_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_SET   0x8024
#define AW_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_STATUS    0x8025
#define AW_MESH_ACCESS_MSG_CONFIG_RELAY_GET  0x8026
#define AW_MESH_ACCESS_MSG_CONFIG_RELAY_SET  0x8027
#define AW_MESH_ACCESS_MSG_CONFIG_RELAY_STATUS   0x8028
#define AW_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_SUBSCRIPTION_GET 0x8029
#define AW_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_SUBSCRIPTION_LIST    0x802A
#define AW_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_SUBSCRIPTION_GET  0x802B
#define AW_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_SUBSCRIPTION_LIST 0x802C
#define AW_MESH_ACCESS_MSG_CONFIG_LOW_POWER_NODE_POLL_TIMEOUT_GET    0x802D
#define AW_MESH_ACCESS_MSG_CONFIG_LOW_POWER_NODE_POLL_TIMEOUT_STATUS 0x802E
#define AW_MESH_ACCESS_MSG_CONFIG_NETKEY_ADD 0x8040
#define AW_MESH_ACCESS_MSG_CONFIG_NETKEY_DELETE  0x8041
#define AW_MESH_ACCESS_MSG_CONFIG_NETKEY_GET 0x8042
#define AW_MESH_ACCESS_MSG_CONFIG_NETKEY_LIST    0x8043
#define AW_MESH_ACCESS_MSG_CONFIG_NETKEY_STATUS  0x8044
#define AW_MESH_ACCESS_MSG_CONFIG_NETKEY_UPDATE  0x8045
#define AW_MESH_ACCESS_MSG_CONFIG_APPKEY_ADD 0x00
#define AW_MESH_ACCESS_MSG_CONFIG_APPKEY_UPDATE  0x01
#define AW_MESH_ACCESS_MSG_CONFIG_APPKEY_DELETE  0x8000
#define AW_MESH_ACCESS_MSG_CONFIG_APPKEY_GET 0x8001
#define AW_MESH_ACCESS_MSG_CONFIG_APPKEY_LIST    0x8002
#define AW_MESH_ACCESS_MSG_CONFIG_APPKEY_STATUS  0x8003
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_APP_BIND 0x803D
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_APP_STATUS   0x803E
#define AW_MESH_ACCESS_MSG_CONFIG_MODEL_APP_UNBIND   0x803F
#define AW_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_APP_GET  0x804B
#define AW_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_APP_LIST 0x804C
#define AW_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_APP_GET   0x804D
#define AW_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_APP_LIST  0x804E
#define AW_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_GET  0x8046
#define AW_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_SET  0x8047
#define AW_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_STATUS   0x8048
#define AW_MESH_ACCESS_MSG_CONFIG_NODE_RESET 0x8049
#define AW_MESH_ACCESS_MSG_CONFIG_NODE_RESET_STATUS  0x804A
#define AW_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_GET  0x8015
#define AW_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_SET  0x8016
#define AW_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_STATUS   0x8017
#define AW_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_GET  0x8038
#define AW_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_SET  0x8039
#define AW_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_STATUS   0x06
#define AW_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_GET 0x803A
#define AW_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_SET 0x803B
#define AW_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS  0x803C

#define AW_MESH_ACCESS_MSG_HEALTH_CURRENT_STATUS 0x04
#define AW_MESH_ACCESS_MSG_HEALTH_FAULT_STATUS   0x05
#define AW_MESH_ACCESS_MSG_HEALTH_FAULT_CLEAR    0x802F
#define AW_MESH_ACCESS_MSG_HEALTH_FAULT_CLEAR_UNACKNOWLEDGED 0x8030
#define AW_MESH_ACCESS_MSG_HEALTH_FAULT_GET  0x8031
#define AW_MESH_ACCESS_MSG_HEALTH_FAULT_TEST 0x8032
#define AW_MESH_ACCESS_MSG_HEALTH_FAULT_TEST_UNACKNOWLEDGED  0x8033
#define AW_MESH_ACCESS_MSG_HEALTH_PERIOD_GET 0x8034
#define AW_MESH_ACCESS_MSG_HEALTH_PERIOD_SET 0x8035
#define AW_MESH_ACCESS_MSG_HEALTH_PERIOD_SET_UNACKNOWLEDGED  0x8036
#define AW_MESH_ACCESS_MSG_HEALTH_PERIOD_STATUS  0x8037
#define AW_MESH_ACCESS_MSG_HEALTH_ATTENTION_GET  0x8004
#define AW_MESH_ACCESS_MSG_HEALTH_ATTENTION_SET  0x8005
#define AW_MESH_ACCESS_MSG_HEALTH_ATTENTION_SET_UNACKNOWLEDGED   0x8006
#define AW_MESH_ACCESS_MSG_HEALTH_ATTENTION_STATUS   0x8007

//Config Message Status Definitions

/**
 * @brief Mesh Config Beacon Status event paramater type.
 */
typedef struct
{
    UINT8 beacon;
} AW_MESH_CONF_BEACON_STATUS_T;

/**
 * @brief Mesh Config Composition Data Status event paramater type.
 */
typedef struct
{
    UINT8 page;
    UINT16 company_id;
    UINT16 product_id;
    UINT16 version_id;
    UINT16 crpl;
    UINT16 features;
    UINT16 data_len;
    VOID *data;
} AW_MESH_CONF_COMPO_DATA_STATUS_T;


/**
 * @brief Mesh Config Default TTL Status event paramater type.
 */
typedef struct
{
    UINT8 ttl;
} AW_MESH_CONF_DEF_TTL_STATUS_T;

/**
 * @brief Mesh Config GATT Proxy Status event paramater type.
 */
typedef struct
{
  UINT8 gatt_proxy;
} AW_MESH_CONF_GATT_PROXY_STATUS_T;

/**
 * @brief Mesh Config Relay Status event paramater type.
 */
typedef struct
{
    UINT8 relay;
    UINT8 relay_retrans_cnt;
    UINT8 relay_retrans_intvlsteps;
} AW_MESH_CONF_RELAY_STATUS_T;

/**
 * @brief Mesh Config Model Publication Status event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 elem_addr;
    UINT16 pub_addr;
    UINT16 appkey_index;
    UINT8 cred_flag;
    UINT8 pub_ttl;
    UINT8 pub_perid;
    UINT8 pub_retrans_cnt;
    UINT8 pub_retrans_intvl_steps;
    AW_MESH_MODEL_ID_T model_id;
} AW_MESH_CONF_MODEL_PUB_STATUS_T;

/**
 * @brief Mesh Config Model Sublication Status event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 elem_addr;
    UINT16 address;
    AW_MESH_MODEL_ID_T model_id;
}AW_MESH_CONF_MODEL_SUB_STATUS_T;

/**
 * @brief Mesh Config Sig Model Sublication List event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 elem_addr;
    UINT16 sig_model_id;
    UINT16 num;
    UINT16 *addresses;
} AW_MESH_CONF_SIG_MODEL_SUB_LIST_T;

/**
 * @brief Mesh Config Vendor Model Sublication List event paramater type.
 */
typedef struct
{
  UINT8 status;
  UINT16 elem_addr;
  AW_MESH_MODEL_ID_T vnd_model_id;
  UINT16 num;
  UINT16 *addresses;
} AW_MESH_CONF_VND_MODEL_SUB_LIST_T;

/**
 * @brief Mesh Config NetKey Status event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 netkey_index;
} AW_MESH_CONF_NETKEY_STATUS_T;

/**
 * @brief Mesh Config NetKey List event paramater type.
 */
typedef struct
{
    UINT16 num_of_netkey;
    UINT16 *pnetkeyindexes;
} AW_MESH_CONF_NETKEY_LIST_T;

/**
 * @brief Mesh Config AppKey List event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 netkey_index;
    UINT16 num_of_appkey;
    UINT16 *pappkeyindexes;
} AW_MESH_CONF_APPKEY_LIST_T;

/**
 * @brief Mesh Config AppKey Status event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 netkey_index;
    UINT16 appkey_index;
} AW_MESH_CONF_APPKEY_STATUS_T;

/**
 * @brief Mesh Config Node Identity Status event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 netkey_index;
    UINT8 identity;
} AW_MESH_CONF_NODE_IDENT_STATUS_T;

/**
 * @brief Mesh Config Model App Status event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 elem_addr;
    UINT16 appkey_index;
    AW_MESH_MODEL_ID_T model_id;
} AW_MESH_CONF_MODEL_APP_STATUS_T;

/**
 * @brief Mesh Config SIG Model App List event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 elem_addr;
    UINT16 model_id;
    UINT16 num_of_appkey;
    UINT16 *pappkeyindexes;
} AW_MESH_CONF_SIG_MODEL_APP_LIST_T;

/**
 * @brief Mesh Config Vendor Model App List event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 elem_addr;
    AW_MESH_MODEL_ID_T vnd_model_id;
    UINT16 num_of_appkey;
    UINT16 *pappkeyindexes;
} AW_MESH_CONF_VND_MODEL_APP_LIST_T;

/**
 * @brief Mesh Config Friend Status event paramater type.
 */
typedef struct
{
    UINT8 friend;
} AW_MESH_CONF_FRND_STATUS_T;

/**
 * @brief Mesh Config Key Refresh Phase  Status event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 netkey_index;
    UINT8 phase;
} AW_MESH_CONF_KEYREFRESH_PHASE_STATUS_T;

/**
 * @brief Mesh Config    Heartbeat Publication Status event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 dest_addr;
    UINT8 count_log;
    UINT8 period_log;
    UINT8 ttl;
    UINT16 features;
    UINT16 netkey_index;
} AW_MESH_CONF_HB_PUB_STATUS_T;

/**
 * @brief Mesh Config    Heartbeat Subscription Status event paramater type.
 */
typedef struct
{
    UINT8 status;
    UINT16 src_addr;
    UINT16 dst_addr;
    UINT8 period_log;
    UINT8 count_log;
    UINT8 min_hops;
    UINT8 max_hops;
} AW_MESH_CONF_HB_SUB_STATUS_T;

/**
 * @brief Mesh Config Low Power Node PollTimeout Status event paramater type.
 */
typedef struct
{
    UINT16 lpn_addr;
    uint32_t polltimeout;
} AW_MESH_CONF_LPN_POLLTIMEOUT_STATUS_T;

/**
 * @brief Mesh Config Network Transmit Status event paramater type.
 */
typedef struct
{
    UINT8 nwk_transcnt;
    UINT8 nwk_trans_intvl_steps;
} AW_MESH_CONF_NWK_TRANS_STATUS_T;

/**
 * @brief Mesh Health Current/Fault Status event paramater type.
 */
typedef struct
{
  UINT8 test_id;
  UINT16 company_id;
  UINT8 fault_num;
  UINT8 *fault_array;
} AW_MESH_HLTH_FAULT_STATUS_T;

/**
 * @brief Mesh Health Period Status event paramater type.
 */
typedef struct
{
  UINT8 fast_peirod_div;
} AW_MESH_HLTH_PERIOD_STATUS_T;

/**
 * @brief Mesh Health Attention Status event paramater type.
 */
typedef struct
{
  UINT8 attention;
} AW_MESH_HLTH_ATTEN_STATUS_T;

/**
 * @brief Mesh Model Client event paramater type.
 * you should implement all status corresponsing to mible_mesh_node_*.
 */

typedef struct
{
    AW_MESH_ACCESS_OPCODE_T opcode;
    AW_MESH_ACCESS_RX_META_T meta_data;
    union{
        AW_MESH_CONF_BEACON_STATUS_T            beacon_status;
        AW_MESH_CONF_COMPO_DATA_STATUS_T        compo_data_status;
        AW_MESH_CONF_DEF_TTL_STATUS_T           def_ttl_status;
        AW_MESH_CONF_GATT_PROXY_STATUS_T        gatt_proxy_status;
        AW_MESH_CONF_RELAY_STATUS_T             relay_status;
        AW_MESH_CONF_MODEL_PUB_STATUS_T         model_pub_status;
        AW_MESH_CONF_MODEL_SUB_STATUS_T         model_sub_status;
        AW_MESH_CONF_SIG_MODEL_SUB_LIST_T       sig_model_sub_list;
        AW_MESH_CONF_VND_MODEL_SUB_LIST_T       vnd_model_sub_list;
        AW_MESH_CONF_NETKEY_STATUS_T            netkey_status;
        AW_MESH_CONF_NETKEY_LIST_T              netkey_list;
        AW_MESH_CONF_APPKEY_LIST_T              appkey_list;
        AW_MESH_CONF_APPKEY_STATUS_T            appkey_status;
        AW_MESH_CONF_NODE_IDENT_STATUS_T        node_ident_status;
        AW_MESH_CONF_MODEL_APP_STATUS_T         model_app_status;
        AW_MESH_CONF_SIG_MODEL_APP_LIST_T       sig_model_app_list;
        AW_MESH_CONF_VND_MODEL_APP_LIST_T       vnd_model_app_list;
        AW_MESH_CONF_FRND_STATUS_T              frnd_status;
        AW_MESH_CONF_KEYREFRESH_PHASE_STATUS_T  keyrefresh_phase_status;
        AW_MESH_CONF_HB_PUB_STATUS_T            hb_pub_status;
        AW_MESH_CONF_HB_SUB_STATUS_T            hb_sub_status;
        AW_MESH_CONF_LPN_POLLTIMEOUT_STATUS_T   lpn_polltimeout_status;
        AW_MESH_CONF_NWK_TRANS_STATUS_T         nwk_trans_status;
    };
}AW_MESH_CONFIG_MODEL_STATUS_T;

typedef struct
{
    AW_MESH_ACCESS_OPCODE_T opcode;
    AW_MESH_ACCESS_RX_META_T meta_data;
    union{
        AW_MESH_HLTH_FAULT_STATUS_T             hlth_fault_status;
        AW_MESH_HLTH_PERIOD_STATUS_T            hlth_period_status;
        AW_MESH_HLTH_ATTEN_STATUS_T             hlth_atten_status;
    };
}AW_MESH_HLTH_MODEL_STATUS_T;

#endif
