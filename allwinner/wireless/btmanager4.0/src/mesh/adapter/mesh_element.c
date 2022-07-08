#include <ell/ell.h>
//add for new models architecture
#ifdef NORDIC_MODELS_ENABLE
#include "access.h"
#include "models_mgr.h"
#include "mesh_access.h"
#endif
#include "mesh_internal_api.h"
#include "bluez/include/error.h"
#include "dbus.h"

static bool mesh_model_properties_dbus_iter(struct mesh_model *model,
				struct l_dbus_message_iter *iter_properties);

static bool match_element_idx(const void *a, const void *b)
{
	const struct node_element *element = a;
	uint32_t index = L_PTR_TO_UINT(b);

	return (element->idx == index);
}

static bool match_element_path(const void *a, const void *b)
{
	const struct node_element *element = a;
	const char *path = b;

	if (!element->path)
		return false;

	return (!strcmp(element->path, path));
}

static bool match_model_id(const void *a, const void *b)
{
	const struct mesh_model *model = a;
	uint32_t id = L_PTR_TO_UINT(b);

	return (model->id == id);
}

static bool match_model_idx(const void *a, const void *b)
{
	const struct mesh_model *model = a;
	uint32_t id = L_PTR_TO_UINT(b);

	return (model->mod_idx == id);
}
#if 0

static bool has_binding(struct l_queue *bindings, uint16_t idx)
{
	const struct l_queue_entry *l;

	for (l = l_queue_get_entries(bindings); l; l = l->next) {
		if (L_PTR_TO_UINT(l->data) == idx)
			return true;
	}
	return false;
}


static void model_process_message(void *a, void *b)
{
/*
	struct mesh_model *model = a;
	struct mod_forward *fwd = b;
	AG_MESH_CBK_ACCESS_MSG_T access_msg;
	AW_MESH_ACCESS_MESSAGE_RX_T msg_rx;
	uint32_t opcode;
	uint16_t n = 0;
	bool is_model_data = false;


	if (!has_binding(model->bindings, fwd->appkey_idx)) {
		//l_info("has_binding\n");
		return;
	}

    if (mesh_model_opcode_get(fwd->data, fwd->data_len, &opcode, &n) == false) {
        return;
    }

	msg_rx.opcode.opcode = opcode;
	msg_rx.buf = fwd->data + n;
	msg_rx.buf_len = fwd->data_len - n;
	msg_rx.meta_data.appkey_index = fwd->appkey_idx;
	msg_rx.meta_data.src_addr = fwd->src;
	access_msg.msg = &msg_rx;

	if (is_model_data)
		model->access_rx_cb(&access_msg, NULL);
*/
}
#endif
static struct l_dbus_message *update_model_cfg_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct l_dbus_message_iter iter_properties;
	struct node_element *element = user_data;
	struct mesh_model *model = NULL;
	uint16_t model_id = 0;

	//l_info("update model configuration call");

	if (!l_dbus_message_get_arguments(msg, "qa{sv}", &model_id, &iter_properties))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	model = mesh_model_find_by_id(element, model_id | VENDOR_ID_MASK);
	if (!model) {
		return NULL;
		/* TODO: add manually? */
	}

	l_info("Update (element 0x%x model 0x%x) configuration", element->idx, model->id);
	mesh_model_properties_dbus_iter(model, &iter_properties);

	return NULL;
}
//DevKeyMessageReceived
extern bool config_client_msg_receive(uint16_t src,uint16_t net_idx,const uint8_t *data, uint16_t size);
static struct l_dbus_message *devkey_message_receive_call(struct l_dbus *dbus,
                        struct l_dbus_message *msg,
                        void *user_data)
{
    struct l_dbus_message_iter iter_data;
    //struct node_element *element = user_data;
    uint16_t src, key_idx;
    bool is_remote;
    uint8_t *data;
    uint32_t data_len;
    //struct mod_forward forward;
    //uint32_t opcode;
    //uint16_t n;
    //uint32_t i = 0;
    //l_info("devkey message receive call");

    if (!l_dbus_message_get_arguments(msg, "qbqay", &src, &is_remote, &key_idx, &iter_data))
        return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

    l_dbus_message_iter_get_fixed_array(&iter_data, &data, &data_len);
    /* combine data for model to process */
    //forward.appkey_idx = key_idx;
    //forward.src = src;
    //forward.data = data;
    //forward.data_len = data_len;

    l_info("%s\t src[%d]len[%d]=%s",__FUNCTION__,src,data_len,getstr_hex2str(data,data_len));
    config_client_msg_receive(src,key_idx,data,data_len);

    return NULL;
}

static struct l_dbus_message *message_receive_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct l_dbus_message_iter iter_data;
	//struct node_element *element = user_data;
	uint16_t src, key_idx;
	bool is_sub;
	uint8_t *data;
	uint32_t data_len;
	struct mod_forward forward;
#ifdef NORDIC_MODELS_ENABLE
    access_message_rx_t message;
#endif
    uint32_t opcode;
    uint16_t n;
    //uint32_t i = 0;
	//l_info("message receive call");

	if (!l_dbus_message_get_arguments(msg, "qqbay", &src, &key_idx, &is_sub, &iter_data))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	l_dbus_message_iter_get_fixed_array(&iter_data, &data, &data_len);
	/* combine data for model to process */
	forward.appkey_idx = key_idx;
	forward.src = src;
	forward.data = data;
	forward.data_len = data_len;
#if 0
    l_info("%s\t src[%d]len[%d]=",__FUNCTION__,src,data_len);
    for(i = 0; i < data_len; i++)
    {
        l_info("%02x ",data[i]);
    }
    l_info("end\n");
#endif
#if 0//def _ALI_MESH_APP
    l_info("%s\tmessage receive call:appkey_index:%x,src:%x\n",__FUNCTION__,key_idx,src);
    l_queue_foreach(element->models, model_process_message, &forward);
#else
    if (mesh_model_opcode_get(forward.data, forward.data_len, &opcode, &n) == false) {
        return NULL;
    }
    #ifdef NORDIC_MODELS_ENABLE
    message.meta_data.appkey_idx = key_idx;
    message.meta_data.src = src;
    message.opcode.opcode = opcode;
    message.p_data = data + n;
    message.length = data_len - n;
    #endif
    #if 0
    l_info("\n%s\trx opcode:%x,%d,%d\t",__FUNCTION__,opcode,data_len,n);
    l_info("len[%d]:\t",message.length);
    for(i = 0; i< message.length;i++)
    {
        l_info("%02x ",message.p_data[i]);
    }
    l_info("\n");
    #endif
    #ifdef NORDIC_MODELS_ENABLE
    access_incoming_handle(&message);
    message.p_data = data;
    message.length = data_len;
    rtk_access_incoming_handle(&message);
    #endif
#endif

	return NULL;
}

void virtual_rx_message()
{
#ifdef NORDIC_MODELS_ENABLE
#if 0
    char data[2] = {0x82,0x6D};
    uint32_t opcode;
    uint16_t n;
    uint32_t i = 0;
    uint32_t data_len = 2;
    access_message_rx_t message;
#else
    const uint8_t data[3] = {0x82, 0x0e,0x00};
    uint32_t opcode;
    uint16_t n;
    uint32_t i = 0;
    uint32_t data_len = 3;
    access_message_rx_t message;
#endif
    if (mesh_model_opcode_get(data, data_len, &opcode, &n) == false) {
        return ;
    }
    message.meta_data.appkey_idx = 0;
    message.meta_data.src = 0;
    message.opcode.opcode = opcode;
    message.p_data = data + n;
    message.length = data_len - n;
    l_info("\n%s\trx opcode:%x,%d,%d,%d,%d\t",__FUNCTION__,opcode,data_len,n,message.length,data_len - n);
    l_info("len%d:\t",message.length);
    for(i = 0; i< message.length;i++)
    {
        l_info("%x\t",message.p_data[i]);
    }
    l_info("\n");
    access_incoming_handle(&message);
    message.p_data = data;
    message.length = data_len;
    l_info("message_len = %d,%d\n",message.length,n);
    rtk_access_incoming_handle(&message);
#endif
}

static bool element_property_get_index(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	struct node_element *element = user_data;
    //l_info("%s\n",__func__);
	if (!element)
		return false;

	l_dbus_message_builder_append_basic(builder, 'y', &(element->idx));

	return true;
}

static void get_model_property(void *mdl, void *msg_builder)
{
	struct l_dbus_message_builder *builder = msg_builder;
	struct mesh_model *model = mdl;
    //l_info("%s,%x\n",__func__,model->id);
	/* internal model not need to append? */
	if (model->id == CONFIG_SRV_MODEL || model->id == CONFIG_CLI_MODEL)
		return;

	if ((model->id & VENDOR_ID_MASK) != VENDOR_ID_MASK){
		return;
	}
	l_dbus_message_builder_append_basic(builder, 'q', &(model->id));
}


static void get_vendor_model_property(void *mdl, void *msg_builder)
{
	struct l_dbus_message_builder *builder = msg_builder;
	struct mesh_model *model = mdl;
	uint16_t vendor = 0;
	uint16_t model_idx = 0;

	vendor = model->id >> 16;
	model_idx = model->id & (~VENDOR_ID_MASK);
	if ((model->id & VENDOR_ID_MASK) == VENDOR_ID_MASK){
		return;
	}
	l_info("get_vendor_model_property  vendor %x model_idx %x", vendor, model_idx);
	l_dbus_message_builder_enter_struct(builder, "qq");

	l_dbus_message_builder_append_basic(builder, 'q', &vendor);
	l_dbus_message_builder_append_basic(builder, 'q', &model_idx);

	l_dbus_message_builder_leave_struct(builder);
}

static bool element_property_get_models(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	struct node_element *element = user_data;
	//uint8_t i = 0;
    //l_info("%s\n",__func__);

	if (!element)
		return false;

    l_dbus_message_builder_enter_array(builder, "q");
	l_queue_foreach(element->models, get_model_property, builder);
    l_dbus_message_builder_leave_array(builder);

	return true;
}

static bool element_property_get_vendor_models(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	struct node_element *element = user_data;
	//uint8_t i = 0;

	if (!element)
		return false;

	l_dbus_message_builder_enter_array(builder, "(qq)");
	l_queue_foreach(element->models, get_vendor_model_property, builder);
	l_dbus_message_builder_leave_array(builder);

	return true;
}

static void setup_element_interface(struct l_dbus_interface *iface)
{
	l_dbus_interface_method(iface, "MessageReceived", 0, message_receive_call, "",
				"qqbay", "source", "key", "is_sub", "data");
	l_dbus_interface_method(iface, "DevKeyMessageReceived", 0, devkey_message_receive_call, "",
				"qbqay", "source", "key", "is_sub", "data");
	l_dbus_interface_method(iface, "UpdateModelConfiguration", 0, update_model_cfg_call, "",
				"qa{sv}", "model_id", "config");
	l_dbus_interface_property(iface, "Index", 0, "y",
				element_property_get_index, NULL);
	l_dbus_interface_property(iface, "Models", 0, "aq",
				element_property_get_models, NULL);
	l_dbus_interface_property(iface, "VendorModels", 0, "a(qq)",
				element_property_get_vendor_models, NULL);
}

static void mod_id(void *a, void *b)
{
	 //struct mesh_model *model = a;
	 //l_info("mod id %d\n",model->id);
}

bool element_model_binding(struct node_element *element, struct mesh_model *model)
{
	if (!element->models)
		element->models = l_queue_new();

	model->ele_idx = element->idx;
	l_queue_push_tail(element->models, model);
    l_queue_foreach(element->models,mod_id,NULL);
	return true;
}

struct node_element *mesh_element_init(char *path, uint16_t location, uint8_t idx)
{
	struct node_element *element = NULL;

	element = l_new(struct node_element, 1);

	element->path = path;
	element->location = location;
	element->idx = idx;

	return element;
}

struct mesh_model *app_mesh_model_init(uint32_t model_id)
{
	struct mesh_model *model = NULL;

	model = l_new(struct mesh_model, 1);

	model->id = model_id;

	return model;
}

void app_mesh_model_free(void *data)
{
	struct mesh_model *model = data;

	l_queue_destroy(model->bindings, NULL);
	l_free(model);
}

void mesh_element_free(void *data)
{
	struct node_element *element = data;

	l_queue_destroy(element->models, app_mesh_model_free);
	l_dbus_object_remove_interface(app_dbus_get_bus(), element->path,
			MESH_ELEMENT_INTERFACE);
	l_free(element->path);
	l_free(element);
}

struct mesh_model *mesh_model_find_by_idx(struct node_element *element, uint32_t id)
{
	struct mesh_model *model = NULL;

	model = l_queue_find(element->models,
	match_model_idx, L_UINT_TO_PTR(id));

	return model;
}

struct node_element *mesh_element_find_by_idx(uint8_t idx)
{
	struct mesh_application *mesh_app = NULL;
	struct node_element *element = NULL;

	mesh_app = mesh_application_get_instance();
	if (!mesh_app)
		goto done;

	element = l_queue_find(mesh_app->elements, match_element_idx,
				L_UINT_TO_PTR(idx));

done:
	return element;
}

struct node_element *mesh_element_find_by_path(char *path)
{
	struct mesh_application *mesh_app = NULL;
	struct node_element *element = NULL;

	mesh_app = mesh_application_get_instance();
	if (!mesh_app)
		goto done;

	element = l_queue_find(mesh_app->elements, match_element_path, path);

done:
	return element;
}

struct mesh_model *mesh_model_find_by_id(struct node_element *element, uint32_t id)
{
	struct mesh_model *model = NULL;

	model = l_queue_find(element->models, match_model_id, L_UINT_TO_PTR(id));

	return model;
}

static bool mesh_model_properties_dbus_iter(struct mesh_model *model,
				struct l_dbus_message_iter *iter_properties)
{
	const char *key;
	struct l_dbus_message_iter variant, iter_bindings;
	uint16_t vendor;
	uint32_t period;
	struct l_queue *bindings;
	uint16_t idx;

	while (l_dbus_message_iter_next_entry(iter_properties, &key, &variant)) {
		if (!strcmp(key, "Vendor")) {
			l_dbus_message_iter_get_variant(&variant, "q", &vendor);
			/* TODO */
		} else if (!strcmp(key, "Bindings")) {
			l_dbus_message_iter_get_variant(&variant, "aq", &iter_bindings);
			bindings = l_queue_new();

			while (l_dbus_message_iter_next_entry(&iter_bindings, &idx)) {
				l_queue_push_tail(bindings, L_UINT_TO_PTR(idx));
			}

			/* update bindings */
			l_queue_destroy(model->bindings, NULL);
			model->bindings = bindings;
		} else if (!strcmp(key, "PublicationPeriod")) {
			l_dbus_message_iter_get_variant(&variant, "u", &period);
			/* TODO */
		}
	}

	return true;
}

bool mesh_element_dbus_iter(struct l_dbus_message_iter *iter_elements)
{
	struct l_dbus_message_iter iter_models, iter_properties;
	struct node_element *element;
	struct mesh_model *model;
	uint8_t ele_idx;
	uint16_t model_id;

	while (l_dbus_message_iter_next_entry(iter_elements, &ele_idx, &iter_models)) {
		element = mesh_element_find_by_idx(ele_idx);
		if (!element) {
			continue;
			/* TODO: add manually? */
		}

		while (l_dbus_message_iter_next_entry(&iter_models, &model_id,
							&iter_properties)) {
			model = mesh_model_find_by_id(element, model_id | VENDOR_ID_MASK);
			if (!model) {
				continue;
				/* TODO: add manually? */
			}

			l_info("Update (element 0x%x model 0x%x) config", element->idx, model->id);
			mesh_model_properties_dbus_iter(model, &iter_properties);
		}
	}

	return true;
}

//Internal API
int32_t mesh_element_init_dbus(struct l_dbus *dbus)
{
	if (!l_dbus_register_interface(dbus, MESH_ELEMENT_INTERFACE,
						setup_element_interface,
						NULL, false)) {
		l_info("*register interface fail");
		return AW_MESH_ERROR_ELEMENT_IFACE_REG_FAIL;
	}
    return AW_MESH_ERROR_NONE;
}

void aw_mesh_model_receive_call(uint16_t opcode, uint8_t ele_idx, uint16_t src, uint16_t dst, uint8_t *data,uint32_t len)
{
#ifdef NORDIC_MODELS_ENABLE
    uint8_t rtk_model_data[512];
    access_message_rx_t message;
    message.meta_data.ele_idx = ele_idx;
    message.meta_data.dst = dst;
    message.meta_data.src = src;
    message.opcode.opcode = opcode;
    message.p_data = data;
    message.length = len;
    //access_incoming_handle(&message);
    rtk_model_data[0] = (opcode >> 8)&0xFF;
    rtk_model_data[1] = (opcode)&0xFF;
    if(len < 512)
    {
        memcpy(&rtk_model_data[2],data,len);
        message.p_data = &rtk_model_data[0];
        message.length += 2; //rtk model need opcode
        rtk_access_incoming_handle(&message);
    }
#endif
}
