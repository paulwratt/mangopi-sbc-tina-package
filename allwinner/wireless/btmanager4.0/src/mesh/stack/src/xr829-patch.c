
#include "mesh/xr829-patch.h"
#ifdef CONFIG_XR829_BT
#include <ell/ell.h>
#include "mesh/dbus.h"
#include "mesh/appkey.h"
#include "mesh/util.h"
#include "mesh/mesh-defs.h"
#include "mesh/node.h"
#include "mesh/cfgmod.h"
#include "mesh/net.h"
#include "mesh/mesh.h"
#include "mesh/crypto.h"
#include "mesh/node.h"
#include "mesh/mesh-config.h"
#include "mesh/cfgmod.h"
#include "mesh/error.h"
#include "mesh/dbus.h"
#include "mesh/model.h"
#include "mesh/keyring.h"
/*TRACE DEBUG LOG CONFIGURE*/

/*END OF TRACE DEBUG LOG CONFIGURE*/

//may define  in appkey.c
void appkey_append_key_indexes(void *a, void *b)
{
	struct mesh_app_key *key = a;
	struct l_dbus_message_builder *builder = b;

	l_dbus_message_builder_enter_struct(builder, "qq");

	l_dbus_message_builder_append_basic(builder, 'q', &key->app_idx);
	l_dbus_message_builder_append_basic(builder, 'q', &key->net_idx);

	l_dbus_message_builder_leave_struct(builder);
}
//may define in cfg-server.c
int lcl_key_ctl(struct mesh_node *node,lcl_key_mgr_t *p_key_mgr)
{
    int b_res = MESH_STATUS_SUCCESS;
    struct mesh_net *net = node_get_net(node);
    switch(p_key_mgr->opcode)
    {
        case OP_APPKEY_ADD:
            b_res = appkey_key_add(net,p_key_mgr->netkey_idx,p_key_mgr->appkey_idx,p_key_mgr->key);
            break;
        case OP_APPKEY_UPDATE:
            b_res = appkey_key_update(net,p_key_mgr->netkey_idx,p_key_mgr->appkey_idx,p_key_mgr->key);
            break;
        case OP_APPKEY_DELETE:
            b_res = appkey_key_delete(net, p_key_mgr->netkey_idx, p_key_mgr->appkey_idx);
            break;
        case OP_NETKEY_ADD:
            b_res = mesh_net_add_key(net,p_key_mgr->netkey_idx,p_key_mgr->key);
            break;
        case OP_NETKEY_UPDATE:
            b_res = mesh_net_update_key(net,p_key_mgr->netkey_idx,p_key_mgr->key);
            break;
        case OP_NETKEY_DELETE:
            b_res = mesh_net_del_key(net,p_key_mgr->netkey_idx);
            break;
        case OP_MODEL_APP_BIND:
            b_res = mesh_model_binding_add(node, p_key_mgr->element_addr, p_key_mgr->model_idx, p_key_mgr->appkey_idx);
            l_info("app bind return %d\n",b_res);
            break;
        case OP_MODEL_APP_UNBIND:
           b_res =  mesh_model_binding_del(node, p_key_mgr->element_addr, p_key_mgr->model_idx, p_key_mgr->appkey_idx);
            break;
        case OP_CONFIG_KEY_REFRESH_PHASE_SET:
            b_res = mesh_net_key_refresh_phase_set(net,p_key_mgr->netkey_idx,p_key_mgr->key_phase);
        default:
            break;
    }
    return b_res;
}

//may define  in util.c
char * getstr_hex2str(uint8_t *in, size_t in_len)
{
    static char str[512]={'\0'};
    if(hex2str(in,in_len,str,sizeof(str)) !=0)
       return str;
    else
        return NULL;
}

char * getstr_hex2str1(uint8_t *in, size_t in_len)
{
    static char str[512]={'\0'};
    if(hex2str(in,in_len,str,sizeof(str)) !=0)
       return str;
    else
        return NULL;
}

char * getstr_hex2str2(uint8_t *in, size_t in_len)
{
    static char str[512]={'\0'};
    if(hex2str(in,in_len,str,sizeof(str)) !=0)
       return str;
    else
        return NULL;
}


#if 0
void print_time(const char *label)
{
	struct timeval pkt_time;

	gettimeofday(&pkt_time, NULL);

   /* l_debug("time:%05d.%03d %s",
                    (uint32_t) pkt_time.tv_sec % 100000,
                    (uint32_t) pkt_time.tv_usec/1000, label);*/

}
#endif
#endif
