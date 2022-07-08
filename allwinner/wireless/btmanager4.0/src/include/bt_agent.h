#ifndef __BT_AGENTX_H__
#define __BT_AGENTX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gio/gio.h>
static const GDBusArgInfo arg_device = { -1, "device", "o", NULL };
static const GDBusArgInfo arg_pincode = { -1, "pincode", "s", NULL };
static const GDBusArgInfo arg_uuid = { -1, "uuid", "s", NULL };
static const GDBusArgInfo arg_passkey = { -1, "passkey", "u", NULL };
static const GDBusArgInfo arg_entered = { -1, "entered", "q", NULL };

static const GDBusArgInfo *in_RequestPinCode[] = {
    &arg_device,
    NULL,
};
static const GDBusArgInfo *out_RequestPinCode[] = {
    &arg_pincode,
    NULL,
};
static const GDBusArgInfo *in_DisplayPinCode[] = {
    &arg_device,
    &arg_pincode,
    NULL,
};
static const GDBusArgInfo *in_RequestPasskey[] = {
    &arg_device,
    NULL,
};
static const GDBusArgInfo *out_RequestPasskey[] = {
    &arg_passkey,
    NULL,
};
static const GDBusArgInfo *in_DisplayPasskey[] = {
    &arg_device,
    &arg_passkey,
    &arg_entered,
    NULL,
};
static const GDBusArgInfo *in_RequestConfirmation[] = {
    &arg_device,
    &arg_passkey,
    NULL,
};
static const GDBusArgInfo *in_RequestAuthorization[] = {
    &arg_device,
    NULL,
};
static const GDBusArgInfo *in_AuthorizeService[] = {
    &arg_device,
    &arg_uuid,
    NULL,
};
static const GDBusMethodInfo bluez_iface_agent_Release = {
    -1, "Release", NULL, NULL, NULL,
};
static const GDBusMethodInfo bluez_iface_agent_RequestPinCode = {
    -1,   "RequestPinCode", (GDBusArgInfo **)in_RequestPinCode, (GDBusArgInfo **)out_RequestPinCode,
    NULL,
};
static const GDBusMethodInfo bluez_iface_agent_DisplayPinCode = {
    -1, "DisplayPinCode", (GDBusArgInfo **)in_DisplayPinCode, NULL, NULL,
};
static const GDBusMethodInfo bluez_iface_agent_RequestPasskey = {
    -1,   "RequestPasskey", (GDBusArgInfo **)in_RequestPasskey, (GDBusArgInfo **)out_RequestPasskey,
    NULL,
};
static const GDBusMethodInfo bluez_iface_agent_DisplayPasskey = {
    -1, "DisplayPasskey", (GDBusArgInfo **)in_DisplayPasskey, NULL, NULL,
};
static const GDBusMethodInfo bluez_iface_agent_RequestConfirmation = {
    -1, "RequestConfirmation", (GDBusArgInfo **)in_RequestConfirmation, NULL, NULL,
};
static const GDBusMethodInfo bluez_iface_agent_RequestAuthorization = {
    -1, "RequestAuthorization", (GDBusArgInfo **)in_RequestAuthorization, NULL, NULL,
};
static const GDBusMethodInfo bluez_iface_agent_AuthorizeService = {
    -1, "AuthorizeService", (GDBusArgInfo **)in_AuthorizeService, NULL, NULL,
};
static const GDBusMethodInfo bluez_iface_agent_Cancel = {
    -1, "Cancel", NULL, NULL, NULL,
};
static const GDBusMethodInfo *bluez_iface_agent_methods[] = {
    &bluez_iface_agent_Release,
    &bluez_iface_agent_RequestPinCode,
    &bluez_iface_agent_DisplayPinCode,
    &bluez_iface_agent_RequestPasskey,
    &bluez_iface_agent_DisplayPasskey,
    &bluez_iface_agent_RequestConfirmation,
    &bluez_iface_agent_RequestAuthorization,
    &bluez_iface_agent_AuthorizeService,
    &bluez_iface_agent_Cancel,
    NULL,
};

int bt_agent_register(btmg_io_capability_t io_cap);
int bt_agent_unregister(void);
int bt_agent_set_default(void);
int bt_agent_send_pincode(void *handle, char *pincode);
int bt_agent_send_passkey(void *handle, unsigned int passkey);
int bt_agent_send_error(void *handle, btmg_pair_request_error_t e, const char *err_msg);
int bt_agent_send_empty_response(void *handle);
#ifdef __cplusplus
}
#endif

#endif
