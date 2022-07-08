#include "model_adaptor.h"

#define LOCAL_MODEL AW_APP_SIG_MDL_MODULE
#define LOG_PRINTF(LEVEL,FMT,...)   mesh_log(LEVEL,LOCAL_MODEL,FMT,##__VA_ARGS__)

static struct mesh_model_adaptor_db_t   {
    struct l_queue *models;
}m_model_db = {
    NULL
};

static bool match_by_model_info(const void *a, const void *b)
{
    if(b == NULL)
        return false;

    if(a == NULL)
        return true;

    if((((mesh_model_info_p)a)->model_id == ((mesh_model_info_p)b)->model_id)
        &&(((mesh_model_info_p)a)->element_index == ((mesh_model_info_p)b)->element_index))
    {
        return true;
    }

    return false;
}

static bool match_by_rx_msg(const void *pmodel_info, const void *pmdl_rx_msg)
{
    if((pmodel_info == NULL)||(pmdl_rx_msg == NULL))
        return false;

    if((((mesh_model_info_p)pmodel_info)->element_index) != (((model_adaptor_message_p)pmdl_rx_msg)->ele_idx))
        return false;

    if(((mesh_model_info_p)pmodel_info)->is_opcode_of_model == NULL)
        return false;

    return ((mesh_model_info_p)pmodel_info)->is_opcode_of_model(((model_adaptor_message_p)pmdl_rx_msg)->opcode);

}

static bool match_by_tx_msg(const void *pmodel_info, const void *pmdl_tx_msg)
{
    if((pmodel_info == NULL)||(pmdl_tx_msg == NULL))
        return false;

    if((((mesh_model_info_p)pmodel_info)->element_index) != (((AW_MESH_SIG_MSG_TX_P)pmdl_tx_msg)->meta.src_element_index))
        return false;

    if((((mesh_model_info_p)pmodel_info)->model_id) != (((AW_MESH_SIG_MSG_TX_P)pmdl_tx_msg)->meta.model_id))
        return false;

    return true;
}

//internal api
int32_t mesh_model_adaptor_reg(uint8_t element_index, mesh_model_info_p pmodel_info)
{
    uint32_t status = 0;
    mesh_model_info_p pmodel_info_new = NULL;

    if(m_model_db.models == NULL)
    {
        m_model_db.models = l_queue_new();
    }

    if(m_model_db.models == NULL)
    {
        return MODEL_ADPATOR_ERROR;
    }

    pmodel_info_new = l_queue_find(m_model_db.models, match_by_model_info, pmodel_info);

    if(pmodel_info_new)
    {
        return MODEL_ADPATOR_SUCCESS;
    }

    pmodel_info_new = new0(mesh_model_info_t, 1);

    if(!pmodel_info_new)
    {
        return MODEL_ADPATOR_ERROR;
    }

    memcpy(pmodel_info_new,pmodel_info,sizeof(mesh_model_info_t));

    pmodel_info_new->element_index = element_index;

    status = aw_mesh_add_model(element_index,pmodel_info->model_id,(void*)pmodel_info_new);

    if (!l_queue_push_tail(m_model_db.models, pmodel_info_new)) {
        free(pmodel_info_new);
        return MODEL_ADPATOR_ERROR;
    }

    LOG_PRINTF(AW_DBG_VERB_LEVE,"mesh model reg and current count %d\n",l_queue_length(m_model_db.models));

    return status;
}

int32_t mesh_model_adaptor_receive_msg(model_adaptor_message_p pmdl_rx_msg)
{

    uint8_t rtk_model_data[512];
    struct _mesh_msg_t mesh_msg;
    uint16_t opcode;

    mesh_model_info_p pmodel_info = NULL;
    pmodel_info = l_queue_find(m_model_db.models, match_by_rx_msg, pmdl_rx_msg);

    if(pmodel_info == NULL)
        return MODEL_ADPATOR_ERROR;

    if(pmdl_rx_msg->size <= 510)
    {
        mesh_msg.pmodel_info = pmodel_info;
        mesh_msg.pmodel_info->pmsg = (void *)pmdl_rx_msg;
        mesh_msg.pbuffer = &rtk_model_data[0];
        mesh_msg.msg_offset = 0;
        mesh_msg.msg_len = pmdl_rx_msg->size + 2;//rtk model need opcode
        mesh_msg.access_opcode = pmdl_rx_msg->opcode;
        opcode = pmdl_rx_msg->opcode;
        mesh_msg.src = pmdl_rx_msg->src;
        mesh_msg.dst = pmdl_rx_msg->dst;
        mesh_msg.app_key_index = pmdl_rx_msg->appkey_index;
        mesh_msg.net_key_index = pmdl_rx_msg->netkey_index;
        mesh_msg.rssi = pmdl_rx_msg->rssi;
        memcpy(&rtk_model_data[2],pmdl_rx_msg->buf,pmdl_rx_msg->size);
        rtk_model_data[0] = (opcode >> 8)&0xFF;
        rtk_model_data[1] = (opcode)&0xFF;
        mesh_msg.pmodel_info->model_receive(&mesh_msg);
        return MODEL_ADPATOR_SUCCESS;
    }

    return MODEL_ADPATOR_ERROR;
}


//public api
int32_t aw_mesh_send_sig_model_msg(AW_MESH_SIG_MDL_MSG_TX_T* mdl_msg)
{
    mesh_model_info_p pmodel_info = NULL;
    mesh_adaptor_send_msg pmesh_tx_cb;
    MESH_POINTER_ACCESS(mdl_msg);
    pmodel_info = l_queue_find(m_model_db.models, match_by_tx_msg, mdl_msg);
    if(pmodel_info == NULL)
        return AW_MESH_ERROR_NOT_FOUND_MODEL;

    pmesh_tx_cb = (mesh_adaptor_send_msg)pmodel_info->adaptor_tx_cb;
    return pmesh_tx_cb(mdl_msg,pmodel_info);
}

int32_t aw_mesh_register_sig_model(AW_MESH_SIG_MDL_REG_T *mdl_reg)
{
    int32_t status = AW_ERROR_NONE;
    struct _mesh_model_info_t modle_info;

    MESH_POINTER_ACCESS(mdl_reg);
    memset(&modle_info,0,sizeof(modle_info));
    modle_info.model_index = mdl_reg->ele_idx;
    modle_info.app_private = mdl_reg->app_private;
    switch(mdl_reg->model_id)
    {
        case AW_MESH_SIG_MODEL_ID_GENERIC_ONOFF_SERVER:
            model_adaptor_goo_server_reg(mdl_reg->ele_idx,&modle_info);
            break;
        case AW_MESH_SIG_MODEL_ID_GENERIC_ONOFF_CLIENT:

            model_adaptor_goo_client_reg(mdl_reg->ele_idx,&modle_info);
            break;

        default:
            status = AW_MESH_ERROR_NOT_RELEASE_API;
            break;

    }
    return status;
}
