#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <glib-object.h>
#include <glib.h>

#include "bt_hfp.h"
#include "hfp_rfcomm.h"
#include "bt_log.h"
#include "at.h"
#include "bt_hfp_alsa.h"
#include "common.h"

/* Introspection data for the service*/
static const gchar _introspection_xml[] = "<node>"
                                          "<interface name='org.bluez.Profile1'>"
                                          "<method name='Release'>"
                                          "</method>"
                                          "<method name='NewConnection'>"
                                          "<arg type='o' name='device' direction='in'/>"
                                          "<arg type='h' name='fd' direction='in'/>"
                                          "<arg type='a{sv}' name='fd_properties' direction='in'/>"
                                          "</method>"
                                          "<method name='RequestDisconnection'>"
                                          "<arg type='o' name='device' direction='in'/>"
                                          "</method>"
                                          "</interface>"
                                          "</node>";

static GDBusNodeInfo *_introspection_data;
static guint registration_id;

/* set of features exposed via Service Discovery */
unsigned int features_sdp_hf;
/* set of features exposed via RFCOMM connection */
unsigned int features_rfcomm_hf;
/* information exposed via Apple AT extension */
unsigned int xapl_vendor_id;
unsigned int xapl_product_id;
const char *xapl_software_version;
const char *xapl_product_name;
unsigned int xapl_features;

struct hfp_rfcomm_t *hfp_rfcomm;

int get_hfp_features_hf(void)
{
#if 0
	//TODO
	if (BA_TEST_ESCO_SUPPORT(a)) {
		features_rfcomm_hf |= HFP_HF_FEAT_ESCO;
	}
	return features_rfcomm_hf;
#endif
    return features_rfcomm_hf |= HFP_HF_FEAT_ESCO;
}

static void new_hfp_connection(GVariant *parameters, GDBusMethodInvocation *inv)
{
    GError *err = NULL;
    int rfcomm_fd = -1;
    const char *device_path;
    GVariantIter *properties;
    GUnixFDList *fd_list;
    GDBusMessage *msg = g_dbus_method_invocation_get_message(inv);

    BTMG_DEBUG("enter");

    g_variant_get(parameters, "(&oha{sv})", &device_path, &rfcomm_fd, &properties);
    fd_list = g_dbus_message_get_unix_fd_list(msg);
    if ((rfcomm_fd = g_unix_fd_list_get(fd_list, 0, &err)) == -1) {
        BTMG_ERROR("Couldn't obtain RFCOMM socket: %s", err->message);
        goto fail;
    }

    if (rfcomm_fd != -1) {
        if ((hfp_rfcomm = hfp_rfcomm_new(rfcomm_fd)) == NULL) {
            BTMG_ERROR("init hfp rfcomm fail");
            goto fail;
        }
    }

    g_dbus_method_invocation_return_value(inv, NULL);
    goto final;

fail:
    g_dbus_method_invocation_return_error(inv, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                          "Unable to connect profile");
    if (rfcomm_fd != -1)
        close(rfcomm_fd);

final:

    g_variant_iter_free(properties);
    if (err != NULL)
        g_error_free(err);
}

static void request_hfp_disconnection(GVariant *parameters, GDBusMethodInvocation *inv)
{
    BTMG_INFO("Disconnecting hands-free profile");

    g_object_unref(inv);
}

static void release_hfp(void)
{
    BTMG_INFO("Releasing hands-free profile");
}

static void method_call_handle(GDBusConnection *connection, const gchar *sender,
                               const gchar *object_path, const gchar *interface_name,
                               const gchar *method_name, GVariant *parameters,
                               GDBusMethodInvocation *invocation, gpointer user_data)
{
    if (g_strcmp0(method_name, "Release") == 0)
        release_hfp();
    if (g_strcmp0(method_name, "NewConnection") == 0)
        new_hfp_connection(parameters, invocation);
    if (g_strcmp0(method_name, "RequestDisconnection") == 0)
        request_hfp_disconnection(parameters, invocation);
}

static const GDBusInterfaceVTable _interface_vtable = { .method_call = method_call_handle,
                                                        .get_property = NULL,
                                                        .set_property = NULL };

static int register_hfp_object(void)
{
    int ret = BT_OK;
    GError *error = NULL;

    BTMG_DEBUG("enter");
    /* register objects */
    _introspection_data = g_dbus_node_info_new_for_xml(_introspection_xml, NULL);
    GDBusInterfaceInfo *interface =
            g_dbus_node_info_lookup_interface(_introspection_data, "org.bluez.Profile1");
    registration_id =
            g_dbus_connection_register_object(bluez_mg.dbus, "/org/bluez/HFP/HandsFree", interface,
                                              &_interface_vtable, NULL, NULL, &error);

    ret = g_dbus_call_sync_check_error(error);
    if (ret < 0) {
        BTMG_ERROR("register object fail");
        return ret;
    }

    BTMG_DEBUG("registration id : %d", registration_id);

    return BT_OK;
}

/**
 * Register hands-free profile in BlueZ. */
static int register_hfp_profile(const char *uuid, uint16_t version, uint16_t features)
{
    int ret = BT_OK;
    GError *error = NULL;
    GDBusMessage *msg = NULL, *rep = NULL;

    BTMG_DEBUG("enter");

    msg = g_dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.ProfileManager1",
                                         "RegisterProfile");

    GVariantBuilder options;
    g_variant_builder_init(&options, G_VARIANT_TYPE("a{sv}"));

    if (version)
        g_variant_builder_add(&options, "{sv}", "Version", g_variant_new_uint16(version));
    if (features)
        g_variant_builder_add(&options, "{sv}", "Features", g_variant_new_uint16(features));
    g_dbus_message_set_body(msg,
                            g_variant_new("(osa{sv})", "/org/bluez/HFP/HandsFree", uuid, &options));
    g_variant_builder_clear(&options);
    if ((rep = g_dbus_connection_send_message_with_reply_sync(bluez_mg.dbus, msg,
                                                              G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1,
                                                              NULL, NULL, &error)) == NULL) {
        goto fail;
    }
    if (g_dbus_message_get_message_type(rep) == G_DBUS_MESSAGE_TYPE_ERROR) {
        g_dbus_message_to_gerror(rep, &error);
        goto fail;
    }

    goto final;

fail:

    if (error != NULL) {
        g_error_free(error);
    }
    BTMG_ERROR("Couldn't register hands-free profile,error:%s", error->message);
    ret = BT_ERROR;

final:
    if (msg != NULL)
        g_object_unref(msg);
    if (rep != NULL)
        g_object_unref(rep);

    return ret;
}

void hfp_event_ind(btmg_hfp_even_t event, btmg_hfp_data_t *data)
{
    BTMG_INFO("EVENT :%d", event);

    if (btmg_cb_p->btmg_hfp_cb.hfp_hf_event_cb)
        btmg_cb_p->btmg_hfp_cb.hfp_hf_event_cb(event, data);
}

int bt_hfp_hf_init()
{
    int ret = BT_OK;

    BTMG_DEBUG("enter");

    features_sdp_hf = SDP_HFP_HF_FEAT_CLI | SDP_HFP_HF_FEAT_VOLUME;
    features_rfcomm_hf = HFP_HF_FEAT_CLI | HFP_HF_FEAT_VOLUME | HFP_HF_FEAT_ECS | HFP_HF_FEAT_ECC;

    /* build-in Apple accessory identification */
    xapl_vendor_id = 0xB103, xapl_product_id = 0xA15A, xapl_software_version = "0300",
    xapl_product_name = "BTMG4.0", xapl_features = XAPL_FEATURE_BATTERY | XAPL_FEATURE_DOCKING;

    register_hfp_profile(BLUETOOTH_UUID_HFP_HF, 0x0106, features_sdp_hf);
    register_hfp_object();
    if (bt_hfp_alsa_init() != 0) {
        BTMG_ERROR("hfp alsa init fail");
        return BT_ERROR;
    }

    return BT_OK;
}

int bt_hfp_hf_deinit()
{
    BTMG_DEBUG("enter");

    bt_hfp_alsa_deinit();

    return BT_OK;
}

int bt_hfp_hf_send_at_ata(void)
{
    char *buf;
    buf = "ATA\r";

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_RAW, buf, NULL);
}

int bt_hfp_hf_send_at_chup(void)
{
    char *buf;
    buf = "+CHUP\r";

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_CMD, buf, NULL);
}

int bt_hfp_hf_send_at_atd(char *number)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "ATD%s;\r", number);

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_RAW, buf, NULL);
}

int bt_hfp_hf_send_at_bldn(void)
{
    char *buf;
    buf = "+BLDN\r";

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_CMD, buf, NULL);
}

int bt_hfp_hf_send_at_btrh(bool query, uint32_t val)
{
    char buf[128];

    if (query) {
        snprintf(buf, sizeof(buf), "AT+BTRH?\r");
    } else {
        snprintf(buf, sizeof(buf), "AT+BTRH=%u\r", val);
    }

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_RAW, buf, NULL);
}

int bt_hfp_hf_send_at_vts(char code)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "AT+VTS=%c\r", code);

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_RAW, buf, NULL);
}

int bt_hfp_hf_send_at_bcc(void)
{
    char *buf;
    buf = "+BCC\r";

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_CMD, buf, NULL);
}

int bt_hfp_hf_send_at_cnum(void)
{
    char *buf;
    buf = "+CNUM\r";

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_CMD, buf, NULL);
}


int bt_hfp_hf_send_at_vgs(uint32_t volume)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "AT+VGS=%u\r", volume);

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_RAW, buf, NULL);
}

int bt_hfp_hf_send_at_vgm(uint32_t volume)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "AT+VGM=%u\r", volume);

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_RAW, buf, NULL);
}

int bt_hfp_hf_send_at_cmd(char *cmd)
{
    char buf[96];
    char *p1, *p2;
    int len = strlen(cmd);

    if (!len || len < 3)
        return BT_ERROR_INVALID_ARGS;

    strncpy(buf, cmd, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    p1 = buf;
    /* AT command */
    if (strncasecmp(p1, "AT", 2) != 0) {
        BTMG_ERROR("invalid AT command: %s", cmd);
        return BT_ERROR_INVALID_ARGS;
    }

    len = strlen(buf);
    p2 = buf + len;
    p2--;

    while (len > 0) {
        if (*p2 == '\r') {
            *p2 = '\0';
            break;
        }
    }

    return rfcomm_write_at(hfp_rfcomm->fd, AT_TYPE_RAW, buf, NULL);
}
