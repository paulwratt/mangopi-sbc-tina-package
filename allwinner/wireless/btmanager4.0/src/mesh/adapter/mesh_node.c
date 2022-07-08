#include <stdio.h>
#include "dbus.h"
#include "mesh_db.h"
#include "AWTypes.h"
#include "bluez/include/error.h"
#include "mesh_internal_api.h"
#include "model_adaptor.h"

static void send_call_reply(struct l_dbus_message *reply, void *user_data)
{
	int err;

	err = dbus_get_reply_error(reply);

	if (err == MESH_ERROR_NONE)
		l_info("Send procedure started");
	else {
		l_error("Send procedure failed");
	}
}

static void set_protocol_reply(struct l_dbus_message *reply, void *user_data)
{
	int err;

	err = dbus_get_reply_error(reply);
	mesh_stack_reply(AW_MESH_SET_PROCOTOL_PARAM_REQ, __func__, err);
}

static void iv_test_mode_reply(struct l_dbus_message *reply, void *user_data)
{
	int err;
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;
    uint8_t test_mode;
	err = dbus_get_reply_error(reply);

	if (err != MESH_ERROR_NONE) {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,err);
		return;
	}
	if (!l_dbus_message_get_arguments(reply, "y",&test_mode)) {
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
        return ;
	}

    l_info("%s test_mode %d",__func__,test_mode);

    if(event_cb)
    {
        // event callback to customer api
        mesh_event.evt_code = AW_MESH_EVENT_IV_TEST_MODE;
        mesh_event.param.iv_test_mode.state = test_mode;
        event_cb(&mesh_event,NULL);
    }

}

static void update_iv_info_reply(struct l_dbus_message *reply, void *user_data)
{
	int err;
	uint32_t iv_index;
	uint8_t state,flags;

    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;
    l_info("%s",__func__);
	err = dbus_get_reply_error(reply);

	if (err != MESH_ERROR_NONE) {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,err);
		return;
	}

	if (!l_dbus_message_get_arguments(reply, "uyy", &iv_index, &state, &flags)) {
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
        return ;
	}

    if(event_cb)
    {
        // event callback to customer api
        mesh_event.evt_code = AW_MESH_EVENT_IV_UPDATE;
        mesh_event.param.iv_update.iv_phase = state;
        mesh_event.param.iv_update.iv_index = iv_index;
        mesh_event.param.iv_update.state = flags;
        event_cb(&mesh_event,NULL);
    }
}

static void send_beacon_reply(struct l_dbus_message *reply, void *user_data)
{
	int err;

	err = dbus_get_reply_error(reply);
	if (err != MESH_ERROR_NONE) {
		mesh_stack_reply(AW_MESH_SET_PROCOTOL_PARAM_REQ, __func__, err);
	}
}

static int32_t mesh_node_send_beacon(struct mesh_application *app, uint16_t num, uint16_t interval)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;

	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
									BLUEZ_MESH_PATH, MESH_NETWORK_INTERFACE,
									"SendBeacon");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'o', app->path);
	l_dbus_message_builder_append_basic(builder, 'q', &num);
	l_dbus_message_builder_append_basic(builder, 'q', &interval);

	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_dbus_send_with_reply(app->dbus, msg, send_beacon_reply, NULL, NULL);

	return AW_ERROR_NONE;
}

//Internal Api
char *build_dbus_path(const char* prefix, uint8_t num)
{
	char *path = NULL;
	uint8_t path_len = strlen(prefix) + MESH_PATH_INDEX_LEN + 1;

	path = l_malloc(path_len);
	if (!path) {
		l_error("build dbus path failed");
		return NULL;
	}

	snprintf(path, path_len, "%s%02x", prefix, num);
    l_info("build_dbus_path-%s\n",path);

	return path;
}

struct l_dbus_message *dbus_iv_update_cb(struct l_dbus *dbus,struct l_dbus_message *message,
				   void *user_data)
{
	struct l_dbus_message *reply;
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;

	if (l_dbus_message_is_error(message)) {
     mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
	   return dbus_error(message, MESH_ERROR_NONE,
				 "Mesh message is empty");
	}

	if (!l_dbus_message_get_arguments(
	   message, "uy", &mesh_event.param.iv_update.iv_index, &mesh_event.param.iv_update.state))
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
	   return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
    }

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

    if(event_cb)
    {
        // event callback to customer api
        mesh_event.evt_code = AW_MESH_EVENT_IV_UPDATE;
        event_cb(&mesh_event,NULL);
    }

	return reply;
}

struct l_dbus_message *dbus_friendship_state_cb(struct l_dbus *dbus,struct l_dbus_message *message,
				  void *user_data)
{
   struct l_dbus_message *reply;
   AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
   AW_MESH_EVENT_T mesh_event;

	if (l_dbus_message_is_error(message)) {
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
		return dbus_error(message, MESH_ERROR_NONE,
				"Mesh message is empty");
   }

   if (!l_dbus_message_get_arguments(
	  message, "qyy", &mesh_event.param.friendship_status.lpn_addr, &mesh_event.param.friendship_status.status , \
	    &mesh_event.param.friendship_status.reason))
   {
	   mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
	   return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
   }

   reply = l_dbus_message_new_method_return(message);
   l_dbus_message_set_arguments(reply, "");

   if(event_cb)
   {
	   // event callback to customer api
	   mesh_event.evt_code = AW_MESH_EVENT_FRIEND_STATUS;
	   event_cb(&mesh_event,NULL);
   }

   return reply;
}

struct l_dbus_message *dbus_generic_message_cb(struct l_dbus *dbus, struct l_dbus_message *message,
			void *user_data)
{
	struct l_dbus_message *reply;
	struct l_dbus_message_iter iter_data;

	uint8_t *data;
	uint32_t len;
	uint16_t company_id, opcode, src, dst, netkey_idx, appkey_idx;
	uint8_t rssi, ttl, ele_idx;
    int status;
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;
    model_adaptor_message_t model_adaptor_msg;

	if (l_dbus_message_is_error(message)) {
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
		return dbus_error(message, MESH_ERROR_NONE,
				  "Mesh message is empty");
	}

	if (!l_dbus_message_get_arguments(message, "qqqqqqyyyay", &company_id,
					  &opcode, &src, &dst, &appkey_idx,
					  &netkey_idx, &rssi, &ttl, &ele_idx, &iter_data))
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
    }

	l_dbus_message_iter_get_fixed_array(&iter_data, &data, &len);

	if(event_cb)
    {
        mesh_event.evt_code = AW_MESH_EVENT_ACCESS_MSG;
        mesh_event.param.access_rx_msg.opcode.company_id = company_id;
        mesh_event.param.access_rx_msg.opcode.opcode= opcode;
        mesh_event.param.access_rx_msg.meta_data.ele_index = ele_idx;
        mesh_event.param.access_rx_msg.meta_data.src_addr = src;
        mesh_event.param.access_rx_msg.meta_data.dst_addr = dst;
        mesh_event.param.access_rx_msg.meta_data.netkey_index = netkey_idx;
        mesh_event.param.access_rx_msg.meta_data.appkey_index = appkey_idx;
        mesh_event.param.access_rx_msg.meta_data.rssi = rssi;
        mesh_event.param.access_rx_msg.meta_data.ttl = ttl;
        mesh_event.param.access_rx_msg.data = data;
        mesh_event.param.access_rx_msg.dlen = len;
        event_cb(&mesh_event,NULL);
    }
#ifdef NORDIC_MODELS_ENABLE
    model_adaptor_msg.src = src;
    model_adaptor_msg.dst = dst;
    model_adaptor_msg.ele_idx = ele_idx;
    model_adaptor_msg.opcode = opcode;
    model_adaptor_msg.buf = data;
    model_adaptor_msg.size = len;
    model_adaptor_msg.netkey_index = netkey_idx;
    model_adaptor_msg.appkey_index = appkey_idx;
    model_adaptor_msg.rssi = rssi;
    model_adaptor_msg.ttl = ttl;
    //l_info("src %x dst %x ele %x  rssi %d ttl %x\n",model_adaptor_msg.src,model_adaptor_msg.dst,model_adaptor_msg.ele_idx,
    //    model_adaptor_msg.rssi,model_adaptor_msg.ttl);
    status = mesh_model_adaptor_receive_msg(&model_adaptor_msg);
    l_info("aw mesh sig models message recevie flag %d opcode 0x%x msg opcode 0x%x",!status,opcode,model_adaptor_msg.opcode);
    //aw_mesh_model_receive_call(opcode,ele_idx,src,dst,data,len);
#endif
	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	return reply;
}

int32_t aw_mesh_local_key_mgr(void *pdata,uint8_t len)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();


    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);

	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
				app->node_path, MESH_MANAGEMENT_INTERFACE,
				"MgrLclKey");
	builder = l_dbus_message_builder_new(msg);
    dbus_append_byte_array(builder, pdata, len);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
    l_dbus_send_with_reply(app->dbus, msg, send_call_reply, NULL, NULL);

	return AW_ERROR_NONE;
}

int32_t aw_mesh_send_packet(int32_t dst, int32_t dst_addr_type, uint8_t ele_idx,
			int32_t ttl, int32_t net_key_idx, int32_t app_key_idx,
			uint8_t *data, int32_t data_len)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();
	struct node_element *element = mesh_element_find_by_idx(ele_idx);

    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);
    ELEMENT_READY_ACCESS(element);

	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
					app->node_path, MESH_NODE_INTERFACE,
					"Send");

	/* find element by idx */
	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'o', element->path);
	l_dbus_message_builder_append_basic(builder, 'q', &dst);
	l_dbus_message_builder_append_basic(builder, 'q', &app_key_idx);
	dbus_append_byte_array(builder, data, data_len);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send_with_reply(app->dbus, msg, send_call_reply, NULL, NULL);

	return AW_ERROR_NONE;
}

int32_t aw_mesh_model_publish(int32_t mod_id, int32_t dst_addr_type, uint8_t ele_idx,
			int32_t ttl, int32_t net_key_idx, int32_t app_key_idx,
			uint8_t *data, int32_t data_len)
{

	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();
	struct node_element *element = mesh_element_find_by_idx(ele_idx);

    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);
    ELEMENT_READY_ACCESS(element);


	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
					app->node_path, MESH_NODE_INTERFACE,"Publish");
	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'o', element->path);
	l_dbus_message_builder_append_basic(builder, 'q', &mod_id);
	dbus_append_byte_array(builder, data, data_len);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_dbus_send_with_reply(app->dbus, msg, send_call_reply, NULL, NULL);

	return AW_ERROR_NONE;
}

int32_t aw_mesh_vendor_model_publish(int32_t mod_id, int32_t dst_addr_type, uint8_t ele_idx,
			int32_t ttl, int32_t net_key_idx, int32_t app_key_idx,
			uint8_t *data, int32_t data_len)
{

	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();
	struct node_element *element = mesh_element_find_by_idx(ele_idx);
    uint16_t model_id = mod_id & 0xFFFF;
    uint16_t vendor = (mod_id>>16) & 0xFFFF;
    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);
    ELEMENT_READY_ACCESS(element);


	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
					app->node_path, MESH_NODE_INTERFACE,"VendorPublish");
	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'o', element->path);
    l_dbus_message_builder_append_basic(builder, 'q', &vendor);
	l_dbus_message_builder_append_basic(builder, 'q', &model_id);
	dbus_append_byte_array(builder, data, data_len);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_dbus_send_with_reply(app->dbus, msg, send_call_reply, NULL, NULL);

	return AW_ERROR_NONE;
}

struct l_dbus_message *dbus_heartbeat_message_cb(struct l_dbus *dbus, struct l_dbus_message *message,
			void *user_data)
{
	struct l_dbus_message *reply;
	uint16_t feature, src, dst;
	uint8_t rssi, ttl, hops;
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;

	if (l_dbus_message_is_error(message)) {
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
		return dbus_error(message, MESH_ERROR_NONE,
				  "Mesh message is empty");
	}

	if (!l_dbus_message_get_arguments(message, "qqqyyy", &src,
					  &dst, &feature, &rssi, &hops, &ttl))
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
    }

    l_debug("\n %s %x %x %x %d %x %x",__func__,src,dst,feature,rssi,hops,ttl);
	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

    if(event_cb)
    {
        // event callback to customer api
        mesh_event.evt_code = AW_MESH_EVENT_HEARTBEAT;
        mesh_event.param.heartbeat.src = src;
        mesh_event.param.heartbeat.dst = dst;
        mesh_event.param.heartbeat.feature = feature;
        mesh_event.param.heartbeat.hops = hops;
        mesh_event.param.heartbeat.rssi = rssi;
        mesh_event.param.heartbeat.ttl = ttl;
        event_cb(&mesh_event,NULL);
    }

	return reply;
}

//Public API
int32_t aw_mesh_model_goo_send(uint16_t dst,uint16_t appkey_idx,uint16_t on_off)
{
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[dst:%x onoff:%x ]",STR_GOO_CL_TAG,dst,on_off);
#endif
    goo_onoff(dst,appkey_idx,on_off);
    return AW_ERROR_NONE;
}

int32_t aw_mesh_set_start_beacon(uint16_t dst, uint16_t net_id, uint16_t phase)
{
	struct l_dbus *dbus = NULL;
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();
    struct node_element *element = mesh_element_find_by_idx(AW_MESH_FOUNDATION_MMDL_INDEX);

    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);
    ELEMENT_READY_ACCESS(element);

	msg = l_dbus_message_new_method_call(dbus, BLUEZ_MESH_NAME,
					app->node_path, MESH_NODE_INTERFACE,
					"SetStartBeacon");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'o', element->path);
	l_dbus_message_builder_append_basic(builder, 'q', &dst);
	l_dbus_message_builder_append_basic(builder, 'q', &net_id);
	l_dbus_message_builder_append_basic(builder, 'q', &phase);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send_with_reply(dbus, msg, send_call_reply, NULL, NULL);

    return AW_ERROR_NONE;
}

int32_t aw_mesh_send_beacon(uint16_t num, uint16_t interval)
{
	struct mesh_application *app = mesh_application_get_instance();

    MESH_READY_ACCESS(app);

	return mesh_node_send_beacon(app,num,interval);
}

int32_t aw_mesh_send_mesh_msg(access_tx_msg_p pmsg)
{
    if((pmsg == NULL)||(pmsg->data == NULL) \
        ||(pmsg->dlen == 0))
    {
        return AW_ERROR_INVALID_ARGS;
    }

    return aw_mesh_send_packet(pmsg->meta_data.dst_addr,0,pmsg->meta_data.ele_idx, \
        pmsg->meta_data.ttl,pmsg->meta_data.netkey_index,pmsg->meta_data.appkey_index,pmsg->data,pmsg->dlen);
}

int32_t aw_mesh_publish_mesh_msg(access_tx_publish_msg_p pmsg)
{
    if((pmsg == NULL)||(pmsg->data == NULL) \
        ||(pmsg->dlen == 0))
    {
        return AW_ERROR_INVALID_ARGS;
    }
    return aw_mesh_model_publish(pmsg->mod_id,0,pmsg->meta_data.ele_idx,    \
        pmsg->meta_data.ttl,pmsg->meta_data.netkey_index,pmsg->meta_data.appkey_index,pmsg->data,pmsg->dlen);
}

int32_t aw_mesh_publish_vendor_mesh_msg(access_tx_publish_msg_p pmsg)
{
    if((pmsg == NULL)||(pmsg->data == NULL) \
        ||(pmsg->dlen == 0))
    {
        return AW_ERROR_INVALID_ARGS;
    }
    return aw_mesh_vendor_model_publish(pmsg->mod_id,0,pmsg->meta_data.ele_idx,    \
        pmsg->meta_data.ttl,pmsg->meta_data.netkey_index,pmsg->meta_data.appkey_index,pmsg->data,pmsg->dlen);
}

int32_t aw_mesh_set_protocol_param(uint16_t crpl, uint16_t prov_data_interval, uint32_t feature)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();
	struct node_element *element = mesh_element_find_by_idx(AW_MESH_FOUNDATION_MMDL_INDEX);

	MESH_READY_ACCESS(app);
	NODE_READY_ACCESS(app->node_path);
	ELEMENT_READY_ACCESS(element);

	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
					app->node_path, MESH_NODE_INTERFACE,
					"SetMeshProtocolParam");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'q', &crpl);
	l_dbus_message_builder_append_basic(builder, 'q', &prov_data_interval);
    l_dbus_message_builder_append_basic(builder, 'i', &feature);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send_with_reply(app->dbus, msg, set_protocol_reply, NULL, NULL);
	return AW_ERROR_NONE;
}

int32_t aw_mesh_set_prov_data(uint16_t net_idx,uint16_t unicast)
{
    struct mesh_application *app = mesh_application_get_instance();
    MESH_READY_ACCESS(app);
    app->prov_unicast = unicast;
    app->prov_net_idx = net_idx;
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s unicast %x net_idx %x",STR_APP_PROV_DATA_SET,unicast,net_idx);
#endif
    return AW_ERROR_NONE;
}

//mesh friend api
int32_t aw_mesh_frnd_request_friend(uint8_t cache, uint8_t offer_delay, uint8_t delay, uint32_t timeout)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();
	struct node_element *element = mesh_element_find_by_idx(AW_MESH_FOUNDATION_MMDL_INDEX);

    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);
    ELEMENT_READY_ACCESS(element);

	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
					app->node_path, MESH_NODE_INTERFACE,
					"Start_Frnd_Request_Friend");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'o', element->path);
	l_dbus_message_builder_append_basic(builder, 'y', &cache);
	l_dbus_message_builder_append_basic(builder, 'y', &offer_delay);
	l_dbus_message_builder_append_basic(builder, 'y', &delay);
	l_dbus_message_builder_append_basic(builder, 'u', &timeout);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send_with_reply(app->dbus, msg, send_call_reply, NULL, NULL);

    return AW_ERROR_NONE;
}

int32_t aw_mesh_iv_test_mode(uint8_t test_mode)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();;

    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);

	msg = l_dbus_message_new_method_call(
	    app->dbus, BLUEZ_MESH_NAME, app->node_path, MESH_NODE_INTERFACE,
	    "IvTestMode");
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[test_mode %x]\n",STR_IV_TEST_MODE_SET_TAG,test_mode);
#endif

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'y', &test_mode);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send_with_reply(app->dbus, msg, iv_test_mode_reply, NULL, NULL);

	return AW_ERROR_NONE;
}

int32_t aw_mesh_update_iv_info(uint32_t iv_index, uint8_t flags)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();;

    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);

	msg = l_dbus_message_new_method_call(
	    app->dbus, BLUEZ_MESH_NAME, app->node_path, MESH_NODE_INTERFACE,
	    "UpdateIvInfo");
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[iv_index %x flags %x]\n",STR_IV_UPDATE_SET_TAG,iv_index,flags);
#endif

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'u', &iv_index);
	l_dbus_message_builder_append_basic(builder, 'y', &flags);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send_with_reply(app->dbus, msg, update_iv_info_reply, NULL, NULL);

	return AW_ERROR_NONE;
}

int32_t aw_mesh_get_iv_info()
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();;

    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);

	msg = l_dbus_message_new_method_call(
	    app->dbus, BLUEZ_MESH_NAME, app->node_path, MESH_NODE_INTERFACE,
	    "GetIvInfo");

#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s\n",STR_IV_UPDATE_GET_TAG);
#endif

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_dbus_send_with_reply(app->dbus, msg, update_iv_info_reply, NULL, NULL);

	return AW_ERROR_NONE;
}

//local node mgr
int32_t aw_mesh_primary_addr_get(uint16_t *primary)
{
    struct mesh_application *app = mesh_application_get_instance();
    *primary = 0;
    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);

    *primary = app->primary_addr;
    return AW_ERROR_NONE;
}

int32_t aw_mesh_add_element(uint16_t location,uint16_t* element_idx)
{
    char *ele_path = NULL;
    struct node_element *element = NULL;
    struct mesh_application *app = mesh_application_get_instance();

    MESH_READY_ACCESS(app);

    ele_path = build_dbus_path(MESH_ELEMENT_PATH_PREFIX, app->element_cnt);
    element = mesh_element_init(ele_path, location, app->element_cnt);
    *element_idx = app->element_cnt++;
    mesh_application_element_binding(app, element);

    if (!l_dbus_object_add_interface(app->dbus, element->path,
                MESH_ELEMENT_INTERFACE, element)) {
        return AW_MESH_ERROR_FAILED_ADD_ELEMENT;
    }

    return AW_ERROR_NONE;
}

int32_t aw_mesh_add_model(uint8_t ele_idx,uint32_t model_id, void *model_data)
{
    struct mesh_model *reg_model = NULL;
    struct node_element *element = mesh_element_find_by_idx(ele_idx);
    struct mesh_application *app = mesh_application_get_instance();

    MESH_READY_ACCESS(app);
    ELEMENT_READY_ACCESS(element);

	if (model_id < 0xffff) {
		model_id = model_id | VENDOR_ID_MASK;
	}

    reg_model = app_mesh_model_init(model_id);
    element_model_binding(element, reg_model);
    reg_model->data = model_data;

    return AW_ERROR_NONE;
}

int32_t aw_mesh_local_app_key_add(uint16_t net_idx,uint16_t app_idx,uint8_t *app_key)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();

    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);

	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
				app->node_path, MESH_MANAGEMENT_INTERFACE,
				"InsertAppKey");
	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'q', &net_idx);
    l_dbus_message_builder_append_basic(builder, 'q', &app_idx);
    dbus_append_byte_array(builder, app_key, AW_MESH_APP_KEY_SIZE);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
    l_dbus_send_with_reply(app->dbus, msg, send_call_reply, NULL, NULL);

	return AW_ERROR_NONE;

}

int32_t aw_mesh_key_phase_set(uint16_t net_idx, uint8_t phase)
{
    struct l_dbus *dbus = app_dbus_get_bus();
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *mesh_app = NULL;

    mesh_app = mesh_application_get_instance();
	msg = l_dbus_message_new_method_call(dbus, BLUEZ_MESH_NAME,
				mesh_app->node_path, MESH_MANAGEMENT_INTERFACE,
				"SetKeyPhase");
    builder = l_dbus_message_builder_new(msg);
    l_dbus_message_builder_append_basic(builder, 'q', &net_idx);
    l_dbus_message_builder_append_basic(builder, 'y', &phase);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

    l_dbus_send_with_reply(dbus, msg, send_beacon_reply, NULL, NULL);

    return AW_ERROR_NONE;
}
