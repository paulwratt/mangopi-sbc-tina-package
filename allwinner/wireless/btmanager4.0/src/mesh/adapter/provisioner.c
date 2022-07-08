
#include "dbus.h"
#include "mesh_db.h"
#include "bluez/include/error.h"
#include "AWTypes.h"
#include "mesh_internal_api.h"

#define LOCAL_MODEL AW_APP_PROV_MODULE
#define LOG_PRINTF(LEVEL,FMT,...)   mesh_log(LEVEL,LOCAL_MODEL,FMT,##__VA_ARGS__)

static void import_local_node_cmplt_cb(struct l_dbus_message *reply, void *user_data)
{
	uint64_t token;
	int err;

	err = dbus_get_reply_error(reply);

	if (err != MESH_ERROR_NONE) {
		goto fail;
	}

	if (!l_dbus_message_get_arguments(reply, "t", &token))
		goto fail;

	if (!mesh_application_attach(&token))
		goto fail;

	return;

fail:
    mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
	return;
}

static void prov_req_reply(struct l_dbus_message *reply, void *user_data)
{
    int err;

	err = dbus_get_reply_error(reply);
	mesh_stack_reply(AW_MESH_PROV_INVITE_REQ, __func__, err);
}

static void req_done_reply(struct l_dbus_message *reply, void *user_data)
{
    int err;

	err = dbus_get_reply_error(reply);
	mesh_stack_reply(AW_MESH_PROV_CANCEL_REQ, __func__, err);
}

static struct l_dbus_message *dbus_add_node_cmplt_cb(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
    struct l_dbus_message *reply;
    struct l_dbus_message_iter iter_uuid, iter_dev_key;
    uint32_t len_uuid, len_devkey;
    uint16_t unicast;
    uint8_t *uuid,num_ele, *device_key;

    struct mesh_application * app = mesh_application_get_instance();
    AwMeshEventCb_t event_cb = app->mesh_event_cb_handle;
    AW_MESH_EVENT_T mesh_event;

    l_info("%s\n",__func__);

    if (l_dbus_message_is_error(message)) {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
        return dbus_error(message, MESH_ERROR_NONE,
                                "Mesh message is empty");
    }

    if (!l_dbus_message_get_arguments(message, "ayayqy", &iter_uuid, &iter_dev_key, &unicast, &num_ele))
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
        return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
    }

	if (!l_dbus_message_iter_get_fixed_array(&iter_uuid, &uuid, &len_uuid)
								|| len_uuid != AW_MESH_UUID_SIZE)
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS,
							"Bad device UUID");
    }

	if (!l_dbus_message_iter_get_fixed_array(&iter_dev_key, &device_key, &len_devkey)
								|| len_devkey != AW_MESH_DEVICE_KEY_SIZE)
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS,
							"Bad device KEY");
    }

    reply = l_dbus_message_new_method_return(message);
    l_dbus_message_set_arguments(reply, "");

    if(app->prov_timeout != NULL)
    {
        l_timeout_remove(app->prov_timeout);
        app->prov_timeout = NULL;
    }

    if(event_cb)
    {
        mesh_event.evt_code = AW_MESH_EVENT_PROV_DONE;
        memcpy(&mesh_event.param.prov_done.uuid,uuid,len_uuid);
		memcpy(&mesh_event.param.prov_done.device_key, device_key, len_devkey);
        mesh_event.param.prov_done.address      = unicast;
        mesh_event.param.prov_done.element_num  = num_ele;
        mesh_event.param.prov_done.success      = true;
        mesh_event.param.prov_done.gatt_bearer  = false;
        mesh_event.param.prov_done.reason       = AW_MESH_PROV_ERR_SUCCESS;
        event_cb(&mesh_event,NULL);
    }

    return reply;
}

static struct l_dbus_message *dbus_add_node_fail_cb(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
    struct l_dbus_message *reply;
    struct l_dbus_message_iter iter_uuid;

    uint32_t len;
    char *prov_status;
    uint8_t *uuid;
	uint8_t status_value;

    struct mesh_application * app = mesh_application_get_instance();
    AwMeshEventCb_t event_cb = app->mesh_event_cb_handle;
    AW_MESH_EVENT_T mesh_event;

    if (l_dbus_message_is_error(message)) {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
        return dbus_error(message, MESH_ERROR_NONE,
                                "Mesh message is empty");
    }

    if (!l_dbus_message_get_arguments(message, "aysy", &iter_uuid, &prov_status, &status_value))
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
        return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
    }

	if (!l_dbus_message_iter_get_fixed_array(&iter_uuid, &uuid, &len)
								|| len != AW_MESH_UUID_SIZE)
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS,
							"Bad device UUID");
    }

    reply = l_dbus_message_new_method_return(message);

    l_dbus_message_set_arguments(reply, "");

    if(event_cb)
    {
        mesh_event.evt_code = AW_MESH_EVENT_PROV_FAILED;
        memcpy(&mesh_event.param.prov_done.uuid,uuid,len);
        mesh_event.param.prov_done.success      = false;
        mesh_event.param.prov_done.gatt_bearer  = false;
        mesh_event.param.prov_done.reason       = status_value;
        event_cb(&mesh_event,NULL);
    }

    return reply;
}

/*
static struct l_dbus_message *dbus_init_done_cb(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;

    if(event_cb)
    {
        // event callback to customer api
        mesh_event.evt_code = AW_MESH_EVENT_STACK_INIT_DONE;
        event_cb(&mesh_event,NULL);
    }

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	return reply;
}
*/

static struct l_dbus_message * dbus_req_provision_data_cb(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;
	uint8_t num_ele;
	uint16_t net_idx = 0;
	uint16_t prov_unicast;
    uint8_t prov_path = 0;
	struct mesh_application *app = mesh_application_get_instance();
    AwMeshEventCb_t event_cb = app->mesh_event_cb_handle;
    AW_MESH_EVENT_T mesh_event;

	if (!l_dbus_message_get_arguments(message, "qy", &prov_unicast, &num_ele)){
		l_info(" dbus_error");
		return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
	}

    if(app->prov_unicast)
    {
        prov_path = 1;
        prov_unicast = app->prov_unicast;
        net_idx = app->prov_net_idx;
        app->prov_unicast = 0;
    }
    else if(event_cb)
    {
        mesh_event.evt_code = AW_MESH_EVENT_PROV_REQUEST_DATA;
        mesh_event.param.prov_request_data.prov_set = false;
        mesh_event.param.prov_request_data.net_idx = net_idx;
        mesh_event.param.prov_request_data.unicast = prov_unicast;
        mesh_event.param.prov_request_data.num_ele = num_ele;
        event_cb(&mesh_event,NULL);
        if(mesh_event.param.prov_request_data.prov_set == true)
        {
            prov_path = 2;
            net_idx = mesh_event.param.prov_request_data.net_idx;
            prov_unicast = mesh_event.param.prov_request_data.unicast;
        }
    }

	l_info("receive RequestProvData dbus msg,prov_path %d ,num_ele = %d prov_unicast %d\n", prov_path, num_ele, prov_unicast);
	/*notify to app and wait app to set prov unicast and net_idx*/
	//wait_app_interaction();
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s unicast %x net_idx %x",STR_APP_PROV_DATA_GET,prov_unicast,net_idx);
#endif
	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "qq", net_idx, prov_unicast);
	l_info("response net_idx = %d,prov_unicast = %d\n", net_idx, prov_unicast);

	return reply;
}

static int32_t mesh_add_node_cancel(struct mesh_application *app)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;

	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
				app->node_path, MESH_MANAGEMENT_INTERFACE,
				"AddNodeCancel");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
    l_dbus_send_with_reply(app->dbus, msg, req_done_reply, NULL, NULL);

    return AW_ERROR_NONE;
}

static void mesh_add_node_timeout(struct l_timeout *timeout, void *user_data)
{
	struct mesh_application *app = user_data;
    AwMeshEventCb_t event_cb = app->mesh_event_cb_handle;
    AW_MESH_EVENT_T mesh_event;

    if(app->prov_timeout != NULL)
    {
        l_timeout_remove(app->prov_timeout);
        app->prov_timeout = NULL;
    }

	mesh_add_node_cancel(app);

#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s",STR_APP_PROV_TO);
#endif

    if(event_cb)
    {
        mesh_event.evt_code = AW_MESH_EVENT_PROV_DONE;
        mesh_event.param.prov_done.reason = AW_MESH_PROV_ERR_TIMEOUT;
        event_cb(&mesh_event,NULL);
    }

}

static int mesh_add_node(struct mesh_application *app, uint8_t *uuid, uint32_t size, uint32_t attentionDuration)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;

    if(app->prov_timeout != NULL)
    {
        return AW_ERROR_MESH_PROV_ONGOING;
    }

	l_info("%s:%d", __func__,attentionDuration);
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s[duration %d seconds uuid:%s]",STR_APP_PROV_START,attentionDuration, getstr_hex2str(uuid,16));
#endif

	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
				app->node_path, MESH_MANAGEMENT_INTERFACE,
				"AddNode");

	builder = l_dbus_message_builder_new(msg);
    dbus_append_byte_array(builder, uuid, AW_MESH_UUID_SIZE);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send_with_reply(app->dbus, msg, prov_req_reply, NULL, NULL);
	app->prov_timeout = l_timeout_create(attentionDuration, mesh_add_node_timeout, app, NULL);

	return AW_ERROR_NONE;
}

static void setup_mesh_provisioner_interface(struct l_dbus_interface *interface)
{
	l_dbus_interface_method(interface, "AddNodeComplete", 0,
				dbus_add_node_cmplt_cb, "", "ayayqy", "uuid", "device key", "unicast", "num");

	l_dbus_interface_method(interface, "AddNodeFailed", 0,
				dbus_add_node_fail_cb, "", "aysy", "uuid", "status", "status_value");

	l_dbus_interface_method(interface, "RequestProvData", 0,
				dbus_req_provision_data_cb, "", "qy");

	l_dbus_interface_method(interface, "ScanResult", 0,
				dbus_unprov_device_cb, "", "nayay", "rssi", "adv_info", "mac");
}

static int32_t dbus_mesh_provisioner_register(struct l_dbus *dbus)
{

	struct mesh_application *mesh_app = mesh_application_get_instance();

	if (!l_dbus_register_interface(dbus, MESH_PROVISIONER_INTERFACE,
											setup_mesh_provisioner_interface,
					NULL, true)) {
		goto fail;
	}

	if (!l_dbus_object_add_interface(dbus, mesh_app->path,
									MESH_PROVISIONER_INTERFACE, NULL)) {
		goto fail;
	}

	if (!l_dbus_object_add_interface(dbus, mesh_app->path,
					L_DBUS_INTERFACE_PROPERTIES, NULL)) {
		goto fail;
	}
	return AW_MESH_ERROR_NONE;

fail:
	return AW_MESH_ERROR_FAILED_INIT_PROV_REG;
}

//Internal Api
uint32_t mesh_provisioner_create_network(struct mesh_application *app, uint8_t *uuid)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;

    app->provisioner = AW_MESH_ROLE_PROVISIONER;

	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
				BLUEZ_MESH_PATH, MESH_NETWORK_INTERFACE,
				"CreateNetwork");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'o', app->path);
	dbus_append_byte_array(builder, uuid, AW_MESH_UUID_SIZE);

	l_info("mesh_provisioner_create_network");
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
    l_dbus_send_with_reply(app->dbus, msg, import_local_node_cmplt_cb, NULL, NULL);
	return AW_MESH_ERROR_NONE;
}

struct l_dbus_message *dbus_unprov_device_cb(struct l_dbus *dbus, struct l_dbus_message *message, void *user_data)
{
	struct l_dbus_message *reply;
	struct l_dbus_message_iter iter_data;
	struct l_dbus_message_iter iter_data2;
	uint8_t *adv_info;
	uint8_t *mac;
	uint32_t len;
	int16_t rssi;

    AW_MESH_EVENT_T mesh_event;
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();

	if (l_dbus_message_is_error(message)) {
	   mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
	   return dbus_error(message, MESH_ERROR_NONE,
			"Mesh message is empty");
	}

	if (!l_dbus_message_get_arguments(message, "nayay", &rssi,
			   &iter_data,
			   &iter_data2))
    {
       mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
	   return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
	}

	mesh_event.param.prov_scan_ud.rssi = rssi;
	l_dbus_message_iter_get_fixed_array(&iter_data, &adv_info, &len);
	if (len)
    {
        memcpy(&mesh_event.param.prov_scan_ud.uuid, adv_info, AW_MESH_UUID_SIZE);

		memset(mesh_event.param.prov_scan_ud.uri_hash, 0, AW_MESH_URI_HASH_LEN);
		mesh_event.param.prov_scan_ud.oob_info = l_get_be16(adv_info + AW_MESH_UUID_SIZE);

		if (len > (AW_MESH_UUID_SIZE + 2))
			 memcpy(&mesh_event.param.prov_scan_ud.uri_hash, adv_info + AW_MESH_UUID_SIZE + 2, len - AW_MESH_UUID_SIZE - 2);
    }
    else
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
	   return dbus_error(message, MESH_ERROR_INVALID_ARGS,
		   "Mesh message device key is empty");
    }

	l_dbus_message_iter_get_fixed_array(&iter_data2, &mac, &len);
    if(len)
    {
        memcpy(&mesh_event.param.prov_scan_ud.mac, mac, len);
    }
    else
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
	    return dbus_error(message, MESH_ERROR_INVALID_ARGS,
		   "Mesh message device key is empty");
    }

    if(event_cb)
    {
        // event callback to customer api
        /*
        LOG_PRINTF(AW_DBG_VERB_LEVE,"rssi %d", mesh_event.param.prov_scan_ud.rssi);
        LOG_PRINTF(AW_DBG_VERB_LEVE,"uuid: %s", getstr_hex2str(mesh_event.param.prov_scan_ud.uuid, 16));
		LOG_PRINTF(AW_DBG_VERB_LEVE,"uri_hash: %s", getstr_hex2str(mesh_event.param.prov_scan_ud.uri_hash, 4));
		LOG_PRINTF(AW_DBG_VERB_LEVE,"mac: %s", getstr_hex2str(mesh_event.param.prov_scan_ud.mac, 6));
		*/
        mesh_event.evt_code = AW_MESH_EVENT_PROV_SCAN_UD_RESULT;
        event_cb(&mesh_event,NULL);
    }

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	return reply;
}

static void scan_req_reply(struct l_dbus_message *reply, void *user_data)
{
	int err;

	err = dbus_get_reply_error(reply);

	mesh_stack_reply(AW_MESH_UNPROV_SCAN_START, __func__, err);
}

static void scan_cancel_reply(struct l_dbus_message *reply, void *user_data)
{
	int err;

	err = dbus_get_reply_error(reply);

	mesh_stack_reply(AW_MESH_UNPROV_SCAN_STOP, __func__, err);
}

static void mesh_adapter_prov_scan_timeout(struct l_timeout *timeout, void *user_data)
{
	struct mesh_application *app = user_data;
#ifdef MEST_TEST_LOG_ENABLE
    mesh_test_log("%s",STR_APP_UD_SCAN_TIMEOUT);
#endif
	l_info(" Mesh Scan timeout");
	if (app->scan_timeout != NULL) {
		l_timeout_remove(app->scan_timeout);
		app->scan_timeout = NULL;
	}
}

int mesh_prov_scan_start(struct mesh_application *app, bool start, uint32_t duration)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;

	MESH_READY_ACCESS(app);
	l_info("mesh_adapter_prov_scan:%s duration:%d", start ? "start" : "stop", duration);

	if (start && app->scan_timeout != NULL) {
		l_info("mesh adapter is scaning");
		return AW_MESH_ERROR_FAILED;
	}
	if (duration == 0)
	{
		duration = MESH_SCAN_TIMEOUT;
	}

    if(start == true)
    {
	l_info("start scan\n");
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[duration %d seconds]",STR_APP_UD_SCAN_START,duration);
#endif
		msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
				app->node_path, MESH_MANAGEMENT_INTERFACE,
				"UnprovisionedScan");
		builder = l_dbus_message_builder_new(msg);
		l_dbus_message_builder_append_basic(builder, 'q', &duration);
		l_dbus_message_builder_finalize(builder);
		l_dbus_message_builder_destroy(builder);
		l_dbus_send_with_reply(app->dbus, msg, scan_req_reply, NULL, NULL);
    } else {
		l_info("cancel scan");
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s",STR_APP_UD_SCAN_STOP);
#endif
		msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
				app->node_path, MESH_MANAGEMENT_INTERFACE,
				"UnprovisionedScanCancel");

		builder = l_dbus_message_builder_new(msg);
		l_dbus_message_builder_finalize(builder);
		l_dbus_message_builder_destroy(builder);
		l_dbus_send_with_reply(app->dbus, msg, scan_cancel_reply, NULL, NULL);
    }

	if (start) {
		app->scan_timeout = l_timeout_create(duration, mesh_adapter_prov_scan_timeout, app, NULL);
	} else if (app->scan_timeout != NULL) {
		mesh_adapter_prov_scan_timeout(app->scan_timeout, app);
	}
	return AW_MESH_ERROR_NONE;
}

//Internal API
int32_t mesh_provisioner_init_dbus(struct mesh_application *app)
{
    int32_t ret = AW_MESH_ERROR_NONE;

    ret = dbus_mesh_provisioner_register(app->dbus);
	return ret;
}

//Public API
int32_t aw_mesh_prov_invite(uint8_t *uuid, int32_t size, int32_t attentionDuration)
{
	int32_t ret = AW_ERROR_NONE;
	struct mesh_application *app = mesh_application_get_instance();

    if(uuid == NULL)
        return AW_ERROR_INVALID_ARGS;

    if(app)
    {
        ret = mesh_add_node(app, uuid, size, attentionDuration);
    }
    else
    {
        ret = AW_ERROR_MESH_NOT_INIT;
    }
	return ret;
}

int32_t aw_mesh_prov_cancel(int reason)
{
	int32_t ret = reason;
	struct mesh_application *app = mesh_application_get_instance();
    if(app)
    {
        ret = mesh_add_node_cancel(app);
    }
    else
    {
        ret = AW_ERROR_MESH_NOT_INIT;
    }
	return ret;
}

int32_t aw_mesh_scan(bool start, uint32_t duration)
{
	int32_t ret = AW_ERROR_NONE;
	struct mesh_application *app = mesh_application_get_instance();

    if(app)
    {
        ret = mesh_prov_scan_start(app, start, duration);
    }
    else
    {
        ret = AW_ERROR_MESH_NOT_INIT;
    }
	return ret;
}
