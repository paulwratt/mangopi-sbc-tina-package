/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     firmware_update.h
  * @brief    Head file for firmware update models.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2018-5-21
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _FIRMWARE_UPDATE_H
#define _FIRMWARE_UPDATE_H

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include "platform_misc.h"
#include "mesh_api.h"

/** @defgroup HEALTH_OPCODE health opcode
  * @brief
  * @{
  */
#define MESH_MSG_FW_INFO_GET                            0xB601
#define MESH_MSG_FW_INFO_STAT                           0xB602
#define MESH_MSG_FW_UPDATE_GET                          0xB603
#define MESH_MSG_FW_UPDATE_PREPARE                      0xB604
#define MESH_MSG_FW_UPDATE_START                        0xB605
#define MESH_MSG_FW_UPDATE_ABORT                        0xB606
#define MESH_MSG_FW_UPDATE_APPLY                        0xB607
#define MESH_MSG_FW_UPDATE_STAT                         0xB608

#define MESH_MODEL_FW_UPDATE_SERVER                     0xFE00FFFF
#define MESH_MODEL_FW_UPDATE_CLIENT                     0xFE01FFFF
/** @} */

#define FW_UPDATE_FW_ID_LEN                             4
typedef uint8_t *fw_update_fw_id_t;
#define FW_UPDATE_FW_ID(a,b)                            memcpy(a,b,FW_UPDATE_FW_ID_LEN)

/** @defgroup Health_Msg health msg
  * @brief Msg types used by health models
  * @{
  */
typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_INFO_GET)];
} _PACKED_ fw_info_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_INFO_STAT)];
    uint16_t company_id;
    uint8_t firmware_id[FW_UPDATE_FW_ID_LEN];
    uint8_t update_url[1]; //!< variable length, optional
} _PACKED_ fw_info_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_UPDATE_GET)];
    uint16_t company_id;
    uint8_t firmware_id[FW_UPDATE_FW_ID_LEN];
} _PACKED_ fw_update_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_UPDATE_PREPARE)];
    uint16_t company_id;
    uint8_t firmware_id[FW_UPDATE_FW_ID_LEN];
    uint8_t object_id[8];
    uint8_t vendor_validate_data[1]; //!< optional, Vendor specific validation data for update
} _PACKED_ fw_update_prepare_t;

typedef enum
{
    FW_UPDATE_POLICY_NONE, //!< Do not apply new firmware when Object transfer is completed.
    FW_UPDATE_POLICY_AUTO_UPDATE //!< Apply new firmware when Object transfer is completed.
} _PACKED_ fw_update_policy_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_UPDATE_START)];
    fw_update_policy_t policy;
    uint16_t company_id;
    uint8_t firmware_id[FW_UPDATE_FW_ID_LEN];
} _PACKED_ fw_update_start_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_UPDATE_ABORT)];
    uint16_t company_id;
    uint8_t firmware_id[FW_UPDATE_FW_ID_LEN];
} _PACKED_ fw_update_abort_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_UPDATE_APPLY)];
    uint16_t company_id;
    uint8_t firmware_id[FW_UPDATE_FW_ID_LEN];
} _PACKED_ fw_update_apply_t;

typedef enum
{
    FW_UPDATE_STAT_SUCCESS,
    FW_UPDATE_STAT_WRONG_COMPANY_FIRMWARE_COMBINATION,
    FW_UPDATE_STAT_DIFF_OBJECT_TRANSFER_ALREADY_ONGOING,
    FW_UPDATE_STAT_COMPANY_FIRMWARE_APPLY_FAILED,
    FW_UPDATE_STAT_NEWER_FW_VERSION_PRESENT, //!< Company ID and Firmware ID combination permanently rejected
    FW_UPDATE_STAT_NOT_AVAILABLE //!< Company ID and Firmware ID combination temporary rejected
} _PACKED_ fw_update_stat_stat_t;

typedef enum
{
    FW_UPDATE_PHASE_IDLE, //!< No DFU update in progress
    FW_UPDATE_PHASE_PREPARED, //!< DFU update is prepared and awaiting start
    FW_UPDATE_PHASE_IN_PROGRESS, //!< DFU update is in progress
    FW_UPDATE_PHASE_DFU_READY //!< DFU upload is finished and waiting to be apply
} _PACKED_ fw_update_phase_t;

typedef struct
{
    uint8_t prov_needed: 1;
} _PACKED_ fw_update_addi_info_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_UPDATE_STAT)];
    fw_update_stat_stat_t stat;
    uint8_t phase: 3; //!< @ref fw_update_phase_t
    uint8_t addi_info: 5; //!< @ref fw_update_addi_info_t
    uint16_t company_id;
    uint8_t firmware_id[FW_UPDATE_FW_ID_LEN];
    uint8_t object_id[8]; //!< optional, Unique object identifier
} _PACKED_ fw_update_stat_t;
/** @} */

/** @defgroup HEALTH_SERVER_API   Health Server API
  * @brief Functions declaration
  * @{
  */
void fw_update_server_reg(uint16_t company_id, fw_update_fw_id_t firmware_id);
mesh_msg_send_cause_t fw_info_stat(mesh_msg_p pmesh_msg, uint16_t company_id,
                                   fw_update_fw_id_t firmware_id,
                                   uint8_t update_url[], uint16_t url_len);
mesh_msg_send_cause_t fw_update_stat(mesh_msg_p pmesh_msg, fw_update_stat_stat_t stat,
                                     uint8_t phase, uint8_t addi_info, uint16_t company_id, fw_update_fw_id_t firmware_id,
                                     uint8_t object_id[8]);
/** @} */

/** @defgroup HEALTH_CLIENT_API   Health Client API
  * @brief Functions declaration
  * @{
  */
void fw_update_client_reg(model_receive_pf pf);
mesh_msg_send_cause_t fw_info_get(uint16_t dst);
mesh_msg_send_cause_t fw_update_get(uint16_t dst, uint16_t company_id,
                                    fw_update_fw_id_t firmware_id);
mesh_msg_send_cause_t fw_update_prepare(uint16_t dst, uint16_t company_id,
                                        fw_update_fw_id_t firmware_id,
                                        uint8_t object_id[8], uint8_t vendor_validate_data[], uint16_t validate_len);
mesh_msg_send_cause_t fw_update_start(uint16_t dst, fw_update_policy_t policy, uint16_t company_id,
                                      fw_update_fw_id_t firmware_id);
mesh_msg_send_cause_t fw_update_abort(uint16_t dst, uint16_t company_id,
                                      fw_update_fw_id_t firmware_id);
mesh_msg_send_cause_t fw_update_apply(uint16_t dst, uint16_t company_id,
                                      fw_update_fw_id_t firmware_id);
/** @} */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif /* _FIRMWARE_UPDATE_H */
