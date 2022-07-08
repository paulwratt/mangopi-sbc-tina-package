enum { __FILE_NUM__ = 0 };

/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     light_hsl_setup_server.c
  * @brief    Source file for light hsl setup server model.
  * @details  Data types and external functions declaration.
  * @author   hector
  * @date     2018-8-1
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include "light_hsl.h"

static bool light_hsl_setup_server_receive(mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    switch (pmesh_msg->access_opcode)
    {
    case MESH_MSG_LIGHT_HSL_DEFAULT_SET:
    case MESH_MSG_LIGHT_HSL_DEFAULT_SET_UNACK:
        if (pmesh_msg->msg_len == sizeof(light_hsl_default_set_t))
        {
            light_hsl_default_set_t *pmsg = (light_hsl_default_set_t *)pbuffer;
            light_hsl_server_set_default_t set_default = {pmsg->lightness, pmsg->hue, pmsg->saturation};
            if (NULL != pmesh_msg->pmodel_info->model_data_cb)
            {
                pmesh_msg->pmodel_info->model_data_cb(pmesh_msg->pmodel_info, LIGHT_HSL_SERVER_SET_DEFAULT,
                                                      &set_default);
            }
            if (MESH_MSG_LIGHT_HSL_DEFAULT_SET == pmesh_msg->access_opcode)
            {
                light_hsl_default_stat(pmesh_msg->pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                       pmsg->lightness,
                                       pmsg->hue, pmsg->saturation);
            }
        }
        break;
    case MESH_MSG_LIGHT_HSL_RANGE_SET:
    case MESH_MSG_LIGHT_HSL_RANGE_SET_UNACK:
        if (pmesh_msg->msg_len == sizeof(light_hsl_range_set_t))
        {
            light_hsl_range_set_t *pmsg = (light_hsl_range_set_t *)pbuffer;
            if ((pmsg->hue_range_min <= pmsg->hue_range_max) &&
                (pmsg->saturation_range_min <= pmsg->saturation_range_max))
            {
                light_hsl_server_set_range_t set_range = {pmsg->hue_range_min, pmsg->hue_range_max,
                                                          pmsg->saturation_range_min,
                                                          pmsg->saturation_range_max
                                                         };
                if (NULL != pmesh_msg->pmodel_info->model_data_cb)
                {
                    pmesh_msg->pmodel_info->model_data_cb(pmesh_msg->pmodel_info, LIGHT_HSL_SERVER_SET_RANGE,
                                                          &set_range);
                }
                if (MESH_MSG_LIGHT_HSL_RANGE_SET == pmesh_msg->access_opcode)
                {
                    light_hsl_range_stat(pmesh_msg->pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                         GENERIC_STAT_SUCCESS,
                                         pmsg->hue_range_min, pmsg->hue_range_max,
                                         pmsg->saturation_range_min, pmsg->saturation_range_max);
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
        case MESH_MSG_LIGHT_HSL_DEFAULT_SET:
        case MESH_MSG_LIGHT_HSL_DEFAULT_SET_UNACK:
        case MESH_MSG_LIGHT_HSL_RANGE_SET:
        case MESH_MSG_LIGHT_HSL_RANGE_SET_UNACK:
        ret = true;
        break;
        default:
            break;
    }
   return ret;
}

bool light_hsl_setup_server_reg(uint8_t element_index, mesh_model_info_p pmodel_info)
{
    if (NULL == pmodel_info)
    {
        return FALSE;
    }

    pmodel_info->model_id = RTK_MESH_MODEL_LIGHT_HSL_SETUP_SERVER;
    if (NULL == pmodel_info->model_receive)
    {
        pmodel_info->model_receive = light_hsl_setup_server_receive;
        if (NULL == pmodel_info->model_data_cb)
        {
            printw("light_hsl_setup_server_reg: missing model data process callback!");
        }
    }
    pmodel_info->is_opcode_of_model = &is_model_opcode;
    return mesh_model_reg(element_index, pmodel_info);
}
