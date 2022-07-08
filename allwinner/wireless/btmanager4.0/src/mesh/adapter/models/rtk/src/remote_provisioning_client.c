enum { __FILE_NUM__ = 0 };

/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     remote_provisioning_client.c
  * @brief    Source file for remote provisioning client model.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2016-5-14
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include <string.h>
#include "trace.h"
#include "mesh_common.h"
#include "remote_provisioning.h"

static mesh_model_info_t rmtProClient;

static inline bool RmtProClient_Send(uint16_t dst, void *pbuffer, uint16_t len)
{
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = pbuffer;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = len;
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

bool RmtProClient_ScanStart(uint16_t dst)
{
    TMeshRmtProScanStart scan_start;
    scan_start.opcode[0] = (MESH_MSG_RMT_PRO_SCAN_START >> 8) & 0xff;
    scan_start.opcode[1] = MESH_MSG_RMT_PRO_SCAN_START & 0xff;

    // send
    return RmtProClient_Send(dst, &scan_start, sizeof(TMeshRmtProScanStart));
}

bool RmtProClient_ScanStartWithFilter(uint16_t dst, uint8_t device_uuid[16])
{
    TMeshRmtProScanStartWithFilter scan_start;
    scan_start.opcode[0] = (MESH_MSG_RMT_PRO_SCAN_START_WITH_FILTER >> 8) & 0xff;
    scan_start.opcode[1] = MESH_MSG_RMT_PRO_SCAN_START_WITH_FILTER & 0xff;
    memcpy(scan_start.device_uuid, device_uuid, 16);

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = &scan_start;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProScanStartWithFilter);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

bool RmtProClient_ScanUnproDeviceNum(uint16_t dst, uint8_t report_num)
{
    TMeshRmtProScanUnproDeviceNum scan_start;
    scan_start.opcode[0] = (MESH_MSG_RMT_PRO_SCAN_UNPRO_DEVICE_NUM >> 8) & 0xff;
    scan_start.opcode[1] = MESH_MSG_RMT_PRO_SCAN_UNPRO_DEVICE_NUM & 0xff;
    scan_start.report_num = report_num;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = &scan_start;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProScanUnproDeviceNum);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProClient_ScanReportStatus(uint16_t dst, TMeshRmtProReportStatus report_status,
                                          uint8_t device_id)
{
    TMeshRmtProScanReportStatus scan_report_status;
    scan_report_status.opcode[0] = (MESH_MSG_RMT_PRO_SCAN_REPORT_STATUS >> 8) & 0xff;
    scan_report_status.opcode[1] = MESH_MSG_RMT_PRO_SCAN_REPORT_STATUS & 0xff;
    scan_report_status.report_status = report_status;
    scan_report_status.unpro_device_id = device_id;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = &scan_report_status;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProScanReportStatus);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProClient_ScanCancel(uint16_t dst)
{
    TMeshRmtProScanCancel scan_cancel;
    scan_cancel.opcode[0] = (MESH_MSG_RMT_PRO_SCAN_CANCEL >> 8) & 0xff;
    scan_cancel.opcode[1] = MESH_MSG_RMT_PRO_SCAN_CANCEL & 0xff;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = &scan_cancel;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProScanCancel);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProClient_LinkOpen(uint16_t dst, uint8_t device_id)
{
    TMeshRmtProLinkOpen link_open;
    link_open.opcode[0] = (MESH_MSG_RMT_PRO_LINK_OPEN >> 8) & 0xff;
    link_open.opcode[1] = MESH_MSG_RMT_PRO_LINK_OPEN & 0xff;
    link_open.unpro_device_id = device_id;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = &link_open;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProLinkOpen);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProClient_LinkClose(uint16_t dst, uint8_t reason)
{
    TMeshRmtProLinkClose link_close;
    link_close.opcode[0] = (MESH_MSG_RMT_PRO_LINK_CLOSE >> 8) & 0xff;
    link_close.opcode[1] = MESH_MSG_RMT_PRO_LINK_CLOSE & 0xff;
    link_close.reason = reason;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = &link_close;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProLinkClose);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProClient_LinkStatus(uint16_t dst, TMeshRmtProLinkStatusEnum link_status,
                                    TMeshRmtProBearerType bearer_type)
{
    TMeshRmtProLinkStatus status;
    status.opcode[0] = (MESH_MSG_RMT_PRO_LINK_STATUS >> 8) & 0xff;
    status.opcode[1] = MESH_MSG_RMT_PRO_LINK_STATUS & 0xff;
    status.link_status = link_status;
    status.bearer_type = bearer_type;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = &status;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProLinkStatus);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProClient_PktTransfer(uint16_t dst, uint8_t *pdata, uint16_t len)
{
    uint16_t pkt_len = 1 + len;
    PMeshRmtProPktTransfer ppkt_transfer = pvPortMalloc(pkt_len, RAM_TYPE_DATA_OFF);
    if (ppkt_transfer == NULL)
    {
        DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "RmtProClient_PktTransfer: failed to allocate memory", 0);
        return FALSE;
    }
    ppkt_transfer->opcode[0] = MESH_MSG_RMT_PRO_PKT_TRANSFER;
    memcpy(ppkt_transfer->buffer, pdata, len);

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = ppkt_transfer;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = pkt_len;
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        access_send(&mesh_msg);
    }

    vPortFree(ppkt_transfer, RAM_TYPE_DATA_OFF);
    return TRUE;
}

static bool RmtProClient_PktTransferReport(uint16_t dst, TMeshRmtProDeliveryStatus delivery_status)
{
    TMeshRmtProPktTransferReport transfer_report;
    transfer_report.opcode[0] = MESH_MSG_RMT_PRO_PKT_TRANSFER_REPORT;
    //transfer_report.opcode[0] = (MESH_MSG_RMT_PRO_PKT_TRANSFER_REPORT >> 8) & 0xff;
    //transfer_report.opcode[1] = MESH_MSG_RMT_PRO_PKT_TRANSFER_REPORT & 0xff;
    transfer_report.delivery_status = delivery_status;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = &transfer_report;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProPktTransferReport);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProClient_PktTransferStatus(uint16_t dst, TMeshRmtProTransferStatus transfer_status)
{
    TMeshRmtProPktTransferStatus pkt_transfer_status;
    pkt_transfer_status.opcode[0] = (MESH_MSG_RMT_PRO_PKT_TRANSFER_STATUS >> 8) & 0xff;
    pkt_transfer_status.opcode[1] = MESH_MSG_RMT_PRO_PKT_TRANSFER_STATUS & 0xff;
    pkt_transfer_status.transfer_status = transfer_status;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProClient;
    mesh_msg.pbuffer = &pkt_transfer_status;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProPktTransferStatus);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

void RmtProClient_HandlePktTransfer(uint16_t client_addr, uint8_t *pbuffer, uint16_t len)
{
    DBG_BUFFER(MODULE_APP, LEVEL_INFO, "RmtProClient_ReceiveMsg: RmtProClient_HandlePktTransfer", 0);
    dprintt(pbuffer, len);
}

bool RmtProClient_ReceiveMsg(uint32_t opcode, mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    switch (opcode)
    {
    case MESH_MSG_RMT_PRO_SCAN_UUID_NUM_REPORT:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProScanUUIDNumReport))
        {
            DBG_BUFFER(MODULE_APP, LEVEL_INFO, "RmtProClient_ReceiveMsg: unprovisioned device number = %d", 1,
                       ((PMeshRmtProScanUUIDNumReport)pbuffer)->unpro_device_count);
        }
        break;
    case MESH_MSG_RMT_PRO_SCAN_UUID_REPORT:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProScanUUIDReport))
        {
            PMeshRmtProScanUUIDReport pmsg = (PMeshRmtProScanUUIDReport)pbuffer;
            DBG_BUFFER(MODULE_APP, LEVEL_INFO, "RmtProClient_ReceiveMsg: unprovisioned device id = %d", 1,
                       pmsg->unpro_device_id);
            dprintt(pmsg->device_uuid, 16);
        }
        break;
    case MESH_MSG_RMT_PRO_SCAN_STATUS:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProScanStatus))
        {
            DBG_BUFFER(MODULE_APP, LEVEL_INFO, "RmtProClient_ReceiveMsg: scan status = %d", 1,
                       ((PMeshRmtProScanStatus)pbuffer)->scan_status);
        }
        break;
    case MESH_MSG_RMT_PRO_SCAN_STOPPED:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProScanStopped))
        {
            DBG_BUFFER(MODULE_APP, LEVEL_INFO, "RmtProClient_ReceiveMsg: scan stopped, status = %d", 1,
                       ((PMeshRmtProScanStopped)pbuffer)->status);
        }
        break;
    case MESH_MSG_RMT_PRO_LINK_STATUS:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProLinkStatus))
        {
            DBG_BUFFER(MODULE_APP, LEVEL_INFO, "RmtProClient_ReceiveMsg: link status = %d, bearer type = %d", 2,
                       ((PMeshRmtProLinkStatus)pbuffer)->link_status, ((PMeshRmtProLinkStatus)pbuffer)->bearer_type);
        }
        break;
    case MESH_MSG_RMT_PRO_LINK_STATUS_REPORT:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProLinkStatusReport))
        {
            DBG_BUFFER(MODULE_APP, LEVEL_INFO, "RmtProClient_ReceiveMsg: link status report = %d, reason = %d",
                       2, ((PMeshRmtProLinkStatusReport)pbuffer)->link_status,
                       ((PMeshRmtProLinkStatusReport)pbuffer)->reason);
        }
        break;
    case MESH_MSG_RMT_PRO_PKT_TRANSFER:
        {
            PMeshRmtProPktTransfer ppkt_transfer = (PMeshRmtProPktTransfer)pbuffer;
            RmtProClient_HandlePktTransfer(pmesh_msg->src, ppkt_transfer->buffer,
                                           pmesh_msg->msg_len - 5); // todo: - 1
        }
        break;
    case MESH_MSG_RMT_PRO_PKT_TRANSFER_REPORT:
        break;
    case MESH_MSG_RMT_PRO_PKT_TRANSFER_STATUS:
        break;
    default:
        ret = FALSE;
        break;
    }

    return ret;
}

void RmtProClient_AddModel(void)
{
    rmtProClient.model_id = MESH_MODEL_RMT_PROVISIONING_CLIENT;
    rmtProClient.model_receive = RmtProClient_ReceiveMsg;
    mesh_model_reg(0, &rmtProClient);
}
