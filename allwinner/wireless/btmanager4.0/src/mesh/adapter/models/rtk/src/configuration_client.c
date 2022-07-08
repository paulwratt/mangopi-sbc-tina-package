/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     configuration_client.c
  * @brief    Source file for configuration client model.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2016-3-24
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include <string.h>
#include "mesh_api.h"
#include "configuration.h"

mesh_model_info_t cfg_client;
uint16_t cfg_client_key_index; //!< NetKey or AppKey depends on the mesh_node.features.cfg_model_use_app_key

static mesh_msg_send_cause_t cfg_client_send(uint16_t dst, uint8_t *pmsg, uint16_t len)
{
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = &cfg_client;
    access_cfg(&mesh_msg);
    mesh_msg.pbuffer = pmsg;
    mesh_msg.msg_len = len;
#if MESH_CONFIGURATION_MODEL_USE_APP_KEY
    if (mesh_node.features.cfg_model_use_app_key)
    {
        mesh_msg.app_key_index = cfg_client_key_index;
    }
    else
#endif
    {
        mesh_msg.akf = 0;
        mesh_msg.net_key_index = cfg_client_key_index;
    }
    mesh_msg.dst = dst;
    return access_send(&mesh_msg);
}

mesh_msg_send_cause_t cfg_compo_data_get(uint16_t dst, uint8_t page)
{
    cfg_compo_data_get_t msg;
    msg.page = page;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_COMPO_DATA_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_beacon_get(uint16_t dst)
{
    cfg_beacon_get_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_BEACON_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_beacon_set(uint16_t dst, uint8_t state)
{
    cfg_beacon_set_t msg;
    msg.state = state;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_BEACON_SET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_default_ttl_get(uint16_t dst)
{
    cfg_default_ttl_get_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_DEFAULT_TTL_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_default_ttl_set(uint16_t dst, uint8_t ttl)
{
    cfg_default_ttl_set_t msg;
    msg.ttl = ttl;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_DEFAULT_TTL_SET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_proxy_get(uint16_t dst)
{
    cfg_proxy_get_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_PROXY_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_proxy_set(uint16_t dst, uint8_t state)
{
    cfg_proxy_set_t msg;
    msg.state = state;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_PROXY_SET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_relay_get(uint16_t dst)
{
    cfg_relay_get_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_RELAY_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_relay_set(uint16_t dst, uint8_t state, uint8_t count, uint8_t steps)
{
    cfg_relay_set_t msg;
    msg.state = state;
    msg.count = count;
    msg.steps = steps;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_RELAY_SET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_net_transmit_get(uint16_t dst)
{
    cfg_net_transmit_get_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_NET_TRANS_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_net_transmit_set(uint16_t dst, uint8_t count, uint8_t steps)
{
    cfg_net_transmit_set_t msg;
    msg.count = count;
    msg.steps = steps;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_NET_TRANS_SET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_model_pub_get(uint16_t dst, uint16_t element_addr, uint32_t model_id)
{
    cfg_model_pub_get_t msg;
    msg.element_addr = element_addr;
    msg.model_id = MESH_MODEL_CONVERT(model_id);
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_MODEL_PUB_GET);
    return cfg_client_send(dst, (uint8_t *)&msg,
                           sizeof(msg) - (MESH_IS_VENDOR_MODEL(model_id) ? 0 : 2));
}

mesh_msg_send_cause_t cfg_model_pub_set(uint16_t dst, uint16_t element_addr, bool va_flag,
                                        uint8_t *pub_addr, pub_key_info_t pub_key_info, uint8_t pub_ttl, pub_period_t pub_period,
                                        pub_retrans_info_t pub_retrans_info, uint32_t model_id)
{
    cfg_model_pub_va_set_t pub_set;
    uint8_t *pbuffer = (uint8_t *)&pub_set;
    uint8_t index;
    if (va_flag)
    {
        ACCESS_OPCODE_BYTE(pbuffer, MESH_MSG_CFG_MODEL_PUB_VA_SET);
        index = ACCESS_OPCODE_SIZE(MESH_MSG_CFG_MODEL_PUB_VA_SET);
    }
    else
    {
        ACCESS_OPCODE_BYTE(pbuffer, MESH_MSG_CFG_MODEL_PUB_SET);
        index = ACCESS_OPCODE_SIZE(MESH_MSG_CFG_MODEL_PUB_SET);
    }
    LE_WORD2EXTRN(pbuffer + index, element_addr);
    index += 2;
    memcpy(pbuffer + index, pub_addr, va_flag ? 16 : 2);
    index += va_flag ? 16 : 2;
    pub_key_info.rfu = 0;
    LE_WORD2EXTRN(pbuffer + index, *(uint16_t *)&pub_key_info);
    pbuffer[index + 2] = pub_ttl;
    pbuffer[index + 3] = *(uint8_t *)&pub_period;
    pbuffer[index + 4] = *(uint8_t *)&pub_retrans_info;
    index += 5;
    LE_DWORD2EXTRN(pbuffer + index, model_id);
    index += MESH_IS_VENDOR_MODEL(model_id) ? 4 : 2;
    return cfg_client_send(dst, pbuffer, index);
}

mesh_msg_send_cause_t cfg_model_sub_add(uint16_t dst, uint16_t element_addr, bool va_flag,
                                        uint8_t *addr, uint32_t model_id)
{
    cfg_model_sub_va_add_t sub_add;
    uint8_t *pbuffer = (uint8_t *)&sub_add;
    if (va_flag)
    {
        ACCESS_OPCODE_BYTE(pbuffer, MESH_MSG_CFG_MODEL_SUB_VA_ADD);
    }
    else
    {
        ACCESS_OPCODE_BYTE(pbuffer, MESH_MSG_CFG_MODEL_SUB_ADD);
    }
    LE_WORD2EXTRN(pbuffer + 2, element_addr);
    memcpy(pbuffer + 4, addr, va_flag ? 16 : 2);
    uint8_t index = 4 + (va_flag ? 16 : 2);
    if (MESH_IS_VENDOR_MODEL(model_id))
    {
        LE_DWORD2EXTRN(pbuffer + index, model_id);
        index += 4;
    }
    else
    {
        LE_WORD2EXTRN(pbuffer + index, model_id >> 16);
        index += 2;
    }
    return cfg_client_send(dst, pbuffer, index);
}

mesh_msg_send_cause_t cfg_model_sub_delete(uint16_t dst, uint16_t element_addr, bool va_flag,
                                           uint8_t *addr, uint32_t model_id)
{
    cfg_model_sub_delete_t sub_delete;
    uint8_t *pbuffer = (uint8_t *)&sub_delete;
    if (va_flag)
    {
        ACCESS_OPCODE_BYTE(pbuffer, MESH_MSG_CFG_MODEL_SUB_VA_DELETE);
    }
    else
    {
        ACCESS_OPCODE_BYTE(pbuffer, MESH_MSG_CFG_MODEL_SUB_DELETE);
    }
    LE_WORD2EXTRN(pbuffer + 2, element_addr);
    memcpy(pbuffer + 4, addr, va_flag ? 16 : 2);
    uint8_t index = 4 + (va_flag ? 16 : 2);
    if (MESH_IS_VENDOR_MODEL(model_id))
    {
        LE_DWORD2EXTRN(pbuffer + index, model_id);
        index += 4;
    }
    else
    {
        LE_WORD2EXTRN(pbuffer + index, model_id >> 16);
        index += 2;
    }
    return cfg_client_send(dst, pbuffer, index);
}

mesh_msg_send_cause_t cfg_model_sub_delete_all(uint16_t dst, uint16_t element_addr,
                                               uint32_t model_id)
{
    cfg_model_sub_delete_all_t msg;
    msg.element_addr = element_addr;
    msg.model_id = MESH_MODEL_CONVERT(model_id);
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_MODEL_SUB_DELETE_ALL);
    return cfg_client_send(dst, (uint8_t *)&msg,
                           sizeof(msg) - (MESH_IS_VENDOR_MODEL(model_id) ? 0 : 2));
}

mesh_msg_send_cause_t cfg_model_sub_overwrite(uint16_t dst, uint16_t element_addr, bool va_flag,
                                              uint8_t *addr, uint32_t model_id)
{
    cfg_model_sub_overwrite_t sub_overwrite;
    uint8_t *pbuffer = (uint8_t *)&sub_overwrite;
    if (va_flag)
    {
        ACCESS_OPCODE_BYTE(pbuffer, MESH_MSG_CFG_MODEL_SUB_VA_OVERWRITE);
    }
    else
    {
        ACCESS_OPCODE_BYTE(pbuffer, MESH_MSG_CFG_MODEL_SUB_OVERWRITE);
    }
    LE_WORD2EXTRN(pbuffer + 2, element_addr);
    memcpy(pbuffer + 4, addr, va_flag ? 16 : 2);
    uint8_t index = 4 + (va_flag ? 16 : 2);
    if (MESH_IS_VENDOR_MODEL(model_id))
    {
        LE_DWORD2EXTRN(pbuffer + index, model_id);
        index += 4;
    }
    else
    {
        LE_WORD2EXTRN(pbuffer + index, model_id >> 16);
        index += 2;
    }
    return cfg_client_send(dst, pbuffer, index);
}

mesh_msg_send_cause_t cfg_model_sub_get(uint16_t dst, uint16_t element_addr, uint32_t model_id)
{
    cfg_model_sub_get_t msg;
    if (MESH_IS_VENDOR_MODEL(model_id))
    {
        ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_VENDOR_MODEL_SUB_GET);
    }
    else
    {
        ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_SIG_MODEL_SUB_GET);
    }
    msg.element_addr = element_addr;
    msg.model_id = MESH_MODEL_CONVERT(model_id);
    return cfg_client_send(dst, (uint8_t *)&msg,
                           sizeof(msg) - (MESH_IS_VENDOR_MODEL(model_id) ? 0 : 2));
}

mesh_msg_send_cause_t cfg_net_key_add(uint16_t dst, uint16_t net_key_index, uint8_t net_key[16])
{
    cfg_net_key_add_t msg;
    msg.net_key_index = net_key_index;
    memcpy(msg.net_key, net_key, 16);
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_NET_KEY_ADD);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_net_key_update(uint16_t dst, uint16_t net_key_index, uint8_t net_key[16])
{
    cfg_net_key_update_t msg;
    msg.net_key_index = net_key_index;
    memcpy(msg.net_key, net_key, 16);
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_NET_KEY_UPDATE);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_net_key_delete(uint16_t dst, uint16_t net_key_index)
{
    cfg_net_key_delete_t msg;
    msg.net_key_index = net_key_index;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_NET_KEY_DELETE);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_net_key_get(uint16_t dst)
{
    cfg_net_key_get_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_NET_KEY_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_app_key_add(uint16_t dst, uint16_t net_key_index, uint16_t app_key_index,
                                      uint8_t app_key[16])
{
    cfg_app_key_add_t msg;
    msg.key_index[0] = net_key_index & 0xff;
    msg.key_index[1] = ((net_key_index >> 8) & 0x0f) + ((app_key_index & 0x0f) << 4);
    msg.key_index[2] = (app_key_index >> 4) & 0xff;
    memcpy(msg.app_key, app_key, 16);
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_APP_KEY_ADD);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_app_key_update(uint16_t dst, uint16_t net_key_index,
                                         uint16_t app_key_index, uint8_t app_key[16])
{
    cfg_app_key_update_t msg;
    msg.key_index[0] = net_key_index & 0xff;
    msg.key_index[1] = ((net_key_index >> 8) & 0x0f) + ((app_key_index & 0x0f) << 4);
    msg.key_index[2] = (app_key_index >> 4) & 0xff;
    memcpy(msg.app_key, app_key, 16);
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_APP_KEY_UPDATE);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_app_key_delete(uint16_t dst, uint16_t net_key_index,
                                         uint16_t app_key_index)
{
    cfg_app_key_delete_t msg;
    msg.key_index[0] = net_key_index & 0xff;
    msg.key_index[1] = ((net_key_index >> 8) & 0x0f) + ((app_key_index & 0x0f) << 4);
    msg.key_index[2] = (app_key_index >> 4) & 0xff;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_APP_KEY_DELETE);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_app_key_get(uint16_t dst, uint16_t net_key_index)
{
    cfg_app_key_get_t msg;
    msg.net_key_index = net_key_index;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_APP_KEY_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_node_identity_get(uint16_t dst, uint16_t net_key_index)
{
    cfg_node_identity_get_t msg;
    msg.net_key_index = net_key_index;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_NODE_IDENTITY_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_node_identity_set(uint16_t dst, uint16_t net_key_index, uint8_t identity)
{
    cfg_node_identity_set_t msg;
    msg.net_key_index = net_key_index;
    msg.identity = identity;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_NODE_IDENTITY_SET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_model_app_bind(uint16_t dst, uint16_t element_addr,
                                         uint16_t app_key_index,
                                         uint32_t model_id)
{
    cfg_model_app_bind_t msg;
    msg.element_addr = element_addr;
    msg.app_key_index = app_key_index;
    msg.model_id = MESH_MODEL_CONVERT(model_id);
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_MODEL_APP_BIND);
    return cfg_client_send(dst, (uint8_t *)&msg,
                           sizeof(msg) - (MESH_IS_VENDOR_MODEL(model_id) ? 0 : 2));
}

mesh_msg_send_cause_t cfg_model_app_unbind(uint16_t dst, uint16_t element_addr,
                                           uint8_t app_key_index,
                                           uint32_t model_id)
{
    cfg_model_app_unbind_t msg;
    msg.element_addr = element_addr;
    msg.app_key_index = app_key_index;
    msg.model_id = MESH_MODEL_CONVERT(model_id);
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_MODEL_APP_UNBIND);
    return cfg_client_send(dst, (uint8_t *)&msg,
                           sizeof(msg) - (MESH_IS_VENDOR_MODEL(model_id) ? 0 : 2));
}

mesh_msg_send_cause_t cfg_model_app_get(uint16_t dst,  uint16_t element_addr, uint32_t model_id)
{
    cfg_model_app_get_t msg;
    if (MESH_IS_VENDOR_MODEL(model_id))
    {
        ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_VENDOR_MODEL_APP_GET);
    }
    else
    {
        ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_SIG_MODEL_APP_GET);
    }
    msg.element_addr = element_addr;
    msg.model_id = MESH_MODEL_CONVERT(model_id);
    return cfg_client_send(dst, (uint8_t *)&msg,
                           sizeof(msg) - (MESH_IS_VENDOR_MODEL(model_id) ? 0 : 2));
}

mesh_msg_send_cause_t cfg_node_reset(uint16_t dst)
{
    cfg_node_reset_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_NODE_RESET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_frnd_get(uint16_t dst)
{
    cfg_frnd_get_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_FRND_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_frnd_set(uint16_t dst, uint8_t state)
{
    cfg_frnd_set_t msg;
    msg.state = state;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_FRND_SET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_lpn_poll_timeout_get(uint16_t dst, uint16_t lpn_addr)
{
    cfg_lpn_poll_timeout_get_t msg;
    msg.lpn_addr = lpn_addr;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_LPN_POLL_TO_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_key_refresh_phase_get(uint16_t dst, uint16_t net_key_index)
{
    cfg_key_refresh_phase_get_t msg;
    msg.net_key_index = net_key_index;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_KEY_REFRESH_PHASE_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_key_refresh_phase_set(uint16_t dst, uint16_t net_key_index, uint8_t state)
{
    cfg_key_refresh_phase_set_t msg;
    msg.net_key_index = net_key_index;
    msg.state = state;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_KEY_REFRESH_PHASE_SET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_hb_pub_get(uint16_t dst)
{
    cfg_hb_pub_get_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_HB_PUB_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_hb_pub_set(uint16_t dst, uint16_t dst_pub, uint8_t count_log,
                                     uint8_t period_log,
                                     uint8_t ttl, hb_pub_features_t features, uint16_t net_key_index)
{
    cfg_hb_pub_set_t msg;
    msg.dst = dst_pub;
    msg.count_log = count_log;
    msg.period_log = period_log;
    msg.ttl = ttl;
    msg.features = features;
    msg.net_key_index = net_key_index;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_HB_PUB_SET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_hb_sub_get(uint16_t dst)
{
    cfg_hb_sub_get_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_HB_SUB_GET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

mesh_msg_send_cause_t cfg_hb_sub_set(uint16_t dst, uint16_t src, uint16_t dst_set,
                                     uint8_t period_log)
{
    cfg_hb_sub_set_t msg;
    msg.src = src;
    msg.dst = dst_set;
    msg.period_log = period_log;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_CFG_HB_SUB_SET);
    return cfg_client_send(dst, (uint8_t *)&msg, sizeof(msg));
}

bool cfg_client_receive(mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;

    switch (pmesh_msg->access_opcode)
    {
    case MESH_MSG_CFG_BEACON_STAT:
        break;
    case MESH_MSG_CFG_COMPO_DATA_STAT:
        break;
    case MESH_MSG_CFG_DEFAULT_TTL_STAT:
        break;
    case MESH_MSG_CFG_PROXY_STAT:
        break;
    case MESH_MSG_CFG_RELAY_STAT:
        break;
    case MESH_MSG_CFG_MODEL_PUB_STAT:
        break;
    case MESH_MSG_CFG_MODEL_SUB_STAT:
        break;
    case MESH_MSG_CFG_SIG_MODEL_SUB_LIST:
    case MESH_MSG_CFG_VENDOR_MODEL_SUB_LIST:
        break;
    case MESH_MSG_CFG_NET_KEY_STAT:
        break;
    case MESH_MSG_CFG_NET_KEY_LIST:
        break;
    case MESH_MSG_CFG_APP_KEY_STAT:
        break;
    case MESH_MSG_CFG_APP_KEY_LIST:
        break;
    case MESH_MSG_CFG_NODE_IDENTITY_STAT:
        break;
    case MESH_MSG_CFG_MODEL_APP_STAT:
        break;
    case MESH_MSG_CFG_SIG_MODEL_APP_LIST:
    case MESH_MSG_CFG_VENDOR_MODEL_APP_LIST:
        break;
    case MESH_MSG_CFG_NODE_RESET_STAT:
        break;
    case MESH_MSG_CFG_FRND_STAT:
        break;
    case MESH_MSG_CFG_KEY_REFRESH_PHASE_STAT:
        break;
    case MESH_MSG_CFG_HB_PUB_STAT:
        break;
    case MESH_MSG_CFG_HB_SUB_STAT:
        break;
    default:
        ret = FALSE;
        break;
    }

    if (ret == TRUE)
    {
        data_uart_debug("cfg_client_receive: opcode = 0x%x, len = %d, value = ", pmesh_msg->access_opcode,
                        pmesh_msg->msg_len);
        data_uart_dump(pbuffer, pmesh_msg->msg_len);
        printi("cfg_client_receive: opcode = 0x%x, len = %d, value = ", pmesh_msg->access_opcode,
               pmesh_msg->msg_len);
        dprintt(pbuffer, pmesh_msg->msg_len);
    }
    return ret;
}

bool cfg_client_reg(void)
{
    if (NULL != mesh_model_info_get_by_model_id(0, MESH_MODEL_CFG_CLIENT))
    {
        return FALSE;
    }

    cfg_client.model_id = MESH_MODEL_CFG_CLIENT;
    cfg_client.model_receive = cfg_client_receive;
    return mesh_model_reg(0, &cfg_client);
}

bool cfg_client_key_set(uint16_t key_index)
{
    if (mesh_node.features.cfg_model_use_app_key && key_index >= mesh_node.app_key_num)
    {
        return false;
    }
    else if (mesh_node.features.cfg_model_use_app_key == 0 && key_index >= mesh_node.net_key_num)
    {
        return false;
    }
    else
    {
        cfg_client_key_index = key_index;
        return true;
    }
}
