/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     provision_generic.h
  * @brief    Head file for provision generic layer.
  * @details
  * @author   bill
  * @date     2016-2-24
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _PROVISION_GENERIC_H_
#define _PROVISION_GENERIC_H_

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include "platform_misc.h"

/** @addtogroup Mesh_Prov Mesh Prov
  * @brief
  * @{
  */

/** @defgroup Provision_Generic_Exported_Types Provision Generic Exported Types
  * @brief  types that other.c files may use all defined here
  * @{
  */
typedef enum
{

    PB_GENERIC_TRANSACTION_START,
    PB_GENERIC_TRANSACTION_ACK,
    PB_GENERIC_TRANSACTION_CONTINUE,
    PB_GENERIC_BEARER_CTRL
} prov_generic_ctrl_format_t;

typedef struct
{
    uint8_t gpcf: 2; //!< Generic Provisioning Control Format @ref prov_generic_ctrl_format_t
    uint8_t info: 6;
} _PACKED_ prov_generic_ctrl_t;

typedef enum
{
    PB_GENERIC_CB_LINK_OPENED,
    PB_GENERIC_CB_LINK_OPEN_FAILED,
    PB_GENERIC_CB_LINK_CLOSED,
    PB_GENERIC_CB_MSG_ACKED,
    PB_GENERIC_CB_MSG,
    //todo: PB_GENERIC_CB_MSG_TRANSMITED maybe usefull calc ecdh secret after send public key to save time
} prov_generic_cb_type_t;

typedef void (*prov_bearer_cb_pf)(prov_generic_cb_type_t type, uint8_t *pbuffer,
                                  uint16_t len); //todo: add ctx index & ctx type when use multiple bearer
/** @} */

/** @} End of group Mesh_Prov */

#ifdef __cplusplus
}
#endif

#endif /* _PROVISION_GENERIC_H_ */
