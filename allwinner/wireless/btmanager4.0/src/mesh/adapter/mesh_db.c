#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ell/ell.h>
#include <pthread.h>
#include <unistd.h>

#include "dbus.h"
#include "mesh_db.h"
#include "bluez/include/error.h"
#include "bluez/include/util.h"
#include "mesh_internal_api.h"

#define LOCAL_MODEL AW_APP_DB_MODULE
#define LOG_PRINTF(LEVEL,FMT,...)   mesh_log(LEVEL,LOCAL_MODEL,FMT,##__VA_ARGS__)

static struct mesh_db_prov *g_prov_db;


static void print_uint8(char *str,uint8_t *data,uint8_t data_len)
{
    char name[] ="print_uint8";
    char str_data[64];

    if(str == NULL)
        str = name;
    hex2str(data,data_len,str_data,64);
    l_info("%s:%s",str,str_data);
}

static bool match_net_key_index(const void *a, const void *b)
{
	const struct mesh_db_net *subnet = a;
	uint16_t idx = L_PTR_TO_UINT(b);

	return subnet->idx == idx;
}

static struct mesh_db_node *prov_db_node_new(void)
{
	struct mesh_db_node *db_node;
	struct mesh_db_prov *prov_db;// = mesh_db_prov_db_get();
    mesh_db_prov_db_init();
    prov_db = mesh_db_prov_db_get();
	db_node = l_new(struct mesh_db_node, 1);
	if (!prov_db->nodes)
		prov_db->nodes = l_queue_new();
	l_queue_push_tail(prov_db->nodes, db_node);
	return db_node;
}

static struct mesh_db_element *prov_db_element_new(struct mesh_db_node *db_node)
{
	struct mesh_db_element *db_element;

	db_element = l_new(struct mesh_db_element, 1);

	if (!db_node->elements)
		db_node->elements = l_queue_new();

	l_queue_push_tail(db_node->elements, db_element);

	return db_element;
}

static struct mesh_db_model *prov_db_model_new(struct mesh_db_element *db_element)
{
	struct mesh_db_model *db_model;

	db_model = l_new(struct mesh_db_model, 1);

	if (!db_element->models)
		db_element->models = l_queue_new();

	l_queue_push_tail(db_element->models, db_model);

	return db_model;
}

static struct mesh_db_net *prov_db_subnet_new(struct mesh_db_node *db_node)
{
	struct mesh_db_net *db_subnet;

	db_subnet = l_new(struct mesh_db_net, 1);

	if (!db_node->subnets)
		db_node->subnets = l_queue_new();

	l_queue_push_tail(db_node->subnets, db_subnet);

	return db_subnet;
}

static struct mesh_db_app_key *prov_db_app_key_new(struct mesh_db_net *db_subnet)
{
	struct mesh_db_app_key *db_app_key;

	db_app_key = l_new(struct mesh_db_app_key, 1);

	if (!db_subnet->app_keys)
		db_subnet->app_keys = l_queue_new();

	l_queue_push_tail(db_subnet->app_keys, db_app_key);

	return db_app_key;
}

static bool parse_uuid(struct mesh_db_node *db_node,
							struct l_dbus_message_iter *iter_dev_key)
{
	uint8_t *m_uuid;
	uint32_t n;

	l_dbus_message_iter_get_fixed_array(iter_dev_key, &m_uuid, &n);

	memcpy(db_node->uuid, m_uuid, 16);
    print_uint8("parse_uuid",m_uuid,16);

	return true;
}
static bool parse_dev_key(struct mesh_db_node *db_node,
							struct l_dbus_message_iter *iter_dev_key)
{
	uint8_t *dev_key;
	uint32_t n;

	l_dbus_message_iter_get_fixed_array(iter_dev_key, &dev_key, &n);

	memcpy(db_node->dev_key, dev_key, 16);
    print_uint8("parse_dev_key",dev_key,16);

	return true;
}

static bool parse_net_keys(struct mesh_db_node *db_node,
							struct l_dbus_message_iter *iter_net_keys)
{
	uint16_t key_idx;
	struct mesh_db_net *db_subnet;

	while (l_dbus_message_iter_next_entry(iter_net_keys, &key_idx)) {
		db_subnet = prov_db_subnet_new(db_node);

		db_subnet->idx = key_idx;
		db_subnet->key.idx = key_idx;
        l_info("%s,key index=%d\n","parse_net_keys",key_idx);
	}

	return true;
}

static bool parse_app_keys(struct mesh_db_node *db_node,
							struct l_dbus_message_iter *iter_app_keys)
{
	uint16_t net_idx;
	uint16_t app_idx;
	struct mesh_db_net *db_subnet;
	struct mesh_db_app_key *db_app_key;

	while (l_dbus_message_iter_next_entry(iter_app_keys, &app_idx, &net_idx)) {
		db_subnet = l_queue_find(db_node->subnets, match_net_key_index, L_UINT_TO_PTR(net_idx));
		if (!db_subnet)
        {
            l_info("%s,no db_subnet",__FUNCTION__);
			return false;
        }
		db_app_key = prov_db_app_key_new(db_subnet);

		db_app_key->net_idx = net_idx;
		db_app_key->app_idx = app_idx;
        l_info("net_idx:%d,app_idx:%d\n",net_idx,app_idx);
	}

	return true;
}

static bool parse_model_properties(struct mesh_db_model *db_model,
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
			l_queue_destroy(db_model->bindings, NULL);
			db_model->bindings = bindings;
		} else if (!strcmp(key, "PublicationPeriod")) {
			l_dbus_message_iter_get_variant(&variant, "u", &period);
			/* TODO */
		}
	}

	return true;
}

static bool parse_models(struct mesh_db_element *db_element,
						struct l_dbus_message_iter *iter_models)
{
	struct l_dbus_message_iter iter_properties;
	struct mesh_db_model *db_model;
	uint16_t model_id;

	while (l_dbus_message_iter_next_entry(iter_models, &model_id,
										&iter_properties)) {
		db_model = prov_db_model_new(db_element);

		db_model->ele_idx = db_element->idx;
		db_model->id = model_id;
		if (!parse_model_properties(db_model, &iter_properties))
			return false;
	}

	return true;
}

static bool parse_elements(struct mesh_db_node *db_node,
						struct l_dbus_message_iter *iter_elements)
{
	struct l_dbus_message_iter iter_models;
	struct mesh_db_element *db_element;
	uint8_t ele_idx;

	while (l_dbus_message_iter_next_entry(iter_elements, &ele_idx, &iter_models)) {
		db_element = prov_db_element_new(db_node);

		db_element->idx = ele_idx;

		if (!parse_models(db_element, &iter_models))
			return false;
	}

	return true;
}

/*
 * callback for GetProvDb()
 */
void get_prov_db_call_reply(struct l_dbus_message *reply, void *user_data)
{
	struct l_dbus_message_iter iter_nodes, iter_dev_key, iter_uuid,
							iter_net_keys, iter_app_keys,
							iter_elements;
	struct mesh_db_node *db_node;
	uint8_t ttl;
	uint16_t unicast;
    uint64_t token;
	int err = 0;
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    struct mesh_application *app = mesh_application_get_instance();
    AW_MESH_EVENT_T mesh_event;

    if(event_cb)
    {
        // event callback to customer api
        mesh_event.evt_code = AW_MESH_EVENT_STACK_INIT_DONE;
        event_cb(&mesh_event,NULL);
    }

	err = dbus_get_reply_error(reply);
	if (err != MESH_ERROR_NONE) {
        LOG_PRINTF(AW_DBG_ERR_LEVEL,"Failed To Get DB");
		return;
	}

	if (!l_dbus_message_get_arguments(reply, "a(yqtayayaqa(qq)a(ya(qa{sv})))", &iter_nodes))
    {
        LOG_PRINTF(AW_DBG_VERB_LEVE,"Get DB Not Exist\n");
		return;
    }
    else
    {
        LOG_PRINTF(AW_DBG_VERB_LEVE,"Get DB Exist");
    }

	while (l_dbus_message_iter_next_entry(&iter_nodes, &ttl, &unicast,&token,&iter_uuid,&iter_dev_key,
									&iter_net_keys, &iter_app_keys, &iter_elements)) {

		db_node = prov_db_node_new();

		db_node->ttl = ttl;
		db_node->unicast = unicast;
        db_node->token = l_get_be64((uint8_t*)(&token));
        if(unicast == 0x01)
        {
            db_node->role = 0x01;
            print_uint8("node provisioner and token",(uint8_t*)&token,sizeof(token));
        }
        else
        {
            db_node->role = 0x00;
            print_uint8("node provisionee and token",(uint8_t*)&token,sizeof(token));
        }

        if (!parse_uuid(db_node, &iter_uuid))
            return;

        if(app)
        {
            if(!memcmp(app->uuid,db_node->uuid,16))
                app->db_node = db_node;
        }

		print_packet("uuid ", db_node->uuid, 16);
		if (!parse_dev_key(db_node, &iter_dev_key))
			return;

		if (!parse_net_keys(db_node, &iter_net_keys))
			return;

		if (!parse_app_keys(db_node, &iter_app_keys))
			return;

		if (!parse_elements(db_node, &iter_elements))
			return;
	}
}


void dbus_get_prov_db_call(void)
{
	struct l_dbus *dbus;
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;

	dbus = app_dbus_get_bus();

	l_info("dbus_get_prov_db_call\n");
	msg = l_dbus_message_new_method_call(dbus, BLUEZ_MESH_NAME,
				BLUEZ_MESH_PATH, MESH_NETWORK_INTERFACE,
				"GetProvDb");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_dbus_send_with_reply(dbus, msg, get_prov_db_call_reply, NULL, NULL);
}

static void prov_db_show_app_key(void *a, void *b)
{
	struct mesh_db_app_key *db_app_key = a;

	l_info("      index(0x%04x)",
								db_app_key->app_idx);
}

static void prov_db_show_subnet(void *a, void *b)
{
	struct mesh_db_net *db_net = a;

	l_info("    index(0x%04x)\n"
			"    boundAppKeys:",
								db_net->idx);

	l_queue_foreach(db_net->app_keys, prov_db_show_app_key, NULL);
}

static void prov_db_show_model_binding(void *a, void *b)
{
	uint32_t idx = L_PTR_TO_UINT(a);

	l_info("        0x%04x", idx);
}

static void prov_db_show_model(void *a, void *b)
{
	struct mesh_db_model *db_model = a;

	l_info("      index(0x%04x)\n"
			"      bindings:",
								db_model->id);

	l_queue_foreach(db_model->bindings, prov_db_show_model_binding, NULL);
}

static void prov_db_show_element(void *a, void *b)
{
	struct mesh_db_element *db_element = a;

	l_info("    index(0x%02x)\n"
			"    models:",
								db_element->idx);

	l_queue_foreach(db_element->models, prov_db_show_model, NULL);
}

void mesh_db_show_node(void *a, void *b)
{
	struct mesh_db_node *db_node = a;

	l_info("Node(0x%04x):\n"
			"  defaultTTL: %u",
								db_node->unicast,
								db_node->ttl);

	l_info("  subnets:");
	l_queue_foreach(db_node->subnets, prov_db_show_subnet, NULL);

	l_info("  elements:");
	l_queue_foreach(db_node->elements, prov_db_show_element, NULL);

	l_info("");
}

bool mesh_db_prov_db_init(void)
{
	struct mesh_db_prov *prov_db;

	if (g_prov_db) {
		l_error("the provision database already exists");
		return false;
	}

	prov_db = l_new(struct mesh_db_prov, 1);

	g_prov_db = prov_db;

	return true;
}

void mesh_db_prov_db_free(void)
{
	//struct mesh_db_prov *prov_db = g_prov_db;

	if (!g_prov_db)
		return;

	/* TODO */
}

struct mesh_db_prov *mesh_db_prov_db_get(void)
{
	return g_prov_db;
}

static bool match_device_uuid(const void *a, const void *b)
{
	const struct mesh_db_node *db_node = a;
	const uint8_t *uuid = b;
	print_packet("db_node->uuid ", db_node->uuid, 16);
	print_packet("uuid ", uuid, 16);
	return (memcmp(db_node->uuid, uuid, 16) == 0);
}

/*
static bool match_role(const void *a, const void *b)
{
    const struct  mesh_db_node *db_node = a;
	uint32_t role = L_PTR_TO_UINT(b);
    return (db_node->role == role);
}
*/
void* mesh_db_find_node(uint8_t *uuid)
{
    //struct mesh_db_node *db_node;

    if(g_prov_db == NULL)
        return NULL;

    return l_queue_find(g_prov_db->nodes, match_device_uuid, uuid);
}
//Internal API
int32_t mesh_db_fetch(struct mesh_application *app)
{
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
				BLUEZ_MESH_PATH, MESH_NETWORK_INTERFACE,
				"GetProvDb");
    builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send_with_reply(app->dbus, msg, get_prov_db_call_reply, NULL, NULL);
    return AW_ERROR_NONE;
}

//Public API
void aw_mesh_show_prov_db(void)
{
	struct mesh_db_prov *prov_db = mesh_db_prov_db_get();

	l_queue_foreach(prov_db->nodes, mesh_db_show_node, NULL);
}
