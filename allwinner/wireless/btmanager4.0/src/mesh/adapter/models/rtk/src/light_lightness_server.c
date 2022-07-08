enum { __FILE_NUM__ = 0 };

/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     light_lightness_server.c
  * @brief    Source file for light lightness server model.
  * @details  Data types and external functions declaration.
  * @author   hector
  * @date     2018-7-30
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include <math.h>
#include "light_lightness.h"

typedef struct
{
    uint8_t tid;
    uint16_t target_lightness;
    uint16_t target_lightness_linear;
} light_lightness_info_t, *light_lightness_info_p;


uint16_t light_lightness_linear_to_actual(uint16_t lightness_linear)
{
    return (uint16_t)(65535 * sqrt(lightness_linear / 65535.0));
}

uint16_t light_lightness_actual_to_linear(uint16_t lightness_actual)
{
    return (uint16_t)(lightness_actual / 65535.0 * lightness_actual);
}

int16_t light_lightness_to_generic_level(uint16_t lightness)
{
    return lightness - 32768;
}

uint16_t generic_level_to_light_lightness(int16_t level)
{
    return level + 32768;
}

int16_t light_lightness_linear_to_generic_level(uint16_t lightness)
{
    return light_lightness_to_generic_level(light_lightness_linear_to_actual(lightness));
}

uint16_t generic_level_to_light_lightness_linear(int16_t level)
{
    return light_lightness_actual_to_linear(generic_level_to_light_lightness(level));
}

static mesh_msg_send_cause_t light_lightness_server_send(mesh_model_info_p pmodel_info,
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

mesh_msg_send_cause_t light_lightness_stat(mesh_model_info_p pmodel_info, uint16_t dst,
                                           uint8_t app_key_index, uint16_t present_lightness, bool optional, uint16_t target_lightness,
                                           generic_transition_time_t remaining_time)
{
    light_lightness_stat_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_LIGHT_LIGHTNESS_STAT);
    uint16_t msg_len;
    if (optional)
    {
        msg_len = sizeof(light_lightness_stat_t);
        msg.target_lightness = target_lightness;
        msg.remaining_time = remaining_time;
    }
    else
    {
        msg_len = MEMBER_OFFSET(light_lightness_stat_t, target_lightness);
    }
    msg.present_lightness = present_lightness;
    return light_lightness_server_send(pmodel_info, dst, (uint8_t *)&msg, msg_len, app_key_index);
}

mesh_msg_send_cause_t light_lightness_publish(const mesh_model_info_p pmodel_info,
                                              uint16_t lightness)
{
    mesh_msg_send_cause_t ret = MESH_MSG_SEND_CAUSE_INVALID_DST;
    if (mesh_model_pub_check(pmodel_info))
    {
        generic_transition_time_t remaining_time = {0,0};
        ret = light_lightness_stat(pmodel_info, 0, 0, lightness, FALSE, lightness, remaining_time);
    }

    return ret;
}

mesh_msg_send_cause_t light_lightness_linear_stat(mesh_model_info_p pmodel_info, uint16_t dst,
                                                  uint8_t app_key_index, uint16_t present_lightness, bool optional, uint16_t target_lightness,
                                                  generic_transition_time_t remaining_time)
{
    light_lightness_linear_stat_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_LIGHT_LIGHTNESS_LINEAR_STAT);
    uint16_t msg_len;
    if (optional)
    {
        msg_len = sizeof(light_lightness_linear_stat_t);
        msg.target_lightness = target_lightness;
        msg.remaining_time = remaining_time;
    }
    else
    {
        msg_len = MEMBER_OFFSET(light_lightness_linear_stat_t, target_lightness);
    }
    msg.present_lightness = present_lightness;
    return light_lightness_server_send(pmodel_info, dst, (uint8_t *)&msg, msg_len, app_key_index);
}

mesh_msg_send_cause_t light_lightness_linear_publish(const mesh_model_info_p pmodel_info,
                                                     uint16_t lightness)
{
    mesh_msg_send_cause_t ret = MESH_MSG_SEND_CAUSE_INVALID_DST;
    if (mesh_model_pub_check(pmodel_info))
    {
        generic_transition_time_t remaining_time = {0,0};
        ret = light_lightness_linear_stat(pmodel_info, 0, 0, lightness, FALSE, lightness, remaining_time);
    }

    return ret;
}

mesh_msg_send_cause_t light_lightness_last_stat(mesh_model_info_p pmodel_info, uint16_t dst,
                                                uint8_t app_key_index, uint16_t lightness)
{
    light_lightness_last_stat_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_LIGHT_LIGHTNESS_LAST_STAT);
    msg.lightness = lightness;
    return light_lightness_server_send(pmodel_info, dst, (uint8_t *)&msg, sizeof(msg), app_key_index);
}

mesh_msg_send_cause_t light_lightness_default_stat(mesh_model_info_p pmodel_info, uint16_t dst,
                                                   uint8_t app_key_index, uint16_t lightness)
{
    light_lightness_default_stat_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_LIGHT_LIGHTNESS_DEFAULT_STAT);
    msg.lightness = lightness;
    return light_lightness_server_send(pmodel_info, dst, (uint8_t *)&msg, sizeof(msg), app_key_index);
}

mesh_msg_send_cause_t light_lightness_range_stat(mesh_model_info_p pmodel_info, uint16_t dst,
                                                 uint8_t app_key_index, generic_stat_t stat, uint16_t range_min, uint16_t range_max)
{
    light_lightness_range_stat_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_LIGHT_LIGHTNESS_RANGE_STAT);
    msg.stat = stat;
    msg.range_min = range_min;
    msg.range_max = range_max;
    return light_lightness_server_send(pmodel_info, dst, (uint8_t *)&msg, sizeof(msg), app_key_index);
}

static uint16_t get_present_lightness(const mesh_model_info_p pmodel_info, bool linear)
{
    light_lightness_server_get_t get_data = {0};
    uint32_t get_code = LIGHT_LIGHTNESS_SERVER_GET;
    if (linear)
    {
        get_code = LIGHT_LIGHTNESS_SERVER_GET_LINEAR;
    }
    if (NULL != pmodel_info->model_data_cb)
    {
        pmodel_info->model_data_cb(pmodel_info, get_code, &get_data);
    }

    return get_data.lightness;
}

static int32_t light_lightness_trans_step_change(const mesh_model_info_p pmodel_info,
                                                 uint32_t type,
                                                 generic_transition_time_t total_time,
                                                 generic_transition_time_t remaining_time)
{
    /* TODO: delay message execution */
    int32_t ret = MODEL_SUCCESS;
    light_lightness_server_set_t set_data;
    light_lightness_info_p plightness_info = pmodel_info->pargs;
    if (type == GENERIC_TRANSITION_TYPE_LIGHT_LIGHTNESS)
    {
        set_data.lightness = plightness_info->target_lightness;
    }
    else
    {
        set_data.lightness = plightness_info->target_lightness_linear;
    }
    set_data.total_time = total_time;
    set_data.remaining_time = remaining_time;

    if (NULL != pmodel_info->model_data_cb)
    {
        if (type == GENERIC_TRANSITION_TYPE_LIGHT_LIGHTNESS)
        {
            ret = pmodel_info->model_data_cb(pmodel_info, LIGHT_LIGHTNESS_SERVER_SET, &set_data);
        }
        else
        {
            ret = pmodel_info->model_data_cb(pmodel_info, LIGHT_LIGHTNESS_SERVER_SET_LINEAR, &set_data);
        }
    }

    if (0 == remaining_time.num_steps)
    {
        uint16_t present_lightness = 0;
        if (type == GENERIC_TRANSITION_TYPE_LIGHT_LIGHTNESS)
        {
            present_lightness = get_present_lightness(pmodel_info, FALSE);
            light_lightness_publish(pmodel_info, present_lightness);
        }
        else
        {
            present_lightness = get_present_lightness(pmodel_info, TRUE);
            light_lightness_linear_publish(pmodel_info, present_lightness);
        }
    }

    return ret;
}

static uint16_t light_lightness_process(const mesh_model_info_p pmodel_info,
                                        uint16_t target_lightness, bool linear,
                                        generic_transition_time_t trans_time)
{
    uint16_t lightness_before_set = 0;
    uint16_t lightness_after_set = 0;
    uint32_t trans_set_code = LIGHT_LIGHTNESS_SERVER_SET;
    uint32_t trans_timer_code = GENERIC_TRANSITION_TYPE_LIGHT_LIGHTNESS;

    if (linear)
    {
        trans_set_code = LIGHT_LIGHTNESS_SERVER_SET_LINEAR;
        trans_timer_code = GENERIC_TRANSITION_TYPE_LIGHT_LIGHTNESS_LINEAR;
    }

    /* get lightness before set */
    lightness_before_set = get_present_lightness(pmodel_info, linear);

    /* TODO: delay message execution */
    int32_t ret = MODEL_SUCCESS;
    light_lightness_server_set_t trans_set_data;
    trans_set_data.lightness = target_lightness;
    trans_set_data.total_time = trans_time;
    trans_set_data.remaining_time = trans_time;

    if (NULL != pmodel_info->model_data_cb)
    {
        ret = pmodel_info->model_data_cb(pmodel_info, trans_set_code, &trans_set_data);
    }

    if (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE != trans_time.num_steps)
    {
        if ((ret >= 0) && (MODEL_STOP_TRANSITION != ret))
        {
            generic_transition_timer_start(pmodel_info, trans_timer_code, trans_time,
                                           light_lightness_trans_step_change);
        }
#if ENABLE_USER_STOP_TRANSITION_NOTIFICATION
        else if (MODEL_STOP_TRANSITION == ret)
        {
            if (NULL != pmodel_info->model_data_cb)
            {
                ret = pmodel_info->model_data_cb(pmodel_info, trans_set_code, &trans_set_data);
            }
        }
#endif
    }

    /* get lightness set */
    lightness_after_set = get_present_lightness(pmodel_info, linear);
    if (lightness_before_set != lightness_after_set)
    {
        if (!linear)
        {
            light_lightness_publish(pmodel_info, lightness_after_set);
        }
        else
        {
            light_lightness_linear_publish(pmodel_info, lightness_after_set);
        }
    }

    if (0 != target_lightness)
    {
        if (NULL != pmodel_info->model_data_cb)
        {
            if (linear)
            {
                target_lightness = light_lightness_linear_to_actual(target_lightness);
            }
            light_lightness_server_set_last_t set_last = {target_lightness};
            pmodel_info->model_data_cb(pmodel_info, LIGHT_LIGHTNESS_SERVER_SET_LAST, &set_last);
        }
    }

    return lightness_after_set;
}

static bool light_lightness_server_receive(mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;

    switch (pmesh_msg->access_opcode)
    {
    case MESH_MSG_LIGHT_LIGHTNESS_GET:
        if (pmesh_msg->msg_len == sizeof(light_lightness_get_t))
        {
            generic_transition_time_t remaining_time = generic_transition_time_get(pmesh_msg->pmodel_info,
                                                                                   GENERIC_TRANSITION_TYPE_LIGHT_LIGHTNESS);

            mesh_model_info_p pmodel_info = pmesh_msg->pmodel_info;

            light_lightness_server_get_t get_data = {0};
            if (NULL != pmodel_info->model_data_cb)
            {
                pmodel_info->model_data_cb(pmodel_info, LIGHT_LIGHTNESS_SERVER_GET, &get_data);
            }

            light_lightness_info_p plightness_info = pmodel_info->pargs;
            light_lightness_stat(pmesh_msg->pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                 get_data.lightness,
                                 (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE == remaining_time.num_steps) ? FALSE : TRUE,
                                 plightness_info->target_lightness, remaining_time);
        }
        break;
    case MESH_MSG_LIGHT_LIGHTNESS_SET:
    case MESH_MSG_LIGHT_LIGHTNESS_SET_UNACK:
        {
            light_lightness_set_t *pmsg = (light_lightness_set_t *)pbuffer;
            generic_transition_time_t trans_time = {GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE, 0};
            mesh_model_info_p pmodel_info = pmesh_msg->pmodel_info;
            if (pmesh_msg->msg_len == MEMBER_OFFSET(light_lightness_set_t, trans_time))
            {
                if (NULL != pmodel_info->model_data_cb)
                {
                    pmodel_info->model_data_cb(pmodel_info, LIGHT_LIGHTNESS_SERVER_GET_DEFAULT_TRANSITION_TIME,
                                               &trans_time);
                }
            }
            else if (pmesh_msg->msg_len == sizeof(light_lightness_set_t))
            {
                trans_time = pmsg->trans_time;
            }
            if (IS_GENERIC_TRANSITION_STEPS_VALID(trans_time.num_steps))
            {
                light_lightness_info_p plightness_info = pmodel_info->pargs;
                light_lightness_server_get_range_t range = {0, 0};
                if (NULL != pmodel_info->model_data_cb)
                {
                    pmodel_info->model_data_cb(pmodel_info, LIGHT_LIGHTNESS_SERVER_GET_RANGE, &range);
                }
                if ((0 != range.range_min) && (0 != range.range_max))
                {
                    /** need to clamp lightness */
                    plightness_info->target_lightness = CLAMP(pmsg->lightness, range.range_min, range.range_max);
                }
                else
                {
                    plightness_info->target_lightness = pmsg->lightness;
                }
                plightness_info->tid = pmsg->tid;

                uint16_t present_lightness = light_lightness_process(pmodel_info, plightness_info->target_lightness,
                                                                     FALSE, trans_time);

                if (pmesh_msg->access_opcode == MESH_MSG_LIGHT_LIGHTNESS_SET)
                {
                    light_lightness_stat(pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                         present_lightness,
                                         (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE == trans_time.num_steps) ? FALSE : TRUE,
                                         plightness_info->target_lightness, trans_time);
                }
            }
        }
        break;
    case MESH_MSG_LIGHT_LIGHTNESS_LINEAR_GET:
        if (pmesh_msg->msg_len == sizeof(light_lightness_linear_get_t))
        {
            generic_transition_time_t remaining_time = generic_transition_time_get(pmesh_msg->pmodel_info,
                                                                                   GENERIC_TRANSITION_TYPE_LIGHT_LIGHTNESS_LINEAR);

            mesh_model_info_p pmodel_info = pmesh_msg->pmodel_info;
            light_lightness_server_get_t get_data = {0};
            if (NULL != pmodel_info->model_data_cb)
            {
                pmodel_info->model_data_cb(pmodel_info, LIGHT_LIGHTNESS_SERVER_GET_LINEAR, &get_data);
            }

            light_lightness_info_p plightness_info = pmodel_info->pargs;

            light_lightness_linear_stat(pmesh_msg->pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                        get_data.lightness,
                                        (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE == remaining_time.num_steps) ? FALSE : TRUE,
                                        plightness_info->target_lightness_linear, remaining_time);
        }
        break;
    case MESH_MSG_LIGHT_LIGHTNESS_LINEAR_SET:
    case MESH_MSG_LIGHT_LIGHTNESS_LINEAR_SET_UNACK:
        {
            light_lightness_linear_set_t *pmsg = (light_lightness_linear_set_t *)pbuffer;
            generic_transition_time_t trans_time = {GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE, 0};
            mesh_model_info_p pmodel_info = pmesh_msg->pmodel_info;
            if (pmesh_msg->msg_len == MEMBER_OFFSET(light_lightness_linear_set_t, trans_time))
            {
                if (NULL != pmodel_info->model_data_cb)
                {
                    pmodel_info->model_data_cb(pmodel_info, LIGHT_LIGHTNESS_SERVER_GET_DEFAULT_TRANSITION_TIME,
                                               &trans_time);
                }
            }
            else if (pmesh_msg->msg_len == sizeof(light_lightness_linear_set_t))
            {
                trans_time = pmsg->trans_time;
            }
            if (IS_GENERIC_TRANSITION_STEPS_VALID(trans_time.num_steps))
            {
                light_lightness_info_p plightness_info = pmodel_info->pargs;
                plightness_info->target_lightness_linear = pmsg->lightness;
                plightness_info->tid = pmsg->tid;

                uint16_t present_linear = light_lightness_process(pmodel_info,
                                                                  plightness_info->target_lightness_linear,
                                                                  TRUE, trans_time);

                if (pmesh_msg->access_opcode == MESH_MSG_LIGHT_LIGHTNESS_LINEAR_SET)
                {
                    light_lightness_linear_stat(pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                                present_linear,
                                                (GENERIC_TRANSITION_NUM_STEPS_IMMEDIATE == trans_time.num_steps) ? FALSE : TRUE,
                                                plightness_info->target_lightness_linear, trans_time);
                }
            }
        }
        break;
    case MESH_MSG_LIGHT_LIGHTNESS_LAST_GET:
        if (pmesh_msg->msg_len == sizeof(light_lightness_last_get_t))
        {
            light_lightness_server_get_t last_data = {0};
            if (NULL != pmesh_msg->pmodel_info->model_data_cb)
            {
                pmesh_msg->pmodel_info->model_data_cb(pmesh_msg->pmodel_info, LIGHT_LIGHTNESS_SERVER_GET_LAST,
                                                      &last_data);
            }

            light_lightness_last_stat(pmesh_msg->pmodel_info, pmesh_msg->src,
                                      pmesh_msg->app_key_index, last_data.lightness);
        }
        break;
    case MESH_MSG_LIGHT_LIGHTNESS_DEFAULT_GET:
        if (pmesh_msg->msg_len == sizeof(light_lightness_default_get_t))
        {
            light_lightness_server_get_t default_data = {0};
            if (NULL != pmesh_msg->pmodel_info->model_data_cb)
            {
                pmesh_msg->pmodel_info->model_data_cb(pmesh_msg->pmodel_info, LIGHT_LIGHTNESS_SERVER_GET_DEFAULT,
                                                      &default_data);
            }

            light_lightness_default_stat(pmesh_msg->pmodel_info, pmesh_msg->src,
                                         pmesh_msg->app_key_index, default_data.lightness);

        }
        break;
    case MESH_MSG_LIGHT_LIGHTNESS_RANGE_GET:
        if (pmesh_msg->msg_len == sizeof(light_lightness_range_get_t))
        {
            light_lightness_server_get_range_t range_data = {0, 0};
            if (NULL != pmesh_msg->pmodel_info->model_data_cb)
            {
                pmesh_msg->pmodel_info->model_data_cb(pmesh_msg->pmodel_info, LIGHT_LIGHTNESS_SERVER_GET_RANGE,
                                                      &range_data);
            }

            light_lightness_range_stat(pmesh_msg->pmodel_info, pmesh_msg->src, pmesh_msg->app_key_index,
                                       GENERIC_STAT_SUCCESS, range_data.range_min, range_data.range_max);
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
        case MESH_MSG_LIGHT_LIGHTNESS_GET:
        case MESH_MSG_LIGHT_LIGHTNESS_SET:
        case MESH_MSG_LIGHT_LIGHTNESS_SET_UNACK:
        case MESH_MSG_LIGHT_LIGHTNESS_LINEAR_GET:
        case MESH_MSG_LIGHT_LIGHTNESS_LINEAR_SET:
        case MESH_MSG_LIGHT_LIGHTNESS_LINEAR_SET_UNACK:
        case MESH_MSG_LIGHT_LIGHTNESS_LAST_GET:
        case MESH_MSG_LIGHT_LIGHTNESS_DEFAULT_GET:
        case MESH_MSG_LIGHT_LIGHTNESS_RANGE_GET:
            ret = true;
            break;
        default:
            break;
    }
   return ret;
}

bool light_lightness_server_reg(uint8_t element_index, mesh_model_info_p pmodel_info)
{
    if (NULL == pmodel_info)
    {
        return FALSE;
    }

    pmodel_info->model_id = RTK_MESH_MODEL_LIGHT_LIGHTNESS_SERVER;
    if (NULL == pmodel_info->model_receive)
    {
        light_lightness_info_p plightness_info = plt_malloc(sizeof(light_lightness_info_t),
                                                            RAM_TYPE_DATA_ON);
        if (NULL == plightness_info)
        {
            printe("light_lightness_server_reg: fail to allocate memory for the new model extension data!");
            return FALSE;
        }
        memset(plightness_info, 0, sizeof(light_lightness_info_t));
        pmodel_info->pargs = plightness_info;

        pmodel_info->model_receive = light_lightness_server_receive;
        if (NULL == pmodel_info->model_data_cb)
        {
            printw("light_lightness_server_reg: missing model data process callback!");
        }
    }
    pmodel_info->is_opcode_of_model = &is_model_opcode;
    generic_transition_time_init();
    return mesh_model_reg(element_index, pmodel_info);
}
