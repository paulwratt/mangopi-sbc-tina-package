#ifndef __AW_MESH_INTERNAL_H__
#define __AW_MESH_INTERNAL_H__

#include <ell/ell.h>
#include "AWTypes.h"

#define MESH_STORAGEDIR "/data/aw/bluetooth/mesh"
#define MESH_LOCAL_NODE_PATH (MESH_STORAGEDIR"/local_node.json")
#define MESH_PROV_DB_PATH (MESH_STORAGEDIR"/prov_db.json")
#define BLUEZ_MESH_PATH "/org/bluez/mesh"
#define BLUEZ_MESH_NAME "org.bluez.mesh"

#define MESH_MANAGEMENT_INTERFACE "org.bluez.mesh.Management1"
#define MESH_NETWORK_INTERFACE "org.bluez.mesh.Network1"
#define MESH_ADAPTER_INTERFACE "org.bluez.mesh.Adapter"
#define MESH_NODE_INTERFACE "org.bluez.mesh.Node1"
#define MESH_ELEMENT_INTERFACE "org.bluez.mesh.Element1"
#define MESH_APPLICATION_INTERFACE "org.bluez.mesh.Application1"
#define MESH_PROVISION_AGENT_INTERFACE "org.bluez.mesh.ProvisionAgent1"
#define MESH_PROVISIONER_INTERFACE "org.bluez.mesh.Provisioner1"
#define ERROR_INTERFACE "org.bluez.mesh.Error"

#define BLUEZ_MESH_ADAPTER_NAME "org.mesh.adapter"
#define BLUEZ_MESH_ADAPTER_PATH "/adapter"

#define MESH_APPLICATION_PATH_PREFIX "/application"
#define MESH_ELEMENT_PATH_PREFIX (MESH_APPLICATION_PATH_PREFIX"/ele")
#define MESH_AGENT_PATH_PREFIX (MESH_APPLICATION_PATH_PREFIX"/agent")
#define MESH_PATH_INDEX_LEN 2

/* Default element location: unknown */
#define ELEMENT_DEFAULT_LOCATION 0x0000

#define MESH_CAPS_MAX 15
#define MESH_OOB_INFO_MAX 15
#define MESH_APP_KEY_SIZE 16
#define MESH_SCAN_TIMEOUT 60
/* generic mesh model message handler */
typedef bool (*model_process_msg_func_t)(uint16_t src, uint16_t key_idx, bool is_sub,
									uint8_t *data, uint32_t data_len);

struct mesh_agent_prov_caps {
	uint32_t uri_hash;
	uint16_t oob_info;
	uint16_t output_action;
	uint16_t input_action;
	uint8_t pub_type;
	uint8_t static_type;
	uint8_t output_size;
	uint8_t input_size;
};

struct mesh_agent {
	char *path;
	char *owner;
	char *caps[MESH_CAPS_MAX];
	char *oob[MESH_OOB_INFO_MAX];
	char *uri;
};

struct mesh_application {
	char *path;
	char *node_path;
	struct l_queue *elements;
    uint8_t element_cnt;
	uint16_t company_id;
	uint16_t product_id;
	uint16_t version_id;
    uint32_t feature;
    uint16_t primary_addr;
    uint8_t uuid[16];
    uint16_t prov_net_idx;
    uint16_t prov_unicast;
    AwMeshEventCb_t mesh_event_cb_handle;
    AW_MESH_ROLE_T provisioner;
    AW_MESH_REQ_T req;
    struct mesh_db_node *db_node;
    struct l_dbus *dbus;
    struct l_timeout *scan_timeout;
    struct l_timeout *prov_timeout;
};

struct node_element {
	char *path;
	struct l_queue *models;
	uint16_t location;
	uint8_t idx;
};

/* These struct is used to pass lots of params to l_queue_foreach */
struct mod_forward {
	uint8_t *data;
	uint32_t data_len;
	uint16_t src;
	uint16_t appkey_idx;
};

struct mesh_model {
	struct l_queue *bindings;
	uint32_t id;
	uint8_t ele_idx;
	uint8_t mod_idx;
    void *data;
    //AwMeshAccessRxCb access_rx_cb;
};

typedef enum {
	MESH_SCAN_ENABLE,
	MESH_SCAN_DISABLE,
	MESH_SET_SCAN_PARA,
	MESH_SET_ADV_PARA,
	MESH_ADD_NODE,
	MESH_ADD_NODE_CANCEL,
	MESH_SCAN_NODE_START,
	MESH_SCAN_NODE_STOP,
	MESH_KEY_REFRESH_SET,
	MESH_TYPE_MAX
} mesh_request_type_t;

struct mesh_request {
	mesh_request_type_t type;
	void *cb;
};

#endif
