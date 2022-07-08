#include "mesh_access.h"
#include "mesh_internal_api.h"
/****others*************/
#include "mesh_config_app.h"
#include <stdio.h>
#include "mesh_node.h"

static access_rtk_t m_model_pool[ACCESS_MODEL_COUNT];
static uint32_t m_model_cnt = 0;

#define ACCESS_MODEL_IDLE 0
#define ACCESS_MODEL_ALLOC 1
#define MESH_ACCESS_HANDLE_INVALID 0XFFFF

uint32_t g_send_repeat_cnt = 1;
uint32_t g_publish_repeat_cnt = 1;

static void rtk_config_mesh_msg(mesh_msg_p pmesh_msg,const access_message_rx_t * p_message)
{
    pmesh_msg->pbuffer = (uint8_t *)p_message->p_data;
    pmesh_msg->msg_offset = 0;
    pmesh_msg->msg_len = p_message->length;
    pmesh_msg->access_opcode = p_message->opcode.opcode;
    pmesh_msg->src = p_message->meta_data.src;
    pmesh_msg->app_key_index = p_message->meta_data.appkey_idx;
}

void rtk_access_incoming_handle(const access_message_rx_t * p_message)
{
    struct _mesh_msg_t mesh_msg;
    //mesh_model_info_p pmodel_info ;

    //l_info("%s,model_cnt%d\n",__FUNCTION__,m_model_cnt);

    for (int i = 0; i < m_model_cnt; ++i)
    {
         access_rtk_t * p_model = &m_model_pool[i];
        if((p_model->internal_state == ACCESS_MODEL_ALLOC)  \
            &&(p_model->p_model_info->is_opcode_of_model))
        {
            if(p_model->p_model_info->is_opcode_of_model(p_message->opcode.opcode) == true)
            {
                mesh_msg.pmodel_info = p_model->p_model_info;
                rtk_config_mesh_msg(&mesh_msg,p_message);
                l_info("%s\topcode:%x match model:%x","access_incoming_handle",p_message->opcode.opcode,p_model->p_model_info->model_id);
                mesh_msg.pmodel_info->model_receive(&mesh_msg);
            }
            else
            {
                ;//l_info("%s\topcode:%x not match model:%x",__FUNCTION__,p_message->opcode.opcode,p_model->p_model_info->model_id);
            }

        }
        else
        {
                ;//l_info("%s\t:%d,%p",__FUNCTION__,p_model->internal_state,p_model->p_model_info->is_opcode_of_model);
        }
    }
}

mesh_msg_send_cause_t access_send(mesh_msg_p pmesh_msg)
{
    //l_info("%s,%x,%d,%d,%d",__FUNCTION__,pmesh_msg->access_opcode,pmesh_msg->msg_len,pmesh_msg->dst,g_send_repeat_cnt);
    //send twice because pts not received sometime
    uint8_t i = 0;

    if(g_send_repeat_cnt < 1)
        g_send_repeat_cnt = 1;
    l_info("access_send ele idx %d",pmesh_msg->pmodel_info->element_index);
    for(i = 0; i < g_send_repeat_cnt; i++)
    {
        aw_mesh_send_packet(pmesh_msg->dst, 0, pmesh_msg->pmodel_info->element_index, 127, 0, pmesh_msg->app_key_index , pmesh_msg->pbuffer, pmesh_msg->msg_len);
    }
    //aw_mesh_send_packet(0xC002, 0, 0, 127, 0, pmesh_msg->app_key_index , pmesh_msg->pbuffer, pmesh_msg->msg_len);
    return MESH_MSG_SEND_CAUSE_SUCCESS;
}

mesh_msg_send_cause_t access_publish(mesh_msg_p pmesh_msg)
{
    //l_info("%s,%x,%d,%d,%d",__FUNCTION__,pmesh_msg->pmodel_info->model_id,pmesh_msg->msg_len,pmesh_msg->dst,g_publish_repeat_cnt);
     //send twice because pts not received sometime
     uint8_t i = 0;
     //if(g_publish_repeat_cnt < 1)
     //   g_publish_repeat_cnt = 1;
     //return MESH_MSG_SEND_CAUSE_SUCCESS;
     l_info("access_publish ele idx %d",pmesh_msg->pmodel_info->element_index);
     for(i = 0; i < g_publish_repeat_cnt; i++)
     {
        aw_mesh_model_publish(pmesh_msg->pmodel_info->model_id,0,pmesh_msg->pmodel_info->element_index, 127,0, 0, pmesh_msg->pbuffer, pmesh_msg->msg_len);
     }
    return MESH_MSG_SEND_CAUSE_SUCCESS;
}

mesh_msg_send_cause_t access_cfg(mesh_msg_p pmesh_msg)
{
    return MESH_MSG_SEND_CAUSE_SUCCESS;
}

void access_dispatch(mesh_msg_p pmesh_msg)
{

}

void access_receive(mesh_msg_p pmesh_msg)
{

}

bool mesh_model_pub_check(mesh_model_info_p pmodel_info)
{
    return true;
}

static inline uint16_t alloc_available_model(void)
{
    for (unsigned i = 0; i < ACCESS_MODEL_COUNT; ++i)
    {
        if(m_model_pool[i].internal_state == ACCESS_MODEL_IDLE)
        {
            return i;
        }
    }
    return MESH_ACCESS_HANDLE_INVALID;
}

static void rtk_access_state_clear(void)
{
    memset(&m_model_pool[0], 0, sizeof(m_model_pool));
    //memset(&m_element_pool[0], 0, sizeof(m_element_pool));
    m_model_cnt = 0;
}

void rtk_access_clear(void)
{
    rtk_access_state_clear();
}

void rtk_access_init(void)
{
    l_info("%s\n",__FUNCTION__);
    rtk_access_state_clear();
}

bool mesh_model_reg(uint8_t element_index, mesh_model_info_p pmodel_info)
{
     uint32_t status = 0;
    pmodel_info->model_hdl = ACCESS_HANDLE_INVALID;
    pmodel_info->model_hdl = alloc_available_model();
    if (MESH_ACCESS_HANDLE_INVALID == pmodel_info->model_hdl)
    {
        return false;
    }
    if(m_model_cnt >= ACCESS_MODEL_COUNT)
    {
        return false;
    }
    status = aw_mesh_add_model(element_index,pmodel_info->model_id,NULL);
    if(status!= 0)
    {
        //l_info("%s,aw add model fail %d",__FUNCTION__,status);
        return false;
    }
    m_model_cnt++;
    m_model_pool[pmodel_info->model_hdl].internal_state = ACCESS_MODEL_ALLOC;
    pmodel_info->element_index = element_index;
    m_model_pool[pmodel_info->model_hdl].p_model_info = pmodel_info;
    //l_info("%s,m_model_pool[%d].p_model_info =%p\n",__FUNCTION__,pmodel_info->model_hdl,m_model_pool[pmodel_info->model_hdl].p_model_info);
    return true;
}
