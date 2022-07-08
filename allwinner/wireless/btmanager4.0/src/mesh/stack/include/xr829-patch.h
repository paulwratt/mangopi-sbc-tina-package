
#ifndef __XR829_PATCH_H__
#define __XR829_PATCH_H__
#define CONFIG_XR829_BT

//resolve uint32_t uint16_t
#include <stdint.h>
//resolve size_t
#include <stddef.h>
//resolve bool
#include <stdbool.h>
#include "mesh/util.h"

//debug configure as follows
#define ENABLE_ENHANCE_DEBUG_LOG    1

//#include <ell/ell.h>

/*TRACE DEBUG LOG CONFIGURE*/
typedef struct
{
    uint16_t opcode;
    uint16_t netkey_idx;
    uint16_t appkey_idx;
    uint16_t element_addr;
    uint16_t key_phase;
    uint32_t model_idx;
    uint8_t key[16];
}lcl_key_mgr_t;

//node feature bit
#define AW_MESH_FEATURE_NONE     0x00    /**< A bit field indicating no feature. */
#define AW_MESH_FEATURE_RELAY    0x01    /**< A bit field indicating feature relay. */
#define AW_MESH_FEATURE_PROXY    0x02    /**< A bit field indicating feature proxy. */
#define AW_MESH_FEATURE_FRIEND   0x04    /**< A bit field indicating feature friend. */
#define AW_MESH_FEATURE_LPN      0x08    /**< A bit field indicating feature low power node. */
#define AW_MESH_FEATURE_SNB      0x10

#define MESH_MAX_ACCESS_PAYLOAD		380

//define debug mask classified by mesh netowrking layer
#define TRC_COMMON      (0x00000001)
#define TRC_MODEL       (0x00000002)
#define TRC_ACCESS      (0x00000004)
#define TRC_UTNPT       (0x00000008)
#define TRC_LTNPT       (0x00000010)
#define TRC_NET         (0x00000020)
#define TRC_HCI         (0x00000040)

//define debug mask by Prov phase
#define TRC_PROV        (0x00000080)

//#define debug mask by mesh function
#define TRC_MGR         (0x00000100)
#define TRC_JSON        (0x00000200)

//define debug mask for urgency event
#define TRC_URG         (0x80000000)

//set debug mask
#define XR829_TRACE_MASK    (0  \
                            | TRC_COMMON    \
                            | TRC_MODEL     \
                            | TRC_ACCESS    \
                            | TRC_UTNPT     \
                            | TRC_LTNPT     \
                            | TRC_NET       \
                            | TRC_HCI       \
                            | TRC_PROV      \
                            | TRC_MGR       \
                            | TRC_URG  \
                            )
//define debug level
#ifdef CONFIG_XR829_BT
#define xr_error(TRC_L,format, ...)     {if(TRC_L&XR829_TRACE_MASK) l_log(L_LOG_ERR, "XR_ERR\t"format, ##__VA_ARGS__);}
#define xr_warn(TRC_L,format, ...)      {if(TRC_L&XR829_TRACE_MASK) l_log(L_LOG_WARNING, "XR_WARN\t"format, ##__VA_ARGS__);}
#define xr_info(TRC_L,format, ...)      {if(TRC_L&XR829_TRACE_MASK) l_log(L_LOG_INFO, "XR_INFO\t"format, ##__VA_ARGS__);}
#define xr_debug(TRC_L,format, ...)     {if(TRC_L&XR829_TRACE_MASK) l_log(L_LOG_DEBUG, "XR_DBG\t"format, ##__VA_ARGS__);}
//void print_packet(const char *label, const void *data, uint16_t size)
#define xr_print_packet(label,data,size)    print_packet(label,data,size)
/*END OF TRACE DEBUG LOG CONFIGURE*/
#else
#define xr_error(TRC_L,format, ...)
#define xr_warn(TRC_L,format, ...)
#define xr_info(TRC_L,format, ...)
#define xr_debug(TRC_L,format, ...)
#define xr_print_packet(label,data,size)
#define xr_goto_fail() { goto fail; }
#endif

void mesh_test_log(const char *fmt,...);

#ifdef CONFIG_XR829_BT
    #if (ENABLE_ENHANCE_DEBUG_LOG == 1)
#define xr_goto_fail()    {   \
            xr_error(TRC_URG,"%s:%d\n",__func__,__LINE__);  \
            goto fail;  \
        }
    #else
#define xr_goto_fail() { goto fail; }
    #endif
#else
#define xr_goto_fail() { goto fail; }
#endif

#ifdef CONFIG_XR829_BT
//dbus use old AIP
#define XR_DBUS_MGR_ENABLE(Dbus)    l_dbus_object_manager_enable(Dbus)
#else
#define XR_DBUS_MGR_ENABLE(Dbus)    l_dbus_object_manager_enable(Dbus,"/")
#endif

#ifdef CONFIG_XR829_BT
//fix warning
struct mesh_net;
struct mesh_node;
struct node_import;
struct l_dbus_message_builder;
#endif

#ifdef CONFIG_XR829_BT
//workaround for bluez5.52 to bluez5.54 upgrade
#define XR_GET_SIG_MODELS_FROM_PROP(ele ,property) xr_get_sig_models_from_properties(ele,property)
#define XR_GET_VENDOR_MODELS_FROM_PROP(ele ,property) xr_get_vendor_models_from_properties(ele,property)
#else
#define XR_GET_SIG_MODELS_FROM_PROP(ele ,property) get_sig_models_from_properties(ele,property)
#define XR_GET_VENDOR_MODELS_FROM_PROP(ele ,property) get_vendor_models_from_properties(ele,property)
#endif

#ifdef CONFIG_XR829_BT
//extern api
extern void update_iv_ivu_state(struct mesh_net *net, uint32_t iv_index,
								bool ivu);
extern int appkey_key_add(struct mesh_net *net, uint16_t net_idx, uint16_t app_idx,
							const uint8_t *new_key);
//add api
bool xr_mesh_model_rx(struct mesh_node *node, bool szmict, uint32_t seq0,
			uint32_t seq, uint32_t iv_index, uint8_t ttl,
			uint16_t net_idx, uint16_t src, uint16_t dst,
			uint8_t key_aid, const uint8_t *data, uint16_t size);
void update_iv_to_app(struct mesh_node *node, uint32_t iv_index, uint8_t flags);
struct l_queue *node_get_all_nodes(void);
void node_build_config(void *a, void *b);
void node_print_composition(struct mesh_node *node);
void appkey_append_key_indexes(void *a, void *b);
void mesh_net_build_net_key_indexes(struct mesh_net *net,
							struct l_dbus_message_builder *builder);
void mesh_net_build_app_key_indexes(struct mesh_net *net,
							struct l_dbus_message_builder *builder);
int lcl_key_ctl(struct mesh_node *node,lcl_key_mgr_t *p_key_mgr);
struct mesh_node * xr_get_scan_node();
char * getstr_hex2str(uint8_t *in, size_t in_len);
#define XR_MESH_MODEL_RX_PATCH(NODE,SIMICT,SEQ0,SEQ,IV_INDEX,TTL,NET_IDX,SRC,DST,KEY_AID,DATA,SIZE) \
            xr_mesh_model_rx(NODE,SIMICT,SEQ0,SEQ,IV_INDEX,TTL,NET_IDX,SRC,DST,KEY_AID,DATA,SIZE)

#define XR_CREATE_APP_CALL_PATCH(NODE,NET_IDX,APP_IDX,KEY)   {   \
    struct mesh_net *net = node_get_net(NODE);  \
    l_info("create app key: net_idx %d,app_idx %d,key=%s",NET_IDX,APP_IDX,getstr_hex2str(KEY,16));  \
    appkey_key_add(net, NET_IDX, APP_IDX, KEY); \
    }
#define XR_IMPORT_CALL_PATCH(UUID)
#define XR_NET_IV_INDEX_UPDATE_PATCH(NODE,IV_IDX,FLAG)    update_iv_to_app(net->node, net->iv_index, FLAG)
#define XR_MESH_CONFIG_CREATE_PATCH(name_buf)   {   \
            char sys_buf[128];  \
            sprintf(sys_buf,"rm -rf %s",name_buf);  \
            system(sys_buf);\
        }
#define XR_CFG_SEND_HCI_CMD_PATCH(HCI,OPCODE,DATA,SIZE,CALLBACK,USER_DATA,DESTRORY)
#define XR_HCI_INIT_PATCH(HCI,IDX)  {   \
            io->pvt->hci = bluez554_bt_hci_new_raw_device(io->pvt->index);   \
        }
#define XR_DBUS_DEV_KEY_SEND_INFACE_PATCH() l_dbus_interface_method(iface, "DevKeySend", 0, dev_key_send_call, "", "oqqay",\
					"element_path", "destination", \
					"net_index", "data")
#define XR_ADD_INTERNAL_MODEL_PATCH(NODE)   {   \
            add_internal_model(NODE, CONFIG_SRV_MODEL, PRIMARY_ELE_IDX);\
            add_internal_model(NODE, CONFIG_CLI_MODEL, PRIMARY_ELE_IDX);\
        }

#define XR_CFGMOD_INIT_PATCH(NODE)  {   \
            cfgmod_server_init(NODE, PRIMARY_ELE_IDX);  \
            cfgmod_cli_init(NODE, PRIMARY_ELE_IDX); \
        }

#define XR_GEN_NETKEY_PATCH(KEY)    {   \
            memset(KEY, 0x11, 16);  \
        }
#define XR_INITIATOR_FORCE_TO_NOOB_PATH(PROV)   {   \
            PROV->conf_inputs.start.auth_method = 0x00; \
            PROV->conf_inputs.start.auth_action = 0x00; \
            PROV->conf_inputs.start.auth_size = 0x00;   \
        }
#define XR_COMPATIBLE_JSON_V1_2_STR_DEL_PATCH(JARRAY,STR,LEN,JMODEL,OJB_NAME) \
            jarray_string_del(JARRAY, STR, LEN,JMODEL,OJB_NAME)
#define XR_COMPATIBLE_JSON_V1_2_KEY_DEL_PATCH(JARRAY,IDX,JNODE,OBJ_NAME)\
            jarray_key_del(JARRAY, IDX, JNODE, OBJ_NAME);
#else
#define XR_MESH_MODEL_RX_PATCH(NODE,SIMICT,SEQ0,SEQ,IV_INDEX,TTL,NET_IDX,SRC,DST,KEY_AID,DATA,SIZE) \
            mesh_model_rx(NODE,SIMICT,SEQ0,SEQ,IV_INDEX,NET_IDX,SRC,DST,KEY_AID,DATA,SIZE)
#define appkey_append_key_indexes(a,b);
#define getstr_hex2str(in,in_len)   ("")
#define XR_CREATE_APP_CALL_PATCH(NODE,NET_IDX,APP_IDX,KEY)
#define XR_IMPORT_CALL_PATCH(UUID)  {   \
            if (node_find_by_uuid(UUID))    \
                return dbus_error_0(msg, MESH_ERROR_ALREADY_EXISTS,   \
                        "Node already exists"); \
    }
#define XR_NET_IV_INDEX_UPDATE_PATCH(NODE,IV_IDX,FLAG)
#define XR_CFG_SEND_HCI_CMD_PATCH(HCI,OPCODE,DATA,SIZE,CALLBACK,USER_DATA,DESTRORY)  \
    bluez554_bt_hci_send(HCI,OPCODE,DATA,SIZE,CALLBACK,USER_DATA,DESTRORY)
#define XR_HCI_INIT_PATCH(HCI,IDX)  {   \
                HCI = bluez554_bt_hci_new_user_channel(IDX);   \
            }
#define XR_DBUS_DEV_KEY_SEND_INFACE_PATCH() l_dbus_interface_method(iface, "AddNetKey", 0, add_netkey_call, "",\
					"oqqqb", "element_path", "destination", \
					"subnet_index", "net_index", "update")
#define XR_ADD_INTERNAL_MODEL_PATCH(NODE)   {   \
                add_internal_model(NODE, CONFIG_SRV_MODEL, PRIMARY_ELE_IDX);\
            }
#define XR_CFGMOD_INIT_PATCH(NODE)  {   \
            cfgmod_server_init(NODE, PRIMARY_ELE_IDX);  \
        }
#define XR_GEN_NETKEY_PATCH(KEY)    {   \
            l_getrandom(KEY, sizeof(net_key.old_key));  \
        }
#define XR_INITIATOR_FORCE_TO_NOOB_PATH(PROV)
#define XR_MESH_CONFIG_CREATE_PATCH(name_buf)
#define XR_COMPATIBLE_JSON_V1_2_STR_DEL_PATCH(JARRAY,STR,LEN,JMODEL,OJB_NAME) { \
            jarray_string_del(JARRAY, STR, LEN) \
		if (!json_object_array_length(JARRAY))  \
                    json_object_object_del(JMODEL, OJB_NAME); \
        }

#define XR_COMPATIBLE_JSON_V1_2_KEY_DEL_PATCH(JARRAY,IDX,JNODE,OBJ_NAME)    { \
            jarray_key_del(JARRAY, IDX);    \
            if (!json_object_array_length(JARRAY))  \
                json_object_object_del(JNODE, OBJ_NAME);   \
        }
#endif

#endif//endof __XR829_PATCH_H__
