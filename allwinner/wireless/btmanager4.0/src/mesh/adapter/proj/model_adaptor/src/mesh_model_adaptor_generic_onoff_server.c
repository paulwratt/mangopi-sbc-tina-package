#include "model_adaptor.h"

static uint32_t model_send_msg(AW_MESH_SIG_MDL_MSG_TX_T *mdl_msg, mesh_model_info_p pmodel_info)
{
    uint32_t status = AW_ERROR_NONE;
    uint16_t opcode = mdl_msg->opcode;
    int32_t dst = mdl_msg->meta.dst_addr;
    int32_t app_key_idx = mdl_msg->meta.src_element_index;
    AW_MESH_GOO_STATUS_T *p_sat = &mdl_msg->data.goo_status;
    generic_transition_time_t remaining_time;

    remaining_time.num_steps = p_sat->remaining_time.num_steps;
    remaining_time.step_resolution = p_sat->remaining_time.step_resolution;
    switch(opcode)
    {
        case AW_MESH_MSG_GENERIC_ONOFF_STATUS:
            generic_on_off_stat(pmodel_info,dst,app_key_idx,p_sat->present_onoff,p_sat->b_target_present    \
                ,p_sat->target_onoff,remaining_time);
            break;
        default:
            status = AW_MESH_ERROR_NOT_FOUND_OPCODE;
            break;
    }

    return status;
}

static uint32_t model_puclish_msg(AW_MESH_SIG_MDL_MSG_TX_T *mdl_msg, mesh_model_info_p pmodel_info)
{
    uint32_t status = AW_ERROR_NONE;
    uint16_t opcode = mdl_msg->opcode;
    AW_MESH_GOO_STATUS_T *p_sat = &mdl_msg->data.goo_status;

    switch(opcode)
    {
        case AW_MESH_MSG_GENERIC_ONOFF_STATUS:
            generic_on_off_publish(pmodel_info,p_sat->b_target_present);
            break;
        default:
            status = AW_MESH_ERROR_NOT_FOUND_OPCODE;
            break;
    }

    return status;
}

static uint32_t model_adaptor_goo_server_send_msg(AW_MESH_SIG_MDL_MSG_TX_T *mdl_msg, mesh_model_info_p pmodel_info)
{
    AW_MESH_MODEL_TX_TYPE_T tx_type;
    tx_type = mdl_msg->meta.tx_type;

    switch(tx_type)
    {
        case AW_MESH_MODEL_SEND:
            return model_send_msg(mdl_msg,pmodel_info);
            break;
        case AW_MESH_MODEL_PUB:
            return model_puclish_msg(mdl_msg,pmodel_info);
            break;
        default:
            break;
    }

    return AW_MESH_ERROR_INVALID_ARGS;
}

static int32_t generic_on_off_server_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;
    model_adaptor_message_p pmsg_rx;
    generic_transition_time_t transition_time;
    if((event_cb == NULL)||(pmodel_info == NULL)||(pmodel_info->pmsg == NULL))
    {
        l_info("error goo model rx , event_cb %p mode_info %p pmsg %p",event_cb,pmodel_info,pmodel_info->pmsg);
        return AW_MESH_ERROR_INVALID_ARGS;
    }
    pmsg_rx = (model_adaptor_message_p)pmodel_info->pmsg;
    mesh_event.param.model_rx_msg.meta.ttl =        pmsg_rx->ttl;
    mesh_event.param.model_rx_msg.meta.rssi =       pmsg_rx->rssi;
    mesh_event.param.model_rx_msg.meta.dst_addr =   pmsg_rx->dst;
    mesh_event.param.model_rx_msg.meta.src_addr =   pmsg_rx->src;
    mesh_event.param.model_rx_msg.meta.src_element_index = pmsg_rx->ele_idx;
    mesh_event.param.model_rx_msg.meta.msg_netkey_index = pmsg_rx->netkey_index;
    mesh_event.param.model_rx_msg.meta.msg_appkey_index = pmsg_rx->appkey_index;
    mesh_event.param.model_rx_msg.meta.model_id = pmodel_info->model_id;
    mesh_event.evt_code = AW_MESH_EVENT_MODEL_RX_CB;
    switch (type)
    {
    case GENERIC_ON_OFF_SERVER_GET:
        {
            generic_on_off_server_get_t *pdata = pargs;

            mesh_event.param.model_rx_msg.opcode = AW_MESH_MSG_GENERIC_ONOFF_GET;
            event_cb(&mesh_event,pmodel_info->app_private);
            pdata->on_off = mesh_event.param.model_rx_msg.data.goo_get.present_onoff;
            //l_info("event_cb get on_off %d",pdata->on_off);
        }
        break;

    case GENERIC_ON_OFF_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
           generic_transition_time_t *pdata = pargs;
           mesh_event.param.model_rx_msg.opcode = AW_MESH_MSG_GENERIC_DTT_GET;
            event_cb(&mesh_event,pmodel_info->app_private);
            transition_time.num_steps = mesh_event.param.model_rx_msg.data.gdtt_status.trans_time.num_steps;
            transition_time.step_resolution = mesh_event.param.model_rx_msg.data.gdtt_status.trans_time.step_resolution;
            *pdata = transition_time;
           //l_info("get gdtt trans_time step 0x%x resolution 0x%x",transition_time.num_steps,transition_time.step_resolution);
        }
        break;

    case GENERIC_ON_OFF_SERVER_SET:
        {
            generic_on_off_server_set_t *pdata = pargs;
            mesh_event.param.model_rx_msg.opcode = AW_MESH_MSG_GENERIC_ONOFF_SET;
            mesh_event.param.model_rx_msg.data.goo_set.total_time.num_steps = pdata->total_time.num_steps;
            mesh_event.param.model_rx_msg.data.goo_set.total_time.step_resolution = pdata->total_time.step_resolution;
            mesh_event.param.model_rx_msg.data.goo_set.remaining_time.num_steps = pdata->remaining_time.num_steps;
            mesh_event.param.model_rx_msg.data.goo_set.remaining_time.step_resolution = pdata->remaining_time.step_resolution;
            mesh_event.param.model_rx_msg.data.goo_set.target_onoff = pdata->on_off;
            event_cb(&mesh_event,pmodel_info->app_private);
            //l_info("event_cb set target on_off %d",pdata->on_off);
        }
        break;

    default:
        break;
    }


    return 0;
}


//pts_generic_onoff_reg
uint32_t model_adaptor_goo_server_reg(uint8_t ele_idx,mesh_model_info_p pmodel_info)
{
    bool ret = false;

    pmodel_info->model_data_cb = generic_on_off_server_data;
    pmodel_info->adaptor_tx_cb = (void*)&model_adaptor_goo_server_send_msg;

    generic_on_off_server_init(pmodel_info);
    ret = mesh_model_adaptor_reg(ele_idx,pmodel_info);
    return ret;
}
