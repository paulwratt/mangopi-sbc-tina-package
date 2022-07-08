#ifndef __AW_MESH_CFG_MDL_H__
#define __AW_MESH_CFG_MDL_H__

#include "AWmeshDefine.h"
#include "AWmeshfoundationModel.h"

//structures for configure models
typedef struct {
    UINT16      src_addr;
    UINT16      dst_addr;
    UINT8       ttl;
    UINT16      msg_netkey_index;
} AW_MESH_CONFIG_META_T;

typedef struct {
    AW_MESH_CONFIG_META_T meta;
} AW_MESH_CONFIG_BEACON_GET_T,  AW_MESH_CONFIG_DEFAULT_TTL_GET_T,
AW_MESH_CONFIG_GATT_PROXY_GET_T, AW_MESH_CONFIG_FRIEND_GET_T,
AW_MESH_CONFIG_RELAY_GET_T, AW_MESH_CONFIG_NETWORK_TRANSMIT_GET_T,
AW_MESH_CONFIG_NETKEY_GET_T, AW_MESH_CONFIG_NODE_RESET_T,
AW_MESH_CONFIG_HB_PUB_GET_T, AW_MESH_CONFIG_HB_SUB_GET_T,
AW_MESH_CONFIG_FRIEND_GET_T;

typedef struct {
    AW_MESH_CONFIG_META_T meta;
    UINT16      address;
} AW_MESH_CONFIG_POLL_TIMEOUT_T;

typedef struct {
    AW_MESH_CONFIG_META_T meta;
    UINT8       beacon;
} AW_MESH_CONFIG_BEACON_SET_T;

typedef struct {
    AW_MESH_CONFIG_META_T meta;
    UINT8       page;
} AW_MESH_CONFIG_COMPOSITION_DATA_GET_T;

typedef struct {
    AW_MESH_CONFIG_META_T meta;
    UINT8       TTL;
} AW_MESH_CONFIG_DEFAULT_TTL_SET_T;

typedef struct {
    AW_MESH_CONFIG_META_T meta;
    UINT8       gatt_proxy;
} AW_MESH_CONFIG_GATT_PROXY_SET_T;

typedef struct {
    AW_MESH_CONFIG_META_T meta;
    UINT8       mesh_friend;
} AW_MESH_CONFIG_FRIEND_SET_T;

typedef struct {
    AW_MESH_CONFIG_META_T meta;
    UINT8       relay;
    UINT8       retransmit_count;
    UINT8       retransmit_interval_steps;
} AW_MESH_CONFIG_RELAY_SET_T;

typedef struct {
    AW_MESH_CONFIG_META_T meta;
    UINT16      element_addr;
    UINT32      model_id;
} AW_MESH_CONFIG_MODEL_PUB_GET_T;

typedef struct {
    UINT16 element_address;               /**< Address of the element. */
    AW_MESH_ADDRESS_T publish_address;     /**< Value of the publish address. */
    UINT16 appkey_index;                  /**< Index of the application key. */
    BOOL friendship_credential_flag;         /**< Value of the Friendship Credential Flag. */
    UINT8 publish_ttl;                    /**< Default TTL value for the outgoing messages. */
    UINT8 publish_period;                 /**< Period for periodic status publishing. */
    UINT8 retransmit_count;               /**< Number of retransmissions for each published message. */
    UINT8 retransmit_interval_steps;      /**< Number of 50-millisecond steps between retransmissions. */
    UINT32 model_id;                      /**< SIG Model ID or Vendor Model ID. */
} AW_MESH_MODEL_PUBLICATION_STATE_T;

typedef struct {
    AW_MESH_CONFIG_META_T meta;
    AW_MESH_MODEL_PUBLICATION_STATE_T *state;
} AW_MESH_CONFIG_MODEL_PUB_SET_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    AW_MESH_ADDRESS_T       address;
    UINT32      model_id;
} AW_MESH_CONFIG_MODEL_SUB_ADD_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    AW_MESH_ADDRESS_T       address;
    UINT32      model_id;
} AW_MESH_CONFIG_MODEL_SUB_DEL_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    AW_MESH_ADDRESS_T       address;
    UINT32      model_id;
} AW_MESH_CONFIG_MODEL_SUB_OW_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT32      model_id;
} AW_MESH_CONFIG_MODEL_SUB_DEL_ALL_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT16      model_id;
} AW_MESH_CONFIG_SIG_MODEL_SUB_GET_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT32      model_id;
} AW_MESH_CONFIG_VENDOR_MODEL_SUB_GET_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
    UINT8           *netkey;
} AW_MESH_CONFIG_NETKEY_ADD_T, AW_MESH_CONFIG_NETKEY_UPDATE_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
} AW_MESH_CONFIG_NETKEY_DEL_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
    UINT16      appkey_index;
    UINT8           *appkey;
} AW_MESH_CONFIG_APPKEY_ADD_T, AW_MESH_CONFIG_APPKEY_UPDATE_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
    UINT16      appkey_index;
} AW_MESH_CONFIG_APPKEY_DEL_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
} AW_MESH_CONFIG_APPKEY_GET_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT16      appkey_index;
    UINT32      model_id;
} AW_MESH_CONFIG_MODEL_APP_BIND_T, AW_MESH_CONFIG_MODEL_APP_UNBIND_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT16      model_id;
} AW_MESH_CONFIG_SIG_MODEL_APP_GET_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT32      model_id;
} AW_MESH_CONFIG_VENDOR_MODEL_APP_GET_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
} AW_MESH_CONFIG_NODE_IDENTITY_GET_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
    UINT8           identity;
} AW_MESH_CONFIG_NODE_IDENTITY_SET_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
} AW_MESH_CONFIG_KEY_REFRESH_PHASE_GET_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
    UINT8           transition;
} AW_MESH_CONFIG_KEY_REFRESH_PHASE_SET_T;

typedef struct {
    UINT16 destination;                   /**< Destination address for Heartbeat messages. */
    UINT8 count_log;                      /**< Destination address for Heartbeat messages. */
    UINT8 period_log;                     /**< Period for sending Heartbeat messages. */
    UINT8 ttl;                            /**< TTL to be used when sending Heartbeat messages. */
    UINT16 features;                      /**< Bit field indicating features that trigger Heartbeat messages when changed. */
    UINT16 netkey_index;                  /**< Network key index. */
} AW_MESH_HEARTBEAT_PUBLICATION_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    AW_MESH_HEARTBEAT_PUBLICATION_T *publication;
} AW_MESH_CONFIG_HB_PUB_SET_T;

typedef struct {
    UINT16 source;                        /**< Source address for Heartbeat messages. */
    UINT16 destination;                   /**< Destination address for Heartbeat messages. */
    UINT8 period_log;                     /**< Period for receiving Heartbeat messages. */
} AW_MESH_HEARTBEAT_SUBSCRIPTION_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    AW_MESH_HEARTBEAT_SUBSCRIPTION_T *subscription;
} AW_MESH_CONFIG_HB_SUB_SET_T;

typedef struct {
    AW_MESH_CONFIG_META_T   meta;
    UINT8           count;
    UINT8           interval_steps;
} AW_MESH_CONFIG_NETWORK_TRANSMIT_SET_T;

//structures for configure models messages
typedef struct {
    UINT16                            opcode;                                /**<The operation code information */
    AW_MESH_CONFIG_META_T             meta;
    union {
        AW_MESH_CONFIG_BEACON_GET_T             beacon_get;                            /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_BEACON_GET*/
        AW_MESH_CONFIG_BEACON_SET_T             beacon_set;                            /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_BEACON_SET */
        AW_MESH_CONFIG_COMPOSITION_DATA_GET_T   composition_data_get;                  /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_GET */
        AW_MESH_CONFIG_DEFAULT_TTL_GET_T        default_ttl_get;                       /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_GET */
        AW_MESH_CONFIG_DEFAULT_TTL_SET_T        default_ttl_set;                       /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_SET */
        AW_MESH_CONFIG_GATT_PROXY_GET_T         gatt_proxy_get;                        /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_GET */
        AW_MESH_CONFIG_GATT_PROXY_SET_T         gatt_proxy_set;                        /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_SET */
        AW_MESH_CONFIG_FRIEND_GET_T             friend_get;                            /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_FRIEND_GET */
        AW_MESH_CONFIG_FRIEND_SET_T             friend_set;                            /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_FRIEND_SET */
        AW_MESH_CONFIG_RELAY_GET_T              relay_get;                             /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_RELAY_GET */
        AW_MESH_CONFIG_RELAY_SET_T              relay_set;                             /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_RELAY_SET */
        AW_MESH_CONFIG_MODEL_PUB_GET_T          model_pub_get;                         /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_GET */
        AW_MESH_CONFIG_MODEL_PUB_SET_T          model_pub_set;                         /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_SET */
        AW_MESH_CONFIG_MODEL_SUB_ADD_T          model_sub_add;
        AW_MESH_CONFIG_MODEL_SUB_DEL_T          model_sub_del;
        AW_MESH_CONFIG_MODEL_SUB_OW_T           model_sub_ow;
        AW_MESH_CONFIG_MODEL_SUB_DEL_ALL_T      model_sub_del_all;
        AW_MESH_CONFIG_SIG_MODEL_SUB_GET_T      sig_model_sub_get;
        AW_MESH_CONFIG_VENDOR_MODEL_SUB_GET_T   vendor_model_sub_get;
        AW_MESH_CONFIG_NETKEY_ADD_T             netkey_add;
        AW_MESH_CONFIG_NETKEY_UPDATE_T          netkey_update;
        AW_MESH_CONFIG_NETKEY_DEL_T             netkey_del;
        AW_MESH_CONFIG_NETKEY_GET_T             netkey_get;
        AW_MESH_CONFIG_APPKEY_ADD_T             appkey_add;
        AW_MESH_CONFIG_APPKEY_UPDATE_T          appkey_update;
        AW_MESH_CONFIG_APPKEY_DEL_T             appkey_del;
        AW_MESH_CONFIG_APPKEY_GET_T             appkey_get;
        AW_MESH_CONFIG_MODEL_APP_BIND_T         model_app_bind;
        AW_MESH_CONFIG_MODEL_APP_UNBIND_T       model_app_unbind;
        AW_MESH_CONFIG_SIG_MODEL_APP_GET_T      sig_model_app_get;
        AW_MESH_CONFIG_VENDOR_MODEL_APP_GET_T   vendor_model_app_get;
        AW_MESH_CONFIG_NODE_IDENTITY_GET_T      node_identity_get;
        AW_MESH_CONFIG_NODE_IDENTITY_SET_T      node_identity_set;
        AW_MESH_CONFIG_NODE_RESET_T             node_reset;
		AW_MESH_CONFIG_POLL_TIMEOUT_T			poll_timeout;
        AW_MESH_CONFIG_KEY_REFRESH_PHASE_GET_T  key_ref_pha_get;
        AW_MESH_CONFIG_KEY_REFRESH_PHASE_SET_T  key_ref_pha_set;
        AW_MESH_CONFIG_HB_PUB_GET_T             hb_pub_get;
        AW_MESH_CONFIG_HB_PUB_SET_T             hb_pub_set;
        AW_MESH_CONFIG_HB_SUB_GET_T             hb_sub_get;
        AW_MESH_CONFIG_HB_SUB_SET_T             hb_sub_set;
        AW_MESH_CONFIG_NETWORK_TRANSMIT_GET_T   net_trans_get;
        AW_MESH_CONFIG_NETWORK_TRANSMIT_SET_T   net_trans_set;
    } data;
} AW_MESH_CONFIGURATION_MSG_TX_T;

#define AW_MESH_BUILD_CFG_META_DATA(DATA,DST,TTL,NETKEY_INDEX)      \
    AW_MESH_CONFIG_META_T DATA  = { \
    .dst_addr = DST, \
    .ttl = TTL, \
    .msg_netkey_index = NETKEY_INDEX,\
    };   \


#endif
