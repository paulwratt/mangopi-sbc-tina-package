enum { __FILE_NUM__ = 0 };

/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     generic_on_off_server.c
  * @brief    Source file for generic on off server model.
  * @details  Data types and external functions declaration.
  * @author   hector
  * @date     2018-7-11
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include "generic_on_off.h"


mesh_msg_send_cause_t generic_on_off_stat(mesh_model_info_p pmodel_info, uint16_t dst,
                                          uint8_t app_key_index, generic_on_off_t present_on_off, bool optional,
                                          generic_on_off_t target_on_off, generic_transition_time_t remaining_time)
{
    generic_on_off_stat_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_GENERIC_ON_OFF_STAT);
    uint16_t msg_len;

    if (optional)
    {
        msg_len = sizeof(generic_on_off_stat_t);
        msg.target_on_off = target_on_off;
        msg.remaining_time = remaining_time;
    }
    else
    {
        msg_len = MEMBER_OFFSET(generic_on_off_stat_t, target_on_off);
    }

    msg.present_on_off = present_on_off;

    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = pmodel_info;
    access_cfg(&mesh_msg);
    mesh_msg.pbuffer = (uint8_t *)&msg;
    mesh_msg.msg_len = msg_len;
    if (0 != dst)
    {
        mesh_msg.dst = dst;
        mesh_msg.app_key_index = app_key_index;
    }
    if((dst == 0)&&(app_key_index == 0))
    {
        return access_publish(&mesh_msg);
    }
    else
    {
        return access_send(&mesh_msg);
    }
}

mesh_msg_send_cause_t generic_on_off_publish(const mesh_model_info_p pmodel_info,
                                             generic_on_off_t on_off)
{
    mesh_msg_send_cause_t ret = MESH_MSG_SEND_CAUSE_INVALID_DST;
    if (mesh_model_pub_check(pmodel_info))
    {
        generic_transition_time_t trans_time = {0, 0};
        ret = generic_on_off_stat(pmodel_info, 0, 0, on_off, FALSE, on_off, trans_time);
    }

    return ret;
}

static generic_on_off_t get_present_on_off(const mesh_model_info_p pmodel_info)
{
    generic_on_off_server_get_t get_data = {GENERIC_OFF};
    if (NULL != pmodel_info->model_data_cb)
    {
        pmodel_info->model_data_cb(pmodel_info, GENERIC_ON_OFF_SERVER_GET, &get_data);
    }

    return get_data.on_off;
}

static int32_t generic_on_off_trans_step_change(const mesh_model_info_p pmodel_info, uint32_t type,
                                                generic_transition_time_t total_time,
                                                generic_transition_time_t remaining_time)
{
    int32_t ret = MODEL_SUCCESS;
    generic_on_off_server_set_t set_data;
    generic_on_off_t *ptarget = pmodel_info->pargs;
    set_data.on_off = *ptarget;
    set_data.total_time = total_time;
    set_data.remaining_time = remaining_time;
    if (NULL != pmodel_info->model_data_cb)
    {
        ret = pmodel_info->model_data_cb(pmodel_info, GENERIC_ON_OFF_SERVER_SET, &set_data);
    }

    if (0 == remaining_time.num_steps)
    {
        generic_on_off_t present_on_off = get_present_on_off(pmodel_info);
        generic_on_off_publish(pmodel_info, present_on_off);
    }

    return ret;
}

static generic_on_off_t generic_on_off_process(const mesh_model_info_p pmodel_info,
                                               generic_on_off_t target_on_off,
                                               generic_transition_time_t trans_time)
{
    generic_on_off_t on_off_before_set;
    generic_on_off_t on_off_after_set;

    /* get generic on/off before set */
    on_off_before_set = get_present_on_off(pmodel_info);

    /** TODO: add delay execution */
    int32_t ret = MODEL_SUCCESS;
    generic_on_off_server_set_t trans_set_data;
    trans_set_data.total_time = trans_time;
    trans_set_data.remaining_time = trans_time;
    trans_set_data.on_off = target_on_off;

    if (NULL != pmodel_info->model_data_cb)
    {
        ret = pmodel_info->model_data_cb(pmodel_info, GENERIC_ON_OFF_SERVER_SET, &trans_set_data);
    }

    if (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE != trans_time.num_steps)
    {
        if ((ret >= 0) && (MODEL_STOP_TRANSITION != ret))
        {
            generic_transition_timer_start(pmodel_info, GENERIC_TRANSITION_TYPE_ON_OFF, trans_time,
                                           generic_on_off_trans_step_change);
        }
#if ENABLE_USER_STOP_TRANSITION_NOTIFICATION
        else if (MODEL_STOP_TRANSITION == ret)
        {
            if (NULL != pmodel_info->model_data_cb)
            {
                ret = pmodel_info->model_data_cb(pmodel_info, GENERIC_ON_OFF_SERVER_SET, &trans_set_data);
            }
        }
#endif
    }

    /* get on/off after set */
    on_off_after_set = get_present_on_off(pmodel_info);

    if (on_off_before_set != on_off_after_set)
    {
        generic_on_off_publish(pmodel_info, on_off_after_set);
    }

    return on_off_after_set;
}

/**
 * @brief default generic on/off server receive function
 * @param[in] pmesh_msg: received mesh message
 * @return process result
 */
static bool generic_on_off_server_receive(mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    //l_info("%s opcode %x pmodel_info %p pmsg %p\n",__func__,pmesh_msg->access_opcode,pmesh_msg->pmodel_info,pmesh_msg->pmodel_info->pmsg);
    switch (pmesh_msg->access_opcode)
    {
    case MESH_MSG_GENERIC_ON_OFF_GET:
        if (pmesh_msg->msg_len == sizeof(generic_on_off_get_t))
        {
            mesh_model_info_p pmodel_info = pmesh_msg->pmodel_info;
            /* get present on/off status */
            generic_on_off_t present_on_off = get_present_on_off(pmodel_info);
            /* get target on/off status*/
            generic_on_off_t *ptarget = pmodel_info->pargs;
            /* get remaining time */
            generic_transition_time_t remaining_time = generic_transition_time_get(pmesh_msg->pmodel_info,
                                                                                   GENERIC_TRANSITION_TYPE_ON_OFF);
            generic_on_off_stat(pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                present_on_off,
                                (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE == remaining_time.num_steps) ? FALSE : TRUE,
                                *ptarget, remaining_time);
        }
        break;
    case MESH_MSG_GENERIC_ON_OFF_SET:
    case MESH_MSG_GENERIC_ON_OFF_SET_UNACK:
        {
            generic_on_off_set_t *pmsg = (generic_on_off_set_t *)pbuffer;
            generic_transition_time_t trans_time = {GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE, 0};
            mesh_model_info_p pmodel_info = pmesh_msg->pmodel_info;
            if (pmesh_msg->msg_len == MEMBER_OFFSET(generic_on_off_set_t, trans_time))
            {
                if (NULL != pmodel_info->model_data_cb)
                {
                    pmodel_info->model_data_cb(pmodel_info, GENERIC_ON_OFF_SERVER_GET_DEFAULT_TRANSITION_TIME,
                                               &trans_time);
                }
            }
            else if (pmesh_msg->msg_len == sizeof(generic_on_off_set_t))
            {
                    trans_time = pmsg->trans_time;
            }
            if (IS_GENERIC_ON_OFF_VALID(pmsg->on_off) &&
                IS_GENERIC_TRANSITION_STEPS_VALID(trans_time.num_steps))
            {
                generic_on_off_t *ptarget = pmodel_info->pargs;
                *ptarget = pmsg->on_off;

                generic_on_off_t present_on_off = generic_on_off_process(pmodel_info, *ptarget,
                                                                         trans_time);

                if (pmesh_msg->access_opcode == MESH_MSG_GENERIC_ON_OFF_SET)
                {
                    generic_on_off_stat(pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                        present_on_off,
                                        (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE == trans_time.num_steps) ? FALSE : TRUE,
                                        *ptarget, trans_time);
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
        case MESH_MSG_GENERIC_ON_OFF_GET:
        case MESH_MSG_GENERIC_ON_OFF_SET:
        case MESH_MSG_GENERIC_ON_OFF_SET_UNACK:
            ret = true;
            break;
        default:
            break;
    }
   return ret;
}

bool generic_on_off_server_reg(uint8_t element_index, mesh_model_info_p pmodel_info)
{
    if (NULL == pmodel_info)
    {
        return FALSE;
    }

    pmodel_info->model_id = RTK_MESH_MODEL_GENERIC_ON_OFF_SERVER;
    if (NULL == pmodel_info->model_receive)
    {
        pmodel_info->pargs = plt_malloc(sizeof(generic_on_off_t), RAM_TYPE_DATA_ON);
        if (NULL == pmodel_info->pargs)
        {
            printe("generic_on_off_server_reg: fail to allocate memory for the new model extension data!");
            return FALSE;
        }
        memset(pmodel_info->pargs, 0, sizeof(generic_on_off_t));

        pmodel_info->model_receive = generic_on_off_server_receive;
        if (NULL == pmodel_info->model_data_cb)
        {
            printw("generic_on_off_server_reg: missing model data process callback!");
        }
    }
    pmodel_info->is_opcode_of_model = &is_model_opcode;
    generic_transition_time_init();
    return mesh_model_reg(element_index, pmodel_info);
}

bool generic_on_off_server_init(mesh_model_info_p pmodel_info)
{
    pmodel_info->model_id = RTK_MESH_MODEL_GENERIC_ON_OFF_SERVER;
    if (NULL == pmodel_info->model_receive)
    {
        pmodel_info->pargs = plt_malloc(sizeof(generic_on_off_t), RAM_TYPE_DATA_ON);
        if (NULL == pmodel_info->pargs)
        {
            printe("generic_on_off_server_reg: fail to allocate memory for the new model extension data!");
            return FALSE;
        }
        memset(pmodel_info->pargs, 0, sizeof(generic_on_off_t));

        pmodel_info->model_receive = generic_on_off_server_receive;
        //l_info("pmodel_info %p model_receive %p\n",pmodel_info,pmodel_info->model_receive);
        if (NULL == pmodel_info->model_data_cb)
        {
            printw("generic on off server reg: missing data process callback!");
        }
    }
    pmodel_info->is_opcode_of_model = &is_model_opcode;
    generic_transition_time_init();
    return true;
}
