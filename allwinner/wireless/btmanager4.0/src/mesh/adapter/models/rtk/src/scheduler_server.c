/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     scheduler_server.c
  * @brief    Source file for scheduler server model.
  * @details  Data types and external functions declaration.
  * @author   hector
  * @date     2018-9-14
  * @version  v1.0
  * *************************************************************************************
  */

#include "scheduler.h"
#include "mesh_access.h"
#include "model_common.h"

static mesh_msg_send_cause_t scheduler_server_send(mesh_model_info_p pmodel_info,
                                                   uint16_t dst, void *pmsg, uint16_t msg_len, uint8_t app_key_index)
{
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = pmodel_info;
    access_cfg(&mesh_msg);
    mesh_msg.pbuffer = pmsg;
    mesh_msg.msg_len = msg_len;
    if (0 != dst)
    {
        mesh_msg.dst = dst;
        mesh_msg.app_key_index = app_key_index;
    }
    return access_send(&mesh_msg);
}

mesh_msg_send_cause_t scheduler_status(mesh_model_info_p pmodel_info, uint16_t dst,
                                       uint8_t app_key_index, uint16_t schedulers)
{
    scheduler_status_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_SCHEDULER_STATUS);
    msg.schedulers = schedulers;

    return scheduler_server_send(pmodel_info, dst, &msg, sizeof(msg), app_key_index);
}

mesh_msg_send_cause_t scheduler_action_status(mesh_model_info_p pmodel_info, uint16_t dst,
                                              uint8_t app_key_index, scheduler_register_t scheduler)
{
    scheduler_action_status_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_SCHEDULER_ACTION_STATUS);
    msg.scheduler = scheduler;

    return scheduler_server_send(pmodel_info, dst, &msg, sizeof(msg), app_key_index);
}

mesh_msg_send_cause_t scheduler_publish(const mesh_model_info_p pmodel_info,
                                        scheduler_register_t scheduler)
{
    mesh_msg_send_cause_t ret = MESH_MSG_SEND_CAUSE_INVALID_DST;
    if (mesh_model_pub_check(pmodel_info))
    {
        ret = scheduler_action_status(pmodel_info, 0, 0, scheduler);
    }

    return ret;
}

static bool scheduler_server_receive(mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    mesh_model_info_p pmodel_info = pmesh_msg->pmodel_info;

    switch (pmesh_msg->access_opcode)
    {
    case MESH_MSG_SCHEDULER_GET:
        if (pmesh_msg->msg_len == sizeof(scheduler_get_t))
        {
            scheduler_server_get_t get_data = {0};
            if (NULL != pmodel_info->model_data_cb)
            {
                pmodel_info->model_data_cb(pmodel_info, SCHEDULER_SERVER_GET, &get_data);
            }
            scheduler_status(pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index, get_data.schedulers);
        }
        break;
    case MESH_MSG_SCHEDULER_ACTION_GET:
        if (pmesh_msg->msg_len == sizeof(scheduler_action_get_t))
        {
            scheduler_action_get_t *pmsg = (scheduler_action_get_t *)pbuffer;
            if (IS_SCHEDULER_INDEX_VALID(pmsg->index))
            {
                scheduler_server_get_action_t get_data;
                memset(&get_data, 0, sizeof(get_data));
                if (NULL != pmodel_info->model_data_cb)
                {
                    get_data.index = pmsg->index;
                    pmodel_info->model_data_cb(pmodel_info, SCHEDULER_SERVER_GET_ACTION, &get_data);
                }
                scheduler_action_status(pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index, get_data.scheduler);
            }
        }
        break;
    default:
        ret = FALSE;
        break;
    }
    return ret;
}

static bool is_model_opcode(uint16_t opcode)
{
   bool ret = false;
    l_info("%s,%d\n",__FILE__,opcode);
    switch (opcode)
    {
        case MESH_MSG_SCHEDULER_GET:
        case MESH_MSG_SCHEDULER_ACTION_GET:
            ret = true;
            break;
        default:
            break;
    }
   return ret;
}

bool scheduler_server_reg(uint8_t element_index, mesh_model_info_p pmodel_info)
{
    if (NULL == pmodel_info)
    {
        return FALSE;
    }
    pmodel_info->model_id = RTK_MESH_MODEL_SCHEDULER_SERVER;
    if (NULL == pmodel_info->model_receive)
    {
        pmodel_info->model_receive = scheduler_server_receive;
        if (NULL == pmodel_info->model_data_cb)
        {
            printw("scheduler_server_reg: missing model data process callback!");
        }
    }
    pmodel_info->is_opcode_of_model = &is_model_opcode;
    return mesh_model_reg(element_index, pmodel_info);
}
