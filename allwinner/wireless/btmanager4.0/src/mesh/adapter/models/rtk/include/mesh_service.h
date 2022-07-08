/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     mesh_service.h
  * @brief    Head file for mesh service.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2016-3-14
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _MESH_SERVICE_H
#define _MESH_SERVICE_H

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include "mesh_provision.h"

/** @addtogroup Mesh_Service Mesh Service
  * @brief
  * @{
  */

/** @defgroup Mesh_Service_Exported_Macros Mesh Service Exported Macros
  * @brief
  * @{
  */
#if MESH_PROVISIONER
#define MESH_GATT_SERVER_COUNT                              0
#else
#if MESH_DEVICE_PROV_PROXY_SERVER_COEXIST
#define MESH_GATT_SERVER_COUNT                              2
#else
#define MESH_GATT_SERVER_COUNT                              1
#endif
#endif

#if MESH_PROVISIONER
#define MESH_GATT_CLIENT_COUNT                              2
#else
#define MESH_GATT_CLIENT_COUNT                              1 //!< or 0 ?
#endif

#define MESH_SERVICE_ADV_DATA_HEADER_SERVICE_DATA_OFFSET    7
/** @} */

/** @defgroup Mesh_Service_Exported_Types Mesh Service Exported Types
  * @brief
  * @{
  */

/** little endian */
typedef struct
{
    uint8_t device_uuid[16];
    prov_oob_info_t oob_info;
} _PACKED_ mesh_service_data_provision_t;

typedef enum
{
    PROXY_ADV_TYPE_NET_ID,
    PROXY_ADV_TYPE_NODE_IDENTITY
} proxy_adv_type_t;

typedef struct
{
    proxy_adv_type_t type;
    union
    {
        uint8_t net_id[8];
        struct
        {
            uint8_t hash[8];
            uint8_t random[8];
        };
    };
} _PACKED_ mesh_service_data_proxy_t;

typedef union
{
    mesh_service_data_provision_t  provision;
    mesh_service_data_proxy_t proxy;
} _PACKED_ mesh_service_data_t, *mesh_service_data_p;

/** @} */

/** @defgroup Mesh_Service_Exported_Functions Mesh Service Exported Functions
  * @brief
  * @{
  */

///@cond
void mesh_service_adv_send(void);
void mesh_service_adv_receive(uint8_t bt_addr[6], uint8_t addr_type, int8_t rssi, uint8_t *pbuffer,
                              uint16_t len);
void mesh_service_identity_adv_send(uint8_t net_key_index);
void mesh_service_identity_adv_rr(void);
void mesh_service_identity_adv_start(void);
void mesh_service_identity_adv_trigger(bool on_off);
void mesh_service_init(void);
///@endcond

/**
  * @brief start the mesh service adv
  * @return none
  */
void mesh_service_adv_start(void);

/**
  * @brief stop the mesh service adv
  * @return none
  */
void mesh_service_adv_stop(void);

/** @} */

/** @} End of group Mesh_Service */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MESH_SERVICE_H */
