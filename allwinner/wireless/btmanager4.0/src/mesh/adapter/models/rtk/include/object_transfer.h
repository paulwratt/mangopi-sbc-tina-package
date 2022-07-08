/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     object_transfer.h
  * @brief    Head file for object transfer models.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2018-5-21
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _OBJECT_TRANSFER_H
#define _OBJECT_TRANSFER_H

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
#define MESH_MSG_OBJ_TRANSFER_GET                       0xB701
#define MESH_MSG_OBJ_TRANSFER_START                     0xB702
#define MESH_MSG_OBJ_TRANSFER_ABORT                     0xB703
#define MESH_MSG_OBJ_TRANSFER_STAT                      0xB704
#define MESH_MSG_OBJ_BLOCK_TRANSFER_START               0xB705
#define MESH_MSG_OBJ_BLOCK_TRANSFER_STAT                0xB706
#define MESH_MSG_OBJ_CHUNCK_TRANSFER                    0x7D
#define MESH_MSG_OBJ_BLOCK_GET                          0x7E
#define MESH_MSG_OBJ_BLOCK_STAT                         0xB709
#define MESH_MSG_OBJ_INFO_GET                           0xB70A
#define MESH_MSG_OBJ_INFO_STAT                          0xB70B
#define MESH_MSG_OBJ_TRANSFER_PHASE_GET                 0xB707
#define MESH_MSG_OBJ_TRANSFER_PHASE_STAT                0xB708

#define MESH_MODEL_OBJ_TRANSFER_SERVER                  0xFF00FFFF
#define MESH_MODEL_OBJ_TRANSFER_CLIENT                  0xFF01FFFF
/** @} */

/** @defgroup Health_Msg health msg
  * @brief Msg types used by health models
  * @{
  */
typedef enum
{
    OBJ_TRANSFER_PHASE_IDLE,
    OBJ_TRANSFER_PHASE_WAITING_BLOCK,
    OBJ_TRANSFER_PHASE_WAITING_CHUNK
} _PACKED_ obj_transfer_phase_t;

typedef enum
{
    OBJ_BLOCK_CHECK_ALGO_CRC32
} _PACKED_ obj_block_check_algo_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_TRANSFER_PHASE_GET)];
} _PACKED_ obj_transfer_phase_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_TRANSFER_PHASE_STAT)];
    obj_transfer_phase_t phase;
    uint8_t object_id[8]; //!< optional
} _PACKED_ obj_transfer_phase_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_TRANSFER_GET)];
    uint8_t object_id[8];
} _PACKED_ obj_transfer_get_t;

#define BLOCK_SIZE_LOG_MIN              0x6
#define BLOCK_SIZE_LOG_MAX              0x20

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_TRANSFER_START)];
    uint8_t object_id[8];
    uint32_t object_size;
    uint8_t curr_block_size_log; //!< limited range
} _PACKED_ obj_transfer_start_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_TRANSFER_ABORT)];
    uint8_t object_id[8];
} _PACKED_ obj_transfer_abort_t;

typedef enum
{
    OBJ_TRANSFER_STAT_READY, //!< Object transfer is not active.
    OBJ_TRANSFER_STAT_BUSY, //!< Object transfer is currently active.
    OBJ_TRANSFER_STAT_NOT_SUPPORTED, //!< Requested object size cannot be supported.
    OBJ_TRANSFER_STAT_CANNOT_ABORT,
    OBJ_TRANSFER_STAT_INVALID_PARAMETER
} _PACKED_ obj_transfer_stat_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_TRANSFER_STAT)];
    obj_transfer_stat_stat_t stat;
    uint8_t object_id[8];
    uint32_t object_size;
    uint8_t block_size_log; //!< limited range
} _PACKED_ obj_transfer_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_BLOCK_TRANSFER_START)];
    uint8_t object_id[8];
    uint16_t block_num;
    uint16_t chunk_size;
    obj_block_check_algo_t check_algo;
    uint8_t checksum_value[1];
    //uint16_t current_block_size; //!< optional
} _PACKED_ obj_block_transfer_start_t;

typedef enum
{
    OBJ_BLOCK_TRANSFER_STAT_ACCEPTED, //!< Block transfer accepted
    OBJ_BLOCK_TRANSFER_STAT_DUPLICATE_BLOCK, //!< Block already transferred.
    OBJ_BLOCK_TRANSFER_STAT_INVALID_BLOCK_NUM, //!< Invalid Block number.
    OBJ_BLOCK_TRANSFER_STAT_WRONG_BLOCK_SIZE, //!< Bigger than the Block Size Log
    OBJ_BLOCK_TRANSFER_STAT_WRONG_CHUNK_SIZE, //!< Wrong chunk size. Bigger then Block Size divided by Max Chunks Number.
    OBJ_BLOCK_TRANSFER_STAT_UNKNOWN_CHECK_ALGO, //!< Unknown checksum algorithm.
    OBJ_BLOCK_TRANSFER_STAT_INVALID_STATE, //!< Model is in the invalid state
    OBJ_BLOCK_TRANSFER_STAT_INVALID_PARAMETER, //!< Parameter value cannot be accepted
    OBJ_BLOCK_TRANSFER_STAT_NOT_ALL_CHUNCK_RECEIVED, //!< Not All Chunks Received, and checksum is not computed.
    OBJ_BLOCK_TRANSFER_STAT_WRONG_CHECKSUM, //!< All chunks received, computed checksum value is not equal to expected value
    OBJ_BLOCK_TRANSFER_STAT_WRONG_OBJECT_ID, //!< Requested Object ID not active.
    OBJ_BLOCK_TRANSFER_STAT_FRESH_BLOCKED //!< Requested block was never received.
} _PACKED_ obj_block_transfer_stat_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_BLOCK_TRANSFER_STAT)];
    obj_block_transfer_stat_stat_t stat;
} _PACKED_ obj_block_transfer_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_CHUNCK_TRANSFER)];
    uint16_t chunk_num;
    uint8_t data[1]; //!< variable length
} _PACKED_ obj_chunk_transfer_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_BLOCK_GET)];
    uint8_t object_id[8];
    uint16_t block_num;
} _PACKED_ obj_block_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_BLOCK_STAT)];
    obj_block_transfer_stat_stat_t stat;
    uint16_t chunk_list[1]; //!< variable length
} _PACKED_ obj_block_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_INFO_GET)];
} _PACKED_ obj_info_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_OBJ_INFO_STAT)];
    uint8_t min_block_size_log;
    uint8_t max_block_size_log;
    uint16_t max_chunk_num;
} _PACKED_ obj_info_stat_t;
/** @} */

/** @defgroup HEALTH_SERVER_API   Health Server API
  * @brief Functions declaration
  * @{
  */
void obj_transfer_server_reg(uint32_t max_object_size);
mesh_msg_send_cause_t obj_transfer_phase_stat(mesh_msg_p pmesh_msg, obj_transfer_phase_t phase,
                                              uint8_t object_id[8]);
mesh_msg_send_cause_t obj_transfer_stat(mesh_msg_p pmesh_msg, obj_transfer_stat_stat_t stat,
                                        uint8_t object_id[8], uint32_t object_size, uint8_t block_size_log);
mesh_msg_send_cause_t obj_block_transfer_stat(mesh_msg_p pmesh_msg,
                                              obj_block_transfer_stat_stat_t stat);
mesh_msg_send_cause_t obj_block_stat(mesh_msg_p pmesh_msg, obj_block_transfer_stat_stat_t stat,
                                     uint16_t chunk_list[], uint16_t chunck_count);
mesh_msg_send_cause_t obj_info_stat(mesh_msg_p pmesh_msg, uint8_t min_block_size_log,
                                    uint8_t max_block_size_log, uint16_t max_chunk_num);
/** @} */

/** @defgroup HEALTH_CLIENT_API   Health Client API
  * @brief Functions declaration
  * @{
  */
void obj_transfer_client_reg(model_receive_pf model_receive, model_send_cb_pf model_send_cb);
mesh_msg_send_cause_t obj_transfer_phase_get(uint16_t dst);
mesh_msg_send_cause_t obj_transfer_get(uint16_t dst, uint8_t object_id[8]);
mesh_msg_send_cause_t obj_transfer_start(uint16_t dst, uint8_t object_id[8], uint32_t object_size,
                                         uint8_t curr_block_size_log);
mesh_msg_send_cause_t obj_transfer_abort(uint16_t dst, uint8_t object_id[8]);
mesh_msg_send_cause_t obj_block_transfer_start(uint16_t dst, uint8_t object_id[8],
                                               uint16_t block_num,
                                               uint16_t chunk_size, obj_block_check_algo_t check_algo, uint8_t checksum_value[],
                                               uint16_t checksum_value_len);
mesh_msg_send_cause_t obj_chunk_transfer(uint16_t dst, uint16_t chunk_num, uint8_t data[],
                                         uint16_t len);
mesh_msg_send_cause_t obj_block_get(uint16_t dst, uint8_t object_id[8], uint16_t block_num);
mesh_msg_send_cause_t obj_info_get(uint16_t dst);
/** @} */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif /* _OBJECT_TRANSFER_H */
