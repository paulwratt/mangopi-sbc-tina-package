/*
    create this file on 2019.09.04 by wangchunxing.
    pay attention to the follow usages detail of this file.
    1. platform refenence code is the changes need be modified when platform different, such as mesh stack change from bluze mesh to zephyr st...
    2. this platform reference code is suitable for bluez mesh only
*/

/*platform reference code*/
#include "mesh_internal_api.h"

/****others*************/
#include "mesh_config_app.h"
#include "models_mgr.h"
#include <stdio.h>
#include "nrf_error.h"
#include "access.h"

/*lint -e415 -e416 Lint fails to understand the boundary checking used for handles in this module (MBTLE-1831). */

/** Access model pool. @ref ACCESS_MODEL_COUNT is set by user at compile time. */
static access_common_t m_model_pool[ACCESS_MODEL_COUNT];

/** Access element pool. @ref ACCESS_ELEMENT_COUNT is set by user at compile time. */
static access_element_t m_element_pool[ACCESS_ELEMENT_COUNT];

/** Default TTL value for the node. */
static uint8_t m_default_ttl = ACCESS_DEFAULT_TTL;

static uint32_t m_model_cnt = 0;

/* ********** Static functions ********** */

static void opcode_set(access_opcode_t opcode, uint8_t * p_buffer)
{
    if ((opcode.opcode & 0xFF00) > 0)
    {
        p_buffer[0] = (opcode.opcode >> 8) & 0x00FF;
        p_buffer[1] = opcode.opcode & 0x00FF;
    }
    else
    {
        p_buffer[0] = opcode.opcode & 0x00FF;
    }
}

static inline access_model_handle_t find_available_model(void)
{
    for (unsigned i = 0; i < ACCESS_MODEL_COUNT; ++i)
    {
        if (!ACCESS_INTERNAL_STATE_IS_ALLOCATED(m_model_pool[i].internal_state))
        {
            return i;
        }
    }
    return ACCESS_HANDLE_INVALID;
}

static void increment_model_count(uint16_t element_index, uint16_t model_company_id)
{
    if(m_model_cnt >= ACCESS_MODEL_COUNT)
    {
        return ;
    }
    m_model_cnt++;
    if (model_company_id == ACCESS_COMPANY_ID_NONE)
    {
        m_element_pool[element_index].sig_model_count++;
    }
    else
    {
        m_element_pool[element_index].vendor_model_count++;
    }
}
#if 0
static bool element_has_model_id(uint16_t element_index, access_model_id_t model_id, access_model_handle_t * p_model_handle)
{
    if ((m_element_pool[element_index].sig_model_count +
         m_element_pool[element_index].vendor_model_count) > 0)
    {
        for (access_model_handle_t i = 0; i < ACCESS_MODEL_COUNT; ++i)
        {
            if (m_model_pool[i].model_info.element_index       == element_index &&
                m_model_pool[i].model_info.model_id.model_id   == model_id.model_id &&
                m_model_pool[i].model_info.model_id.company_id == model_id.company_id)
            {
                *p_model_handle = i;
                return true;
            }
        }
    }
    return false;
}
#endif
static bool is_opcode_of_model(access_common_t * p_model, access_opcode_t opcode, uint32_t * p_opcode_index)
{
    static uint32_t count = 0;
    count++;
    for (uint32_t i = 0; i < p_model->opcode_count; ++i)
    {
#if(RFU_USED == 0)
        if (p_model->p_opcode_handlers[i].opcode.opcode     == opcode.opcode     &&
            p_model->p_opcode_handlers[i].opcode.company_id == opcode.company_id)
        {
            *p_opcode_index = i;
            return true;
        }
#else
        if (p_model->p_opcode_handlers[i].opcode.opcode == opcode.opcode)
        {
            *p_opcode_index = i;
            ;//l_info("%s\t[%d] match %x,%x",__FUNCTION__,count,p_model->p_opcode_handlers[i].opcode.opcode,opcode.opcode);
            return true;
        }
        else
        {
            ;//l_info("%s\t[%d]not match %x,%x",__FUNCTION__,count,p_model->p_opcode_handlers[i].opcode.opcode,opcode.opcode);
        }
#endif
    }
    return false;
}

/* ********** Private API ********** */
void access_incoming_handle(const access_message_rx_t * p_message)
{
    //l_info("%s,model_cnt%d\n",__FUNCTION__,m_model_cnt);
    for (int i = 0; i < m_model_cnt; ++i)
    {
        access_common_t * p_model = &m_model_pool[i];
        uint32_t opcode_index;
        if (is_opcode_of_model(p_model, p_message->opcode, &opcode_index))
        {
            p_model->p_opcode_handlers[opcode_index].handler(i, p_message, p_model->p_args);
        }
        else
        {
            ;//l_info("%s\topcode:%x not match model:%x",__FUNCTION__,p_message->opcode.opcode,p_model->model_info.model_id.model_id);
        }
    }
}

static void access_state_clear(void)
{
    memset(&m_model_pool[0], 0, sizeof(m_model_pool));
    memset(&m_element_pool[0], 0, sizeof(m_element_pool));
    m_model_cnt = 0;
}

void access_clear(void)
{
    access_state_clear();
}

void access_init(void)
{
    l_info("%s\n",__FUNCTION__);
    access_state_clear();
}

uint32_t access_model_add(const access_model_add_params_t * p_model_params,
                          access_model_handle_t * p_model_handle)
{
    uint32_t status = NRF_SUCCESS;
    ;//l_info("%s,modle_id:%d",__FUNCTION__,p_model_params->model_id.model_id);
    if (NULL == p_model_params ||
        NULL == p_model_handle ||
        (0 != p_model_params->opcode_count && NULL == p_model_params->p_opcode_handlers))
    {
        return NRF_ERROR_NULL;
    }
    *p_model_handle = ACCESS_HANDLE_INVALID;

    if (0 == p_model_params->opcode_count && NULL != p_model_params->p_opcode_handlers)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    else
    {
        *p_model_handle = find_available_model();
        if (ACCESS_HANDLE_INVALID == *p_model_handle)
        {
            return NRF_ERROR_NO_MEM;
        }

        m_model_pool[*p_model_handle].model_info.element_index = p_model_params->element_index;
        m_model_pool[*p_model_handle].model_info.model_id.model_id = p_model_params->model_id.model_id;
        m_model_pool[*p_model_handle].model_info.model_id.company_id = p_model_params->model_id.company_id;
        m_model_pool[*p_model_handle].model_info.publish_ttl = m_default_ttl;
        increment_model_count(p_model_params->element_index, p_model_params->model_id.company_id);
        ACCESS_INTERNAL_STATE_OUTDATED_SET(m_model_pool[*p_model_handle].internal_state);
    }

    m_model_pool[*p_model_handle].p_args = p_model_params->p_args;
    m_model_pool[*p_model_handle].p_opcode_handlers = p_model_params->p_opcode_handlers;
    m_model_pool[*p_model_handle].opcode_count = p_model_params->opcode_count;

    ACCESS_INTERNAL_STATE_ALLOCATED_SET(m_model_pool[*p_model_handle].internal_state);
    status = aw_mesh_add_model(p_model_params->element_index,p_model_params->model_id.model_id,NULL);
    if(status != 0)
    {
        ;//l_info("%s,aw add model fail %d",__FUNCTION__,status);
    }
    return status;
}

extern uint32_t g_send_repeat_cnt ;
extern uint32_t g_publish_repeat_cnt ;

static uint32_t packet_alloc_and_tx(access_model_handle_t handle,
                                    const access_message_tx_t * p_tx_message,
                                    const access_message_rx_t * p_rx_message,
                                    uint8_t **pp_access_payload,
                                    uint16_t *p_access_payload_len)
{

    //uint32_t status;
    uint8_t p_payload[ACCESS_MSG_MAX_SIZE];
    access_common_t * p_model = &m_model_pool[handle];
    uint32_t modle_id = p_model->model_info.model_id.model_id;
    //uint8_t element_id = p_model->model_info.element_index;
    uint16_t opcode_length = ACCESS_UTILS_SIG_OPCODE_SIZE(p_tx_message->opcode.opcode);
    uint16_t payload_length = opcode_length + p_tx_message->length;
    uint32_t i;
    opcode_set(p_tx_message->opcode, p_payload);

    memcpy(&p_payload[opcode_length], p_tx_message->p_buffer, p_tx_message->length);

    if(p_rx_message == NULL)
    {
        for(i = 0; i < g_publish_repeat_cnt; i++)
            aw_mesh_model_publish(modle_id,0,0,127,0, 0, p_payload, payload_length);
    }
    else
    {
        for(i = 0; i < g_send_repeat_cnt; i++)
            aw_mesh_send_packet(p_rx_message->meta_data.src, 0, 0, 127, 0, p_rx_message->meta_data.appkey_idx, p_payload, payload_length);
    }

    return NRF_SUCCESS;
}


uint32_t access_model_reply(access_model_handle_t handle,
                            const access_message_rx_t * p_message,
                            const access_message_tx_t * p_reply)
{
    if (p_message == NULL || p_reply == NULL || (handle >= ACCESS_MODEL_COUNT))
    {
        return NRF_ERROR_NULL;
    }
    ;//l_info("%s\topcode:%d,len:%d",__FUNCTION__,p_message->opcode.opcode,p_message->length);
    return packet_alloc_and_tx(handle, p_reply, p_message, NULL, NULL);
}

uint32_t access_model_publish(access_model_handle_t handle, const access_message_tx_t * p_message)
{
    uint32_t status;

    if (p_message == NULL || (handle >= ACCESS_MODEL_COUNT))
    {
        return NRF_ERROR_NULL;
    }
    ;//l_info("%s\topcode:%d,len:%d",__FUNCTION__,p_message->opcode.opcode,p_message->length);
    status = packet_alloc_and_tx(handle, p_message, NULL, NULL, NULL);

    return status;
}
