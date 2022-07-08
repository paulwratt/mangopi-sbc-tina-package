#ifndef __AW_MESH_GATEWAY_H__
#define __AW_MESH_GATEWAY_H__
#include "AWDefine.h"
#define AW_MESH_DEV_UUID_LEN                                     16
#define AW_MESH_KEY_LEN                                          16
#define AW_MESH_DEVICEKEY_LEN                                    16

typedef struct{
    uint16_t unicast_address;       /**< gateway unicast address */
    uint16_t group_address;         /**< subscription group address for all initial models */
    uint32_t iv_index;              /**< iv index vaule */
    uint8_t  flags;                 /**< iv update flags*/
    uint16_t netkey_index;          /**< gloabl netkey index */
    //uint16_t appkey_index;          /**< global netkey index */
    uint8_t  primary_netkey[AW_MESH_KEY_LEN];    /**< netkey */
    uint8_t  device_key[AW_MESH_KEY_LEN];    /**< netkey */
    //uint8_t  primary_appkey[MIBLE_MESH_KEY_LEN];    /**< appkey */
    uint8_t  max_num_netkey;        /**< stack support netkey num */
    uint8_t  max_num_appkey;        /**< stack support appkey num */
    uint16_t replay_list_size;      /**< replay protection list size */
    uint16_t default_ttl;           /**< default ttl */
    bool is_provisioner;            /**define local node role**/
}AW_MESH_GATEWAY_INFO_T;

typedef enum {
	AW_MESH_OP_ADD = 0,
	AW_MESH_OP_DEL,
}AW_MESH_GATEWAY_OP_T;

typedef struct {
    uint16_t unicast_address;
    uint16_t elements_num;
    uint8_t device_key[AW_MESH_DEVICEKEY_LEN];
    uint8_t uuid[AW_MESH_DEV_UUID_LEN];
    uint16_t netkey_index;
}AW_MESH_GATEWAY_NODE_INFO_T;

#endif
