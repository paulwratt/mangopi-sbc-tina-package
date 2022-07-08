#ifndef __AW_MESH_DEFINE_H__
#define __AW_MESH_DEFINE_H__
#include <stdint.h>
#include <stdbool.h>
#include "AWDefine.h"

#define AW_MAX_BDADDR_LEN           (18)
#define AW_MESH_VERSION_LEN         (30)
#define AW_MESH_UUID_SIZE           (16)
#define AW_MESH_DEVICE_KEY_SIZE		(16)
#define AW_MESH_KEY_SIZE            (16)
#define AW_MESH_PUBLIC_KEY_SIZE     (16)
#define AW_MESH_BLE_ADDR_LEN        (6)
#define AW_MESH_TTL_MAX             (0x7F)
#define AW_MESH_AUTHENTICATION_SIZE  (16)
#define AW_MESH_DEVKEY_SIZE           AW_MESH_KEY_SIZE
#define AW_MESH_COMPOSITION_DATA_FIXED_FIELD_LEN  (10)
#define AW_MESH_EVENT_MESH          (200)
#define AW_MESH_AW_EVENT_MESH       (300)
#define AW_MESH_EVENT_ADV_REPORT    (0x1003)
#define AW_MESH_ADV_DATA_SIZE       (31)
#define AW_MESH_URI_HASH_LEN        (4)
#define AW_ADDR_LEN                 (6)
#define AW_MESH_CFG_MMDL_MSG_LEN    (64)
#define AW_MESH_APP_KEY_SIZE        (16)
#define AW_MESH_MAX_NETKEY_LIST     (64)
#define AW_MESH_MAX_APPKEY_LIST     (64)

#define AW_MESH_FOUNDATION_MMDL_INDEX (0)
#define AW_MESH_COMPANY_ID_SIG  (0xFFFF)
#define AW_ACCESS_OPCODE_SIG(opcode)    { (opcode), AW_MESH_COMPANY_ID_SIG }

#define AW_MESH_APP_COMPANY_ID	(0x05f1)
#define AW_MESH_APP_PRODUCT_ID	(0x0001)
#define AW_MESH_APP_VERSION_ID	(0x0001)

#define VENDOR_ID_MASK		0xffff0000
#define CONFIG_SRV_MODEL	(VENDOR_ID_MASK | 0x0000)
#define CONFIG_CLI_MODEL	(VENDOR_ID_MASK | 0x0001)
#define AW_MESH_AUTHENTICATION_SIZE  (16)

//node feature bit
#define AW_MESH_FEATURE_NONE     0x00    /**< A bit field indicating no feature. */
#define AW_MESH_FEATURE_RELAY    0x01    /**< A bit field indicating feature relay. */
#define AW_MESH_FEATURE_PROXY    0x02    /**< A bit field indicating feature proxy. */
#define AW_MESH_FEATURE_FRIEND   0x04    /**< A bit field indicating feature friend. */
#define AW_MESH_FEATURE_LPN      0x08    /**< A bit field indicating feature low power node. */
#define AW_MESH_FEATURE_SNB      0x10

//node feature configuration
#define CONFIG_NODE_FEATURE (AW_MESH_FEATURE_RELAY  \
                            |AW_MESH_FEATURE_FRIEND \
                            |AW_MESH_FEATURE_SNB)

typedef enum {
    AW_ERROR_NONE = 0,
    AW_ERROR_FAIL,
    AW_ERROR_INVALID_ARGS,
    AW_ERROR_BUSY,
    //mesh error code
    AW_ERROR_MESH_NOT_INIT,
    AW_MESH_ERROR_NOT_INIT_DONE,
    AW_MESH_ERROR_FAILED_INIT_PROV_REG,
    AW_MESH_ERROR_INVALID_ARGS,
    AW_MESH_ERROR_THREAD_CREATE_FAIL,
    AW_MESH_ERROR_APPLICATION_IFACE_REG_FAIL,
    AW_MESH_ERROR_APPLICATION_IFACE_ADD_FAIL,
    AW_MESH_ERROR_ELEMENT_IFACE_REG_FAIL,
    AW_MESH_ERROR_AGENT_IFACE_REG_FAIL,
    AW_MESH_ERROR_AGENT_IFACE_ADD_FAIL,
    AW_MESH_ERROR_AGENT_INIT_FAIL,
    AW_MESH_ERROR_ATTACH_FAIL,
    AW_MESH_ERROR_INVALID_ROLE,
    AW_MESH_ERROR_ALREADY_ATTACH,
    AW_MESH_ERROR_ALREADY_INIT,
    AW_MESH_ERROR_FAILED_ADD_ELEMENT,
    AW_MESH_ERROR_NODE_NOT_READY,
    AW_MESH_ERROR_NOT_FOUND_ELEMENT,
    AW_MESH_ERROR_NOT_FOUND_OPCODE,
    AW_MESH_ERROR_NOT_FOUND_MODEL,
    AW_MESH_ERROR_NOT_RELEASE_API,
    //provisioner
    AW_ERROR_MESH_PROV_ONGOING,
}AW_ERROR_T;

typedef enum {
    AW_MESH_ERROR_NONE = 0,
    AW_MESH_ERROR_FAILED,
}AW_MESH_ERRCODE_T;

#define MESH_POINTER_ACCESS(POINTER) do{if(!POINTER)return AW_MESH_ERROR_INVALID_ARGS;}while(0)
#define MESH_READY_ACCESS(MESH) do{if(!MESH)return AW_ERROR_MESH_NOT_INIT;}while(0)
#define NODE_READY_ACCESS(NODE) do{if(!NODE)return AW_MESH_ERROR_NODE_NOT_READY;}while(0)
#define ELEMENT_READY_ACCESS(ELEMENT) do{if(!ELEMENT)return AW_MESH_ERROR_NOT_FOUND_ELEMENT;}while(0)

typedef enum
{
    AW_MESH_REPORT_TYPE_IND = 0x00,                 ///< Type for ADV_IND found (passive)
    AW_MESH_REPORT_TYPE_DIRECT_IND = 0x01,          ///< Type for ADV_DIRECT_IND found (passive)
    AW_MESH_REPORT_TYPE_SCAN_IND    = 0x02,         ///< Type for ADV_SCAN_IND found (passive)
    AW_MESH_REPORT_TYPE_NONCONN_IND  = 0x03,        ///< Type for ADV_NONCONN_IND found (passive)
    AW_MESH_REPORT_TYPE_SCAN_RSP = 0x04             ///< Type for SCAN_RSP found (active)
} AW_MESH_REPORT_TYPE;

typedef enum {
    AW_MESH_BLE_ADDR_TYPE_PUBLIC = 0,                /**< public address */
    AW_MESH_BLE_ADDR_TYPE_RANDOM_STATIC = 1,         /**< random static address */
    AW_MESH_BLE_ADDR_TYPE_RANDOM_RESOLVABLE = 2,     /**< random resolvable addresss */
    AW_MESH_BLE_ADDR_TYPE_RANDOM_NON_RESOLVABLE = 3, /**< random non resolvable address */
} AW_MESH_BLE_ADDR_TYPE_T;

typedef struct {
    AW_MESH_BLE_ADDR_TYPE_T addr_type;               /**< address type */
    CHAR addr[AW_MAX_BDADDR_LEN];                      /**< address byte array */
} AW_MESH_BLE_ADDR_T;

typedef enum {
    AW_MESH_ADDRESS_TYPE_UNASSIGNED = 0,   /**< unassigned address */
    AW_MESH_ADDRESS_TYPE_UNICAST,          /**< unicast address */
    AW_MESH_ADDRESS_TYPE_VIRTUAL,          /**< virtual address */
    AW_MESH_ADDRESS_TYPE_GROUP,            /**< group address */
} AW_MESH_ADDRESS_TYPE_T;

typedef struct {
    AW_MESH_ADDRESS_TYPE_T type;   /**< the address type of this address */
    UINT16 value;                 /**< address value */
    const UINT8 *virtual_uuid;    /**< virtual uuid value, must be NULL unless address type is #AW_MESH_ADDRESS_TYPE_VIRTUAL */
} AW_MESH_ADDRESS_T;

typedef struct {
    UINT16 netidx;                /**< index of network key */
    UINT16 appidx;                /**< index of application key, if 0xFFFF means using device key */
    UINT8 *device_key;            /**< device key value, can't be NULL if appidx is 0xFFFF. */
} AW_MESH_SECURITY_T;

typedef struct {
    AW_MESH_ADDRESS_T dst;         /**< destination address information  */
    UINT16 src;                   /**< source unicast address */
    UINT8 ttl;                    /**< ttl value */
    const UINT8 *data;            /**< data buffer to be sent */
    UINT16 data_len;              /**< data buffer length */
    AW_MESH_SECURITY_T security;   /**< security information */
} AW_MESH_TX_PARAMS_T;

typedef struct {
    UINT16 opcode;        /**< Operation code. */
    UINT16 company_id;    /**< Company id, use #MESH_MODEL_COMPANY_ID_NONE if this is a SIG access message */
} AW_MESH_ACCESS_OPCODE_T;

/**
 * @brief mesh model description.
 */
typedef struct {
    uint16_t model_id;
    uint16_t company_id;
} AW_MESH_MODEL_ID_T;

typedef enum {
    AW_MESH_ROLE_PROVISIONEE = 0,      /**< act as a provisionee */
    AW_MESH_ROLE_PROVISIONER,          /**< act as a provisioner */
} AW_MESH_ROLE_T;

//struct for access rx meta data
typedef struct {
    UINT16 src_addr;      /**< The source address of this message. */
    UINT16 dst_addr;      /**< The destination address of this message */
    UINT16 appkey_index;        /**< The application key index used for this message. */
    UINT16 netkey_index;     /**< The network key index used for this message. */
    UINT8  ele_index;
    UINT8 rssi;           /**< The RSSI value . */
    UINT8 ttl;            /**< The received TTL value . */
} AW_MESH_ACCESS_RX_META_T;

//struct for access rx meta data
typedef struct {
    UINT16 ele_idx;      /**< The source address of this message. */
    UINT16 dst_addr;      /**< The destination address of this message */
    UINT16 appkey_index;        /**< The application key index used for this message. */
    UINT16 netkey_index;     /**< The network key index used for this message. */
    UINT8 ttl;            /**< The received TTL value . */
}AW_MESH_ACCESS_TX_META_T;

//struct for access rx messages
typedef struct {
    AW_MESH_ACCESS_OPCODE_T opcode;     /**< The operation code information. */
    UINT8 *data;                        /**< The received message buffer. */
    UINT16 dlen;                        /**< The length of received message. */
    AW_MESH_ACCESS_RX_META_T meta_data; /**< The metadata of this message. */
} AW_MESH_ACCESS_MESSAGE_RX_T,*access_rx_msg_p;

//struct for access rx messages
typedef struct {
    UINT8 *data;                        /**< The received message buffer. */
    UINT16 dlen;                        /**< The length of received message. */
    AW_MESH_ACCESS_TX_META_T meta_data; /**< The metadata of this message. */
} AW_MESH_ACCESS_MESSAGE_TX_T,*access_tx_msg_p;

//struct for access tx publish messages
typedef struct {
    UINT8 *data;                        /**< The received message buffer. */
    UINT16 dlen;                        /**< The length of received message. */
    UINT32 mod_id;
    AW_MESH_ACCESS_TX_META_T meta_data; /**< The metadata of this message. */
} AW_MESH_ACCESS_PUBLISH_MESSAGE_TX_T,*access_tx_publish_msg_p;

int32_t meshd_init();
#endif
