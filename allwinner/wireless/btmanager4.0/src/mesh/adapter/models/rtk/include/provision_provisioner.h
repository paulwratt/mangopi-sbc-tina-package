/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     provision_provisioner.h
  * @brief    Head file for provisioner.
  * @details  Data structs and external functions declaration.
  * @author   bill
  * @date     2015-11-09
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _PROVISION_PROVISIONER_H_
#define _PROVISION_PROVISIONER_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/* Add Includes here */
#include "provision_generic.h"
#include "mesh_api.h"
#include "mesh_provision.h"

/** @addtogroup Mesh_Prov Mesh Prov
  * @brief
  * @{
  */

/** @defgroup Mesh_Prov_Exported_Types Mesh Prov Exported Types
  * @brief
  * @{
  */
extern uint16_t assign_net_key_index;
extern uint16_t assign_addr;
/** @} */

/** @defgroup Mesh_Prov_Exported_Functions Mesh Prov Exported Functions
  * @brief
  * @{
  */

/**
  * @brief start the provisionging
  *
  * The function shall be called at the appropriate timing.
  * @param[in] attn_dur: attention duration
  * @return operation result
  */
bool prov_invite(uint8_t attn_dur);

/**
  * @brief stop the provisionging
  * @deprecated
  * @return operation result
  */
bool prov_reject(void);

/**
  * @brief unprov the device in the pb-gatt
  * @deprecated
  * @return operation result
  */
bool prov_unprovisioning(void);

/**
  * @brief choose one path from the eight prov pathes
  *
  * publick key: no oob publick key & oob publick key
  * auth data: no oob & input & output & static
  * @param[in] pprov_start: using the start pdu to choose
  * @return operation result
  */
bool prov_path_choose(prov_start_p pprov_start);

/**
  * @brief set the public key of the provisioning device
  *
  * The function shall be called at the appropriate timing.
  * @param[in] public_key: the ecc public key of the device
  * @return operation result
  */
bool prov_device_public_key_set(uint8_t public_key[64]);

/** @} */

/** @} End of group Mesh_Prov */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif  /* _PROVISION_PROVISIONER_H_ */
