#include <ell/ell.h>
#include "dbus.h"
#include "mesh_internal_api.h"
#include "bluez/include/error.h"

static struct l_dbus *dbus;
static enum dbus_state g_dbus_state = DBUS_NONE;

struct error_entry {
	const char *dbus_err;
	const char *default_desc;
};

/*
 * Important: The entries in this table follow the order of
 * enumerated values in mesh_error (file error.h)
 */
/*
static struct error_entry const error_table[] =
{
	{ NULL, NULL },
	{ ERROR_INTERFACE ".Failed", "Operation failed" },
	{ ERROR_INTERFACE ".NotAuthorized", "Permission denied"},
	{ ERROR_INTERFACE ".NotFound", "Object not found"},
	{ ERROR_INTERFACE ".InvalidArgs", "Invalid arguments"},
	{ ERROR_INTERFACE ".InProgress", "Already in progress"},
	{ ERROR_INTERFACE ".AlreadyExists", "Already exists"},
	{ ERROR_INTERFACE ".DoesNotExist", "Does not exist"},
	{ ERROR_INTERFACE ".Canceled", "Operation canceled"}
};
*/
/*
 * function handler for l_dbus_set_ready_handler
 */
static void ready_callback(void *user_data)
{
	l_info("DBus ready");
	g_dbus_state = DBUS_INITIALIZED;
}

/*
 * function handler for l_dbus_set_disconnect_handler
 */
static void disconnect_callback(void *user_data)
{
	l_info("DBus disconnect");
	g_dbus_state = DBUS_NONE;
}

/*
 * function handler for l_dbus_register
 */
static void signal_message(struct l_dbus_message *message, void *user_data)
{
	l_info("DBus signal handler");
}

struct l_dbus *app_dbus_get_bus(void)
{
	return dbus;
}

enum dbus_state app_dbus_get_state(void)
{
	return g_dbus_state;
}

bool app_dbus_init(void)
{
	struct l_dbus *bus = NULL;

	if (dbus) {
		l_error("dbus already exists");
		return false;
	}

	bus = l_dbus_new_default(L_DBUS_SYSTEM_BUS);
	if (!bus) {
		l_error("unable to connect to D-Bus");
		return false;
	}
    l_dbus_new_mutex(bus);
	l_dbus_set_ready_handler(bus, ready_callback, bus, NULL);
	l_dbus_set_disconnect_handler(bus, disconnect_callback, NULL, NULL);
	l_dbus_register(bus, signal_message, NULL, NULL);

	dbus = bus;

	return true;
}
int dbus_get_reply_error(struct l_dbus_message *reply)
{
	const char *name, *desc;

	if (l_dbus_message_is_error(reply)) {

		l_dbus_message_get_error(reply, &name, &desc);
		l_error("DBus reply error (%s), %s", name, desc);
		return MESH_ERROR_FAILED;
	}

	return MESH_ERROR_NONE;
}
