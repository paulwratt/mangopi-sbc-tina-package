enum { __FILE_NUM__ = 0 };

/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     generic_power_level_setup_server.c
  * @brief    Source file for generic on off server model.
  * @details  Data types and external functions declaration.
  * @author   hector
  * @date     2018-7-27
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include "generic_power_level.h"

static bool generic_power_level_setup_server_receive(mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;

    switch (pmesh_msg->access_opcode)
    {
    case MESH_MSG_GENERIC_POWER_DEFAULT_SET:
    case MESH_MSG_GENERIC_POWER_DEFAULT_SET_UNACK:
        if (pmesh_msg->msg_len == sizeof(generic_power_default_set_t))
        {
            generic_power_default_set_p pmsg = (generic_power_default_set_p)pbuffer;
            generic_power_level_server_set_default_t set_default = {pmsg->power};
            if (NULL != pmesh_msg->pmodel_info->model_data_cb)
            {
                pmesh_msg->pmodel_info->model_data_cb(pmesh_msg->pmodel_info,
                                                      GENERIC_POWER_LEVEL_SERVER_SET_DEFAULT, &set_default);
            }
            if (MESH_MSG_GENERIC_POWER_DEFAULT_SET == pmesh_msg->access_opcode)
            {
                generic_power_default_stat(pmesh_msg->pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                           pmsg->power);
            }
        }
        break;
    case MESH_MSG_GENERIC_POWER_RANGE_SET:
    case MESH_MSG_GENERIC_POWER_RANGE_SET_UNACK:
        if (pmesh_msg->msg_len == sizeof(generic_power_range_set_t))
        {
            generic_power_range_set_p pmsg = (generic_power_range_set_p)pbuffer;
            if ((pmsg->range_min <= pmsg->range_max) &&
                (0 != pmsg->range_min) &&
                (0 != pmsg->range_max))
            {
                generic_power_level_server_set_range_t set_range = {pmsg->range_min, pmsg->range_max};
                if (NULL != pmesh_msg->pmodel_info->model_data_cb)
                {
                    pmesh_msg->pmodel_info->model_data_cb(pmesh_msg->pmodel_info, GENERIC_POWER_LEVEL_SERVER_SET_RANGE,
                                                          &set_range);
                }
                if (MESH_MSG_GENERIC_POWER_RANGE_SET == pmesh_msg->access_opcode)
                {
                    generic_power_range_stat(pmesh_msg->pmodel_info, pmesh_msg->src,
                                             pmesh_msg->app_key_index, GENERIC_STAT_SUCCESS,
                                             pmsg->range_min, pmsg->range_max);
                }
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
    //l_info("%s,%d\n",__FILE__,opcode);
    switch (opcode)
    {
        case MESH_MSG_GENERIC_POWER_DEFAULT_SET:
        case MESH_MSG_GENERIC_POWER_DEFAULT_SET_UNACK:
        case MESH_MSG_GENERIC_POWER_RANGE_SET:
        case MESH_MSG_GENERIC_POWER_RANGE_SET_UNACK:
            ret = true;
            break;
        default:
            break;
    }
   return ret;
}

bool generic_power_level_setup_server_reg(uint8_t element_index, mesh_model_info_p pmodel_info)
{
    if (NULL == pmodel_info)
    {
        return FALSE;
    }

    pmodel_info->model_id = RTK_MESH_MODEL_GENERIC_POWER_LEVEL_SETUP_SERVER;
    if (NULL == pmodel_info->model_receive)
    {
        pmodel_info->model_receive = generic_power_level_setup_server_receive;
        if (NULL == pmodel_info->model_data_cb)
        {
            printw("generic_power_level_setup_server_reg: missing model data process callback!");
        }
    }
    pmodel_info->is_opcode_of_model = &is_model_opcode;
    return mesh_model_reg(element_index, pmodel_info);
}
