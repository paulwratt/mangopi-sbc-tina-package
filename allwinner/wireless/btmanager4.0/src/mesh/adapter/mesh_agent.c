#include <stdio.h>
#include <ell/ell.h>
#include <semaphore.h>
#include <time.h>
#include "dbus.h"
#include "bluez/include/error.h"
#include "mesh_internal_api.h"
#include "AWTypes.h"

#define INTERACTION_TIME 100
//API FOR DEBUG LOG
#define LOCAL_MODEL AW_APP_PROV_MODULE
#define LOG_PRINTF(LEVEL,FMT,...)   mesh_log(LEVEL,LOCAL_MODEL,FMT,##__VA_ARGS__)

static struct mesh_agent *g_mesh_agent;

uint8_t auth_value[16] = {0x18, 0xee, 0xd9, 0xc2, 0xa5, 0x6a, 0xdd, 0x85,
			0x04, 0x9f, 0xfc, 0x3c, 0x59, 0xad, 0x0e, 0x12};

sem_t sem_g;
struct timespec ts;
uint8_t prov_pub_type, prov_auth, prov_auth_size;
uint16_t prov_auth_action;
uint8_t prov_pub_key_auth[64];
uint8_t prov_static_value[16];
uint32_t prov_number;

static int wait_app_interaction() {

	int32_t sem_ret = 0;
	if (sem_init(&sem_g, 0, 0) != 0) {
		l_info("agent_choose_caps sem_init init error\n");
	}

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += INTERACTION_TIME;

	sem_ret = sem_timedwait(&sem_g, &ts);
	if (sem_ret != 0) {
		l_info("%s,sem_ret %d\n",__FUNCTION__, sem_ret);
    }
	return sem_ret;
}

static bool agent_prov_info_init(struct mesh_agent *agent, char **caps,
			uint32_t caps_len, char **oob, uint32_t oob_len)
{
	uint32_t i = 0;

	if (caps_len > MESH_CAPS_MAX || oob_len > MESH_OOB_INFO_MAX)
		return false;

	/* initialize the capabilities info */
	for (i = 0; i < caps_len; i++)
		agent->caps[i] = l_strdup(caps[i]);

	/* initialize the oob info */
	for (i = 0; i < oob_len; i++)
		agent->oob[i] = l_strdup(oob[i]);

	return true;
}

static bool app_mesh_agent_init(char *path, char *owner, char **caps, uint32_t caps_len,
			char **oob, uint32_t oob_len, char *uri)
{
	struct mesh_agent *mesh_agent = NULL;

	if (g_mesh_agent) {
		l_error("the mesh agent instance already exists");
		return false;
	}

	mesh_agent = l_new(struct mesh_agent, 1);

	mesh_agent->path = l_strdup(path);
	mesh_agent->owner = l_strdup(owner);
	mesh_agent->uri = l_strdup(uri);

	if(!agent_prov_info_init(mesh_agent, caps, caps_len, oob, oob_len)) {
		l_error("agent: provisioning info init failed");
		return false; /* use goto to free resources */
	}

	g_mesh_agent = mesh_agent;

	return true;
}

void mesh_agent_free(void *data)
{
	struct mesh_agent *mesh_agent = data;
	uint32_t i = 0;

	/* free the capabilities info */
	for (i = 0; i < MESH_CAPS_MAX && mesh_agent->caps[i]; i++)
		l_free(mesh_agent->caps[i]);

	/* free the oob info */
	for (i = 0; i < MESH_OOB_INFO_MAX && mesh_agent->oob[i]; i++)
		l_free(mesh_agent->oob[i]);

	l_dbus_object_remove_interface(app_dbus_get_bus(), mesh_agent->path,
			MESH_PROVISION_AGENT_INTERFACE);

	l_free(mesh_agent->path);
	l_free(mesh_agent->owner);
	l_free(mesh_agent->uri);
	l_free(mesh_agent);

	g_mesh_agent = NULL;
}

struct mesh_agent *mesh_agent_get_instance(void)
{
	return g_mesh_agent;
}

static bool agent_property_get_capabilities(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	struct mesh_agent *mesh_agent = user_data;
	uint32_t i = 0;

	if (!mesh_agent)
		return false;

	l_dbus_message_builder_enter_array(builder, "s");

	for (i = 0; i < MESH_CAPS_MAX && mesh_agent->caps[i]; i++)
		l_dbus_message_builder_append_basic(builder, 's',
					mesh_agent->caps[i]);

	l_dbus_message_builder_leave_array(builder);

	return true;
}

static bool agent_property_get_oob(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	struct mesh_agent *mesh_agent = user_data;
	uint32_t i = 0;

	if (!mesh_agent)
		return false;

	l_dbus_message_builder_enter_array(builder, "s");

	for (i = 0; i < MESH_OOB_INFO_MAX && mesh_agent->oob[i]; i++)
		l_dbus_message_builder_append_basic(builder, 's',
					mesh_agent->oob[i]);

	l_dbus_message_builder_leave_array(builder);

	return true;
}

static struct l_dbus_message *agent_prompt_static(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;
	struct l_dbus_message_builder *builder;
	char *action;
    uint8_t static_auth_value[16];
	struct mesh_application *app = mesh_application_get_instance();
	AwMeshEventCb_t event_cb = app->mesh_event_cb_handle;
	AW_MESH_EVENT_T mesh_event;

	if (!l_dbus_message_get_arguments(message, "s", &action)) {
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
	}

	if(event_cb) {
		mesh_event.evt_code = AW_MESH_EVENT_PROV_REQUEST_OOB_AUTH_VALUE;
		event_cb(&mesh_event, NULL);
        memcpy(&static_auth_value[0],&mesh_event.param.prov_show_auth.auth[0],16);
	}

	l_info("agent_prompt -%s", action);

	if (!memcmp(action, "static-oob", strlen("static-oob"))) {
		l_info("please input agent static-oob ");

		//wait_app_interaction();
	} else if (!memcmp(action, "out-alpha", strlen("out-alpha"))) {
		l_info("please input agent out-alpha ");

		wait_app_interaction();
	} else {
		l_info("agent_prompt_static action %s", action);
	}

	LOG_PRINTF(AW_DBG_VERB_LEVE,"static value: %s", getstr_hex2str(static_auth_value, 16));

	reply = l_dbus_message_new_method_return(message);

	builder = l_dbus_message_builder_new(reply);
	dbus_append_byte_array(builder, static_auth_value, 16);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	return reply;
}

static struct l_dbus_message *agent_prompt_numeric(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;
	char  *action;
	struct mesh_application *app = mesh_application_get_instance();
	AwMeshEventCb_t event_cb = app->mesh_event_cb_handle;
	AW_MESH_EVENT_T mesh_event;

	if (!l_dbus_message_get_arguments(message, "s", &action)) {
			mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
			return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
	}

	if(event_cb) {
		mesh_event.evt_code = AW_MESH_EVENT_PROV_REQUEST_OOB_AUTH_NUM;
		event_cb(&mesh_event, NULL);
	}

	l_info("agent_prompt_numeric-%s, please input number:", action);
	wait_app_interaction();
	l_info(" prov_number %d", prov_number);

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "u", prov_number);

	return reply;
}

static struct l_dbus_message *agent_Private_Key(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;
	struct l_dbus_message_builder *builder;
	uint8_t auth_value[32] = {0x52, 0x9A, 0xA0, 0x67, 0x0D, 0x72, 0xCD, 0x64, 0x97, 0x50,
							0x2E, 0xD4, 0x73, 0x50, 0x2B, 0x03, 0x7E, 0x88, 0x03, 0xB5,
							0xC6, 0x08, 0x29, 0xA5, 0xA3, 0xCA, 0xA2, 0x19, 0x50, 0x55,
							0x30, 0xBA};
	char *action;

	l_info("agent_Private_Key:");

	if (!l_dbus_message_get_arguments(message, "s", &action))
		return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);

	reply = l_dbus_message_new_method_return(message);

	builder = l_dbus_message_builder_new(reply);
	dbus_append_byte_array(builder, auth_value, 32);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	return reply;
}

static struct l_dbus_message *agent_Public_Key(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;
	struct l_dbus_message_builder *builder;
	struct mesh_application *app = mesh_application_get_instance();
	AwMeshEventCb_t event_cb = app->mesh_event_cb_handle;
	AW_MESH_EVENT_T mesh_event;
    uint8_t public_key[64];
#if 0
	uint8_t auth_value[64] = {0xf4, 0x65, 0xe4, 0x3f, 0xf2, 0x3d, 0x3f, 0x1b, 0x9d, 0xc7,
							0xDF, 0xC0, 0x4D, 0xA8, 0x75, 0x81, 0x84, 0xDB, 0xC9, 0x66,
							0x20, 0x47, 0x96, 0xEC, 0xCF, 0x0D, 0x6C, 0xF5, 0xE1, 0x65,
							0x00, 0xCC, 0x02, 0x01, 0xD0, 0x48, 0xBC, 0xBB, 0xD8, 0x99,
							0xEE, 0xEF, 0xC4, 0x24, 0x16, 0x4E, 0x33, 0xC2, 0x01, 0xC2,
							0xB0, 0x10, 0xCA, 0x6B, 0x4D, 0x43, 0xA8, 0xA1, 0x55, 0xCA,
							0xD8, 0xEC, 0xB2, 0x79};
	uint8_t auth_value_default[64] = {0xe4, 0xa3, 0x4b, 0xae, 0x82, 0x25, 0xde, 0xea, 0xb8, 0xdd,
						0x03, 0x39, 0xb8, 0xb6, 0x86, 0x6f, 0xab, 0x3f, 0x09, 0x59,
						0xdf, 0xb9, 0xca, 0x30, 0xb8, 0xf2, 0x48, 0xfe, 0x92, 0xfc,
						0xc7, 0x85, 0x25, 0x26, 0xe7, 0x39, 0x75, 0xdd, 0xa6, 0xf5,
						0x23, 0x23, 0x2d, 0xaf, 0xa8, 0xc9, 0x4a, 0x06, 0x9c, 0x88,
						0x31, 0x26, 0x7f, 0x94, 0x16, 0x0f, 0x59, 0x3a, 0x6e, 0x04,
						0xf0, 0x7b, 0x89, 0x42};
#endif
	char *action;

	l_info("please input public key:");

	if (!l_dbus_message_get_arguments(message, "s", &action)){
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
	}

	if(event_cb) {
		mesh_event.evt_code = AW_MESH_EVENT_PROV_REQUEST_OOB_PUBLIC_KEY;
        mesh_event.param.prov_show_pk.pk = &public_key[0];
		event_cb(&mesh_event, NULL);
	}

	//wait_app_interaction();

	LOG_PRINTF(AW_DBG_VERB_LEVE,"public key : %s", getstr_hex2str(public_key, 64));

	reply = l_dbus_message_new_method_return(message);

	builder = l_dbus_message_builder_new(reply);
	dbus_append_byte_array(builder, public_key, 64);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	return reply;
}

static struct l_dbus_message *agent_display_numeric(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;
	char  *action;
	uint32_t cnt;
	struct mesh_application *app = mesh_application_get_instance();
	AwMeshEventCb_t event_cb = app->mesh_event_cb_handle;
	AW_MESH_EVENT_T mesh_event;

	if (!l_dbus_message_get_arguments(message, "su", &action, &cnt)) {
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
	}

	if(event_cb) {
		mesh_event.evt_code = AW_MESH_EVENT_PROV_SHOW_OOB_AUTH_VALUE_NUM;
		mesh_event.param.prov_show_auth_num.auth_num = cnt;
		event_cb(&mesh_event, NULL);
	}
	l_info("agent_display_numeric-%s, value:%d", action, cnt);

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	return reply;
}

static struct l_dbus_message *agent_display_string(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;
	char  *string;
	struct mesh_application *app = mesh_application_get_instance();
	AwMeshEventCb_t event_cb = app->mesh_event_cb_handle;
	AW_MESH_EVENT_T mesh_event;

	if (!l_dbus_message_get_arguments(message, "s", &string)){
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
	}

	if(event_cb) {
		mesh_event.evt_code = AW_MESH_EVENT_PROV_SHOW_OOB_AUTH_VALUE;
		memcpy(mesh_event.param.prov_show_auth.auth, string, AW_MESH_AUTHENTICATION_SIZE);
		event_cb(&mesh_event, NULL);
	}
	l_info("agent_display_string-%s", string);
	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");
	return reply;
}

static struct l_dbus_message *agent_Choose_Caps(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;
	//struct l_dbus_message_builder *builder;
	uint16_t algo, output_action, input_action;
	uint8_t ele_num, pub_type, static_type, output_size, input_size;
	struct mesh_application *app = mesh_application_get_instance();
	AwMeshEventCb_t event_cb = app->mesh_event_cb_handle;
	AW_MESH_EVENT_T mesh_event;

	l_info("please input Choose Caps:");

	if (!l_dbus_message_get_arguments(message, "yqyyyqyq", &ele_num, &algo, &pub_type, &static_type, &output_size, &output_action, &input_size, &input_action)) {
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
		return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
	}

	if(event_cb) {
		mesh_event.evt_code = AW_MESH_EVENT_PROV_CAPABILITIES;
		mesh_event.param.prov_cap.cap.algorithms = algo;
		mesh_event.param.prov_cap.cap.number_of_elements  = ele_num;
		mesh_event.param.prov_cap.cap.input_oob_action = input_action;
		mesh_event.param.prov_cap.cap.input_oob_size = input_size;
		mesh_event.param.prov_cap.cap.output_oob_action = output_action;
		mesh_event.param.prov_cap.cap.output_oob_size = output_size;
		mesh_event.param.prov_cap.cap.public_key_type = pub_type;
		mesh_event.param.prov_cap.cap.static_oob_type = static_type;
		event_cb(&mesh_event, NULL);
	}

	//wait_app_interaction();

	l_info("prov_pub_type %d prov_auth %d prov_auth_size %d prov_auth_action %d", prov_pub_type, prov_auth, prov_auth_size, prov_auth_action);
	reply = l_dbus_message_new_method_return(message);

	l_dbus_message_set_arguments(reply, "yyyq", prov_pub_type, prov_auth, prov_auth_size, prov_auth_action);
	return reply;
}

int32_t aw_mesh_prov_set_start_choose_paramters(uint8_t pub_type, uint8_t auth, uint8_t auth_size, uint16_t auth_action) {

	prov_pub_type = pub_type;
	prov_auth = auth;
	prov_auth_size = auth_size;
	prov_auth_action = auth_action;
	//sem_post(&sem_g);
	return AW_ERROR_NONE;
}

int32_t aw_mesh_prov_set_pub_key(uint8_t *pub_key) {
	int i = 0;

	for (i = 0; i < 64; i++) {
		prov_pub_key_auth[i] = *(pub_key + i);
	}

	//sem_post(&sem_g);
	return AW_ERROR_NONE;
}

int32_t aw_mesh_prov_set_auth_number(uint32_t number) {
	prov_number = number;
	sem_post(&sem_g);
	return AW_ERROR_NONE;
}

int32_t aw_mesh_prov_set_auth_static_value(uint8_t *value) {
	int i = 0;

	for (i = 0; i < 16; i++) {
		prov_static_value[i] = *(value + i);
	}

	//sem_post(&sem_g);
	return AW_ERROR_NONE;
}

static void prov_set_manaual_choose_call_reply(struct l_dbus_message *reply, void *user_data)
{
	int err;

	err = dbus_get_reply_error(reply);

	if (err == MESH_ERROR_NONE) {
		l_info("prov_set_manaual_choose_call_reply error %d", err);
	} else {
		l_info("prov_set_manaual_choose_call_reply error %d", err);
		mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
	}
}

int32_t aw_mesh_prov_set_manaual_choose(uint8_t enable)
{
	struct l_dbus *dbus = NULL;
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	struct mesh_application *mesh_app = NULL;

	dbus = app_dbus_get_bus();
	mesh_app = mesh_application_get_instance();
	MESH_READY_ACCESS(mesh_app);
	msg = l_dbus_message_new_method_call(
		dbus, BLUEZ_MESH_NAME, mesh_app->node_path, MESH_NODE_INTERFACE,
		"SetManaualChoose");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'y', &enable);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send_with_reply(dbus, msg, prov_set_manaual_choose_call_reply, NULL, NULL);

	return AW_ERROR_NONE;
}

static void setup_agent_interface(struct l_dbus_interface *iface)
{
	/* TODO: add agent methods */
	l_dbus_interface_method(iface, "PromptStatic", 0,
					agent_prompt_static, "ay", "s", "authValue");
	l_dbus_interface_method(iface, "PromptNumeric", 0,
					agent_prompt_numeric, "u", "s", "inputnumber");
	l_dbus_interface_method(iface, "DisplayNumeric", 0,
					agent_display_numeric, "", "su");
	l_dbus_interface_method(iface, "DisplayString", 0,
					agent_display_string, "", "s");
	l_dbus_interface_property(iface, "Capabilities", 0, "as",
				agent_property_get_capabilities, NULL);
	l_dbus_interface_property(iface, "OutOfBandInfo", 0, "as",
				agent_property_get_oob, NULL);
	l_dbus_interface_method(iface, "PublicKey", 0,
				agent_Public_Key, "ay", "s", "publicKey");
	l_dbus_interface_method(iface, "PrivateKey", 0,
				agent_Private_Key, "ay", "s", "privateKey");
	l_dbus_interface_method(iface, "ChooseCaps", 0,
				agent_Choose_Caps, "", "yqyyyqyq", "ele_num", "algorithms",
				"pub_type", "static_type", "output_size", "output_action",
				"input_size", "input_action");
}

//Internal API
int32_t mesh_agent_init_dbus(struct l_dbus *dbus)
{
	char *caps_tmp[] = { "blink", "beep", "vibrate", "out-numeric", "out-alpha", "push", "twist", "in-numeric", "in-alpha", "static-oob", "public-oob","no-oob"};
	/*public-oob
	static-oob
	{"blink", 0x0001, 0x0000, 1},
	{"beep", 0x0002, 0x0000, 1},
	{"vibrate", 0x0004, 0x0000, 1},
	{"out-numeric", 0x0008, 0x0000, 8},
	{"out-alpha", 0x0010, 0x0000, 8},
	{"push", 0x0000, 0x0001, 1},
	{"twist", 0x0000, 0x0002, 1},
	{"in-numeric", 0x0000, 0x0004, 8},
	{"in-alpha", 0x0000, 0x0008, 8}*/
	char *oob_tmp[] = {"other", "uri", "machine-code-2d", "barcode", "nfc", "number", "string", "on-box", "in-box", "on-paper", "in-manual", "on-device"};
	//char *oob_tmp[] = {"other"};
	//char *caps_tmp[] = {"no-oob"};

	/*
	{"other", 0x0001},
	{"uri", 0x0002},
	{"machine-code-2d", 0x0004},
	{"barcode", 0x0008},
	{"nfc", 0x0010},
	{"number", 0x0020},
	{"string", 0x0040},
	{"on-box", 0x0800},
	{"in-box", 0x1000},
	{"on-paper", 0x2000},
	{"in-manual", 0x4000},
	{"on-device", 0x8000}
	*/

	struct mesh_application *mesh_app = NULL;
	struct mesh_agent *agent = NULL;

	if (!l_dbus_register_interface(dbus, MESH_PROVISION_AGENT_INTERFACE,
						setup_agent_interface,
						NULL, false)) {
		return AW_MESH_ERROR_AGENT_IFACE_REG_FAIL;
	}

	mesh_app = mesh_application_get_instance();

	if (!app_mesh_agent_init(MESH_AGENT_PATH_PREFIX, mesh_app->path,
				caps_tmp, L_ARRAY_SIZE(caps_tmp),
				oob_tmp, L_ARRAY_SIZE(oob_tmp),
				NULL)) {

        return AW_MESH_ERROR_AGENT_INIT_FAIL;
	}
	agent = mesh_agent_get_instance();
	if (!l_dbus_object_add_interface(dbus, agent->path,
				MESH_PROVISION_AGENT_INTERFACE, agent)) {
		return AW_MESH_ERROR_AGENT_IFACE_ADD_FAIL;
	}

	return AW_MESH_ERROR_NONE;
}
