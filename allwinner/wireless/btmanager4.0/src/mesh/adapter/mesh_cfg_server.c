#include <ell/ell.h>
#include "dbus.h"
#include "mesh_db.h"
#include "bluez/include/error.h"
#include "mesh_internal_api.h"


#define AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message)    {   \
    mesh_event.evt_code = AW_MESH_EVENT_CFG_MDL_MSG;    \
    memcpy(&mesh_event.param.config_mdl_status.meta_data,&p_message->meta_data,sizeof(AW_MESH_ACCESS_RX_META_T)); \
    mesh_event.param.config_mdl_status.opcode.company_id = p_message->opcode.company_id;    \
    mesh_event.param.config_mdl_status.opcode.opcode = p_message->opcode.opcode;    \
}


bool aw_is_opcode_of_model(const aw_access_opcode_handler_t *handler_list, UINT32 *hdl_index,AW_MESH_ACCESS_OPCODE_T opcode);
static uint32_t app_print_mod_id(const uint8_t *data, bool vid)
{
	uint32_t mod_id;

	if (!vid) {
		mod_id = l_get_le16(data);
		l_debug("Model Id\t%4.4x", mod_id);
		mod_id = 0xffff0000 | mod_id;
	} else {
		mod_id = l_get_le16(data + 2);
		l_info("vendor Model Id\t%4.4x %4.4x",
				l_get_le16(data), mod_id);
		mod_id = l_get_le16(data) << 16 | mod_id;
	}
	return mod_id;
}

static void config_composition_data_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    //page number
    mesh_event.param.config_mdl_status.compo_data_status.page = pkt[0];
    pkt++;
    //composition data
    mesh_event.param.config_mdl_status.compo_data_status.company_id = l_get_le16(&pkt[0]);
    mesh_event.param.config_mdl_status.compo_data_status.product_id = l_get_le16(&pkt[2]);
    mesh_event.param.config_mdl_status.compo_data_status.version_id = l_get_le16(&pkt[4]);
    mesh_event.param.config_mdl_status.compo_data_status.crpl       = l_get_le16(&pkt[6]);
    mesh_event.param.config_mdl_status.compo_data_status.features   = l_get_le16(&pkt[8]);
    //element data
    pkt += 10;
    mesh_event.param.config_mdl_status.compo_data_status.data = pkt;
    mesh_event.param.config_mdl_status.compo_data_status.data_len = p_message->dlen - 11;
    event_cb(&mesh_event,NULL);

}

static void config_appkey_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    uint16_t net_idx,app_idx;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
	net_idx = l_get_le16(pkt + 1) & 0xfff;
	app_idx = l_get_le16(pkt + 2) >> 4;
	l_info("NetKey\t%4.4x", net_idx);
	l_info("AppKey\t%4.4x", app_idx);
    mesh_event.param.config_mdl_status.appkey_status.status = pkt[0];
    mesh_event.param.config_mdl_status.appkey_status.appkey_index = app_idx;
    mesh_event.param.config_mdl_status.appkey_status.netkey_index = net_idx;
    event_cb(&mesh_event,NULL);
}

static void config_appkey_list_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint16_t appkey_list[AW_MESH_MAX_APPKEY_LIST];
    uint32_t *key_pair;
    uint8_t key_nb,key_idx;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    key_nb = (p_message->dlen - 3)/3*2;
    for(key_idx = 0; key_idx < key_nb; key_idx += 2)
    {
        key_pair = (uint32_t *)&pkt[key_idx/2*3 + 3];
        appkey_list[key_idx] = (*key_pair>>12)&0xFFF;
        appkey_list[key_idx + 1] = *key_pair&0xFFF;;
    }
    if(p_message->dlen%3 == 2)
    {
        appkey_list[key_idx] = l_get_le16(&pkt[key_nb/2*3+3]) & 0xfff;
        key_nb++;
    }

    mesh_event.param.config_mdl_status.appkey_list.status = pkt[0];
    mesh_event.param.config_mdl_status.appkey_list.netkey_index = l_get_le16(pkt+1) & 0xfff;
    mesh_event.param.config_mdl_status.appkey_list.num_of_appkey = key_nb;
    mesh_event.param.config_mdl_status.appkey_list.pappkeyindexes = &appkey_list[0];
    l_debug("status %x src %x appkey_nb %x p_message->dlen =%x data=%s",pkt[0],p_message->meta_data.src_addr,key_nb,p_message->dlen,getstr_hex2str(p_message->data,p_message->dlen));

    event_cb(&mesh_event,NULL);
}

static void config_netkey_list_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint16_t netkey_list[AW_MESH_MAX_NETKEY_LIST];
    uint8_t  pair_key_num,key_idx;
    uint32_t *pair_key;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    pair_key_num = p_message->dlen/3;

    for(key_idx = 0; pair_key_num ; key_idx += 2, pair_key_num--)
    {
        pair_key = (uint32_t *)&pkt[key_idx/2*3];
        netkey_list[key_idx]    = (*pair_key>>12)&0x0FFF;
        netkey_list[key_idx+1]  = *pair_key&0x0FFF;
    }
    pair_key_num = p_message->dlen/3*2;
    if(p_message->dlen % 3 == 2)
    {
        netkey_list[pair_key_num] = pkt[key_idx/2*3]|(pkt[key_idx/2*3+1]<<8);
        pair_key_num++;
    }
    l_debug("src %x netkey list num %x p_message->dlen =%x data=%s",p_message->meta_data.src_addr,pair_key_num,p_message->dlen,getstr_hex2str(p_message->data,p_message->dlen));
    mesh_event.param.config_mdl_status.netkey_list.num_of_netkey = pair_key_num;
    mesh_event.param.config_mdl_status.netkey_list.pnetkeyindexes = &netkey_list[0];
	event_cb(&mesh_event,NULL);
}

static void config_netkey_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    uint16_t net_idx;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
	net_idx = l_get_le16(pkt + 1) & 0xfff;
	l_debug("\tNetKey %3.3x", net_idx);
	mesh_event.param.config_mdl_status.netkey_status.status = pkt[0];
	mesh_event.param.config_mdl_status.netkey_status.netkey_index = net_idx;
	event_cb(&mesh_event,NULL);
}

static void config_model_app_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
	uint32_t len, mod_id;
	uint16_t app_idx, addr;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    len = p_message->dlen;
	addr = l_get_le16(pkt + 1);
	app_idx = l_get_le16(pkt + 3);

	l_info("Element Addr\t%4.4x",addr);
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
	mod_id = app_print_mod_id(pkt + 5, (len == 9) ? true : false);

	mesh_event.param.config_mdl_status.model_app_status.status = pkt[0];
	mesh_event.param.config_mdl_status.model_app_status.elem_addr = addr;
	mesh_event.param.config_mdl_status.model_app_status.appkey_index = app_idx;

	if(len == 9)
		mesh_event.param.config_mdl_status.model_app_status.model_id.company_id = mod_id >> 16;
	else
		mesh_event.param.config_mdl_status.model_app_status.model_id.company_id = 0xffff;

	mesh_event.param.config_mdl_status.model_app_status.model_id.model_id = mod_id & 0xffff;
    event_cb(&mesh_event,NULL);
//	l_info("%s :Element Addr\t%4.4x addr %x company_id %x model_id %x",__func__,ele_addr, addr, param.config_msg.model_app_status.model_id.company_id, param.config_msg.model_app_status.model_id.model_id);
}

static void config_model_sig_app_list_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data,key_nb,key_idx;
    uint16_t appkey_list[AW_MESH_MAX_APPKEY_LIST];
    uint32_t *key_pair;

    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    key_nb = (p_message->dlen - 5)/3*2;
    for(key_idx = 0; key_idx < key_nb; key_idx += 2)
    {
        key_pair = (uint32_t *)&pkt[key_idx/2*3 + 5];
        appkey_list[key_idx] = (*key_pair>>12)&0xFFF;
        appkey_list[key_idx + 1] = *key_pair&0xFFF;;
    }
    if((p_message->dlen - 5)%3 == 2)
    {
        appkey_list[key_idx] = l_get_le16(&pkt[key_nb/2*3+5]) & 0xfff;
        key_nb++;
    }

    mesh_event.param.config_mdl_status.sig_model_app_list.status = pkt[0];
    mesh_event.param.config_mdl_status.sig_model_app_list.elem_addr = l_get_le16(&pkt[1]) & 0xfff;
    mesh_event.param.config_mdl_status.sig_model_app_list.model_id = l_get_le16(&pkt[3]) & 0xfff;
    mesh_event.param.config_mdl_status.sig_model_app_list.num_of_appkey = key_nb;
    mesh_event.param.config_mdl_status.sig_model_app_list.pappkeyindexes = &appkey_list[0];

    event_cb(&mesh_event,NULL);
    //	l_info("%s :Element Addr\t%4.4x addr %x company_id %x model_id %x",__func__,ele_addr, addr, param.config_msg.model_app_status.model_id.company_id, param.config_msg.model_app_status.model_id.model_id);
}

static void config_node_identity_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
	mesh_event.param.config_mdl_status.node_ident_status.status         = pkt[0];
	mesh_event.param.config_mdl_status.node_ident_status.netkey_index   = l_get_le16(pkt + 1);
	mesh_event.param.config_mdl_status.node_ident_status.identity       = pkt[3];
    event_cb(&mesh_event,NULL);
}

static void config_beacon_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    mesh_event.param.config_mdl_status.beacon_status.beacon = pkt[0];
    event_cb(&mesh_event,NULL);
}

static void config_relay_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    mesh_event.param.config_mdl_status.relay_status.relay = pkt[0];
    mesh_event.param.config_mdl_status.relay_status.relay_retrans_cnt = (pkt[1] & 0x7);
    mesh_event.param.config_mdl_status.relay_status.relay_retrans_intvlsteps = (pkt[1] >> 3);
    event_cb(&mesh_event,NULL);
}

static void config_gatt_proxy_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    mesh_event.param.config_mdl_status.gatt_proxy_status.gatt_proxy = pkt[0];
    event_cb(&mesh_event,NULL);
}

static void config_default_ttl_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    mesh_event.param.config_mdl_status.def_ttl_status.ttl = pkt[0];
    event_cb(&mesh_event,NULL);
}

static void config_model_publication_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    uint32_t mod_id,len;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    len = p_message->dlen;
    mod_id = app_print_mod_id(pkt + 10, (len == 14) ? true : false);
    l_info("Rexmit count\t%d", (pkt[9] & 0x7));
    l_info("Rexmit steps\t%d", (pkt[9] >> 3));
    mesh_event.param.config_mdl_status.model_pub_status.status = pkt[0];
    mesh_event.param.config_mdl_status.model_pub_status.elem_addr = l_get_le16(pkt + 1);
    mesh_event.param.config_mdl_status.model_pub_status.pub_addr = l_get_le16(pkt + 3);
    mesh_event.param.config_mdl_status.model_pub_status.appkey_index = l_get_le16(pkt + 5)&0xfff;
    mesh_event.param.config_mdl_status.model_pub_status.cred_flag = (l_get_le16(pkt + 5)&0x1000)>>12;
    mesh_event.param.config_mdl_status.model_pub_status.pub_ttl = pkt[7];
    mesh_event.param.config_mdl_status.model_pub_status.pub_perid = pkt[8];
    mesh_event.param.config_mdl_status.model_pub_status.pub_retrans_cnt = (pkt[9] & 0x7);
    mesh_event.param.config_mdl_status.model_pub_status.pub_retrans_intvl_steps = (pkt[9] >> 3);
    if(len == 14)
        mesh_event.param.config_mdl_status.model_pub_status.model_id.company_id = mod_id >> 16;
    else
        mesh_event.param.config_mdl_status.model_pub_status.model_id.company_id = AW_MESH_COMPANY_ID_SIG;

    mesh_event.param.config_mdl_status.model_pub_status.model_id.model_id = mod_id & 0xffff;
    //l_info("pub status :Element Addr\t%4.4x addr %x company_id %x model_id %x", l_get_le16(pkt + 1), addr, mesh_event.param.config_mdl_status.model_pub_status.model_id.company_id, mesh_event.param.config_mdl_status.model_pub_status.model_id.model_id);
    event_cb(&mesh_event,NULL);
}

static void config_model_subscription_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    uint32_t mod_id,len;

    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    len = p_message->dlen;
    mod_id = app_print_mod_id(pkt + 5, (len == 9) ? true : false);
    mesh_event.param.config_mdl_status.model_sub_status.status = pkt[0];
    mesh_event.param.config_mdl_status.model_sub_status.elem_addr = l_get_le16(pkt + 1);
    mesh_event.param.config_mdl_status.model_sub_status.address = l_get_le16(pkt + 3);

    if(len == 9)
	mesh_event.param.config_mdl_status.model_sub_status.model_id.company_id = mod_id >> 16;
    else
	mesh_event.param.config_mdl_status.model_sub_status.model_id.company_id = AW_MESH_COMPANY_ID_SIG;

    mesh_event.param.config_mdl_status.model_sub_status.model_id.model_id = mod_id & 0xffff;
    //l_info("sub status :Element Addr\t%4.4x addr %x company_id %x model_id %x", ele_addr, addr, mesh_event.param.config_mdl_status.model_sub_status.model_id.company_id, mesh_event.param.config_mdl_status.model_sub_status.model_id.model_id);
    event_cb(&mesh_event,NULL);
}

static void config_vendor_model_subscription_list_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    uint8_t i;
    uint32_t len = p_message->dlen;

    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    if((len - 7) / 2 > 0)
    {
        for (i = 7; i < len; i += 2)
            l_info("Subscr Addr\t%4.4x", l_get_le16(pkt + i));
    }
    mesh_event.param.config_mdl_status.vnd_model_sub_list.status = pkt[0];
    mesh_event.param.config_mdl_status.vnd_model_sub_list.vnd_model_id.company_id = l_get_le16(pkt + 5);
    mesh_event.param.config_mdl_status.vnd_model_sub_list.vnd_model_id.model_id = l_get_le16(pkt + 3);
    mesh_event.param.config_mdl_status.vnd_model_sub_list.elem_addr = l_get_le16(pkt + 1);
    mesh_event.param.config_mdl_status.vnd_model_sub_list.num = (len - 7) / 2;
    if((len - 7) / 2 > 0)
        mesh_event.param.config_mdl_status.vnd_model_sub_list.addresses = (uint16_t *)(pkt + 7);
    else
        mesh_event.param.config_mdl_status.vnd_model_sub_list.addresses = NULL;
    l_info("vendor sub: compay_id %x model_id %x num %x", l_get_le16(pkt + 5), l_get_le16(pkt + 3), mesh_event.param.config_mdl_status.vnd_model_sub_list.num);
    event_cb(&mesh_event,NULL);
}

static void config_sig_model_subscription_list_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data, i;
    uint32_t len = p_message->dlen;

    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
	l_info("Element Addr\t%4.4x", l_get_le16(pkt + 1));
	l_info("Model ID\t%4.4x", l_get_le16(pkt + 3));

	for (i = 5; i < len; i += 2)
		l_info("Subscr Addr\t%4.4x", l_get_le16(pkt + i));

	mesh_event.param.config_mdl_status.sig_model_sub_list.status = pkt[0];
	mesh_event.param.config_mdl_status.sig_model_sub_list.elem_addr = l_get_le16(pkt + 1);
	mesh_event.param.config_mdl_status.sig_model_sub_list.sig_model_id = l_get_le16(pkt + 3);
	mesh_event.param.config_mdl_status.sig_model_sub_list.num = (len - 5) / 2;
	mesh_event.param.config_mdl_status.sig_model_sub_list.addresses = (uint16_t *)(pkt + 5);
    event_cb(&mesh_event,NULL);
}

static void config_heartbeat_publication_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    l_info("Destination\t%4.4x", l_get_le16(pkt + 1));
    l_info("Count\t\t%2.2x", pkt[3]);
    l_info("Period\t\t%2.2x", pkt[4]);
    l_info("TTL\t\t%2.2x", pkt[5]);
    l_info("Features\t%4.4x", l_get_le16(pkt + 6));
    l_info("Net_Idx\t%4.4x", l_get_le16(pkt + 8));

    mesh_event.param.config_mdl_status.hb_pub_status.status = pkt[0];
    mesh_event.param.config_mdl_status.hb_pub_status.dest_addr = l_get_le16(pkt + 1);
    mesh_event.param.config_mdl_status.hb_pub_status.count_log = pkt[3];
    mesh_event.param.config_mdl_status.hb_pub_status.period_log = pkt[4];
    mesh_event.param.config_mdl_status.hb_pub_status.ttl = pkt[5];
    mesh_event.param.config_mdl_status.hb_pub_status.features = l_get_le16(pkt + 6);
    mesh_event.param.config_mdl_status.hb_pub_status.netkey_index = l_get_le16(pkt + 8);

    event_cb(&mesh_event,NULL);

}

static void config_heartbeat_subscription_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    l_info("Source\t\t%4.4x", l_get_le16(pkt + 1));
    l_info("Destination\t%4.4x", l_get_le16(pkt + 3));
    l_info("Period\t\t%2.2x", pkt[5]);
    l_info("Count\t\t%2.2x", pkt[6]);
    l_info("Min Hops\t%2.2x", pkt[7]);
    l_info("Max Hops\t%2.2x", pkt[8]);
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    mesh_event.param.config_mdl_status.hb_sub_status.status = pkt[0];
    mesh_event.param.config_mdl_status.hb_sub_status.src_addr = l_get_le16(pkt + 1);
    mesh_event.param.config_mdl_status.hb_sub_status.dst_addr = l_get_le16(pkt + 3);
    mesh_event.param.config_mdl_status.hb_sub_status.period_log = pkt[5];
    mesh_event.param.config_mdl_status.hb_sub_status.count_log = pkt[6];
    mesh_event.param.config_mdl_status.hb_sub_status.min_hops = pkt[7];
    mesh_event.param.config_mdl_status.hb_sub_status.max_hops = pkt[8];

    event_cb(&mesh_event,NULL);
}

static void config_friend_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    mesh_event.param.config_mdl_status.frnd_status.friend = pkt[0];
    l_info("%s friend %d", __func__,pkt[0]);
    event_cb(&mesh_event,NULL);
}

static void config_network_transmit_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    mesh_event.param.config_mdl_status.nwk_trans_status.nwk_transcnt = (pkt[0] & 0x7);
    mesh_event.param.config_mdl_status.nwk_trans_status.nwk_trans_intvl_steps = (pkt[0] >> 3);
    l_info("%s network_transmit count  %d step %d",__func__,(pkt[0] & 0x7), (pkt[0] >> 3));

    event_cb(&mesh_event,NULL);
}

static void config_key_refresh_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    mesh_event.param.config_mdl_status.keyrefresh_phase_status.status = pkt[0];
    mesh_event.param.config_mdl_status.keyrefresh_phase_status.netkey_index = l_get_le16(pkt + 1);
    mesh_event.param.config_mdl_status.keyrefresh_phase_status.phase = pkt[3];
    event_cb(&mesh_event,NULL);
}

static void config_vnd_mdl_list_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    uint16_t appkey_list[AW_MESH_MAX_APPKEY_LIST];
    uint8_t key_nb,key_idx,key_len;
    AW_MESH_EVENT_T mesh_event;
    uint32_t *key_pair;
    uint8_t *pkt = p_message->data;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);

    key_len = p_message->dlen - 7;
    key_nb = key_len/3*2;

    for(key_idx = 0; key_idx < key_nb; key_idx += 2)
    {
        key_pair = (uint32_t *)&pkt[key_idx/2*3 + 3];
        appkey_list[key_idx] = (*key_pair>>12)&0xFFF;
        appkey_list[key_idx + 1] = *key_pair&0xFFF;;
    }

    if(key_len%3 == 2)
    {
        appkey_list[key_idx] = l_get_le16(&pkt[key_nb/2*3+3]) & 0xfff;
        key_nb++;
    }

    mesh_event.param.config_mdl_status.vnd_model_app_list.status = pkt[0];
    mesh_event.param.config_mdl_status.vnd_model_app_list.elem_addr = l_get_le16(pkt + 1);
    mesh_event.param.config_mdl_status.vnd_model_app_list.vnd_model_id.model_id = l_get_le32(pkt + 3);
    mesh_event.param.config_mdl_status.vnd_model_app_list.vnd_model_id.company_id= l_get_le32(pkt + 5);
    mesh_event.param.config_mdl_status.vnd_model_app_list.num_of_appkey = key_nb;
    mesh_event.param.config_mdl_status.vnd_model_app_list.pappkeyindexes = &appkey_list[0];
    l_info("%s status %x ele_addr %x model_id %x company_id %x number of keys %x ",__func__,pkt[0],l_get_le16(pkt + 1),l_get_le32(pkt + 3),l_get_le32(pkt + 5),key_nb);
    event_cb(&mesh_event,NULL);
}

static void config_node_reset_status_handle(AwMeshEventCb_t event_cb,AW_MESH_ACCESS_MESSAGE_RX_T *p_message)
{
    AW_MESH_EVENT_T mesh_event;
    AW_ACCESS_MSG_META_RX_COPY(mesh_event,p_message);
    event_cb(&mesh_event,NULL);
}

static const aw_access_opcode_handler_t m_opcode_handlers[] =
{
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_STATUS),       config_composition_data_status_handle, 11},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_APPKEY_STATUS),                 config_appkey_status_handle, 4},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_APPKEY_LIST),                   config_appkey_list_handle, 5},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_NETKEY_LIST),                   config_netkey_list_handle, 2},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_NETKEY_STATUS),                 config_netkey_status_handle, 3},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_MODEL_APP_STATUS),              config_model_app_status_handle, 7},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_APP_LIST),            config_model_sig_app_list_handle, 5},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_STATUS),          config_node_identity_status_handle, 4},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_BEACON_STATUS),                 config_beacon_status_handle, 1},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_RELAY_STATUS),                  config_relay_status_handle, 2},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_STATUS),             config_gatt_proxy_status_handle, 1},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_STATUS),            config_default_ttl_status_handle, 1},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_STATUS),      config_model_publication_status_handle, 12},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_STATUS),     config_model_subscription_status_handle , 7},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_SUBSCRIPTION_LIST),config_vendor_model_subscription_list_handle, 7},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_SUBSCRIPTION_LIST),   config_sig_model_subscription_list_handle, 7},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_STATUS),  config_heartbeat_publication_status_handle, 10},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS), config_heartbeat_subscription_status_handle, 9},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_FRIEND_STATUS),                 config_friend_status_handle, 1},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_STATUS),       config_network_transmit_status_handle, 1},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_STATUS),      config_key_refresh_status_handle, 4},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_APP_LIST),         config_vnd_mdl_list_handle, 7},
    {AW_ACCESS_OPCODE_SIG(AW_MESH_ACCESS_MSG_CONFIG_NODE_RESET_STATUS),             config_node_reset_status_handle, 0}
};

//Internal Api
struct l_dbus_message * dbus_config_message_cb(struct l_dbus *dbus, struct l_dbus_message *message,
				 void *user_data)
{
	struct l_dbus_message *reply;
	struct l_dbus_message_iter iter_data;
	uint8_t *data;
	uint32_t len, handler_index = 0;
	uint16_t compay_id, opcode, src, dst, netkey_idx, appkey_idx;
	uint8_t   rssi, ttl;
    AW_MESH_EVENT_T mesh_event;
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();

	if (l_dbus_message_is_error(message)) {
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
		return dbus_error(message, MESH_ERROR_NONE,
			"Mesh message is empty");
	}

	if (!l_dbus_message_get_arguments(message, "qqqqqqyyay", &compay_id,
					&opcode, &src, &dst, &appkey_idx,
					&netkey_idx, &rssi, &ttl, &iter_data))
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
    }

	l_dbus_message_iter_get_fixed_array(&iter_data, &data, &len);
	if (!len)
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS,
					"Mesh message device key is empty");
    }
    mesh_event.param.access_rx_msg.opcode.company_id    = compay_id;
    mesh_event.param.access_rx_msg.opcode.opcode        = opcode;
    mesh_event.param.access_rx_msg.meta_data.src_addr   = src;
    mesh_event.param.access_rx_msg.meta_data.dst_addr   = dst;
    mesh_event.param.access_rx_msg.meta_data.netkey_index = netkey_idx;
    mesh_event.param.access_rx_msg.meta_data.appkey_index = appkey_idx;
    mesh_event.param.access_rx_msg.meta_data.rssi       = rssi;
    mesh_event.param.access_rx_msg.meta_data.ttl        = ttl;
    mesh_event.param.access_rx_msg.data                 = data;
    mesh_event.param.access_rx_msg.dlen                 = len;
    handler_index = sizeof(m_opcode_handlers)/sizeof(aw_access_opcode_handler_t);
    l_debug("%s opcode %x len %d",__func__,opcode,mesh_event.param.access_rx_msg.dlen);
    if(aw_is_opcode_of_model(&m_opcode_handlers[0],&handler_index,mesh_event.param.access_rx_msg.opcode) == true)
    {
        if((m_opcode_handlers[handler_index].handler)   \
            &&(mesh_event.param.access_rx_msg.dlen >= m_opcode_handlers[handler_index].size)    \
            &&(mesh_event.param.access_rx_msg.data) \
            &&event_cb)
        {
            m_opcode_handlers[handler_index].handler(event_cb,&mesh_event.param.access_rx_msg);
        }
        else
        {
            mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
        }
    }

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	return reply;
}

//Public Api
bool aw_is_opcode_of_model(const aw_access_opcode_handler_t *handler_list, UINT32 *hdl_index,AW_MESH_ACCESS_OPCODE_T opcode)
{
    UINT32 count = *hdl_index;

    if((handler_list == NULL)||(*hdl_index == 0))
        return false;

    for(UINT32 i = 0; i < count ; i++)
    {
        if(handler_list[i].opcode.opcode == opcode.opcode)
        {
            *hdl_index = i;
            return true;
        }
    }

    return false;
}
