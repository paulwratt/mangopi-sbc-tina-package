/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     remote_provisioning_server.h
  * @brief    Head file for remote provisioning server model.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2016-5-17
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _REMOTE_PROVISIONING_SERVER_H
#define _REMOTE_PROVISIONING_SERVER_H

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include "rtl_types.h"

/** @defgroup PING_
  * @brief
  * @{
  */

typedef enum
{
    MESH_RMT_PRO_SERVER_SCAN_STATE_IDLE,
    MESH_RMT_PRO_SERVER_SCAN_STATE_GENERAL_SCANNING,
    MESH_RMT_PRO_SERVER_SCAN_STATE_FILTERED_SCANNING,
    MESH_RMT_PRO_SERVER_SCAN_STATE_REPORT_NUM_SCANNING
} TMeshRmtProServerScanState;

typedef enum
{
    MESH_RMT_PRO_SERVER_LINK_STATE_IDLE,
    MESH_RMT_PRO_SERVER_LINK_STATE_OPENING,
    MESH_RMT_PRO_SERVER_LINK_STATE_OPENED
} TMeshRmtProServerLinkState;

typedef enum
{
    MESH_RMT_PRO_SERVER_IDLE,
    MESH_RMT_PRO_SERVER_SCAN,
    MESH_RMT_PRO_SERVER_LINK_OPEN,
    MESH_RMT_PRO_SERVER_PKT_TRANSFER,
    MESH_RMT_PRO_SERVER_LINK_CLOSE
} TMeshRmtProServerProcedureState;

typedef struct
{
    uint8_t device_uuid[16];
    uint8_t bt_addr[6];
} TMeshRmtProUnproDeviceInfo, *PMeshRmtProUnproDeviceInfo;

/** @} */

/** @defgroup PROVISIONING_SERVER_API   Provisioning Server API
  * @brief Functions declaration
  * @{
  */

/** @} */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif /* _REMOTE_PROVISIONING_SERVER_H */
