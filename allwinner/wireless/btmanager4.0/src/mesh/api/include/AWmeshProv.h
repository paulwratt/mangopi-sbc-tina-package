#ifndef __AW_MESH_PROV_H__
#define __AW_MESH_PROV_H__
#include <stdint.h>
#include <stdbool.h>
#include "AWDefine.h"
#include "AWmeshDefine.h"

/* Provisioning error code used for #AW_MESH_PROV_ERROR_T */
typedef enum {
    AW_MESH_PROV_ERR_SUCCESS = 0,         /**< Provisioning success */
    AW_MESH_PROV_ERR_INVALID_PDU,
    AW_MESH_PROV_ERR_INVALID_FORMAT,
    AW_MESH_PROV_ERR_UNEXPECTED_PDU,
    AW_MESH_PROV_ERR_CONFIRM_FAILED,
    AW_MESH_PROV_ERR_INSUF_RESOURCE,
    AW_MESH_PROV_ERR_DECRYPT_FAILED,
    AW_MESH_PROV_ERR_CANT_ASSIGN_ADDR,
    AW_MESH_PROV_ERR_TIMEOUT,
    AW_MESH_PROV_ERR_UNEXPECTED_ERR,
} AW_MESH_PROV_ERROR_T;

typedef struct {
    UINT8 uuid[AW_MESH_UUID_SIZE];
    UINT16 oob_info;
    UINT8 uri_hash[AW_MESH_URI_HASH_LEN];
    UINT8 mac[AW_ADDR_LEN];
	int16_t rssi;
} AW_MESH_PROV_SCAN_UD_T;

typedef struct {
    AW_MESH_PROV_ERROR_T reason;       /**< Indicate the provisioning process success or failed reason. */
    UINT16 address;      /**< Indicate the target unicast address. */
    UINT8 element_num;
    UINT8 device_key[AW_MESH_DEVKEY_SIZE]; /**< Indicate the device key. */
    UINT8 uuid[AW_MESH_UUID_SIZE];
    BOOL success;       /**< Indicate the provisioning process is successfull or not. */
    BOOL gatt_bearer;
} AW_MESH_PROV_DONE_T;
#endif
