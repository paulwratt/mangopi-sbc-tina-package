enum { __FILE_NUM__ = 0 };

/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     light_hsl_saturation_server.c
  * @brief    Source file for light hsl saturation server model.
  * @details  Data types and external functions declaration.
  * @author   hector
  * @date     2018-8-1
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include "light_hsl.h"

typedef struct
{
    uint8_t tid;
    uint16_t target_saturation;
} light_hsl_saturation_info_t, *light_hsl_saturation_info_p;

int16_t light_saturation_to_generic_level(uint16_t saturation)
{
    return saturation - 32768;
}

uint16_t generic_level_to_light_saturation(int16_t level)
{
    return level + 32768;
}

static mesh_msg_send_cause_t light_hsl_saturation_server_send(mesh_model_info_p pmodel_info,
                                                              uint16_t dst, uint8_t *pmsg, uint16_t msg_len, uint8_t app_key_index)
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
    if((dst == 0)&&(app_key_index == 0))
    {
        return access_publish(&mesh_msg);
    }
    else
    {
        return access_send(&mesh_msg);
    }
}

mesh_msg_send_cause_t light_hsl_saturation_stat(mesh_model_info_p pmodel_info, uint16_t dst,
                                                uint8_t app_key_index, uint16_t present_saturation, bool optional, uint16_t target_saturation,
                                                generic_transition_time_t remaining_time)
{
    light_hsl_saturation_stat_t msg;
    uint32_t len;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_LIGHT_HSL_SATURATION_STAT);
    msg.present_saturation = present_saturation;
    if (optional)
    {
        len = sizeof(light_hsl_saturation_stat_t);
        msg.target_saturation = target_saturation;
        msg.remaining_time = remaining_time;
    }
    else
    {
        len = MEMBER_OFFSET(light_hsl_saturation_stat_t, target_saturation);
    }
    return light_hsl_saturation_server_send(pmodel_info, dst, (uint8_t *)&msg, len, app_key_index);
}

mesh_msg_send_cause_t light_hsl_saturation_publish(const mesh_model_info_p pmodel_info,
                                                   uint16_t saturation)
{
    mesh_msg_send_cause_t ret = MESH_MSG_SEND_CAUSE_INVALID_DST;
    if (mesh_model_pub_check(pmodel_info))
    {
        generic_transition_time_t trans_time = {0, 0};
        ret = light_hsl_saturation_stat(pmodel_info, 0, 0, saturation, FALSE, 0, trans_time);
    }

    return ret;
}

static light_hsl_server_get_saturation_t get_present_saturation(const mesh_model_info_p pmodel_info)
{
    light_hsl_server_get_saturation_t get_data = {0};
    if (NULL != pmodel_info->model_data_cb)
    {
        pmodel_info->model_data_cb(pmodel_info, LIGHT_HSL_SERVER_GET_SATURATION, &get_data);
    }

    return get_data;
}

static int32_t light_hsl_saturation_trans_step_change(const mesh_model_info_p pmodel_info,
                                                      uint32_t type,
                                                      generic_transition_time_t total_time,
                                                      generic_transition_time_t remaining_time)
{
    /* TODO: delay message execution */
    int32_t ret = 0;
    light_hsl_server_set_saturation_t trans_set_data;
    light_hsl_saturation_info_p psaturation_info = pmodel_info->pargs;
    trans_set_data.saturation = psaturation_info->target_saturation;
    trans_set_data.total_time = total_time;
    trans_set_data.remaining_time = remaining_time;

    if (NULL != pmodel_info->model_data_cb)
    {
        ret = pmodel_info->model_data_cb(pmodel_info, LIGHT_HSL_SERVER_SET_SATURATION,
                                         &trans_set_data);
    }

    if (0 == remaining_time.num_steps)
    {
        light_hsl_server_get_saturation_t get_data = {0};
        get_data = get_present_saturation(pmodel_info);
        light_hsl_saturation_publish(pmodel_info, get_data.saturation);
    }

    return ret;
}

static uint16_t light_hsl_saturation_process(const mesh_model_info_p pmodel_info,
                                             uint16_t target_saturation,
                                             generic_transition_time_t trans_time)
{
    light_hsl_server_get_saturation_t saturation_before_set = {0};
    light_hsl_server_get_saturation_t saturation_after_set = {0};

    /* get saturation before set */
    saturation_before_set = get_present_saturation(pmodel_info);

    /* TODO: delay message execution */
    int32_t ret = MODEL_SUCCESS;
    light_hsl_server_set_saturation_t trans_set_data;
    trans_set_data.saturation = target_saturation;
    trans_set_data.total_time = trans_time;
    trans_set_data.remaining_time = trans_time;

    if (NULL != pmodel_info->model_data_cb)
    {
        pmodel_info->model_data_cb(pmodel_info, LIGHT_HSL_SERVER_SET_SATURATION,
                                   &trans_set_data);
    }

    if (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE != trans_time.num_steps)
    {
        if ((ret >= 0) && (MODEL_STOP_TRANSITION != ret))
        {
            generic_transition_timer_start(pmodel_info, GENERIC_TRANSITION_TYPE_LIGHT_HSL_SATURATION,
                                           trans_time,
                                           light_hsl_saturation_trans_step_change);
        }
#if ENABLE_USER_STOP_TRANSITION_NOTIFICATION
        else if (MODEL_STOP_TRANSITION == ret)
        {
            if (NULL != pmodel_info->model_data_cb)
            {
                ret = pmodel_info->model_data_cb(pmodel_info, LIGHT_HSL_SERVER_SET_SATURATION, &trans_set_data);
            }
        }
#endif
    }

    /* get saturation after set */
    saturation_after_set = get_present_saturation(pmodel_info);
    if (saturation_after_set.saturation != saturation_before_set.saturation)
    {
        light_hsl_saturation_publish(pmodel_info, saturation_after_set.saturation);
    }

    return saturation_after_set.saturation;
}

static bool light_hsl_saturation_server_receive(mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    switch (pmesh_msg->access_opcode)
    {
    case MESH_MSG_LIGHT_HSL_SATURATION_GET:
        if (pmesh_msg->msg_len == sizeof(light_hsl_saturation_get_t))
        {
            generic_transition_time_t remaining_time = generic_transition_time_get(pmesh_msg->pmodel_info,
                                                                                   GENERIC_TRANSITION_TYPE_LIGHT_HSL_SATURATION);

            mesh_model_info_p pmodel_info = pmesh_msg->pmodel_info;

            light_hsl_server_get_saturation_t get_data = {0};
            if (NULL != pmodel_info->model_data_cb)
            {
                pmodel_info->model_data_cb(pmodel_info, LIGHT_HSL_SERVER_GET_SATURATION, &get_data);
            }

            light_hsl_saturation_info_p psaturation_info = pmodel_info->pargs;

            light_hsl_saturation_stat(pmesh_msg->pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                      get_data.saturation,
                                      (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE == remaining_time.num_steps) ? FALSE : TRUE,
                                      psaturation_info->target_saturation, remaining_time);
        }
        break;
    case MESH_MSG_LIGHT_HSL_SATURATION_SET:
    case MESH_MSG_LIGHT_HSL_SATURATION_SET_UNACK:
        {
            light_hsl_saturation_set_t *pmsg = (light_hsl_saturation_set_t *)pbuffer;
            generic_transition_time_t trans_time = {GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE, 0};
            mesh_model_info_p pmodel_info = pmesh_msg->pmodel_info;
            if (pmesh_msg->msg_len == MEMBER_OFFSET(light_hsl_saturation_set_t, trans_time))
            {
                if (NULL != pmodel_info->model_data_cb)
                {
                    pmodel_info->model_data_cb(pmodel_info, LIGHT_HSL_SERVER_GET_DEFAULT_TRANSITION_TIME, &trans_time);
                }
            }
            else if (pmesh_msg->msg_len == sizeof(light_hsl_saturation_set_t))
            {
                trans_time = pmsg->trans_time;
            }
            if (IS_GENERIC_TRANSITION_STEPS_VALID(trans_time.num_steps))
            {
                light_hsl_server_get_range_t range = {0, 0, 0, 0};
                if (NULL != pmodel_info->model_data_cb)
                {
                    pmodel_info->model_data_cb(pmodel_info, LIGHT_HSL_SERVER_GET_RANGE, &range);
                }

                light_hsl_saturation_info_p psaturation_info = pmodel_info->pargs;
                if ((0 != range.saturation_range_min) && (0 != range.saturation_range_max))
                {
                    psaturation_info->target_saturation = CLAMP(pmsg->saturation, range.saturation_range_min,
                                                                range.saturation_range_max);
                }
                else
                {
                    psaturation_info->target_saturation = pmsg->saturation;
                }
                psaturation_info->tid = pmsg->tid;

                uint16_t present_saturation = light_hsl_saturation_process(pmodel_info,
                                                                           psaturation_info->target_saturation, trans_time);

                if (pmesh_msg->access_opcode == MESH_MSG_LIGHT_HSL_SATURATION_SET)
                {
                    light_hsl_saturation_stat(pmesh_msg->pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                              present_saturation,
                                              (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE == trans_time.num_steps) ? FALSE : TRUE,
                                              psaturation_info->target_saturation, trans_time);
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
        case MESH_MSG_LIGHT_HSL_SATURATION_GET:
        case MESH_MSG_LIGHT_HSL_SATURATION_SET:
        case MESH_MSG_LIGHT_HSL_SATURATION_SET_UNACK:
            ret = true;
            break;
        default:
            break;
    }
   return ret;
}

bool light_hsl_saturation_server_reg(uint8_t element_index, mesh_model_info_p pmodel_info)
{
    if (NULL == pmodel_info)
    {
        return FALSE;
    }

    pmodel_info->model_id = RTK_MESH_MODEL_LIGHT_HSL_SATURATION_SERVER;
    if (NULL == pmodel_info->model_receive)
    {
        light_hsl_saturation_info_p psaturation_info = plt_malloc(sizeof(light_hsl_saturation_info_t),
                                                                  RAM_TYPE_DATA_ON);
        if (NULL == psaturation_info)
        {
            printe("light_hsl_saturation_server_reg: fail to allocate memory for the new model extension data!");
            return FALSE;
        }
        memset(psaturation_info, 0, sizeof(light_hsl_saturation_info_t));
        pmodel_info->pargs = psaturation_info;

        pmodel_info->model_receive = light_hsl_saturation_server_receive;
        if (NULL == pmodel_info->model_data_cb)
        {
            printw("light_hsl_saturation_server_reg: missing model data process callback!");
        }
    }
    pmodel_info->is_opcode_of_model = &is_model_opcode;
    generic_transition_time_init();
    return mesh_model_reg(element_index, pmodel_info);
}
