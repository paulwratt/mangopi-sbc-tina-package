#include "AWmeshNodeApi.h"

#define LOCAL_BUILD_CFG_TX_MSG(MSG,PARAM_TYPE,OPCODE,META,DATA)      \
    AW_MESH_CONFIGURATION_MSG_TX_T MSG  = { \
    .opcode             = OPCODE, \
    .meta               = META, \
    };   \
    PARAM_TYPE  = DATA;

// API FOR APPKEY CONFIGURE
INT32 aw_mesh_node_cfg_app_key_add(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_APPKEY_ADD_T appkey_add)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.appkey_add,opcode,meta,appkey_add);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_app_key_update(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_APPKEY_UPDATE_T appkey_update)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.appkey_update,opcode,meta,appkey_update);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_app_key_delete(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_APPKEY_DEL_T appkey_del)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.appkey_del,opcode,meta,appkey_del);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_app_key_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_APPKEY_GET_T appkey_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.appkey_get,opcode,meta,appkey_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}
// API FOR NETKEY CONFIGURE
INT32 aw_mesh_node_cfg_net_key_add(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_NETKEY_ADD_T netkey_add)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.netkey_add,opcode,meta,netkey_add);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_net_key_update(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_NETKEY_UPDATE_T netkey_update)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.netkey_update,opcode,meta,netkey_update);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_net_key_delete(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_NETKEY_DEL_T netkey_del)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.netkey_del,opcode,meta,netkey_del);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_net_key_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_NETKEY_GET_T netkey_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.netkey_get,opcode,meta,netkey_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

// API FOR model app CONFIGURE
INT32 aw_mesh_node_cfg_model_app_bind(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_MODEL_APP_BIND_T model_app_bind)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.model_app_bind,opcode,meta,model_app_bind);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_model_app_unbind(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_MODEL_APP_UNBIND_T model_app_unbind)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.model_app_unbind,opcode,meta,model_app_unbind);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_model_app_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_SIG_MODEL_APP_GET_T sig_model_app_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.sig_model_app_get,opcode,meta,sig_model_app_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_vnd_app_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_VENDOR_MODEL_APP_GET_T vendor_model_app_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.vendor_model_app_get,opcode,meta,vendor_model_app_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}



// API FOR composition CONFIGURE
INT32 aw_mesh_node_cfg_compo_data_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_COMPOSITION_DATA_GET_T composition_data_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.composition_data_get,opcode,meta,composition_data_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

// API FOR beacon CONFIGURE
INT32 aw_mesh_node_cfg_beacon_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_BEACON_GET_T beacon_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.beacon_get,opcode,meta,beacon_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_beacon_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_BEACON_SET_T beacon_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.beacon_set,opcode,meta,beacon_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

// API FOR ttl CONFIGURE

INT32 aw_mesh_node_cfg_ttl_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_DEFAULT_TTL_GET_T default_ttl_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.default_ttl_get,opcode,meta,default_ttl_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_ttl_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_DEFAULT_TTL_SET_T default_ttl_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.default_ttl_set,opcode,meta,default_ttl_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

// API FOR proxy CONFIGURE

INT32 aw_mesh_node_cfg_proxy_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_GATT_PROXY_GET_T gatt_proxy_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.gatt_proxy_get,opcode,meta,gatt_proxy_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_proxy_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_GATT_PROXY_SET_T gatt_proxy_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.gatt_proxy_set,opcode,meta,gatt_proxy_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

// API FOR relay CONFIGURE

INT32 aw_mesh_node_cfg_relay_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_RELAY_GET_T relay_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.relay_get,opcode,meta,relay_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_relay_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_RELAY_SET_T relay_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.relay_set,opcode,meta,relay_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

// API FOR net transmit CONFIGURE

INT32 aw_mesh_node_cfg_transmit_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_NETWORK_TRANSMIT_GET_T net_trans_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.net_trans_get,opcode,meta,net_trans_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_transmit_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_NETWORK_TRANSMIT_SET_T net_trans_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.net_trans_set,opcode,meta,net_trans_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}
// API FOR model pub  CONFIGURE

INT32 aw_mesh_node_cfg_model_pub_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_MODEL_PUB_GET_T model_pub_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.model_pub_get,opcode,meta,model_pub_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_model_pub_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_MODEL_PUB_SET_T model_pub_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.model_pub_set,opcode,meta,model_pub_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

// API FOR sub  CONFIGURE
INT32 aw_mesh_node_cfg_model_sub_add(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_MODEL_SUB_ADD_T model_sub_add)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.model_sub_add,opcode,meta,model_sub_add);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_model_sub_delete(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_MODEL_SUB_DEL_T model_sub_del)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.model_sub_del,opcode,meta,model_sub_del);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_model_sub_delete_all(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_MODEL_SUB_DEL_ALL_T model_sub_del_all)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.model_sub_del_all,opcode,meta,model_sub_del_all);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_model_sub_overwrite(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_MODEL_SUB_OW_T model_sub_ow)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.model_sub_ow,opcode,meta,model_sub_ow);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_model_sub_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_SIG_MODEL_SUB_GET_T sig_model_sub_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.sig_model_sub_get,opcode,meta,sig_model_sub_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_vnd_sub_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_VENDOR_MODEL_SUB_GET_T vendor_model_sub_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.vendor_model_sub_get,opcode,meta,vendor_model_sub_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_node_identity_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_NODE_IDENTITY_GET_T node_identity_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.node_identity_get,opcode,meta,node_identity_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_node_identity_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_NODE_IDENTITY_SET_T node_identity_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.node_identity_set,opcode,meta,node_identity_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_node_reset(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_NODE_RESET_T node_reset)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.node_reset,opcode,meta,node_reset);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_frnd_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_FRIEND_GET_T friend_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.friend_get,opcode,meta,friend_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_frnd_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_FRIEND_SET_T friend_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.friend_set,opcode,meta,friend_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_lpn_poll_timeout_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_POLL_TIMEOUT_T poll_timeout)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.poll_timeout,opcode,meta,poll_timeout);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_key_refresh_phase_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_KEY_REFRESH_PHASE_GET_T key_ref_pha_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.key_ref_pha_get,opcode,meta,key_ref_pha_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_key_refresh_phase_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_KEY_REFRESH_PHASE_SET_T key_ref_pha_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.key_ref_pha_set,opcode,meta,key_ref_pha_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_hb_pub_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_HB_PUB_GET_T hb_pub_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.hb_pub_get,opcode,meta,hb_pub_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_hb_pub_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_HB_PUB_SET_T hb_pub_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.hb_pub_set,opcode,meta,hb_pub_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_hb_sub_get(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_HB_SUB_GET_T hb_sub_get)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.hb_sub_get,opcode,meta,hb_sub_get);
    return aw_mesh_send_config_client_msg(&ccMsg);
}

INT32 aw_mesh_node_cfg_hb_sub_set(AW_MESH_CONFIG_META_T meta,UINT16 opcode,AW_MESH_CONFIG_HB_SUB_SET_T hb_sub_set)
{
    LOCAL_BUILD_CFG_TX_MSG(ccMsg,ccMsg.data.hb_sub_set,opcode,meta,hb_sub_set);
    return aw_mesh_send_config_client_msg(&ccMsg);
}
