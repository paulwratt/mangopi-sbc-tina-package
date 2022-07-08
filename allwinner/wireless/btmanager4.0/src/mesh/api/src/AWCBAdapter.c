#include <ell/ell.h>
#include "AWTypes.h"
#include "AWCBAdapter.h"
#include "AWmeshApi.h"

#define LOCAL_MODEL AW_APP_EVENT_CB_MODULE
#define LOG_PRINTF(LEVEL,FMT,...)   mesh_log(LEVEL,LOCAL_MODEL,FMT,##__VA_ARGS__)
void demo_model_msg_receive(AW_MESH_EVENT_T *event,void *private);
void demo_vnd_model_msg_receive(AW_MESH_EVENT_T *event,void *private);

/*start of config client status message info dump event->param.config_mdl_status*/
static void aw_mesh_dump_composition_data_status(AW_MESH_CONF_COMPO_DATA_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s vid %x pid %x cid %x feature %x crpl %x page %x len %x",   \
        __func__,p_data->version_id,p_data->product_id,p_data->company_id,p_data->features,p_data->crpl    \
        ,p_data->page,p_data->data_len);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[vid %x pid %x cid %x feature %x crpl %x page %x len %x]",   \
        STR_COMPOSITION_STATUS,p_data->version_id,p_data->product_id,p_data->company_id,p_data->features,p_data->crpl    \
        ,p_data->page,p_data->data_len);
#endif

}

static void aw_mesh_dump_def_ttl_status(AW_MESH_CONF_DEF_TTL_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s\tttl %x",__func__,p_data->ttl);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[ttl %x]",STR_TTL_STATUS,p_data->ttl);
#endif
}

static void aw_mesh_dump_gatt_proxy_status(AW_MESH_CONF_GATT_PROXY_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s proxy %x",__func__,p_data->gatt_proxy);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[proxy %x]",STR_PROXY_STATUS,p_data->gatt_proxy);
#endif
}

static void aw_mesh_dump_relay_status(AW_MESH_CONF_RELAY_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s relay %x retrans cnt %x intlsteps %x",__func__,p_data->relay,p_data->relay_retrans_cnt,p_data->relay_retrans_intvlsteps);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[relay %x retrans cnt %x intlsteps %x]",STR_RELAY_STATUS,p_data->relay,p_data->relay_retrans_cnt,p_data->relay_retrans_intvlsteps);
#endif

}

static void aw_mesh_dump_pub_status(AW_MESH_CONF_MODEL_PUB_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x cred_flag %x elem_addr %x model_id %x pub_addr %x pub_perid %x pub_retrans_cnt %x pub_retrans_intvl_steps %x pub_ttl %x",  \
        __func__,p_data->status,p_data->cred_flag,p_data->elem_addr,p_data->model_id,p_data->pub_addr \
        ,p_data->pub_perid,p_data->pub_retrans_cnt,p_data->pub_retrans_intvl_steps,p_data->pub_ttl);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[status %x cred_flag %x elem_addr %x model_id %x pub_addr %x pub_perid %x pub_retrans_cnt %x pub_retrans_intvl_steps %x pub_ttl %x]",  \
        STR_PUB_STATUS,p_data->status,p_data->cred_flag,p_data->elem_addr,p_data->model_id,p_data->pub_addr \
        ,p_data->pub_perid,p_data->pub_retrans_cnt,p_data->pub_retrans_intvl_steps,p_data->pub_ttl);
#endif
}

static void aw_mesh_dump_sub_status(AW_MESH_CONF_MODEL_SUB_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x addr %x ele_addr %x model_id %x ",__func__,  \
        p_data->status,p_data->address,p_data->elem_addr,p_data->model_id);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[status %x addr %x ele_addr %x model_id %x]",  \
        STR_SUB_STATUS,p_data->status,p_data->address,p_data->elem_addr,p_data->model_id);
#endif

}

static void aw_mesh_dump_sig_sub_list_status(AW_MESH_CONF_SIG_MODEL_SUB_LIST_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x num %x addresses %x elem_addr %x sig_model_id %x",__func__,  \
        p_data->status,p_data->num,p_data->addresses,p_data->elem_addr,p_data->sig_model_id);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[status %x num %x addresses %x elem_addr %x sig_model_id %x]",  \
        STR_SIG_SUB_LIST_STATUS,p_data->status,p_data->num,p_data->addresses,p_data->elem_addr,p_data->sig_model_id);
#endif
}

static void aw_mesh_dump_vnd_sub_list_status(AW_MESH_CONF_VND_MODEL_SUB_LIST_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x num %x addresses %x elem_addr %x vnd_model_id %x",   \
        __func__,p_data->status,p_data->num,p_data->addresses,p_data->elem_addr,p_data->vnd_model_id);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[status %x num %x addresses %x elem_addr %x model_id %x company_id %x]",  \
        STR_VND_SUB_LIST_STATUS,p_data->status,p_data->num,p_data->addresses,p_data->elem_addr,p_data->vnd_model_id.company_id,p_data->vnd_model_id.model_id);
#endif
}

static void aw_mesh_dump_netkey_status(AW_MESH_CONF_NETKEY_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x netkey_index %x",__func__,p_data->status,p_data->netkey_index);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[status %x netkey_index %x]",  \
       STR_NETKEY_STATUS,p_data->status,p_data->netkey_index);
#endif
}

static void aw_mesh_dump_netkey_list_status(AW_MESH_CONF_NETKEY_LIST_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s num_of_netkey %x pnetkeyindexes %x",__func__,p_data->num_of_netkey,p_data->pnetkeyindexes);
#ifdef MEST_TEST_LOG_ENABLE
    switch(p_data->num_of_netkey)
    {
        case 1:
            mesh_test_log("%s[num_of_netkey %x netkey_idx[0] %x]",STR_NETKEY_LIST_STATUS,p_data->num_of_netkey,p_data->pnetkeyindexes[0]);
            break;
        case 2:
            mesh_test_log("%s[num_of_netkey %x netkey_idx[0] %x netkey_idx[1] %x]",STR_NETKEY_LIST_STATUS,p_data->num_of_netkey,p_data->pnetkeyindexes[0],p_data->pnetkeyindexes[1]);
            break;
        case 3:
            mesh_test_log("%s[num_of_netkey %x netkey_idx[0] %x netkey_idx[1] %x netkey_idx[2] %x]",STR_NETKEY_LIST_STATUS,p_data->num_of_netkey,p_data->pnetkeyindexes[0],p_data->pnetkeyindexes[1],p_data->pnetkeyindexes[2]);
            break;
        default:
            mesh_test_log("%s[num_of_netkey %x pnetkeyindexes %x]",STR_NETKEY_LIST_STATUS,p_data->num_of_netkey,p_data->pnetkeyindexes);
            break;
    }
#endif
}

static void aw_mesh_dump_appkey_list_status(AW_MESH_CONF_APPKEY_LIST_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x net_idx %x num_of_appkey %x pappkeyindexes %x",__func__,p_data->status,p_data->netkey_index,p_data->num_of_appkey,p_data->pappkeyindexes);
#ifdef MEST_TEST_LOG_ENABLE
    switch(p_data->num_of_appkey)
    {
        case 1:
            mesh_test_log("%s[status %x netkey_idx %x num_of_appkey %x appkey_idx[0] %x]",STR_APPKEY_LIST_STATUS,p_data->status,p_data->netkey_index,p_data->num_of_appkey,p_data->pappkeyindexes[0]);
            break;
        case 2:
            mesh_test_log("%s[status %x netkey_idx %x num_of_appkey %x appkey_idx[0] %x appkey_idx[1] %x]",STR_APPKEY_LIST_STATUS,p_data->status,p_data->netkey_index,p_data->num_of_appkey,p_data->pappkeyindexes[0],p_data->pappkeyindexes[1]);
            break;
        case 3:
            mesh_test_log("%s[status %x netkey_idx %x num_of_appkey %x appkey_idx[0] %x appkey_idx[1] %x appkey_idx[2] %x]",STR_APPKEY_LIST_STATUS,p_data->status,p_data->netkey_index,p_data->num_of_appkey,p_data->pappkeyindexes[0],p_data->pappkeyindexes[1],p_data->pappkeyindexes[2]);
            break;
        default:
            mesh_test_log("%s[status %x netkey_idx %x num_of_appkey %x pappkeyindexes %x]",  \
                STR_APPKEY_LIST_STATUS,p_data->status,p_data->netkey_index,p_data->num_of_appkey,p_data->pappkeyindexes);
            break;
    }
#endif

}

static void aw_mesh_dump_sig_appkey_list_status(AW_MESH_CONF_SIG_MODEL_APP_LIST_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x ele_idx %x mod_id %x num_of_appkey %x pappkeyindexes %x",__func__,p_data->status,p_data->elem_addr,p_data->model_id,p_data->num_of_appkey,p_data->pappkeyindexes);
#ifdef MEST_TEST_LOG_ENABLE
        switch(p_data->num_of_appkey)
        {
            case 1:
                mesh_test_log("%s[status %x ele_idx %x mod_id %x num_of_appkey %x appkey_idx[0] %x]",STR_SIG_MODEL_APPKEY_LIST_STATUS,p_data->status,p_data->elem_addr,p_data->model_id,p_data->num_of_appkey,p_data->pappkeyindexes[0]);
                break;
            case 2:
                mesh_test_log("%s[status %x ele_idx %x mod_id %x num_of_appkey %x appkey_idx[0] %x appkey_idx[1] %x]",STR_SIG_MODEL_APPKEY_LIST_STATUS,p_data->status,p_data->elem_addr,p_data->model_id,p_data->num_of_appkey,p_data->pappkeyindexes[0],p_data->pappkeyindexes[1]);
                break;
            case 3:
                mesh_test_log("%s[status %x ele_idx %x mod_id %x num_of_appkey %x appkey_idx[0] %x appkey_idx[1] %x appkey_idx[2] %x]",STR_SIG_MODEL_APPKEY_LIST_STATUS,p_data->status,p_data->elem_addr,p_data->model_id,p_data->num_of_appkey,p_data->pappkeyindexes[0],p_data->pappkeyindexes[1],p_data->pappkeyindexes[2]);
                break;
            default:
                mesh_test_log("%s[status %x ele_idx %x mod_id %x num_of_appkey %x pappkeyindexes %x]",  \
                    STR_SIG_MODEL_APPKEY_LIST_STATUS,p_data->status,p_data->elem_addr,p_data->model_id,p_data->num_of_appkey,p_data->pappkeyindexes);
                break;
        }
#endif

}

static void aw_mesh_dump_node_ident_status(AW_MESH_CONF_NODE_IDENT_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x identity %x netkey_index %x",__func__,p_data->status,p_data->identity,p_data->netkey_index);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[status %x identity %x netkey_index %x]",  \
       STR_NODE_IDENTITY_STATUS,p_data->status,p_data->identity,p_data->netkey_index);
#endif

}

static void aw_mesh_dump_app_status(AW_MESH_CONF_MODEL_APP_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x elem_addr %x model_id %x appkey_index %x",__func__,  \
        p_data->status,p_data->elem_addr,p_data->model_id,p_data->appkey_index);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[status %x elem_addr %x model_id %x appkey_index %x]",  \
       STR_APP_STATUS,p_data->status,p_data->elem_addr,p_data->model_id,p_data->appkey_index);
#endif

}

static void aw_mesh_dump_appkey_status(AW_MESH_CONF_APPKEY_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x netkey_index %x appkey_index %x ",   \
        __func__,p_data->status,p_data->netkey_index,p_data->appkey_index);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[status %x netkey_index %x appkey_index %x]",  \
       STR_APPKEY_STATUS,p_data->status,p_data->netkey_index,p_data->appkey_index);
#endif
}

static void aw_mesh_dump_vnd_app_list_status(AW_MESH_CONF_VND_MODEL_APP_LIST_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x app_number %x ele_addr %x vnd_model_id %x pappkeyindexes %x ",   \
        __func__,p_data->status,p_data->num_of_appkey,p_data->elem_addr,p_data->vnd_model_id,p_data->pappkeyindexes);

#ifdef MEST_TEST_LOG_ENABLE
    switch(p_data->num_of_appkey)
    {
        case 1:
            mesh_test_log("%s[status %x ele_idx %x mod_id %x company_id %x num_of_appkey %x appkey_idx[0] %x]",STR_VND_APP_LIST_STATUS,p_data->status,p_data->elem_addr,p_data->vnd_model_id.model_id,p_data->vnd_model_id.company_id,p_data->num_of_appkey,p_data->pappkeyindexes[0]);
            break;
        case 2:
            mesh_test_log("%s[status %x ele_idx %x mod_id %x company_id %x num_of_appkey %x appkey_idx[0] %x appkey_idx[1] %x]",STR_VND_APP_LIST_STATUS,p_data->status,p_data->elem_addr,p_data->vnd_model_id.model_id,p_data->vnd_model_id.company_id,p_data->num_of_appkey,p_data->pappkeyindexes[0],p_data->pappkeyindexes[1]);
            break;
        case 3:
            mesh_test_log("%s[status %x ele_idx %x mod_id %x company_id %x num_of_appkey %x appkey_idx[0] %x appkey_idx[1] %x appkey_idx[2] %x]",STR_VND_APP_LIST_STATUS,p_data->status,p_data->elem_addr,p_data->vnd_model_id.model_id,p_data->vnd_model_id.company_id,p_data->num_of_appkey,p_data->pappkeyindexes[0],p_data->pappkeyindexes[1],p_data->pappkeyindexes[2]);
            break;
        default:
            mesh_test_log("%s[status %x ele_idx %x mod_id %x company_id %x num_of_appkey %x pappkeyindexes %x]",  \
                STR_VND_APP_LIST_STATUS,p_data->status,p_data->elem_addr,p_data->vnd_model_id.model_id,p_data->vnd_model_id.company_id,p_data->num_of_appkey,p_data->pappkeyindexes);
            break;
    }
#endif

}

static void aw_mesh_dump_friend_status(AW_MESH_CONF_FRND_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s friend %x",__func__,p_data->friend);

#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[friend %x]",  \
           STR_FRIEND_STATUS_TAG,p_data->friend);
#endif
}

static void aw_mesh_dump_hb_pub_status(AW_MESH_CONF_HB_PUB_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x",__func__,p_data->status);
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[status %x netkey_idx %x dst %x countlog %x periodlog %x feat %x ttl %x]",  \
           STR_HEARTBEAT_PUB_STATUS,p_data->status,p_data->netkey_index,p_data->dest_addr,p_data->count_log,p_data->period_log,   \
            p_data->features,p_data->ttl);
#endif

}

static void aw_mesh_dump_hb_sub_status(AW_MESH_CONF_HB_SUB_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x src %x dst %x countlog %x periodlog %x max_hops %x min_hops %x",__func__,p_data->status,p_data->src_addr,p_data->dst_addr,  \
        p_data->count_log,p_data->period_log,p_data->max_hops,p_data->min_hops);
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[status %x src %x dst %x countlog %x periodlog %x max_hops %x min_hops %x]",  \
           STR_HEARTBEAT_SUB_STATUS,p_data->status,p_data->src_addr,p_data->dst_addr,  \
           p_data->count_log,p_data->period_log,p_data->max_hops,p_data->min_hops);
#endif

}

static void aw_mesh_dump_lpn_polltimeout_status(AW_MESH_CONF_LPN_POLLTIMEOUT_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s lpn_addr %x polltimeout %x",__func__,p_data->lpn_addr,p_data->polltimeout);
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[lpn_addr %x polltimeout %x]",  \
           STR_LPN_POLLTIMEOUT_STATUS,p_data->lpn_addr,p_data->polltimeout);
#endif

}

static void aw_mesh_dump_nwk_trans_status(AW_MESH_CONF_NWK_TRANS_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s nwk_transcnt %x nwk_trans_intvl_steps %x",__func__,p_data->nwk_transcnt,p_data->nwk_trans_intvl_steps);
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[nwk_transcnt %x nwk_trans_intvl_steps %x]",  \
           STR_NETWORK_TRANS_STATUS,p_data->nwk_transcnt,p_data->nwk_trans_intvl_steps);
#endif
}

static void aw_mesh_dump_key_refresh_status(AW_MESH_CONF_KEYREFRESH_PHASE_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s status %x netkey_index %x phase %x",__func__,p_data->status,p_data->netkey_index,p_data->phase);
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[status %x netkey_idx %x phase %x]",  \
           STR_KEYREFRESH_PHASE_STATUS,p_data->status,p_data->netkey_index,p_data->phase);
#endif
}

static void aw_mesh_dump_node_reset_status()
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s",__func__);
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[status %x]",  STR_NODE_RESET_STATUS,0);
#endif
}

static void aw_mesh_dump_beacon_status(AW_MESH_CONF_BEACON_STATUS_T * p_data)
{
    mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s beacon %x",__func__,p_data->beacon);
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[beacon %x]",  \
           STR_BEACON_STATUS,p_data->beacon);
#endif

}

/*end of aw mesh config client model message dump*/

static void aw_mesh_stack_init_result(AW_MESH_EVENT_T *event,void *private)
{
#ifdef MEST_TEST_LOG_ENABLE
      mesh_test_log("%s[ver %s]",STR_AW_MESH_APP_RUN_STATUS,STR_AW_MESH_APP_VERIFY_VERSION_ID);
#endif
}

static void aw_mesh_node_attach_result(AW_MESH_EVENT_T *event,void *private)
{
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[attached]",STR_AW_MESH_APP_ATTACHED);
#endif
}

static void aw_mesh_access_message_recevice(AW_MESH_EVENT_T *event,void *private)
{
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[notify]",STR_AW_MESH_APP_ACCESS_RX);
#endif
    demo_vnd_model_msg_receive(event,private);
}

static void aw_mesh_cfg_client_message_receice(AW_MESH_EVENT_T *event,void *private)
{
    switch(event->param.config_mdl_status.opcode.opcode)
    {
        case AW_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_STATUS:
            aw_mesh_dump_composition_data_status(&event->param.config_mdl_status.compo_data_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_APPKEY_STATUS:
            aw_mesh_dump_appkey_status(&event->param.config_mdl_status.appkey_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_APPKEY_LIST:
            aw_mesh_dump_appkey_list_status(&event->param.config_mdl_status.appkey_list);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_NETKEY_LIST:
            aw_mesh_dump_netkey_list_status(&event->param.config_mdl_status.netkey_list);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_NETKEY_STATUS:
            aw_mesh_dump_netkey_status(&event->param.config_mdl_status.netkey_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_MODEL_APP_STATUS:
            aw_mesh_dump_app_status(&event->param.config_mdl_status.model_app_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_STATUS:
            aw_mesh_dump_node_ident_status(&event->param.config_mdl_status.node_ident_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_BEACON_STATUS:
            aw_mesh_dump_beacon_status(&event->param.config_mdl_status.beacon_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_RELAY_STATUS:
            aw_mesh_dump_relay_status(&event->param.config_mdl_status.relay_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_STATUS:
            aw_mesh_dump_gatt_proxy_status(&event->param.config_mdl_status.gatt_proxy_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_STATUS:
            aw_mesh_dump_def_ttl_status(&event->param.config_mdl_status.def_ttl_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_STATUS:
            aw_mesh_dump_pub_status(&event->param.config_mdl_status.model_pub_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_STATUS:
            aw_mesh_dump_sub_status(&event->param.config_mdl_status.model_sub_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_SUBSCRIPTION_LIST:
            aw_mesh_dump_vnd_sub_list_status(&event->param.config_mdl_status.vnd_model_sub_list);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_SUBSCRIPTION_LIST:
            aw_mesh_dump_sig_sub_list_status(&event->param.config_mdl_status.sig_model_sub_list);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_APP_LIST:
            aw_mesh_dump_sig_appkey_list_status(&event->param.config_mdl_status.sig_model_app_list);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_APP_LIST:
            aw_mesh_dump_vnd_app_list_status(&event->param.config_mdl_status.vnd_model_app_list);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_STATUS:
            aw_mesh_dump_hb_pub_status(&event->param.config_mdl_status.hb_pub_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS:
            aw_mesh_dump_hb_sub_status(&event->param.config_mdl_status.hb_sub_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_FRIEND_STATUS:
            aw_mesh_dump_friend_status(&event->param.config_mdl_status.frnd_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_STATUS:
            aw_mesh_dump_nwk_trans_status(&event->param.config_mdl_status.nwk_trans_status);
            break;
        case AW_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_STATUS:
            aw_mesh_dump_key_refresh_status(&event->param.config_mdl_status.keyrefresh_phase_status);
            break;
         case AW_MESH_ACCESS_MSG_CONFIG_NODE_RESET_STATUS:
            aw_mesh_dump_node_reset_status();
            break;
        default:
            break;
    }
}

static void aw_mesh_hlth_client_message_receive(AW_MESH_EVENT_T *event,void *private)
{
#ifdef MEST_TEST_LOG_ENABLE
      mesh_test_log("%s[notify]",STR_AW_MESH_HEALTH_CLIENT_RX);
#endif
}

static void aw_mesh_testmode_result(AW_MESH_EVENT_T *event,void *private)
{
#ifdef MEST_TEST_LOG_ENABLE
      mesh_test_log("%s[testmode %x]",STR_AW_MESH_IV_TEST_MODE_STATUS,event->param.iv_test_mode.state);
#endif
}

static void aw_mesh_ivupdate_result(AW_MESH_EVENT_T *event,void *private)
{
#ifdef MEST_TEST_LOG_ENABLE
      mesh_test_log("%s[iv_index %x iv_phase %x state %x]",STR_AW_MESH_IV_UPDATE_STATUS,event->param.iv_update.iv_index,event->param.iv_update.iv_phase,event->param.iv_update.state);
#endif
}

static void aw_mesh_key_refresh_result(AW_MESH_EVENT_T *event,void *private)
{

}

static void aw_mesh_unprov_scan_result(AW_MESH_EVENT_T *event, void *private)
{
	AW_MESH_PROV_SCAN_UD_T scan_ud = event->param.prov_scan_ud;

	l_info("oob_info %d", scan_ud.oob_info);
	LOG_PRINTF(AW_DBG_VERB_LEVE,"uuid : %s", getstr_hex2str(scan_ud.uuid, AW_MESH_UUID_SIZE));
	LOG_PRINTF(AW_DBG_VERB_LEVE,"uri_hash : %s", getstr_hex2str(scan_ud.uri_hash, AW_MESH_URI_HASH_LEN));
	LOG_PRINTF(AW_DBG_VERB_LEVE,"mac addr : %s", getstr_hex2str(scan_ud.mac, AW_MESH_BLE_ADDR_LEN));
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[oob_info %d uuid : %s uri_hash : %s mac addr : %s]",STR_AW_MESH_UD_SCAN_RESULT,scan_ud.oob_info,  \
        getstr_hex2str(scan_ud.uuid, AW_MESH_UUID_SIZE),getstr_hex2str1(scan_ud.uri_hash, AW_MESH_URI_HASH_LEN), \
        getstr_hex2str2(scan_ud.mac, AW_MESH_BLE_ADDR_LEN));
#endif
}

static void aw_mesh_prov_device_capabilities(AW_MESH_EVENT_T *event, void *private)
{
	AW_MESH_PROV_CAPABILITIES_T cap = event->param.prov_cap.cap;
	uint8_t pub_type, auth_type, auth_size;
	uint16_t auth_action;
	l_info("algo %d ele_num %d public type %d static type %d input_oob_action %d input_oob_size %d output_action %d output_size %d", cap.algorithms, cap.number_of_elements, cap.public_key_type,
			cap.static_oob_type, cap.input_oob_action, cap.input_oob_size, cap.output_oob_action, cap.output_oob_size);

	if (cap.public_key_type != 0)
		pub_type = 1;
	else
		pub_type = 0;

	if (cap.static_oob_type != 0) {
		auth_type = 1;
		auth_size = 0;
		auth_action = 0;
	} else if (cap.output_oob_action != 0 && cap.output_oob_size != 0) {
		auth_type = 2;
		auth_size = cap.output_oob_size;
		auth_action = cap.output_oob_action;
	} else if (cap.input_oob_action != 0 && cap.input_oob_size != 0) {
		auth_type = 3;
		auth_size = cap.input_oob_size;
		auth_action = cap.input_oob_action;
	} else {
		auth_type = 0;
		auth_size = 0;
		auth_action = 0;
	}
	aw_mesh_prov_set_start_choose_paramters(pub_type, auth_type, auth_size, auth_action);
}

static void aw_mesh_prov_request_oob_public_key(AW_MESH_EVENT_T *event, void *private)
{
	l_info("for test");
     uint8_t auth_value_default[64] = { \
         0x4c,0xc4,0x60,0x57,0x14,0x71,0xdd,0x90,0x10,0x61,0xba,0xbd,0xe5,0x34,0x05,0x9c,
         0x75,0xdd,0xf2,0x36,0xe8,0x35,0xfd,0x69,0xdc,0xc4,0xbb,0x57,0x5a,0x50,0xea,0xdf,
         0xa9,0x24,0xac,0x48,0x23,0x19,0xa1,0xd0,0x3b,0x2f,0x3c,0xa0,0x93,0x9b,0x50,0xd2,
         0x66,0x3e,0x95,0xbf,0xf8,0x5f,0x8d,0x1c,0x76,0x7c,0x2b,0x75,0xaa,0xa0,0xec,0x41
     };

    if((event == NULL)||(event->param.prov_show_pk.pk == NULL))
        return ;

	memcpy(event->param.prov_show_pk.pk,&auth_value_default[0],64);
	//aw_mesh_prov_set_pub_key(auth_value_default);
}

static void aw_mesh_prov_request_oob_auth_value(AW_MESH_EVENT_T *event, void *private)
{
	l_info("for test");
	uint8_t static_value[16] = {0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8};
    if(event == NULL)
        return ;
    memcpy(&event->param.prov_show_auth.auth[0],&static_value[0],16);
	//aw_mesh_prov_set_auth_static_value(static_value);
}

static void aw_mesh_prov_request_oob_auth_num(AW_MESH_EVENT_T *event, void *private)
{
	l_info("");
}

static void aw_mesh_prov_request_data(AW_MESH_EVENT_T *event, void *private)
{
    AW_MESH_EVT_PROV_DATA_REQUEST_T *pprov_data = NULL;
    if(event == NULL)
        return ;
    pprov_data = &event->param.prov_request_data;
    pprov_data->prov_set = false;
    l_info("%s prov_set %x net_idx %x unicast %x num_ele %x" ,__func__,pprov_data->prov_set,pprov_data->net_idx,pprov_data->unicast,pprov_data->num_ele);
}

static void aw_mesh_prov_show_oob_auth_value(AW_MESH_EVENT_T *event, void *private)
{
	uint8_t *auth_value = event->param.prov_show_auth.auth;

	LOG_PRINTF(AW_DBG_VERB_LEVE,"auth value : %s", getstr_hex2str(auth_value, 16));
}

static void aw_mesh_prov_show_oob_auth_num(AW_MESH_EVENT_T *event, void *private)
{
	uint32_t auth_num = event->param.prov_show_auth_num.auth_num;
	l_info("auth_num %d", auth_num);
}

static void aw_mesh_prov_done(AW_MESH_EVENT_T *event, void *private)
{
	AW_MESH_PROV_DONE_T prov_done = event->param.prov_done;

	LOG_PRINTF(AW_DBG_VERB_LEVE,"uuid value : %s", getstr_hex2str(prov_done.uuid, 16));
	LOG_PRINTF(AW_DBG_VERB_LEVE,"device key : %s", getstr_hex2str(prov_done.device_key, 16));
	l_info("addr %x ele_num %d success %d gatt_bear %d reason %d", prov_done.address, prov_done.element_num, prov_done.success, prov_done.gatt_bearer, prov_done.reason);

#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[success >> uuid:%s device-key:%s addr %x ele_num %d success %d gatt_bear %d reason %d]",STR_AW_MESH_PROV_DONE,getstr_hex2str(prov_done.uuid, 16),    \
        getstr_hex2str1(prov_done.device_key, 16),prov_done.address, prov_done.element_num, prov_done.success, prov_done.gatt_bearer, prov_done.reason);
#endif
}

static void aw_mesh_prov_failed(AW_MESH_EVENT_T *event, void *private)
{
	AW_MESH_PROV_DONE_T prov_done = event->param.prov_done;

	LOG_PRINTF(AW_DBG_VERB_LEVE,"uuid value : %s", getstr_hex2str(prov_done.uuid, 16));
	l_info("fail reason %d", prov_done.reason);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[fail >> uuid:%s reason %d]",STR_AW_MESH_PROV_DONE,getstr_hex2str(prov_done.uuid, 16),prov_done.reason);
#endif

}


static void aw_mesh_friendship_status(AW_MESH_EVENT_T *event, void *private)
{
	AW_MESH_EVT_FRIENDSHIP_STATUS_T friendship_status = event->param.friendship_status;

	LOG_PRINTF(AW_DBG_VERB_LEVE,"friendship change lpn_addr %d status %d reason %d", friendship_status.lpn_addr, friendship_status.status, friendship_status.reason);
#ifdef MEST_TEST_LOG_ENABLE
    switch(friendship_status.status)
    {
        case AW_MESH_FRIENDSHIP_ESTABLISHED:
            mesh_test_log("%s[lpn_addr %d status %d reason %d]",STR_AW_MESH_FRIEND_ESTABLISHED, friendship_status.lpn_addr,friendship_status.reason);
            break;
        case AW_MESH_FRIENDSHIP_TERMINATED:
            mesh_test_log("%s[lpn_addr %d status %d reason %d]",STR_AW_MESH_FRIEND_TERMINATED, friendship_status.lpn_addr,friendship_status.reason);
            break;
        default:
            mesh_test_log("%s[lpn_addr %d status %d reason %d]",STR_AW_MESH_FRIEND_TERMINATED, friendship_status.lpn_addr, friendship_status.status,friendship_status.reason);
            break;
    }
#endif
}

static void aw_mesh_heartbeat_notify(AW_MESH_EVENT_T *event, void *private)
{
    AW_MESH_EVT_HEARTBEAT_T *heartbeat = &event->param.heartbeat;

    LOG_PRINTF(AW_DBG_VERB_LEVE,"heartbeat notify address %x dst %x rssi %d hops %x feat %x",heartbeat->src,heartbeat->dst,heartbeat->rssi,heartbeat->ttl,heartbeat->hops,heartbeat->feature);
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[address %x dst %x rssi %d hops %x feat %x",STR_HEARTBEAT_PKT_TAG, heartbeat->src,heartbeat->dst,heartbeat->rssi,heartbeat->ttl,heartbeat->hops,heartbeat->feature);
#endif

}

static void aw_mesh_stack_reply_status(AW_MESH_EVENT_T *event, void *private)
{
	AW_MESH_STACK_REPLY_T stack_reply = event->param.stack_reply;

	switch (stack_reply.req.type) {
	case AW_MESH_SET_PROCOTOL_PARAM_REQ:
		LOG_PRINTF(AW_DBG_VERB_LEVE,"AW_MESH_SET_PROCOTOL_PARAM_REQ status %d", stack_reply.status);
		break;
	case AW_MESH_UNPROV_SCAN_START:
		LOG_PRINTF(AW_DBG_VERB_LEVE,"AW_MESH_UNPROV_SCAN_START status %d", stack_reply.status);
		break;
	case AW_MESH_UNPROV_SCAN_STOP:
		LOG_PRINTF(AW_DBG_VERB_LEVE,"AW_MESH_UNPROV_SCAN_STOP status %d", stack_reply.status);
		break;
	case AW_MESH_PROV_INVITE_REQ:
		LOG_PRINTF(AW_DBG_VERB_LEVE,"AW_MESH_PROV_INVITE_REQ status %d", stack_reply.status);
		break;
	case AW_MESH_PROV_CANCEL_REQ:
		LOG_PRINTF(AW_DBG_VERB_LEVE,"AW_MESH_PROV_CANCEL_REQ status %d", stack_reply.status);
        break;
    case AW_MESH_NODE_CFG_REQ:
        LOG_PRINTF(AW_DBG_VERB_LEVE,"AW_MESH_NODE_CFG_REQ status %d", stack_reply.status);
        break;
	default:
		LOG_PRINTF(AW_DBG_VERB_LEVE,"not match req type %d",stack_reply.status);
		break;
	}
}

void aw_mesh_cfg_info_req(AW_MESH_EVENT_T *event,void *private)
{
    AW_MESH_CFG_INFO_REQ_T          *p_info;
    if(event == NULL)
    {
        LOG_PRINTF(AW_DBG_VERB_LEVE,"%s error ,event %p private %p\n",__func__,event,private);
        return ;
    }
    p_info = &event->param.config_info;
	p_info->company_id  = AW_MESH_APP_COMPANY_ID;
	p_info->product_id  = AW_MESH_APP_PRODUCT_ID;
	p_info->version_id  = AW_MESH_APP_VERSION_ID;
    p_info->feature     = CONFIG_NODE_FEATURE;
    LOG_PRINTF(AW_DBG_VERB_LEVE,"%s app config company_id %x product_id %x version_id %x feature %x\n",__func__,p_info->company_id,p_info->product_id,p_info->version_id,p_info->feature);
}

struct
{
    UINT32 evt_code;
    AwMeshEventCb_t handler;
}m_adapter_cb_handler[AW_MESH_EVENT_MAX] =
{
    [AW_MESH_EVENT_STACK_INIT_DONE]                 = {AW_MESH_EVENT_STACK_INIT_DONE,&aw_mesh_stack_init_result},
    [AW_MESH_EVENT_LOCAL_NODE_ATTACH_NODE]          = {AW_MESH_EVENT_LOCAL_NODE_ATTACH_NODE,&aw_mesh_node_attach_result},
    [AW_MESH_EVENT_ACCESS_MSG]                      = {AW_MESH_EVENT_ACCESS_MSG,&aw_mesh_access_message_recevice},
    [AW_MESH_EVENT_CFG_MDL_MSG]                     = {AW_MESH_EVENT_CFG_MDL_MSG,&aw_mesh_cfg_client_message_receice},
    [AW_MESH_EVENT_HLTH_MDL_MSG]                    = {AW_MESH_EVENT_HLTH_MDL_MSG,&aw_mesh_hlth_client_message_receive},
    [AW_MESH_EVENT_IV_TEST_MODE]                    = {AW_MESH_EVENT_IV_TEST_MODE,&aw_mesh_testmode_result},//AW_MESH_EVENT_PROV_REQUEST_DATA
    [AW_MESH_EVENT_IV_UPDATE]                       = {AW_MESH_EVENT_IV_UPDATE,&aw_mesh_ivupdate_result},
    [AW_MESH_EVENT_KEY_REFRESH]                     = {AW_MESH_EVENT_KEY_REFRESH,&aw_mesh_key_refresh_result},
    [AW_MESH_EVENT_HEARTBEAT]                       = {AW_MESH_EVENT_HEARTBEAT,&aw_mesh_heartbeat_notify},
    [AW_MESH_EVENT_PROV_SCAN_UD_RESULT]             = {AW_MESH_EVENT_PROV_SCAN_UD_RESULT, &aw_mesh_unprov_scan_result},
    [AW_MESH_EVENT_PROV_CAPABILITIES]               = {AW_MESH_EVENT_PROV_CAPABILITIES, &aw_mesh_prov_device_capabilities},
    [AW_MESH_EVENT_PROV_REQUEST_OOB_PUBLIC_KEY]     = {AW_MESH_EVENT_PROV_REQUEST_OOB_PUBLIC_KEY, &aw_mesh_prov_request_oob_public_key},
    [AW_MESH_EVENT_PROV_REQUEST_OOB_AUTH_VALUE]     = {AW_MESH_EVENT_PROV_REQUEST_OOB_AUTH_VALUE, &aw_mesh_prov_request_oob_auth_value},
    [AW_MESH_EVENT_PROV_REQUEST_OOB_AUTH_NUM]       = {AW_MESH_EVENT_PROV_REQUEST_OOB_AUTH_NUM, &aw_mesh_prov_request_oob_auth_num},
    [AW_MESH_EVENT_PROV_REQUEST_DATA]               = {AW_MESH_EVENT_PROV_REQUEST_DATA, &aw_mesh_prov_request_data},
    [AW_MESH_EVENT_PROV_SHOW_OOB_AUTH_VALUE]        = {AW_MESH_EVENT_PROV_SHOW_OOB_AUTH_VALUE, &aw_mesh_prov_show_oob_auth_value},
    [AW_MESH_EVENT_PROV_SHOW_OOB_AUTH_VALUE_NUM]    = {AW_MESH_EVENT_PROV_SHOW_OOB_AUTH_VALUE_NUM, &aw_mesh_prov_show_oob_auth_num},
    [AW_MESH_EVENT_PROV_DONE]                       = {AW_MESH_EVENT_PROV_DONE, &aw_mesh_prov_done},
    [AW_MESH_EVENT_PROV_FAILED]                     = {AW_MESH_EVENT_PROV_FAILED, &aw_mesh_prov_failed},
    [AW_MESH_EVENT_FRIEND_STATUS]                   = {AW_MESH_EVENT_FRIEND_STATUS, &aw_mesh_friendship_status},
    [AW_MESH_STACK_REPLY_STATUS]                    = {AW_MESH_STACK_REPLY_STATUS, &aw_mesh_stack_reply_status},
    [AW_MESH_EVENT_MODEL_RX_CB]                     = {AW_MESH_EVENT_MODEL_RX_CB,&demo_model_msg_receive},
    [AW_MESH_CONFIG_INFO_REQ]                       = {AW_MESH_CONFIG_INFO_REQ,&aw_mesh_cfg_info_req}
};

static void aw_mesh_event_help(AW_MESH_EVENT_T *event , void *user_data)
{
    switch(event->evt_code)
    {
        case AW_MESH_STACK_REPLY_STATUS:
            if(event->param.stack_reply.readme)
                mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"evt code 0x%x status %x stack_status %x log %s",event->evt_code,event->param.stack_reply.status,  \
                    event->param.stack_reply.stack_errcode,event->param.stack_reply.readme);
            else
                mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"evt code 0x%x status %x stack_status %x",event->evt_code,event->param.stack_reply.status,  \
                event->param.stack_reply.stack_errcode);
            break;
        default:
            mesh_log(AW_DBG_VERB_LEVE,AW_APP_EVENT_CB_MODULE,"%s evt code 0x%x\n",__func__,event->evt_code);
            break;
    }
}

static void aw_mesh_event_receive(AW_MESH_EVENT_T *event , void *user_data)
{
    if((!event)||(event->evt_code >= AW_MESH_EVENT_MAX))
        return ;
    if(m_adapter_cb_handler[event->evt_code].handler)
    {
        m_adapter_cb_handler[event->evt_code].handler(event,user_data);
    }
    aw_mesh_event_help(event,user_data);
}

AwMeshEventCb_t aw_mesh_get_event_cb()
{
    return &aw_mesh_event_receive;
}
