#ifndef __AW_MESH_INTERNAL_API_H__
#define __AW_MESH_INTERNAL_API_H__
#include "AWTypes.h"
#include "mesh_internal.h"
//Internal mesh arch iface
void mesh_stack_reply(AW_MESH_APP_REQUEST_TYPE_T req_type,const char *readme,int errcode);
void mesh_stack_response(AW_MESH_APP_REQUEST_TYPE_T req_type,int errcode,const char *fmt,...);

//Internal mesh database iface
int32_t mesh_db_fetch(struct mesh_application *app);
void dbus_get_prov_db_call(void);
bool mesh_db_prov_db_init(void);

//Internal Element iface
bool element_model_binding(struct node_element *element, struct mesh_model *model);
struct node_element *mesh_element_init(char *path, uint16_t location, uint8_t idx);
struct mesh_model *app_mesh_model_init(uint32_t model_id);
struct node_element *mesh_element_find_by_idx(uint8_t idx);
struct node_element *mesh_element_find_by_path(char *path);
struct mesh_model *mesh_model_find_by_id(struct node_element *element, uint32_t id);
bool mesh_element_dbus_iter(struct l_dbus_message_iter *iter_elements);
struct mesh_model *mesh_model_find_by_idx(struct node_element *element, uint32_t id);

//Internal Agent iface
void mesh_agent_free(void *data);
struct mesh_agent *mesh_agent_get_instance(void);

//Internal application iface
AwMeshEventCb_t mesh_application_get_event_cb(void);
bool mesh_application_element_binding(struct mesh_application *mesh_app,struct node_element *element);
bool mesh_application_attach(uint64_t *token);

//Internal Provisionee iface
uint32_t mesh_provisionee_join_network(struct mesh_application *app, uint8_t *uuid);
//Internal  Provisioner iface
struct mesh_application *mesh_application_get_instance(void);
uint32_t mesh_provisioner_create_network(struct mesh_application *app, uint8_t *uuid);
//Internal Mesh Node iface
int32_t aw_mesh_model_publish(int32_t mod_id, int32_t dst_addr_type, uint8_t ele_idx,
			int32_t ttl, int32_t net_key_idx, int32_t app_key_idx,
			uint8_t *data, int32_t data_len);
int32_t aw_mesh_vendor_model_publish(int32_t mod_id, int32_t dst_addr_type, uint8_t ele_idx,
			int32_t ttl, int32_t net_key_idx, int32_t app_key_idx,
			uint8_t *data, int32_t data_len);
//Internal  Dbus iface
struct l_dbus_message * dbus_config_message_cb(struct l_dbus *dbus, struct l_dbus_message *message,
				 void *user_data);
struct l_dbus_message *dbus_generic_message_cb(struct l_dbus *dbus, struct l_dbus_message *message,
			void *user_data);
struct l_dbus_message *dbus_heartbeat_message_cb(struct l_dbus *dbus, struct l_dbus_message *message,
			void *user_data);
struct l_dbus_message *dbus_unprov_device_cb(struct l_dbus *dbus, struct l_dbus_message *message, void *user_data);
struct l_dbus_message *dbus_friendship_state_cb(struct l_dbus *dbus, struct l_dbus_message *message, void *user_data);
struct l_dbus_message *dbus_iv_update_cb(struct l_dbus *dbus,struct l_dbus_message *message,void *user_data);
struct l_dbus_message *dbus_adv_packet_cb(struct l_dbus *dbus,struct l_dbus_message *message,void *user_data);
struct l_dbus_message *join_failed_call(struct l_dbus *dbus, struct l_dbus_message *msg,void *user_data);
struct l_dbus_message *join_complete_call(struct l_dbus *dbus,struct l_dbus_message *msg,void *user_data);

//Internal mesh Init
int32_t mesh_agent_init_dbus(struct l_dbus *dbus);
int32_t mesh_element_init_dbus(struct l_dbus *dbus);
int32_t mesh_provisioner_init_dbus(struct mesh_application *app);
int32_t mesh_application_init_dbus(struct mesh_application *app);
int32_t mesh_application_init(struct l_dbus *dbus,AwMeshEventCb_t cb);
struct node_element *mesh_element_init(char *path, uint16_t location, uint8_t idx);
struct mesh_model *app_mesh_model_init(uint32_t model_id);
char *build_dbus_path(const char* prefix, uint8_t num);


//Internal mesh Deinit
void mesh_application_free(void);
void mesh_element_free(void *data);

#endif

//bluez mesh stack Api
void print_packet(const char *label, const void *data, uint16_t size);
bool mesh_model_opcode_get(const uint8_t *buf, uint16_t size,
					uint32_t *opcode, uint16_t *n);
uint16_t mesh_model_opcode_set(uint32_t opcode, uint8_t *buf);

//ell Api
void l_dbus_new_mutex(struct l_dbus *dbus);

//debug Api and model Api
void mesh_test_log(const char *fmt,...);
void pts_app_run();
void goo_onoff(uint16_t dst,uint16_t appkey_idx,uint16_t on_off);
