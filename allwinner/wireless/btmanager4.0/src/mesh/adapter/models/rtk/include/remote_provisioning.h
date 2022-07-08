/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     remote_provisioning.h
  * @brief    Head file for remote provisioning models.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2016-5-13
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _REMOTE_PROVISIONING_H
#define _REMOTE_PROVISIONING_H

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include "rtl_types.h"

/** @defgroup PING_
  * @brief
  * @{
  */
#define MESH_MSG_RMT_PRO_SCAN_START                  0x8065
#define MESH_MSG_RMT_PRO_SCAN_START_WITH_FILTER      0x8066
#define MESH_MSG_RMT_PRO_SCAN_UNPRO_DEVICE_NUM       0x8068
#define MESH_MSG_RMT_PRO_SCAN_UUID_NUM_REPORT        0x806b
#define MESH_MSG_RMT_PRO_SCAN_UUID_REPORT            0x806a
#define MESH_MSG_RMT_PRO_SCAN_REPORT_STATUS          0x8069
#define MESH_MSG_RMT_PRO_SCAN_STATUS                 0x8067
#define MESH_MSG_RMT_PRO_SCAN_CANCEL                 0x8064
#define MESH_MSG_RMT_PRO_SCAN_STOPPED                0x806d
#define MESH_MSG_RMT_PRO_LINK_OPEN                   0x8062
#define MESH_MSG_RMT_PRO_LINK_STATUS                 0x8063 // Send and receive by both server and client
#define MESH_MSG_RMT_PRO_LINK_CLOSE                  0x8061
#define MESH_MSG_RMT_PRO_LINK_STATUS_REPORT          0x806c
#define MESH_MSG_RMT_PRO_PKT_TRANSFER                0x02
#define MESH_MSG_RMT_PRO_PKT_TRANSFER_REPORT         0x01
#define MESH_MSG_RMT_PRO_PKT_TRANSFER_STATUS         0x8060

#define MESH_MODEL_RMT_PROVISIONING_SERVER           0x0004
#define MESH_MODEL_RMT_PROVISIONING_CLIENT           0x8004
/** @} */

/** @defgroup PING_MSG  Ping Msg
  * @brief Msg types used by ping models
  * @{
  */

typedef enum
{
    MESH_RMT_PRO_REPORT_STATUS_ACCEPTED,
    MESH_RMT_PRO_REPORT_STATUS_REJECTED
} TMeshRmtProReportStatus;

typedef enum
{
    MESH_RMT_PRO_SCAN_STATUS_STARTED,
    MESH_RMT_PRO_SCAN_STATUS_CANCELED,
    MESH_RMT_PRO_SCAN_STATUS_CANNOT_START,
    MESH_RMT_PRO_SCAN_STATUS_CANNOT_CANCEL,
    MESH_RMT_PRO_SCAN_STATUS_ACCEPTED,
    MESH_RMT_PRO_SCAN_STATUS_REJECTED,
} TMeshRmtProScanStatusEnum;

typedef enum
{
    MESH_RMT_PRO_SCAN_STOPPED_STATUS_RFU,
    MESH_RMT_PRO_SCAN_STOPPED_STATUS_OUT_OF_RESOURCE,
    MESH_RMT_PRO_SCAN_STOPPED_STATUS_TIMEOUT,
} TMeshRmtProScanStoppedStatus;

typedef enum
{
    MESH_RMT_PRO_LINK_STATUS_OPENING,
    MESH_RMT_PRO_LINK_STATUS_ALREADY_OPEN,
    MESH_RMT_PRO_LINK_STATUS_CANNOT_CLOSE,
    MESH_RMT_PRO_LINK_STATUS_NOT_ACTIVE,
    MESH_RMT_PRO_LINK_STATUS_INVALID_UNPRO_DEVCIE_ID,
    MESH_RMT_PRO_LINK_STATUS_ACCEPTED,
    MESH_RMT_PRO_LINK_STATUS_REJECTED
} TMeshRmtProLinkStatusEnum;

typedef enum
{
    MESH_RMT_PRO_BEARER_TYPE_PB_ADV,
    MESH_RMT_PRO_BEARER_TYPE_PB_GATT
} TMeshRmtProBearerType;

typedef enum
{
    MESH_RMT_PRO_LINK_STATUS_REPORT_OPENED,
    MESH_RMT_PRO_LINK_STATUS_REPORT_OPEN_TIMEOUT,
    MESH_RMT_PRO_LINK_STATUS_REPORT_CLOSED,
    MESH_RMT_PRO_LINK_STATUS_REPORT_CLOSED_BY_DEVICE,
    MESH_RMT_PRO_LINK_STATUS_REPORT_CLOSED_BY_PROVISION_SERVER
} TMeshRmtProLinkStatusReportEnum;

typedef enum
{
    MESH_RMT_PRO_DELIVERY_STATUS_DELIVERED,
    MESH_RMT_PRO_DELIVERY_STATUS_NOT_DELIVERED
} TMeshRmtProDeliveryStatus;

typedef enum
{
    MESH_RMT_PRO_TRANSFER_STATUS_BUFFER_ACCEPTED,
    MESH_RMT_PRO_TRANSFER_STATUS_LINK_NOT_ACTIVE,
    MESH_RMT_PRO_TRANSFER_STATUS_CANNOT_ACCEPT_BUFFER,
    MESH_RMT_PRO_TRANSFER_STATUS_ACCEPTED,
    MESH_RMT_PRO_TRANSFER_STATUS_REJECTED
} TMeshRmtProTransferStatus;

typedef struct
{
    uint8_t opcode[2];
    uint32_t app_mic;
} _PACKED_ TMeshRmtProScanStart, *PMeshRmtProScanStart;

typedef struct
{
    uint8_t opcode[2];
    uint8_t device_uuid[16];
    uint32_t app_mic;
} _PACKED_ TMeshRmtProScanStartWithFilter, *PMeshRmtProScanStartWithFilter;

typedef struct
{
    uint8_t opcode[2];
    uint8_t report_num;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProScanUnproDeviceNum, *PMeshRmtProScanUnproDeviceNum;

typedef struct
{
    uint8_t opcode[2];
    uint8_t unpro_device_count;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProScanUUIDNumReport, *PMeshRmtProScanUUIDNumReport;

typedef struct
{
    uint8_t opcode[2];
    uint8_t device_uuid[16];
    uint8_t unpro_device_id;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProScanUUIDReport, *PMeshRmtProScanUUIDReport;

typedef struct
{
    uint8_t opcode[2];
    TMeshRmtProReportStatus report_status;
    uint8_t unpro_device_id;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProScanReportStatus, *PMeshRmtProScanReportStatus;

typedef struct
{
    uint8_t opcode[2];
    TMeshRmtProScanStatusEnum scan_status;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProScanStatus, *PMeshRmtProScanStatus;

typedef struct
{
    uint8_t opcode[2];
    uint32_t app_mic;
} _PACKED_ TMeshRmtProScanCancel, *PMeshRmtProScanCancel;

typedef struct
{
    uint8_t opcode[2];
    TMeshRmtProScanStoppedStatus status;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProScanStopped, *PMeshRmtProScanStopped;

typedef struct
{
    uint8_t opcode[2];
    uint8_t unpro_device_id;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProLinkOpen, *PMeshRmtProLinkOpen;

typedef struct
{
    uint8_t opcode[2];
    TMeshRmtProLinkStatusEnum link_status;
    TMeshRmtProBearerType bearer_type;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProLinkStatus, *PMeshRmtProLinkStatus;

typedef struct
{
    uint8_t opcode[2];
    uint8_t reason;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProLinkClose, *PMeshRmtProLinkClose;

typedef struct
{
    uint8_t opcode[2];
    TMeshRmtProLinkStatusReportEnum link_status;
    uint8_t reason; //!< 0xff when pb-adv
    uint32_t app_mic;
} _PACKED_ TMeshRmtProLinkStatusReport, *PMeshRmtProLinkStatusReport;

typedef struct
{
    uint8_t opcode[1];
    uint8_t buffer[3]; // todo
} _PACKED_ TMeshRmtProPktTransfer, *PMeshRmtProPktTransfer;

typedef struct
{
    uint8_t opcode[1];
    TMeshRmtProDeliveryStatus delivery_status;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProPktTransferReport, *PMeshRmtProPktTransferReport;

typedef struct
{
    uint8_t opcode[2];
    TMeshRmtProTransferStatus transfer_status;
    uint32_t app_mic;
} _PACKED_ TMeshRmtProPktTransferStatus, *PMeshRmtProPktTransferStatus;

/** @} */

/** @defgroup PROVISIONING_SERVER_API   Provisioning Server API
  * @brief Functions declaration
  * @{
  */

/** @} */

/** @defgroup PROVISIONING_CLIENT_API   Provisioning Client API
  * @brief Functions declaration
  * @{
  */

/** @} */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif /* _REMOTE_PROVISIONING_H */
