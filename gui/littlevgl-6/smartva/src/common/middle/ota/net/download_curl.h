#ifndef __DOWNLOAD_CURL_H__
#define __DOWNLOAD_CURL_H__
typedef enum {
	CHANNEL_OK,
	CHANNEL_EINIT,
	CHANNEL_ENONET,
	CHANNEL_ENOMEM,
	CHANNEL_EACCES,
	CHANNEL_ENOENT,
	CHANNEL_EIO,
	CHANNEL_EILSEQ,
	CHANNEL_EAGAIN,
	CHANNEL_ELOOP,
	CHANNEL_EBADMSG,
	CHANNEL_ENOTFOUND,
	CHANNEL_EREDIRECT
} channel_op_res_t;

typedef enum {
	SOURCE_UNKNOWN,
	SOURCE_WEBSERVER,
	SOURCE_SURICATTA,
	SOURCE_DOWNLOADER,
	SOURCE_LOCAL
} sourcetype;


typedef enum {
	CHANNEL_PARSE_JSON,
	CHANNEL_PARSE_RAW
} channel_body_t;

typedef struct {
	char *url;
	char *dest;
	char *auth;
	char *request_body;
#ifdef CONFIG_JSON
	json_object *json_reply;
#endif
	char *cafile;
	char *sslkey;
	char *sslcert;
	char *ciphers;
	char *proxy;
	char *info;
	char *header;
	const char *content_type;
	unsigned int retry_sleep;
	unsigned int offs;
	unsigned int method;
	unsigned int retries;
	unsigned int low_speed_timeout;
	channel_body_t format;
	bool debug;
	bool strictssl;
	bool nocheckanswer;
	long http_response_code;
	bool nofollow;
	int (*checkdwl)(void);
	sourcetype source;
	struct dict *headers;
} channel_data_t;

typedef struct channel channel_t;
struct channel {
	channel_op_res_t (*open)(channel_t *this, void *cfg);
	channel_op_res_t (*close)(channel_t *this);
	channel_op_res_t (*get)(channel_t *this, void *data);
	channel_op_res_t (*get_file)(channel_t *this, void *data);
	channel_op_res_t (*put)(channel_t *this, void *data);
	char *(*get_redirect_url)(channel_t *this);
	void *priv;
};
#define SUCCESS (0)
#define FAILURE (-1)
channel_t *channel_new(void);
#endif
