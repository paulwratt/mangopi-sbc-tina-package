struct mesh_db_app_key {
	uint16_t net_idx;
	uint16_t app_idx;
	uint8_t key[16];
};

struct mesh_db_net_key {
	uint16_t idx;
	uint8_t key[16];
	uint8_t key_refresh;
};

struct mesh_db_net {
	uint16_t idx;
	struct mesh_db_net_key key;
	struct l_queue *app_keys;
};

struct mesh_db_node_comp {
	uint16_t cid;
	uint16_t pid;
	uint16_t vid;
	uint16_t crpl;

};

struct mesh_db_model {
	uint32_t id;
	uint8_t ele_idx;
	struct l_queue *bindings;
};

struct mesh_db_element {
	uint8_t idx;
	uint16_t location;
	struct l_queue *models;
};

struct mesh_db_node {
	struct mesh_db_node_comp *comp;
	uint8_t dev_uuid[16];
	struct {
		uint16_t interval;
		uint8_t cnt;
		uint8_t mode;
	} relay;
	uint8_t lpn;
	uint8_t friend;
	uint8_t proxy;
	uint8_t beacon;
	uint8_t ttl;
    uint8_t role;
    uint8_t attached;
    uint64_t token;
	struct l_queue *elements;
	uint8_t num_ele;
	uint32_t iv_index;
	bool iv_update;
	uint16_t unicast;
	uint8_t dev_key[16];
    uint8_t uuid[16];
	uint32_t seq_num;
	struct l_queue *subnets;
};

struct mesh_db_prov {
	struct l_queue *nodes;
};

void mesh_db_prov_db_free(void);
struct mesh_db_prov *mesh_db_prov_db_get(void);
void mesh_db_show_node(void *a, void *b);

void* mesh_db_find_node(uint8_t *uuid);
