/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     tp.h
  * @brief    Head file for ping models.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2018-3-7
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _TP_H
#define _TP_H

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include "mesh_api.h"

/** @defgroup
  * @brief
  * @{
  */
#define MESH_MSG_TP_SEG                                 0xC85D00

#define MESH_MODEL_TP_CONTROL                           0x0003005D
/** @} */

/** @defgroup PING_MSG  Ping Msg
  * @brief
  * @{
  */
typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_TP_SEG)];
    uint32_t tid;
    uint8_t padding[ACCESS_PAYLOAD_MAX_SIZE - sizeof(uint32_t) - ACCESS_OPCODE_SIZE(MESH_MSG_TP_SEG)];
} _PACKED_ tp_seg_t;
/** @} */

/** @defgroup PING_SERVER_API   Ping Server API
  * @brief Functions declaration
  * @{
  */
void tp_control_reg(void);
mesh_msg_send_cause_t tp_seg(uint16_t dst, uint8_t ttl, uint8_t app_key_index);
mesh_msg_send_cause_t tp_seg_start(uint16_t dst, uint8_t ttl, uint8_t app_key_index,
                                   uint32_t count);

/** @} */
#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif /* _TP_H */
