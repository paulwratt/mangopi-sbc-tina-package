#include "dbus.h"
#include "AWTypes.h"
#include "bluez/include/error.h"
#include "mesh_internal_api.h"

/*
 * callback for Join()
 */
static void join_call_reply(struct l_dbus_message *reply, void *user_data)
{
	int err;

	err = dbus_get_reply_error(reply);

	if (err == MESH_ERROR_NONE)
		l_info("Join procedure started");
	else
		l_error("Join procedure failed");
}

//Internal Api
struct l_dbus_message *join_complete_call(struct l_dbus *dbus,struct l_dbus_message *msg,
						void *user_data)
{
	uint64_t token = 0;

	l_info("join complete call");

	if (!l_dbus_message_get_arguments(msg, "t", &token))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!mesh_application_attach(&token))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	return NULL;
}

struct l_dbus_message *join_failed_call(struct l_dbus *dbus, struct l_dbus_message *msg,
						void *user_data)
{
	l_info("join failed call");

	return NULL;
}

uint32_t mesh_provisionee_join_network(struct mesh_application *app, uint8_t *uuid)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
    app->provisioner = AW_MESH_ROLE_PROVISIONEE;

    msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
                BLUEZ_MESH_PATH, MESH_NETWORK_INTERFACE,
                "Join");

    builder = l_dbus_message_builder_new(msg);
    l_dbus_message_builder_append_basic(builder, 'o', app->path);
    dbus_append_byte_array(builder, uuid, AW_MESH_UUID_SIZE);
    l_dbus_message_builder_finalize(builder);
    l_dbus_message_builder_destroy(builder);

    l_dbus_send_with_reply(app->dbus, msg, join_call_reply, NULL, NULL);
	return AW_MESH_ERROR_NONE;
}
