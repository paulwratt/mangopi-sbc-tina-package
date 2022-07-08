enum { __FILE_NUM__ = 0 };

/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     remote_provisioning_server.c
  * @brief    Source file for remote provisioning server model.
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
#include "remote_provisioning_server.h"

static mesh_model_info_t rmtProServer;
static TMeshRmtProUnproDeviceInfo unproDeviceInfo[MESH_RMT_PRO_SERVER_UNPRO_DEVICE_NUM];
static uint8_t unproDeviceInfoSP; //!< stack pointer
//static bool usedFlag[MESH_RMT_PRO_SERVER_UNPRO_DEVICE_NUM];
static TMeshRmtProServerScanState rmtProServerScanState;
static TMeshRmtProServerLinkState rmtProServerLinkState;
static TMeshRmtProServerProcedureState rmtProServerProcedureState;
static uint16_t clientAddr;
static bool filterScanFound;
static uint8_t reportNum;
static uint8_t retryCount; //!< used to send scan report and link open
static TimerHandle_t rmtProServerTimer;

static void vTimerMeshRmtProServerTimeoutCb(void *timer)
{
    mesh_inner_msg_t msg;
    msg.type = MESH_RMT_PRO_TIMEOUT;
    mesh_inner_msg_send(&msg);
}

void RmtProServer_TimerStart(void)
{
    uint32_t timer_period = MESH_RMT_PRO_SERVER_SCAN_TIMEOUT_PERIOD;
    if (rmtProServerProcedureState == MESH_RMT_PRO_SERVER_SCAN)
    {
        if (rmtProServerScanState == MESH_RMT_PRO_SERVER_SCAN_STATE_REPORT_NUM_SCANNING)
        {
            timer_period = MESH_RMT_PRO_SERVER_SCAN_REPORT_PERIOD;
        }
        else
        {
            timer_period = MESH_RMT_PRO_SERVER_SCAN_TIMEOUT_PERIOD;
        }
    }
    else if (rmtProServerProcedureState == MESH_RMT_PRO_SERVER_LINK_OPEN)
    {
        timer_period = MESH_RMT_PRO_SERVER_LINK_OPEN_INTERVAL;
    }
    else
    {

    }

    if (NULL == rmtProServerTimer)
    {
        rmtProServerTimer = xTimerCreate("meshRmtProServerTimer",
                                         timer_period / portTICK_PERIOD_MS,
                                         pdFALSE,
                                         (void *)MESH_PB_MESH_TIMER_ID,
                                         vTimerMeshRmtProServerTimeoutCb
                                        );
        if (rmtProServerTimer == NULL)
        {
            DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "RmtProServer_TimerStart: failed to create timer", 0);
        }
        else
        {
            xTimerStart(rmtProServerTimer, 0);
        }
    }
    else
    {
        xTimerChangePeriod(rmtProServerTimer, timer_period / portTICK_PERIOD_MS, 0);
    }
}

static bool RmtProServer_ScanUUIDNumReport(uint16_t dst, uint8_t device_count)
{
    TMeshRmtProScanUUIDNumReport num_report;
    num_report.opcode[0] = (MESH_MSG_RMT_PRO_SCAN_UUID_NUM_REPORT >> 8) & 0xff;
    num_report.opcode[1] = MESH_MSG_RMT_PRO_SCAN_UUID_NUM_REPORT & 0xff;
    num_report.unpro_device_count = device_count;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProServer;
    mesh_msg.pbuffer = &num_report;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProScanUUIDNumReport);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProServer_ScanUUIDReport(uint16_t dst, uint8_t device_uuid[16], uint8_t device_id)
{
    TMeshRmtProScanUUIDReport uuid_report;
    uuid_report.opcode[0] = (MESH_MSG_RMT_PRO_SCAN_UUID_REPORT >> 8) & 0xff;
    uuid_report.opcode[1] = MESH_MSG_RMT_PRO_SCAN_UUID_REPORT & 0xff;
    uuid_report.unpro_device_id = device_id;
    memcpy(uuid_report.device_uuid, device_uuid, 16);

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProServer;
    mesh_msg.pbuffer = &uuid_report;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProScanUUIDReport);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProServer_ScanStatus(uint16_t dst, TMeshRmtProScanStatusEnum scan_status)
{
    TMeshRmtProScanStatus status;
    status.opcode[0] = (MESH_MSG_RMT_PRO_SCAN_STATUS >> 8) & 0xff;
    status.opcode[1] = MESH_MSG_RMT_PRO_SCAN_STATUS & 0xff;
    status.scan_status = scan_status;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProServer;
    mesh_msg.pbuffer = &status;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProScanStatus);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProServer_ScanStopped(uint16_t dst, TMeshRmtProScanStoppedStatus status)
{
    TMeshRmtProScanStopped scan_stopped;
    scan_stopped.opcode[0] = (MESH_MSG_RMT_PRO_SCAN_STOPPED >> 8) & 0xff;
    scan_stopped.opcode[1] = MESH_MSG_RMT_PRO_SCAN_STOPPED & 0xff;
    scan_stopped.status = status;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProServer;
    mesh_msg.pbuffer = &scan_stopped;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProScanStopped);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProServer_LinkStatus(uint16_t dst, TMeshRmtProLinkStatusEnum link_status,
                                    TMeshRmtProBearerType bearer_type)
{
    TMeshRmtProLinkStatus status;
    status.opcode[0] = (MESH_MSG_RMT_PRO_LINK_STATUS >> 8) & 0xff;
    status.opcode[1] = MESH_MSG_RMT_PRO_LINK_STATUS & 0xff;
    status.link_status = link_status;
    status.bearer_type = bearer_type;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProServer;
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

static bool RmtProServer_LinkStatusReport(uint16_t dst, TMeshRmtProLinkStatusEnum link_status,
                                          uint8_t reason)
{
    TMeshRmtProLinkStatusReport status_report;
    status_report.opcode[0] = (MESH_MSG_RMT_PRO_LINK_STATUS_REPORT >> 8) & 0xff;
    status_report.opcode[1] = MESH_MSG_RMT_PRO_LINK_STATUS_REPORT & 0xff;
    status_report.link_status = link_status;
    status_report.reason = reason;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProServer;
    mesh_msg.pbuffer = &status_report;
    mesh_msg.msg_offset = 0;
    mesh_msg.msg_len = sizeof(TMeshRmtProLinkStatusReport);
    if (0 == MeshMsgConfig(&mesh_msg))
    {
        mesh_msg.akf = 0;
        mesh_msg.dst = dst;
        return access_send(&mesh_msg);
    }

    return FALSE;
}

static bool RmtProServer_PktTransfer(uint16_t dst, uint8_t *pdata, uint16_t len)
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
    mesh_msg.pmodel_info = &rmtProServer;
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

static bool RmtProServer_PktTransferReport(uint16_t dst, TMeshRmtProDeliveryStatus delivery_status)
{
    TMeshRmtProPktTransferReport transfer_report;
    transfer_report.opcode[0] = MESH_MSG_RMT_PRO_PKT_TRANSFER_REPORT;
    //transfer_report.opcode[0] = (MESH_MSG_RMT_PRO_PKT_TRANSFER_REPORT >> 8) & 0xff;
    //transfer_report.opcode[1] = MESH_MSG_RMT_PRO_PKT_TRANSFER_REPORT & 0xff;
    transfer_report.delivery_status = delivery_status;

    // send
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &rmtProServer;
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
    mesh_msg.pmodel_info = &rmtProServer;
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

uint8_t RmtProServer_UnproDeviceFind(uint8_t device_uuid[16])
{
    uint8_t loop;

    for (loop = 0; loop < unproDeviceInfoSP; loop++)
    {
        if (memcmp(device_uuid, unproDeviceInfo[loop].device_uuid, 16) == 0)
        {
            return loop;
        }
    }

    return MESH_RMT_PRO_SERVER_UNPRO_DEVICE_NUM;
}

bool RmtProServer_UnproDeviceFilter(uint8_t device_uuid[16])
{
    //DBG_BUFFER(MODULE_APP, LEVEL_INFO, "RmtProServer_UnproDeviceFilter: beacon received!", 0);
    if (rmtProServerScanState == MESH_RMT_PRO_SERVER_SCAN_STATE_IDLE)
    {
        return FALSE;
    }
    else if (rmtProServerScanState == MESH_RMT_PRO_SERVER_SCAN_STATE_FILTERED_SCANNING)
    {
        if (filterScanFound)
        {
            return FALSE;
        }
        else
        {
            if (memcmp(device_uuid, unproDeviceInfo[0].device_uuid, 16) == 0)
            {
                filterScanFound = TRUE;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
        if (MESH_RMT_PRO_SERVER_UNPRO_DEVICE_NUM == RmtProServer_UnproDeviceFind(device_uuid))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

uint8_t RmtProServer_UnproDeviceAdd(uint8_t device_uuid[16], uint8_t bt_addr[6])
{
    uint8_t id = MESH_RMT_PRO_SERVER_UNPRO_DEVICE_NUM;
    if (unproDeviceInfoSP < MESH_RMT_PRO_SERVER_UNPRO_DEVICE_NUM)
    {
        memcpy(unproDeviceInfo[unproDeviceInfoSP].device_uuid, device_uuid, 16);
        memcpy(unproDeviceInfo[unproDeviceInfoSP].bt_addr, bt_addr, 6);
        id = unproDeviceInfoSP;
        unproDeviceInfoSP += 1;
        if (unproDeviceInfoSP == MESH_RMT_PRO_SERVER_UNPRO_DEVICE_NUM)
        {
            // list is full
            // todo
        }
    }
    DBG_BUFFER(MODULE_APP, LEVEL_INFO, "RmtProServer_UnproDeviceAdd: new devie id = %d!", 1, id);
    return id;
}

void RmtProServer_Clear(void)
{
    unproDeviceInfoSP = 0;
    rmtProServerScanState = MESH_RMT_PRO_SERVER_SCAN_STATE_IDLE;
    //clear timer
    if (rmtProServerTimer != NULL)
    {
        xTimerDelete(rmtProServerTimer, 0);
        rmtProServerTimer = NULL;
    }
}

void RmtProServer_HandleUDB(beacon_udb_p pbeacon, uint8_t bt_addr[6])
{
    switch (rmtProServerScanState)
    {
    case MESH_RMT_PRO_SERVER_SCAN_STATE_IDLE:
        break;
    case MESH_RMT_PRO_SERVER_SCAN_STATE_GENERAL_SCANNING:
        {
            uint8_t id;
            id = RmtProServer_UnproDeviceAdd(pbeacon->device_uuid, bt_addr);
            if (id != MESH_RMT_PRO_SERVER_UNPRO_DEVICE_NUM)
            {
                RmtProServer_ScanUUIDReport(clientAddr, pbeacon->device_uuid, id);
            }
        }
        break;
    case MESH_RMT_PRO_SERVER_SCAN_STATE_FILTERED_SCANNING:
        {
            memcpy(unproDeviceInfo[0].bt_addr, bt_addr, 6);
            RmtProServer_ScanUUIDReport(clientAddr, pbeacon->device_uuid, 0);
        }
        break;
    case MESH_RMT_PRO_SERVER_SCAN_STATE_REPORT_NUM_SCANNING:
        {
            RmtProServer_UnproDeviceAdd(pbeacon->device_uuid, bt_addr);
        }
        break;
    default:
        break;
    }
}

void RmtProServer_HandleScanStart(uint16_t client_addr)
{
    if (rmtProServerScanState == MESH_RMT_PRO_SERVER_SCAN_STATE_IDLE)
    {
        rmtProServerScanState = MESH_RMT_PRO_SERVER_SCAN_STATE_GENERAL_SCANNING;
        rmtProServerProcedureState = MESH_RMT_PRO_SERVER_SCAN;
        clientAddr = client_addr;
        RmtProServer_ScanStatus(client_addr, MESH_RMT_PRO_SCAN_STATUS_STARTED);
    }
    else
    {
        RmtProServer_ScanStatus(client_addr, MESH_RMT_PRO_SCAN_STATUS_CANNOT_START);
    }
}

void RmtProServer_HandleScanStartWithFilter(uint16_t client_addr,
                                            PMeshRmtProScanStartWithFilter pscan_start)
{
    if (rmtProServerScanState == MESH_RMT_PRO_SERVER_SCAN_STATE_IDLE)
    {
        rmtProServerScanState = MESH_RMT_PRO_SERVER_SCAN_STATE_FILTERED_SCANNING;
        rmtProServerProcedureState = MESH_RMT_PRO_SERVER_SCAN;
        clientAddr = client_addr;
        filterScanFound = FALSE;
        // there is no bt addr info here
        RmtProServer_UnproDeviceAdd(pscan_start->device_uuid, pscan_start->device_uuid);
        RmtProServer_ScanStatus(client_addr, MESH_RMT_PRO_SCAN_STATUS_STARTED);
    }
    else
    {
        RmtProServer_ScanStatus(client_addr, MESH_RMT_PRO_SCAN_STATUS_CANNOT_START);
    }
}

void RmtProServer_HandleScanUnproDeviceNum(uint16_t client_addr, uint8_t report_num)
{
    if (rmtProServerScanState == MESH_RMT_PRO_SERVER_SCAN_STATE_IDLE)
    {
        rmtProServerScanState = MESH_RMT_PRO_SERVER_SCAN_STATE_REPORT_NUM_SCANNING;
        rmtProServerProcedureState = MESH_RMT_PRO_SERVER_SCAN;
        clientAddr = client_addr;
        reportNum = report_num;
        RmtProServer_ScanStatus(client_addr, MESH_RMT_PRO_SCAN_STATUS_STARTED);
    }
    else
    {
        RmtProServer_ScanStatus(client_addr, MESH_RMT_PRO_SCAN_STATUS_CANNOT_START);
    }
}

void RmtProServer_HandleLinkOpen(uint16_t client_addr, uint8_t id)
{
    if (rmtProServerLinkState == MESH_RMT_PRO_SERVER_LINK_STATE_IDLE)
    {
        if (id < unproDeviceInfoSP)
        {
            rmtProServerLinkState = MESH_RMT_PRO_SERVER_LINK_STATE_OPENING;
            rmtProServerProcedureState = MESH_RMT_PRO_SERVER_LINK_OPEN;
            clientAddr = client_addr;
            RmtProServer_LinkStatus(client_addr, MESH_RMT_PRO_LINK_STATUS_OPENING,
                                    MESH_RMT_PRO_BEARER_TYPE_PB_ADV);
            // todo
        }
        else
        {
            RmtProServer_LinkStatus(client_addr, MESH_RMT_PRO_LINK_STATUS_INVALID_UNPRO_DEVCIE_ID,
                                    MESH_RMT_PRO_BEARER_TYPE_PB_ADV);
        }
    }
    else
    {
        RmtProServer_LinkStatus(client_addr, MESH_RMT_PRO_LINK_STATUS_ALREADY_OPEN,
                                MESH_RMT_PRO_BEARER_TYPE_PB_ADV);
    }
}

void RmtProServer_HandleLinkClose(uint16_t client_addr, uint8_t reason)
{
    if (rmtProServerLinkState == MESH_RMT_PRO_SERVER_LINK_STATE_IDLE)
    {
        RmtProServer_LinkStatus(client_addr, MESH_RMT_PRO_LINK_STATUS_NOT_ACTIVE,
                                MESH_RMT_PRO_BEARER_TYPE_PB_ADV);
    }
    else
    {
        // todo
        RmtProServer_LinkStatus(client_addr, MESH_RMT_PRO_LINK_STATUS_INVALID_UNPRO_DEVCIE_ID,
                                MESH_RMT_PRO_BEARER_TYPE_PB_ADV);
        // todo
    }
}

void RmtProServer_HandlePktTransfer(uint16_t client_addr, uint8_t *pbuffer, uint16_t len)
{
    if (rmtProServerLinkState == MESH_RMT_PRO_SERVER_LINK_STATE_OPENED)
    {
        RmtProClient_PktTransferStatus(client_addr, MESH_RMT_PRO_TRANSFER_STATUS_BUFFER_ACCEPTED);
        // todo
    }
    else
    {
        RmtProClient_PktTransferStatus(client_addr, MESH_RMT_PRO_TRANSFER_STATUS_LINK_NOT_ACTIVE);
    }
}

void RmtProServer_HandleTimerTimeout(void)
{

}

bool RmtProServer_ReceiveMsg(uint32_t opcode, mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    switch (opcode)
    {
    case MESH_MSG_RMT_PRO_SCAN_START:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProScanStart))
        {
            RmtProServer_HandleScanStart(pmesh_msg->src);
        }
        break;
    case MESH_MSG_RMT_PRO_SCAN_START_WITH_FILTER:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProScanStartWithFilter))
        {
            RmtProServer_HandleScanStartWithFilter(pmesh_msg->src, (PMeshRmtProScanStartWithFilter)pbuffer);
        }
        break;
    case MESH_MSG_RMT_PRO_SCAN_UNPRO_DEVICE_NUM:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProScanUnproDeviceNum))
        {
            RmtProServer_HandleScanUnproDeviceNum(pmesh_msg->src,
                                                  ((PMeshRmtProScanUnproDeviceNum)pbuffer)->report_num);
        }
        break;
    case MESH_MSG_RMT_PRO_SCAN_REPORT_STATUS:
        break;
    case MESH_MSG_RMT_PRO_SCAN_CANCEL:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProScanCancel))
        {
            if (rmtProServerScanState == MESH_RMT_PRO_SERVER_SCAN_STATE_IDLE)
            {
                RmtProServer_ScanStatus(pmesh_msg->src, MESH_RMT_PRO_SCAN_STATUS_CANNOT_CANCEL);
            }
            else
            {
                RmtProServer_ScanStatus(pmesh_msg->src, MESH_RMT_PRO_SCAN_STATUS_CANCELED);
            }
        }
        break;
    case MESH_MSG_RMT_PRO_LINK_OPEN:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProLinkOpen))
        {
            RmtProServer_HandleLinkOpen(pmesh_msg->src, ((PMeshRmtProLinkOpen)pbuffer)->unpro_device_id);
        }
        break;
    case MESH_MSG_RMT_PRO_LINK_CLOSE:
        if (pmesh_msg->msg_len == sizeof(TMeshRmtProLinkClose))
        {
            RmtProServer_HandleLinkClose(pmesh_msg->src, ((PMeshRmtProLinkClose)pbuffer)->reason);
        }
        break;
    case MESH_MSG_RMT_PRO_LINK_STATUS:
        break;
    case MESH_MSG_RMT_PRO_PKT_TRANSFER:
        {
            PMeshRmtProPktTransfer ppkt_transfer = (PMeshRmtProPktTransfer)pbuffer;
            RmtProServer_HandlePktTransfer(pmesh_msg->src, ppkt_transfer->buffer,
                                           pmesh_msg->msg_len - 5); // todo: - 1
        }
        break;
    case MESH_MSG_RMT_PRO_PKT_TRANSFER_STATUS:
        break;
    default:
        ret = FALSE;
        break;
    }

    return ret;
}

void RmtProServer_AddModel(void)
{
    rmtProServer.model_id = MESH_MODEL_RMT_PROVISIONING_SERVER;
    rmtProServer.model_receive = RmtProServer_ReceiveMsg;
    mesh_model_reg(0, &rmtProServer);
}
