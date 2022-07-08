#include "AWTypes.h"
#include "dbus.h"
#include "bluez/include/error.h"
#include "mesh_internal_api.h"

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

static void gateway_cfg_node_reply(struct l_dbus_message *reply,
					 void *user_data)
{
	int err;
	err = dbus_get_reply_error(reply);
    mesh_stack_reply(AW_MESH_UNKNOW_REQ,__FUNCTION__,err);
}

//Public API
int32_t aw_mesh_gateway_enable(AW_MESH_GATEWAY_INFO_T *info, uint8_t *uuid)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();
	bool kr, ivu;

    MESH_READY_ACCESS(app);

	app->provisioner = info->is_provisioner;
    mesh_db_prov_db_init();
	ivu = !!(info->flags & 0x02);
	kr = !!(info->flags & 0x01);

	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
				BLUEZ_MESH_PATH, MESH_NETWORK_INTERFACE,
				"Import");
	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'o', app->path);
	dbus_append_byte_array(builder, uuid, AW_MESH_UUID_SIZE);
	dbus_append_byte_array(builder, info->device_key, AW_MESH_KEY_SIZE);
	dbus_append_byte_array(builder, (uint8_t *)&info->primary_netkey, AW_MESH_KEY_SIZE);
	l_dbus_message_builder_append_basic(builder, 'q', &info->netkey_index);
	l_dbus_message_builder_enter_array(builder, "{sv}");
	dbus_append_dict_entry_basic(builder, "IVUpdate", "b", &kr);
	dbus_append_dict_entry_basic(builder, "KeyRefresh", "b", &ivu);
	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_append_basic(builder, 'u', &info->iv_index);
	l_dbus_message_builder_append_basic(builder, 'q', &info->unicast_address);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

    l_dbus_send_with_reply(app->dbus, msg, import_local_node_cmplt_cb, NULL, NULL);
	return AW_MESH_ERROR_NONE;
}

int32_t aw_mesh_gateway_cfg_node(AW_MESH_GATEWAY_OP_T ops, AW_MESH_GATEWAY_NODE_INFO_T *device)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();

    MESH_READY_ACCESS(app);
    NODE_READY_ACCESS(app->node_path);
    MESH_POINTER_ACCESS(device);

	if (ops == AW_MESH_OP_ADD)
    {
		msg = l_dbus_message_new_method_call(
		    app->dbus, BLUEZ_MESH_NAME, app->node_path, MESH_MANAGEMENT_INTERFACE,
		    "ImportRemoteNode");
	}
    else if(ops == AW_MESH_OP_DEL)
	{
		msg = l_dbus_message_new_method_call(
			app->dbus, BLUEZ_MESH_NAME, app->node_path, MESH_MANAGEMENT_INTERFACE,
			"DeleteRemoteNode");
	}
    else
    {
        return AW_ERROR_INVALID_ARGS;
    }

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'q', &device->unicast_address);
	l_dbus_message_builder_append_basic(builder, 'y', &device->elements_num);
	if (ops == AW_MESH_OP_ADD)
    {
		dbus_append_byte_array(builder, device->device_key, AW_MESH_KEY_SIZE);
	}
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send_with_reply(app->dbus, msg, gateway_cfg_node_reply, NULL,NULL);

	return AW_MESH_ERROR_NONE;
}
