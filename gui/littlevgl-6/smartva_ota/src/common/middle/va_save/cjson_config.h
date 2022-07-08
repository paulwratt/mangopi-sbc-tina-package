#include "cJSON.h"
typedef struct cjson_config_tag {
	__u8  valid;
	pthread_mutex_t param_mutex;
	char *config_path;
	char *config_contents;
	unsigned int config_size;
} cjson_config_t;
int init_cjson_config(cjson_config_t *config);
int uninit_cjson_config(cjson_config_t *config);
int read_int_type_cjson_config(cjson_config_t *config, char *major, char *minor, int *param);
int write_int_type_cjson_config(cjson_config_t *config, char *major, char *minor, int param);
int read_string_type_cjson_config(cjson_config_t *config, char *major, char *minor, char *str, int len);
int write_string_type_cjson_config(cjson_config_t *config, char *major, char *minor, char *str, int len);
