#include <ell/ell.h>
#include "dbus.h"
#include "bluez/include/error.h"
#include "mesh_internal_api.h"

//API FOR DEBUG LOG
#define LOCAL_MODEL AW_APP_MODULE
#define LOG_PRINTF(LEVEL,FMT,...)   mesh_log(LEVEL,AW_APP_MODULE,FMT,##__VA_ARGS__)

static struct mesh_application *g_mesh_app;

/*
 * callback for Attach()
 */
static void attach_call_reply(struct l_dbus_message *reply, void *user_data)
{
	struct l_dbus_message_iter iter_elements;
	struct mesh_application *mesh_app = mesh_application_get_instance();
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;
	const char *node_path = NULL;
	int err = 0;
    uint16_t addr;
	err = dbus_get_reply_error(reply);

	if (err != MESH_ERROR_NONE) {
        mesh_stack_reply(AW_MESH_ENABLE_REQ,__func__,err);
		return;
	}

	if (!l_dbus_message_get_arguments(reply, "oqa(ya(qa{sv}))", &node_path, &addr ,&iter_elements))
    {
        mesh_stack_reply(AW_MESH_ENABLE_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
		return ;
    }

	mesh_log(AW_DBG_INFO_LEVEL,AW_APP_MODULE,"Mesh application register: %s , primary addr %x", node_path,addr);
	mesh_app->node_path = l_strdup(node_path);
    mesh_app->primary_addr = addr;
	mesh_element_dbus_iter(&iter_elements);

	/* get provisioning database if to be a provisioner */
	if (mesh_app->provisioner == true)
		dbus_get_prov_db_call();

    if(event_cb)
    {
        // event callback to customer api
        mesh_event.evt_code = AW_MESH_EVENT_LOCAL_NODE_ATTACH_NODE;
        event_cb(&mesh_event,NULL);
    }

}

static bool application_property_get_company_id(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	struct mesh_application *mesh_app = user_data;
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;

	if (!mesh_app)
		return false;

    if(event_cb)
    {
        // event callback to customer api
        mesh_event.evt_code = AW_MESH_CONFIG_INFO_REQ;
        mesh_event.param.config_info.company_id = mesh_app->company_id;
        mesh_event.param.config_info.product_id = mesh_app->product_id;
        mesh_event.param.config_info.version_id = mesh_app->version_id;
        mesh_event.param.config_info.feature    = mesh_app->feature;
        event_cb(&mesh_event,NULL);
        mesh_app->company_id = mesh_event.param.config_info.company_id;
        mesh_app->product_id = mesh_event.param.config_info.product_id;
        mesh_app->version_id = mesh_event.param.config_info.version_id;
        mesh_app->feature = mesh_event.param.config_info.feature;
    }

	l_dbus_message_builder_append_basic(builder, 'q', &(mesh_app->company_id));

	return true;
}

static bool application_property_get_product_id(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	struct mesh_application *mesh_app = user_data;

	if (!mesh_app)
		return false;

	l_dbus_message_builder_append_basic(builder, 'q', &(mesh_app->product_id));

	return true;
}

static bool application_property_get_version_id(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	struct mesh_application *mesh_app = user_data;

	if (!mesh_app)
		return false;

	l_dbus_message_builder_append_basic(builder, 'q', &(mesh_app->version_id));

	return true;
}
static bool application_property_feature(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	struct mesh_application *mesh_app = user_data;

	if (!mesh_app)
		return false;

	l_dbus_message_builder_append_basic(builder, 'u', &(mesh_app->feature));

    l_debug("application set stack feature is %x ",mesh_app->feature);

	return true;
}

static void setup_application_interface(struct l_dbus_interface *iface)
{
	l_dbus_interface_method(iface, "JoinComplete", 0, join_complete_call, "",
				"t", "token");
	l_dbus_interface_method(iface, "JoinFailed", 0, join_failed_call, "",
				"s", "prov_status");

	l_dbus_interface_property(iface, "CompanyID", 0, "q",
				application_property_get_company_id, NULL);
	l_dbus_interface_property(iface, "ProductID", 0, "q",
				application_property_get_product_id, NULL);
	l_dbus_interface_property(iface, "VersionID", 0, "q",
				application_property_get_version_id, NULL);
	l_dbus_interface_property(iface, "Feature", 0, "u",
				application_property_feature, NULL);

	l_dbus_interface_method(iface, "GenericMessageCb", 0,
				dbus_generic_message_cb, "", "qqqqqqyyyay",
				"comp_id", "opcode", "src", "dst", "appkey_idx",
				"netkey_idx", "rssi", "ttl", "data");

	l_dbus_interface_method(iface, "ConfigMessageCb", 0,
				dbus_config_message_cb, "", "qqqqqqyyay",
				"comp_id", "opcode", "src", "dst", "appkey_idx",
				"netkey_idx", "rssi", "ttl", "data");

	l_dbus_interface_method(iface, "AdvPacket", 0, dbus_adv_packet_cb,
			"", "ayyynyay", "mac", "addr_type", "adv_type",
			"rssi", "data_len", "data");

	l_dbus_interface_method(iface, "IvUpdateState", 0, dbus_iv_update_cb, "",
			"uy", "iv_index", "flags");

	l_dbus_interface_method(iface, "FriendshipState", 0, dbus_friendship_state_cb, "",
			"qyy", "lpn_addr", "status", "reason");

    l_dbus_interface_method(iface, "HearbeatPkt", 0,
                dbus_heartbeat_message_cb, "", "qqqyyy",
                "src", "dst", "feature", "rssi", "hops","ttl");

}

//Internal API
bool mesh_application_attach(uint64_t *token)
{
	struct l_dbus *dbus = NULL;
	struct l_dbus_message *msg = NULL;
	struct l_dbus_message_builder *builder = NULL;
	struct mesh_application *mesh_app = NULL;

	dbus = app_dbus_get_bus();

	mesh_app = mesh_application_get_instance();

	msg = l_dbus_message_new_method_call(dbus, BLUEZ_MESH_NAME,
				BLUEZ_MESH_PATH, MESH_NETWORK_INTERFACE,
				"Attach");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'o', mesh_app->path);
	l_dbus_message_builder_append_basic(builder, 't', token);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_dbus_send_with_reply(dbus, msg, attach_call_reply, NULL, NULL);

	return true;
}

int32_t mesh_application_init_dbus(struct mesh_application *app)
{
	if (!l_dbus_register_interface(app->dbus, MESH_APPLICATION_INTERFACE,
						setup_application_interface,
						NULL, false)) {
		return AW_MESH_ERROR_APPLICATION_IFACE_REG_FAIL;
	}

	app = mesh_application_get_instance();

	if (!l_dbus_object_add_interface(app->dbus, app->path,
				L_DBUS_INTERFACE_OBJECT_MANAGER, app->dbus)) {
		return AW_MESH_ERROR_APPLICATION_IFACE_ADD_FAIL;
	}

	if (!l_dbus_object_add_interface(app->dbus, app->path,
				MESH_APPLICATION_INTERFACE, app)) {
		return AW_MESH_ERROR_APPLICATION_IFACE_ADD_FAIL;
	}

	return AW_MESH_ERROR_NONE;
}

void mesh_application_free(void)
{
	struct mesh_application *mesh_app = g_mesh_app;

	if (!mesh_app)
		return;

	l_free(mesh_app->path);
	l_free(mesh_app->node_path);
	l_free(mesh_app);

	g_mesh_app = NULL;
}

bool mesh_application_element_binding(struct mesh_application *mesh_app,
				struct node_element *element)
{
	if (!mesh_app->elements)
		mesh_app->elements = l_queue_new();

	l_queue_push_tail(mesh_app->elements, element);

	return true;
}

AwMeshEventCb_t mesh_application_get_event_cb(void)
{
	if((g_mesh_app)&&(g_mesh_app->mesh_event_cb_handle))
    {
        return g_mesh_app->mesh_event_cb_handle;
    }
    return NULL;
}

struct mesh_application *mesh_application_get_instance(void)
{
	return g_mesh_app;
}

int32_t mesh_application_init(struct l_dbus *dbus, AwMeshEventCb_t cb)
{
	struct mesh_application *mesh_app = NULL;
    LOG_PRINTF(AW_DBG_VERB_LEVE,"%s,%p\n",__func__,cb);
	if (g_mesh_app) {
		return AW_MESH_ERROR_ALREADY_INIT;
	}

	mesh_app = l_new(struct mesh_application, 1);

	mesh_app->path        = l_strdup(MESH_APPLICATION_PATH_PREFIX);
	mesh_app->company_id  = AW_MESH_APP_COMPANY_ID;
	mesh_app->product_id  = AW_MESH_APP_PRODUCT_ID;
	mesh_app->version_id  = AW_MESH_APP_VERSION_ID;
    mesh_app->feature     = CONFIG_NODE_FEATURE;
    mesh_app->dbus        = dbus;
    mesh_app->element_cnt = 0;
    mesh_app->mesh_event_cb_handle = cb;
	g_mesh_app = mesh_app;

	return AW_ERROR_NONE;
}
