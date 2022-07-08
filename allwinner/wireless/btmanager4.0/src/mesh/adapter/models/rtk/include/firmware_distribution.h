/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     firmware_distribution.h
  * @brief    Head file for firmware distribution models.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2018-5-21
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _FIRMWARE_DISTRIBUTION_H
#define _FIRMWARE_DISTRIBUTION_H

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
#define MESH_MSG_FW_DIST_GET                            0xB609
#define MESH_MSG_FW_DIST_START                          0xB60A
#define MESH_MSG_FW_DIST_STOP                           0xB60B
#define MESH_MSG_FW_DIST_STAT                           0xB60C
#define MESH_MSG_FW_DIST_DETAILS_GET                    0xB60D
#define MESH_MSG_FW_DIST_DETAILS_LIST                   0xB60E

#define MESH_MODEL_FW_DIST_SERVER                       0xFE02FFFF
#define MESH_MODEL_FW_DIST_CLIENT                       0xFE03FFFF
/** @} */

/** @defgroup Health_Msg health msg
  * @brief Msg types used by health models
  * @{
  */
typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_DIST_GET)];
    uint16_t company_id;
    uint16_t firware_id;
} _PACKED_ fw_dist_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_DIST_START)];
    uint16_t company_id;
    uint16_t firware_id;
    uint16_t group_addr;
    uint16_t update_nodes_list[1]; //!< variable length
} _PACKED_ fw_dist_start_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_DIST_STOP)];
    uint16_t company_id;
    uint16_t firware_id;
} _PACKED_ fw_dist_stop_t;

typedef enum
{
    FW_DIST_STAT_READY,
    FW_DIST_STAT_ACTIVE,
    FW_DIST_STAT_NO_SUCH_COMPANY_PRODUCT_COMBINATION,
    FW_DIST_STAT_BUSY_WITH_DIFF_DIST,
    FW_DIST_STAT_UPDATE_NODE_LIST_TOO_LONG
} _PACKED_ fw_dist_stat_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_DIST_STAT)];
    fw_dist_stat_stat_t stat;
    uint16_t company_id;
    uint16_t firware_id;
} _PACKED_ fw_dist_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_DIST_DETAILS_GET)];
    fw_dist_stat_stat_t stat;
    uint16_t company_id;
    uint16_t firware_id;
} _PACKED_ fw_dist_details_get_t;

typedef enum
{
    FW_DIST_DETAILS_STAT_SUCCESS,
    FW_DIST_DETAILS_STAT_IN_PROGRESS,
    FW_DIST_DETAILS_STAT_CANCEL
} _PACKED_ fw_dist_details_stat_t;

typedef struct
{
    uint16_t addr;
    fw_dist_details_stat_t stat;
} _PACKED_ fw_dist_details_unit_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_FW_DIST_DETAILS_LIST)];
    fw_dist_details_unit_t details[1]; //!< variable length
} _PACKED_ fw_dist_details_list_t;
/** @} */

/** @defgroup HEALTH_SERVER_API   Health Server API
  * @brief Functions declaration
  * @{
  */

/** @} */

/** @defgroup HEALTH_CLIENT_API   Health Client API
  * @brief Functions declaration
  * @{
  */

/** @} */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif /* _FIRMWARE_DISTRIBUTION_H */
