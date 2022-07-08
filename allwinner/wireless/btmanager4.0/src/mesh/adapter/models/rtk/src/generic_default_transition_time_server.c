enum { __FILE_NUM__ = 0 };

/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     generic_default_transition_time_server.c
  * @brief    Source file for generic default transition time server model.
  * @details  Data types and external functions declaration.
  * @author   hector
  * @date     2018-7-9
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include "generic_default_transition_time.h"

/**
 * @brief replay generic default transition time status
 * @param[in] pmodel_info: gdtt server model information
 * @param[in] dst: remote address
 * @param[in] app_key_index: mesh message used app key index
 * @param[in] trans_time: curent transition time
 * @return send status
 */
static mesh_msg_send_cause_t generic_default_transition_time_stat(mesh_model_info_p pmodel_info,
                                                                  uint16_t dst, uint8_t app_key_index,
                                                                  generic_transition_time_t trans_time)
{
    generic_default_transition_time_stat_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_GENERIC_DEFAULT_TRANSITION_TIME_STAT);
    msg.trans_time = trans_time;

    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = pmodel_info;
    access_cfg(&mesh_msg);
    mesh_msg.pbuffer = (uint8_t *)&msg;
    mesh_msg.msg_len = sizeof(generic_default_transition_time_stat_t);
    if (0 != dst)
    {
        mesh_msg.dst = dst;
        mesh_msg.app_key_index = app_key_index;
        return access_send(&mesh_msg);
    }
    else
    {
        return access_publish(&mesh_msg);
    }

}

mesh_msg_send_cause_t generic_default_transition_time_publish(const mesh_model_info_p pmodel_info,
                                                              generic_transition_time_t trans_time)
{
    mesh_msg_send_cause_t ret = MESH_MSG_SEND_CAUSE_INVALID_DST;
    if (mesh_model_pub_check(pmodel_info))
    {
        ret = generic_default_transition_time_stat(pmodel_info, 0, 0, trans_time);
    }

    return ret;
}


/**
 * @brief generic default transition time receive callback
 * @param[in] pmesh_msg: received mesh message
 * @return process status
 */
static bool generic_default_transition_time_server_receive(mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    l_info("%s,%d,%d,%d\n",__FUNCTION__,pmesh_msg->access_opcode,pmesh_msg->msg_len,pmesh_msg->src);
    switch (pmesh_msg->access_opcode)
    {
    case MESH_MSG_GENERIC_DEFAULT_TRANSITION_TIME_GET:
        if (pmesh_msg->msg_len == sizeof(generic_default_transition_time_get_t))
        {
            generic_default_transition_time_server_get_t get_data = {{0, 0}};
            if (NULL != pmesh_msg->pmodel_info->model_data_cb)
            {
                pmesh_msg->pmodel_info->model_data_cb(pmesh_msg->pmodel_info,
                                                      GENERIC_DEFAULT_TRANSITION_TIME_SERVER_GET, &get_data);
            }
            generic_default_transition_time_stat(pmesh_msg->pmodel_info, pmesh_msg->src,
                                                 pmesh_msg->app_key_index, get_data.trans_time);
        }
        break;
    case MESH_MSG_GENERIC_DEFAULT_TRANSITION_TIME_SET:
    case MESH_MSG_GENERIC_DEFAULT_TRANSITION_TIME_SET_UNACK:
        if (pmesh_msg->msg_len == sizeof(generic_default_transition_time_set_t))
        {
            generic_default_transition_time_set_t *pmsg = (generic_default_transition_time_set_t *)pbuffer;
            if (IS_GENERIC_TRANSITION_STEPS_VALID(pmsg->trans_time.num_steps))
            {
                generic_default_transition_time_server_set_t get_data = {{0, 0}};
                if (NULL != pmesh_msg->pmodel_info->model_data_cb)
                {
                    generic_default_transition_time_server_set_t set_data;
                    set_data.trans_time = pmsg->trans_time;
                    pmesh_msg->pmodel_info->model_data_cb(pmesh_msg->pmodel_info,
                                                          GENERIC_DEFAULT_TRANSITION_TIME_SERVER_GET, &get_data);
                    pmesh_msg->pmodel_info->model_data_cb(pmesh_msg->pmodel_info,
                                                          GENERIC_DEFAULT_TRANSITION_TIME_SERVER_SET, &set_data);
                }
                if (MESH_MSG_GENERIC_DEFAULT_TRANSITION_TIME_SET == pmesh_msg->access_opcode)
                {
                    generic_default_transition_time_stat(pmesh_msg->pmodel_info, pmesh_msg->src,
                                                         pmesh_msg->app_key_index, pmsg->trans_time);
                }

                if ((get_data.trans_time.num_steps != pmsg->trans_time.num_steps) ||
                    (get_data.trans_time.step_resolution != pmsg->trans_time.step_resolution))
                {
                    /* state change publish */
                    l_info("state change publish");
                    generic_default_transition_time_publish(pmesh_msg->pmodel_info, pmsg->trans_time);
                }
                else
                {
                    l_info("state no change no publish");
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
    l_info("%s,%d\n",__FUNCTION__,opcode);
    switch (opcode)
    {
        case MESH_MSG_GENERIC_DEFAULT_TRANSITION_TIME_GET:
        case MESH_MSG_GENERIC_DEFAULT_TRANSITION_TIME_SET:
        case MESH_MSG_GENERIC_DEFAULT_TRANSITION_TIME_SET_UNACK:
            ret = true;
            break;
        default:
            break;
    }
   return ret;
}

bool generic_default_transition_time_server_reg(uint8_t element_index,
                                                mesh_model_info_p pmodel_info)
{

    //l_info("%s,%p,%p,%p,%p\n",__FUNCTION__,pmodel_info,pmodel_info->model_receive,pmodel_info->model_data_cb,pmodel_info->is_opcode_of_model);

    if (NULL == pmodel_info)
    {
        return FALSE;
    }

    pmodel_info->model_id = MESH_MODEL_GENERIC_DEFAULT_TRANSITION_TIME_SERVER;
    if (NULL == pmodel_info->model_receive)
    {
        pmodel_info->model_receive = generic_default_transition_time_server_receive;
        if (NULL == pmodel_info->model_data_cb)
        {
            printw("generic_default_transition_time_server_reg: missing data process callback!");
        }
        pmodel_info->is_opcode_of_model = &is_model_opcode;
    }


    return mesh_model_reg(element_index, pmodel_info);
}
