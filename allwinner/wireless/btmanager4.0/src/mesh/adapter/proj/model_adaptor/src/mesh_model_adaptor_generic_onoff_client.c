#include "model_adaptor.h"

static uint32_t model_send_msg(AW_MESH_SIG_MDL_MSG_TX_T *mdl_msg, mesh_model_info_p pmodel_info)
{
    uint32_t status = AW_ERROR_NONE;
    uint16_t opcode = mdl_msg->opcode;
    int32_t dst = mdl_msg->meta.dst_addr;
    int32_t app_key_idx = mdl_msg->meta.src_element_index;
    AW_MESH_GOO_SET_T *p_set = &mdl_msg->data.goo_set;
    generic_transition_time_t trans_time;

    trans_time.num_steps = p_set->total_time.num_steps;
    trans_time.step_resolution = p_set->total_time.step_resolution;
    switch(opcode)
    {
        case AW_MESH_MSG_GENERIC_ONOFF_GET:
            generic_on_off_get(pmodel_info,dst,app_key_idx);
 #ifdef MEST_TEST_LOG_ENABLE
            mesh_test_log("%s[src_ele_idx %x dst %x]",STR_APP_CLIENT_GOO_GET,pmodel_info->element_index,dst);
#endif
            break;
        case AW_MESH_MSG_GENERIC_ONOFF_SET_UNACK:
        case AW_MESH_MSG_GENERIC_ONOFF_SET:
            generic_on_off_set(pmodel_info,dst,app_key_idx,p_set->target_onoff,p_set->tid,  \
                p_set->b_trans_present,trans_time,p_set->delay,p_set->b_ack);

#ifdef MEST_TEST_LOG_ENABLE
            mesh_test_log("%s[src_ele_idx %x dst %x ack %x tid %x target-onoff %x trans-time %x]",STR_APP_CLIENT_GOO_SET,pmodel_info->element_index,    \
                dst,p_set->b_ack,p_set->tid,p_set->target_onoff,p_set->b_trans_present);
#endif


            break;
        default:
            status = AW_MESH_ERROR_NOT_FOUND_OPCODE;
            break;
    }

    return status;
}

static uint32_t model_adaptor_goo_client_send_msg(AW_MESH_SIG_MDL_MSG_TX_T *mdl_msg, mesh_model_info_p pmodel_info)
{
    AW_MESH_MODEL_TX_TYPE_T tx_type;
    tx_type = mdl_msg->meta.tx_type;

    if(tx_type == AW_MESH_MODEL_SEND)
    {
        return model_send_msg(mdl_msg,pmodel_info);
    }

    return AW_MESH_ERROR_INVALID_ARGS;
}

static int32_t generic_on_off_client_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
     AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
     AW_MESH_EVENT_T mesh_event;
     model_adaptor_message_p pmsg_rx;
     if((event_cb == NULL)||(pmodel_info == NULL)||(pmodel_info->pmsg == NULL))
     {
         l_info("error goo client model rx , event_cb %p mode_info %p pmsg %p",event_cb,pmodel_info,pmodel_info->pmsg);
         return AW_MESH_ERROR_INVALID_ARGS;
     }
     pmsg_rx = (model_adaptor_message_p)pmodel_info->pmsg;
     mesh_event.param.model_rx_msg.meta.ttl = pmsg_rx->ttl;
     mesh_event.param.model_rx_msg.meta.rssi = pmsg_rx->rssi;
     mesh_event.param.model_rx_msg.meta.dst_addr = pmsg_rx->dst;
     mesh_event.param.model_rx_msg.meta.src_addr = pmsg_rx->src;
     mesh_event.param.model_rx_msg.meta.src_element_index = pmsg_rx->ele_idx;
     mesh_event.param.model_rx_msg.meta.msg_netkey_index = pmsg_rx->netkey_index;
     mesh_event.param.model_rx_msg.meta.msg_appkey_index = pmsg_rx->appkey_index;
     mesh_event.param.model_rx_msg.meta.model_id = pmodel_info->model_id;
     mesh_event.evt_code = AW_MESH_EVENT_MODEL_RX_CB;

    switch (type)
    {
        case GENERIC_ON_OFF_CLIENT_STATUS:
            {
                generic_on_off_client_status_t *pdata = pargs;
                mesh_event.param.model_rx_msg.opcode = AW_MESH_MSG_GENERIC_ONOFF_STATUS;
                mesh_event.param.model_rx_msg.data.goo_status.b_target_present = pdata->optional;
                mesh_event.param.model_rx_msg.data.goo_status.present_onoff = pdata->present_on_off;
                if(pdata->optional)
                {
                    mesh_event.param.model_rx_msg.data.goo_status.target_onoff = pdata->target_on_off;
                    mesh_event.param.model_rx_msg.data.goo_status.remaining_time.num_steps = pdata->remaining_time.num_steps;
                    mesh_event.param.model_rx_msg.data.goo_status.remaining_time.step_resolution = pdata->remaining_time.step_resolution;
                    goo_log("goo client receive: present = %d, target = %d, remain time = step(%d), resolution(%d)\r\n",    \
                        pdata->present_on_off, pdata->target_on_off,pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);

                }
                else
                {
                    goo_log("goo client receive: present = %d\r\n", pdata->present_on_off);
                }
                event_cb(&mesh_event,pmodel_info->app_private);
            }
            break;
        default:
            break;
    }

    return 0;
}


//pts_generic_onoff_reg
uint32_t model_adaptor_goo_client_reg(uint8_t ele_idx,mesh_model_info_p pmodel_info)
{
    bool ret = false;

    pmodel_info->model_data_cb = generic_on_off_client_data;
    pmodel_info->adaptor_tx_cb = (void*)&model_adaptor_goo_client_send_msg;

    generic_on_off_client_init(pmodel_info);

    ret = mesh_model_adaptor_reg(ele_idx,pmodel_info);

    return ret;
}
