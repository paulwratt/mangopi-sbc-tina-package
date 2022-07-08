#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <gio/gio.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/prctl.h>

#include "bt_log.h"
#include "bt_dev_list.h"
#include "bt_semaphore.h"
#include "bt_adapter.h"
#include "bt_device.h"
#include "bt_bluez.h"
#include "common.h"
#include "bt_agent.h"

bool is_agent_register = false;

static const char *io_capability[] = { "KeyboardDisplay", "DisplayOnly", "DisplayYesNo",
                                       "KeyboardOnly", "NoInputNoOutput" };

static const GDBusInterfaceInfo bluez_iface_agent = {
    -1, "org.bluez.Agent1", (GDBusMethodInfo **)bluez_iface_agent_methods, NULL, NULL, NULL,
};

static int agent_dbus_object_id = 0;

static void _request_pincode_handle(GDBusMethodInvocation *invocation, GVariant *parameters)
{
    gchar *path = NULL, *device = NULL;

    g_variant_get(parameters, "(o)", &path);
    bt_device_get_address(path, &device);

    if (btmg_cb_p && btmg_cb_p->btmg_agent_cb.agent_request_pincode)
        btmg_cb_p->btmg_agent_cb.agent_request_pincode(invocation, device);
    else {
        BTMG_INFO("device: %s request pin code, send DefaultPincode", device);
        bt_agent_send_pincode(invocation, "DefaultPincode");
    }

    g_free(device);
    g_free(path);
}

static void _display_pincode_handle(GDBusMethodInvocation *invocation, GVariant *parameters)
{
    gchar *path = NULL, *device = NULL, *pincode = NULL;

    g_variant_get(parameters, "(os)", &path, &pincode);
    bt_device_get_address(path, &device);

    if (btmg_cb_p && btmg_cb_p->btmg_agent_cb.agent_display_pincode)
        btmg_cb_p->btmg_agent_cb.agent_display_pincode(device, pincode);
    else {
        BTMG_INFO("display pincode (%s, %s)", device, pincode);
    }
    bt_agent_send_empty_response(invocation);

    g_free(device);
    g_free(path);
    g_free(pincode);
}

static void _request_passkey_handle(GDBusMethodInvocation *invocation, GVariant *parameters)
{
    gchar *path = NULL, *device = NULL;
    unsigned int passkey;

    g_variant_get(parameters, "(o)", &path);
    bt_device_get_address(path, &device);

    if (btmg_cb_p && btmg_cb_p->btmg_agent_cb.agent_request_passkey) {
        btmg_cb_p->btmg_agent_cb.agent_request_passkey(invocation, device);
    } else {
        passkey = (unsigned int)rand() % 1000000;
        BTMG_INFO("request passkey:%d, please enter it on the device %s", passkey, device);
        bt_agent_send_passkey(invocation, passkey);
    }
    g_free(device);
    g_free(path);
}

static void _display_passkey_handle(GDBusMethodInvocation *invocation, GVariant *parameters)
{
    gchar *path = NULL, *device = NULL;
    guint32 passkey;
    guint16 entered;

    g_variant_get(parameters, "(ouq)", &path, &passkey, &entered);
    bt_device_get_address(path, &device);

    if (btmg_cb_p && btmg_cb_p->btmg_agent_cb.agent_display_passkey) {
        btmg_cb_p->btmg_agent_cb.agent_display_passkey(device, passkey, entered);
    } else {
        BTMG_INFO("display passkey (%s, %06u, entered %u)", device, passkey, entered);
    }
    bt_agent_send_empty_response(invocation);

    g_free(device);
    g_free(path);
}

static void _request_confirmation_handle(GDBusMethodInvocation *invocation, GVariant *parameters)
{
    gchar *path = NULL, *device = NULL;
    guint32 passkey;

    g_variant_get(parameters, "(ou)", &path, &passkey);
    bt_device_get_address(path, &device);

    if (btmg_cb_p && btmg_cb_p->btmg_agent_cb.agent_request_confirm_passkey) {
        btmg_cb_p->btmg_agent_cb.agent_request_confirm_passkey(invocation, device, passkey);
    } else {
        BTMG_INFO("device:%s request confirmation passkey:%06u", device, passkey);
        bt_agent_send_empty_response(invocation);
    }

    g_free(device);
    g_free(path);
}

static void _request_authorization_handle(GDBusMethodInvocation *invocation, GVariant *parameters)
{
    gchar *path = NULL, *device = NULL;

    g_variant_get(parameters, "(o)", &path);
    bt_device_get_address(path, &device);

    if (btmg_cb_p && btmg_cb_p->btmg_agent_cb.agent_request_authorize) {
        btmg_cb_p->btmg_agent_cb.agent_request_authorize(invocation, device);
    } else {
        BTMG_INFO("Request Authorization (%s)", device);
        bt_agent_send_empty_response(invocation);
    }

    g_free(device);
    g_free(path);
}

static void _authorizeservice_handle(GDBusMethodInvocation *invocation, GVariant *parameters)
{
    gchar *path = NULL, *uuid = NULL, *device = NULL;

    g_variant_get(parameters, "(os)", &path, &uuid);
    bt_device_get_address(path, &device);

    if (btmg_cb_p && btmg_cb_p->btmg_agent_cb.agent_authorize_service) {
        btmg_cb_p->btmg_agent_cb.agent_authorize_service(invocation, device, uuid);
    } else {
        BTMG_INFO("Authorize service (%s,  %s)", device, uuid);
        bt_agent_send_empty_response(invocation);
    }

    g_free(device);
    g_free(path);
    g_free(uuid);
}
static void _cancel_handle(GDBusMethodInvocation *invocation, GVariant *parameters)
{
    g_dbus_method_invocation_return_value(invocation, NULL);
}
static void _release_handle(GDBusMethodInvocation *invocation, GVariant *parameters)
{
    g_dbus_method_invocation_return_value(invocation, NULL);
}

void _agent_method_call(GDBusConnection *connection, const gchar *sender, const gchar *object_path,
                        const gchar *interface_name, const gchar *method_name, GVariant *parameters,
                        GDBusMethodInvocation *invocation, gpointer user_data)
{
    const gchar *type = g_variant_get_type_string(parameters);

    BTMG_DEBUG("method_name:%s", method_name);
    if (strcmp(method_name, "RequestPinCode") == 0) {
        _request_pincode_handle(invocation, parameters);
    } else if (strcmp(method_name, "DisplayPinCode") == 0) {
        _display_pincode_handle(invocation, parameters);
    } else if (strcmp(method_name, "RequestPasskey") == 0) {
        _request_passkey_handle(invocation, parameters);
    } else if (strcmp(method_name, "DisplayPasskey") == 0) {
        _display_passkey_handle(invocation, parameters);
    } else if (strcmp(method_name, "RequestConfirmation") == 0) {
        _request_confirmation_handle(invocation, parameters);
    } else if (strcmp(method_name, "RequestAuthorization") == 0) {
        _request_authorization_handle(invocation, parameters);
    } else if (strcmp(method_name, "AuthorizeService") == 0) {
        _authorizeservice_handle(invocation, parameters);
    } else if (strcmp(method_name, "Cancel") == 0) {
        _cancel_handle(invocation, parameters);
    } else if (strcmp(method_name, "Release") == 0) {
        _release_handle(invocation, parameters);
    } else {
        BTMG_WARNG("Unsupported method!");
    }
}

static const GDBusInterfaceVTable _vtable = {
    .method_call = _agent_method_call,
};

int bt_agent_register(btmg_io_capability_t io_cap)
{
    GError *error = NULL;
    gchar *capability = NULL;
    int ret = BT_OK;

    if (is_agent_register == true) {
        BTMG_INFO("there is already an old agent, we unregister it first");
        bt_agent_unregister();
    }
    capability = g_strdup(io_capability[io_cap]);
    BTMG_INFO("set io capability: %s", capability);

    agent_dbus_object_id =
            g_dbus_connection_register_object(bluez_mg.dbus, "/org/bluez/agent",
                                              (GDBusInterfaceInfo *)&bluez_iface_agent, &_vtable,
                                              NULL, NULL, &error);

    if (agent_dbus_object_id == 0) {
        BTMG_ERROR("regist agent dbus object failed: %s",
                   (char *)g_dbus_error_get_remote_error(error));
        goto exit;
    }
    g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", "/org/bluez", "org.bluez.AgentManager1",
                                "RegisterAgent",
                                g_variant_new("(os)", "/org/bluez/agent", capability), NULL,
                                G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret == -1) {
        BTMG_ERROR("register agent fail");
        goto exit;
    }

    is_agent_register = true;
    BTMG_DEBUG("register agent complete");
exit:
    g_free(capability);

    return ret;
}

int bt_agent_unregister(void)
{
    GError *error = NULL;
    int ret = BT_OK;

    if (is_agent_register == false) {
        BTMG_DEBUG("no agent, no need unregister");
        return BT_ERROR;
    }

    g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", "/org/bluez", "org.bluez.AgentManager1",
                                "UnregisterAgent", g_variant_new("(o)", "/org/bluez/agent"), NULL,
                                G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        BTMG_ERROR("agent unregister fail");
        return ret;
    }
    g_dbus_connection_unregister_object(bluez_mg.dbus, agent_dbus_object_id);

    is_agent_register = false;
    BTMG_DEBUG("unregister agent complete");

    return ret;
}

int bt_agent_set_default(void)
{
    GError *error = NULL;
    int ret = BT_OK;

    g_dbus_connection_call_sync(bluez_mg.dbus, "org.bluez", "/org/bluez", "org.bluez.AgentManager1",
                                "RequestDefaultAgent", g_variant_new("(o)", "/org/bluez/agent"),
                                NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        BTMG_ERROR("set default agent fail");
        return ret;
    }

    return ret;
}

static int _send_response(void *handle, GVariant *variant)
{
    GDBusMethodInvocation *invocation = (GDBusMethodInvocation *)handle;
    g_dbus_method_invocation_return_value(invocation, variant);

    return BT_OK;
}

int bt_agent_send_pincode(void *handle, char *pincode)
{
    return _send_response(handle, g_variant_new("(s)", pincode));
}

int bt_agent_send_passkey(void *handle, unsigned int passkey)
{
    return _send_response(handle, g_variant_new("(u)", passkey));
}

int bt_agent_send_error(void *handle, btmg_pair_request_error_t e, const char *err_msg)
{
    GDBusMethodInvocation *invocation = (GDBusMethodInvocation *)handle;
    gchar *error_name = "org.bluez.Error.Rejected";

    if (e == BT_PAIR_REQUEST_CANCELED)
        error_name = "org.bluez.Error.Canceled";
    g_dbus_method_invocation_return_dbus_error(invocation, error_name, err_msg);

    return BT_OK;
}

int bt_agent_send_empty_response(void *handle)
{
    return _send_response(handle, NULL);
}
