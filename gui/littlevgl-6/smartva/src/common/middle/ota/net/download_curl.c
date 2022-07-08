#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include <curl/curl.h>
#include "download_curl.h"
#include "smt_config.h"

/*this transplant from swupdate corelib channel_curl.c if you want complete this£¬you can reference that*/
#define SPEED_LOW_TIME_SEC (300)
#define SPEED_LOW_BYTES_SEC (8)
#define KEEPALIVE_DELAY (204L)
#define KEEPALIVE_INTERVAL (120L)

typedef enum {
	CHANNEL_GET,
	CHANNEL_POST,
	CHANNEL_PUT,
} channel_method_t;
#define USE_PROXY_ENV (char *)0x11
typedef struct {
	char *memory;
	size_t size;
} output_data_t;

typedef struct {
	char *proxy;
	char *effective_url;
	char *redirect_url;
	CURL *handle;
	struct curl_slist *header;
} channel_curl_t;

typedef struct {
	channel_data_t *channel_data;
	int output;
	output_data_t *outdata;
} write_callback_t;

static int channel_callback_xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow,
				     curl_off_t __attribute__((__unused__)) ultotal,
				     curl_off_t __attribute__((__unused__)) ulnow);
channel_op_res_t channel_curl_init(void)
{
#if defined(CONFIG_SURICATTA_SSL) || defined(CONFIG_CHANNEL_CURL_SSL)
#define CURL_FLAGS CURL_GLOBAL_SSL
#else
#define CURL_FLAGS CURL_GLOBAL_NOTHING
#endif
	CURLcode curlrc = curl_global_init(CURL_FLAGS);
	if (curlrc != CURLE_OK) {
		com_err("Initialization of channel failed (%d): '%s'", curlrc, curl_easy_strerror(curlrc));
		return CHANNEL_EINIT;
	}
#undef CURL_FLAGS
	return CHANNEL_OK;
}

channel_op_res_t channel_open(channel_t *this, void *cfg)
{
	assert(this != NULL);
	channel_curl_t *channel_curl = this->priv;
	assert(channel_curl->handle == NULL);

	channel_data_t *channel_cfg = (channel_data_t *)cfg;

	if ((channel_cfg != NULL) && (channel_cfg->proxy != NULL)) {
		channel_curl->proxy = channel_cfg->proxy == USE_PROXY_ENV ? USE_PROXY_ENV : strdup(channel_cfg->proxy);
		if (channel_curl->proxy == NULL) {
			return CHANNEL_EINIT;
		}
	}

	if ((channel_curl->handle = curl_easy_init()) == NULL) {
		com_err("Initialization of channel failed.");
		return CHANNEL_EINIT;
	}

	return CHANNEL_OK;
}
channel_op_res_t channel_close(channel_t *this)
{
	channel_curl_t *channel_curl = this->priv;

	if ((channel_curl->proxy != NULL) && (channel_curl->proxy != USE_PROXY_ENV)) {
		free(channel_curl->proxy);
	}

	if (channel_curl->redirect_url) {
		free(channel_curl->redirect_url);
	}

	if (channel_curl->handle == NULL) {
		return CHANNEL_OK;
	}

	curl_easy_cleanup(channel_curl->handle);
	channel_curl->handle = NULL;
	return CHANNEL_OK;
}

static channel_op_res_t result_channel_callback_write_file;
size_t channel_callback_write_file(void *streamdata, size_t size, size_t nmemb,
				   write_callback_t *data)
{
	if (!nmemb || !data) {
		return 0;
	}
	result_channel_callback_write_file = CHANNEL_OK;
	write(data->output, streamdata, size * nmemb);
	if (data->channel_data->checkdwl && data->channel_data->checkdwl())
		return 0;
	return size * nmemb;
}

channel_op_res_t channel_map_http_code(channel_t *this, long *http_response_code)
{
	char *url = NULL;
	channel_curl_t *channel_curl = this->priv;
	CURLcode curlrc = curl_easy_getinfo(channel_curl->handle, CURLINFO_RESPONSE_CODE, http_response_code);
	if (curlrc != CURLE_OK && curlrc == CURLE_UNKNOWN_OPTION) {
		com_err("Get channel HTTP response code unsupported by libcURL %s.\n",  LIBCURL_VERSION);
		return CHANNEL_EINIT;
	}
	switch (*http_response_code) {
	case 0:   /* libcURL: no server response code has been received yet */
		com_warn("No HTTP response code has been received yet!");
		return CHANNEL_EBADMSG;
	case 401: /* Unauthorized. The request requires user authentication. */
	case 403: /* Forbidden. */
	case 405: /* Method not Allowed. */
	case 407: /* Proxy Authentication Required */
	case 503: /* Service Unavailable */
		return CHANNEL_EACCES;
	case 400: /* Bad Request, e.g., invalid parameters */
	case 406: /* Not acceptable. Accept header is not response compliant */
	case 443: /* Connection refused */
		return CHANNEL_EBADMSG;
	case 404: /* Wrong URL */
		return CHANNEL_ENOTFOUND;
	case 429: /* Bad Request, i.e., too many requests. Try again later. */
		return CHANNEL_EAGAIN;
	case 200:
	case 206:
	case 226:
		return CHANNEL_OK;
	case 302:
		curlrc = curl_easy_getinfo(channel_curl->handle, CURLINFO_REDIRECT_URL, &url);
		if (curlrc == CURLE_OK) {
			if (channel_curl->redirect_url)
				free(channel_curl->redirect_url);
			channel_curl->redirect_url = strdup(url);
		} else if (curlrc == CURLE_UNKNOWN_OPTION) {
			com_err("channel_curl_getinfo response unsupported by libcURL %s.\n", LIBCURL_VERSION);
		}
		return CHANNEL_EREDIRECT;
	case 500:
		return CHANNEL_EBADMSG;
	default:
		com_err("Channel operation returned unhandled HTTP error code %ld\n", *http_response_code);
		return CHANNEL_EBADMSG;
	}
}
size_t channel_callback_membuffer(void *streamdata, size_t size, size_t nmemb,
				  write_callback_t *data)
{
	if (!nmemb) {
		return 0;
	}
	if (!data) {
		return 0;
	}

	size_t realsize = size * nmemb;
	output_data_t *mem = data->outdata;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		com_err("Channel get operation failed with OOM");
		return 0;
	}
	memcpy(&(mem->memory[mem->size]), streamdata, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}

static channel_op_res_t channel_set_content_type(channel_t *this, channel_data_t *channel_data)
{
	channel_curl_t *channel_curl = this->priv;
	const char *content;
	char *contenttype, *accept;
	assert(channel_curl->handle != NULL);

	channel_op_res_t result = CHANNEL_OK;
	if (channel_data->content_type && strlen(channel_data->content_type))
		content = channel_data->content_type;
	else
		content = "application/json";

	if (-1 == asprintf(&contenttype, "Content-Type: %s", content)) {
		result = CHANNEL_EINIT;
		com_err("OOM when setting Content-type.");
	}

	if (-1 == asprintf(&accept, "Accept: %s", content)) {
		result = CHANNEL_EINIT;
		com_err("OOM when setting Content-type.");
	}

	if (result == CHANNEL_OK) {
		if (((channel_curl->header = curl_slist_append(channel_curl->header, contenttype)) == NULL) ||
			((channel_curl->header = curl_slist_append(channel_curl->header, accept)) == NULL) ||
			((channel_curl->header = curl_slist_append(channel_curl->header, "charsets: utf-8")) == NULL)) {
			com_err("Set channel header failed.");
			result = CHANNEL_EINIT;
		}
	}

	return result;
}
static int channel_callback_xferinfo_legacy(void *p, double dltotal, double dlnow, double ultotal, double ulnow)
{
	return channel_callback_xferinfo(p, (curl_off_t)dltotal, (curl_off_t)dlnow, (curl_off_t)ultotal, (curl_off_t)ulnow);
}
static size_t channel_callback_headers(char *buffer, size_t size, size_t nitems, void *userdata)
{
//	struct dict *dict = (struct dict *)userdata;
	char *info = malloc(size * nitems + 1);
	char *p, *key, *val;

	if (!info) {
		com_err("No memory allocated for headers, headers not collected !!");
		return nitems * size;
	}
	memcpy(info, buffer, size * nitems);
	info[size * nitems] = '\0';
	p = memchr(info, ':', size * nitems);
	if (p) {
		*p = '\0';
		key = info;
		val = p + 1; /* Next char after ':' */
		while(isspace((unsigned char)*val)) val++;
/*dict_insert_value(dict, key, val);*/
		com_err("%s : %s", key, val);
	} else {
		com_info("Header not processed: %s", info);
	}

	free(info);
	return nitems * size;
}

static int channel_callback_xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow,
				     curl_off_t __attribute__((__unused__)) ultotal,
				     curl_off_t __attribute__((__unused__)) ulnow)
{
	if ((dltotal <= 0) || (dlnow > dltotal)) {
		return 0;
	}
	double percent = 100.0 * (dlnow/1024.0) / (dltotal/1024.0);
	double *last_percent = (double*)p;
	if ((int)*last_percent == (int)percent) {
		return 0;
	}
	*last_percent = percent;
	char *info;
	if (asprintf(&info,"{\"percent\": %d, \"msg\":\"Received %" CURL_FORMAT_CURL_OFF_T "B "
					"of %" CURL_FORMAT_CURL_OFF_T "B\"}",(int)percent, dlnow, dltotal) != -1) {
		free(info);
	}
	return 0;
}

channel_op_res_t channel_set_options(channel_t *this, channel_data_t *channel_data, channel_method_t method)
{
	if (channel_data->low_speed_timeout == 0) {
		channel_data->low_speed_timeout = SPEED_LOW_TIME_SEC;
		com_info("cURL's low download speed timeout is disabled, this is most probably not what you want."
														"Adapted it to %us instead.\n", SPEED_LOW_TIME_SEC);
	}
	channel_curl_t *channel_curl = this->priv;
	channel_op_res_t result = CHANNEL_OK;
	if ((curl_easy_setopt(channel_curl->handle, CURLOPT_URL, channel_data->url) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_USERAGENT, "libcurl-agent/1.0") != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_LOW_SPEED_LIMIT, SPEED_LOW_BYTES_SEC) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_LOW_SPEED_TIME, channel_data->low_speed_timeout) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_HTTPHEADER, channel_curl->header) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_MAXREDIRS, -1) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_CAINFO, channel_data->cafile) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_SSLKEY, channel_data->sslkey) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_SSLCERT, channel_data->sslcert) != CURLE_OK)) {
		result = CHANNEL_EINIT;
		goto cleanup;
	}

	if ((!channel_data->nofollow) && (curl_easy_setopt(channel_curl->handle, CURLOPT_FOLLOWLOCATION, 1) != CURLE_OK)) {
		result = CHANNEL_EINIT;
		goto cleanup;
	}

	double percent = -INFINITY;
	if ((curl_easy_setopt(channel_curl->handle, CURLOPT_PROGRESSFUNCTION, channel_callback_xferinfo_legacy) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_PROGRESSDATA, &percent) != CURLE_OK)) {
		result = CHANNEL_EINIT;
		goto cleanup;
	}
#if LIBCURL_VERSION_NUM >= 0x072000
	if ((curl_easy_setopt(channel_curl->handle, CURLOPT_XFERINFOFUNCTION, channel_callback_xferinfo) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_XFERINFODATA, &percent) != CURLE_OK)) {
		result = CHANNEL_EINIT;
		goto cleanup;
	}
#endif
	if (curl_easy_setopt(channel_curl->handle, CURLOPT_NOPROGRESS, 0L) != CURLE_OK) {
		result = CHANNEL_EINIT;
		goto cleanup;
	}

	if (channel_data->headers) {
		if ((curl_easy_setopt(channel_curl->handle, CURLOPT_HEADERFUNCTION, channel_callback_headers) != CURLE_OK) ||
		    (curl_easy_setopt(channel_curl->handle, CURLOPT_HEADERDATA,  channel_data->headers) != CURLE_OK)) {
			result = CHANNEL_EINIT;
			goto cleanup;
		}
	}

	if (channel_data->strictssl == true) {
		if ((curl_easy_setopt(channel_curl->handle, CURLOPT_SSL_VERIFYHOST, 2L) != CURLE_OK) ||
		    (curl_easy_setopt(channel_curl->handle, CURLOPT_SSL_VERIFYPEER, 1L) != CURLE_OK)) {
			result = CHANNEL_EINIT;
			goto cleanup;
		}
	}
	else {
		if ((curl_easy_setopt(channel_curl->handle, CURLOPT_SSL_VERIFYHOST, 0L) != CURLE_OK) ||
		    (curl_easy_setopt(channel_curl->handle, CURLOPT_SSL_VERIFYPEER, 0L) != CURLE_OK)) {
			result = CHANNEL_EINIT;
			goto cleanup;
		}
	}

/*Check if there is a restricted list of ciphers to be used*/
	if (channel_data->ciphers) {
		if (curl_easy_setopt(channel_curl->handle, CURLOPT_SSL_CIPHER_LIST, channel_data->ciphers) != CURLE_OK) {
			result = CHANNEL_EINIT;
			goto cleanup;
		}
	}

	if (channel_data->header != NULL) {
		if (((channel_curl->header = curl_slist_append(channel_curl->header, channel_data->header)) == NULL)) {
			result = CHANNEL_EINIT;
			goto cleanup;
		}
	}

	switch (method) {
	case CHANNEL_GET:
		if (curl_easy_setopt(channel_curl->handle, CURLOPT_CUSTOMREQUEST, "GET") != CURLE_OK) {
			result = CHANNEL_EINIT;
			goto cleanup;
		}
		break;
	case CHANNEL_PUT:
		if ((curl_easy_setopt(channel_curl->handle, CURLOPT_PUT, 1L) != CURLE_OK) ||
		     (curl_easy_setopt(channel_curl->handle, CURLOPT_UPLOAD, 1L) != CURLE_OK)) {
			result = CHANNEL_EINIT;
			goto cleanup;
		}
		break;
	case CHANNEL_POST:
		if ((curl_easy_setopt(channel_curl->handle, CURLOPT_POST, 1L) != CURLE_OK) ||
		    (curl_easy_setopt(channel_curl->handle, CURLOPT_POSTFIELDS, channel_data->request_body) != CURLE_OK)) {
			result = CHANNEL_EINIT;
			goto cleanup;
		}
		if (channel_data->debug) {
			com_info("Posted: %s", channel_data->request_body);
		}
		break;
	}

	if (channel_curl->proxy != NULL) {
		if (channel_curl->proxy != USE_PROXY_ENV) {
			if (curl_easy_setopt(channel_curl->handle, CURLOPT_PROXY, channel_curl->proxy) != CURLE_OK) {
				result = CHANNEL_EINIT;
				goto cleanup;
			}
		}
		if (curl_easy_setopt(channel_curl->handle, CURLOPT_NETRC, CURL_NETRC_OPTIONAL) != CURLE_OK) {
			result = CHANNEL_EINIT;
			goto cleanup;
		}
	}

	CURLcode curlrc = curl_easy_setopt(channel_curl->handle, CURLOPT_TCP_KEEPALIVE, 1L);
	if (curlrc == CURLE_OK) {
		if ((curl_easy_setopt(channel_curl->handle, CURLOPT_TCP_KEEPIDLE, KEEPALIVE_DELAY) != CURLE_OK) ||
		    (curl_easy_setopt(channel_curl->handle, CURLOPT_TCP_KEEPINTVL, KEEPALIVE_INTERVAL) != CURLE_OK)) {
			com_err("TCP Keep-alive interval and delay could not be configured.\n");
			result = CHANNEL_EINIT;
			goto cleanup;
		}
	} else {
		if (curlrc != CURLE_UNKNOWN_OPTION) {
			com_err("Channel could not be configured to sent keep-alive probes.\n");
			result = CHANNEL_EINIT;
			goto cleanup;
		}
	}

	if (channel_data->auth) {
		if (curl_easy_setopt(channel_curl->handle, CURLOPT_USERPWD, channel_data->auth) != CURLE_OK) {
			com_err("Basic Auth credentials could not be set.");
			result = CHANNEL_EINIT;
			goto cleanup;
		}
	}

cleanup:
	return result;
}
channel_op_res_t channel_map_curl_error(CURLcode res)
{
	switch (res) {
	case CURLE_NOT_BUILT_IN:
	case CURLE_BAD_FUNCTION_ARGUMENT:
	case CURLE_UNKNOWN_OPTION:
	case CURLE_SSL_ENGINE_NOTFOUND:
	case CURLE_SSL_ENGINE_SETFAILED:
	case CURLE_SSL_CERTPROBLEM:
	case CURLE_SSL_CIPHER:
	case CURLE_SSL_ENGINE_INITFAILED:
	case CURLE_SSL_CACERT_BADFILE:
	case CURLE_SSL_CRL_BADFILE:
	case CURLE_SSL_ISSUER_ERROR:
#if LIBCURL_VERSION_NUM >= 0x072900
	case CURLE_SSL_INVALIDCERTSTATUS:
#endif
#if LIBCURL_VERSION_NUM >= 0x072700
	case CURLE_SSL_PINNEDPUBKEYNOTMATCH:
#endif
		return CHANNEL_EINIT;
	case CURLE_COULDNT_RESOLVE_PROXY:
	case CURLE_COULDNT_RESOLVE_HOST:
	case CURLE_COULDNT_CONNECT:
	case CURLE_INTERFACE_FAILED:
	case CURLE_SSL_CONNECT_ERROR:
	case CURLE_PEER_FAILED_VERIFICATION:
#if LIBCURL_VERSION_NUM < 0x073E00
	case CURLE_SSL_CACERT:
#endif
	case CURLE_USE_SSL_FAILED:
		return CHANNEL_ENONET;
	case CURLE_OPERATION_TIMEDOUT:
	case CURLE_SEND_ERROR:
	case CURLE_RECV_ERROR:
	case CURLE_GOT_NOTHING:
	case CURLE_HTTP_POST_ERROR:
	case CURLE_PARTIAL_FILE:
		return CHANNEL_EAGAIN;
	case CURLE_OUT_OF_MEMORY:
		return CHANNEL_ENOMEM;
	case CURLE_REMOTE_FILE_NOT_FOUND:
		return CHANNEL_ENOENT;
	case CURLE_FILESIZE_EXCEEDED:
	case CURLE_ABORTED_BY_CALLBACK:
	case CURLE_WRITE_ERROR:
	case CURLE_CHUNK_FAILED:
	case CURLE_SSL_SHUTDOWN_FAILED:
		return CHANNEL_EIO;
	case CURLE_TOO_MANY_REDIRECTS:
		return CHANNEL_ELOOP;
	case CURLE_BAD_CONTENT_ENCODING:
	case CURLE_CONV_FAILED:
	case CURLE_CONV_REQD:
		return CHANNEL_EILSEQ;
	case CURLE_REMOTE_ACCESS_DENIED:
	case CURLE_LOGIN_DENIED:
		return CHANNEL_EACCES;
	case CURLE_OK:
		return CHANNEL_OK;
	default:
		return CHANNEL_EINIT;
	}
}
static void channel_log_effective_url(channel_t *this)
{
	channel_curl_t *channel_curl = this->priv;

	CURLcode curlrc = curl_easy_getinfo(channel_curl->handle, CURLINFO_EFFECTIVE_URL, &channel_curl->effective_url);
	if (curlrc != CURLE_OK && curlrc == CURLE_UNKNOWN_OPTION) {
		com_err("Get channel's effective URL response unsupported by libcURL %s.\n", LIBCURL_VERSION);
		return;
	}
	com_info("Channel's effective URL resolved to %s", channel_curl->effective_url);
}

channel_op_res_t channel_get(channel_t *this, void *data)
{
	channel_curl_t *channel_curl = this->priv;
	assert(data != NULL);
	assert(channel_curl->handle != NULL);

	channel_op_res_t result = CHANNEL_OK;
	channel_data_t *channel_data = (channel_data_t *)data;

	if (channel_data->debug) {
		curl_easy_setopt(channel_curl->handle, CURLOPT_VERBOSE, 1L);
	}

	if ((result = channel_set_content_type(this, channel_data)) != CHANNEL_OK) {
		com_err("Set content-type option failed.");
		goto cleanup_header;
	}

	if ((result = channel_set_options(this, channel_data, CHANNEL_GET)) != CHANNEL_OK) {
		com_err("Set channel option failed.");
		goto cleanup_header;
	}

	output_data_t chunk = {.memory = NULL, .size = 0};
	if ((chunk.memory = malloc(1)) == NULL) {
		com_err("Channel buffer reservation failed with OOM.");
		result = CHANNEL_ENOMEM;
		goto cleanup_header;
	}

	write_callback_t wrdata;
	wrdata.channel_data = channel_data;
	wrdata.outdata = &chunk;

	if ((curl_easy_setopt(channel_curl->handle, CURLOPT_WRITEFUNCTION, channel_callback_membuffer) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_WRITEDATA, (void *)&wrdata) != CURLE_OK)) {
		com_err("Cannot setup memory buffer writer callback function.");
		result = CHANNEL_EINIT;
		goto cleanup_chunk;
	}

	if (channel_data->debug) {
		com_warn("Trying to GET %s", channel_data->url);
	}

	CURLcode curlrc = curl_easy_perform(channel_curl->handle);
	if (curlrc != CURLE_OK) {
		com_err("Channel get operation failed (%d): '%s'", curlrc, curl_easy_strerror(curlrc));
		result = channel_map_curl_error(curlrc);
		goto cleanup_chunk;
	}

	if (channel_data->debug) {
		channel_log_effective_url(this);
	}

	long http_response_code;
	result = channel_map_http_code(this, &http_response_code);

	if (channel_data->nocheckanswer)
		goto cleanup_chunk;

	if ((result = channel_map_http_code(this, &http_response_code)) != CHANNEL_OK) {
		com_err("Channel operation returned HTTP error code %ld.",http_response_code);
		switch (http_response_code) {
			case 403:
			case 404:
			case 500:
				com_warn("The error's message is: '%s'\n", chunk.memory);
				break;
			default:
				break;
		}
		goto cleanup_chunk;
	}
	if (channel_data->debug) {
		com_info("Channel operation returned HTTP status code %ld.", http_response_code);
	}

#ifdef CONFIG_JSON
	assert(channel_data->json_reply == NULL);
	enum json_tokener_error json_res;
	struct json_tokener *json_tokenizer = json_tokener_new();
	do {
		channel_data->json_reply = json_tokener_parse_ex(json_tokenizer, chunk.memory, (int)chunk.size);
	} while ((json_res = json_tokener_get_error(json_tokenizer)) == json_tokener_continue);
	if (json_res != json_tokener_success) {
		com_err("Error while parsing channel's returned JSON data: %s", json_tokener_error_desc(json_res));
		result = CHANNEL_EBADMSG;
		goto cleanup_json_tokenizer;
	}
	if (channel_data->debug) {
		com_info("Get JSON: %s", chunk.memory);
	}

cleanup_json_tokenizer:
	json_tokener_free(json_tokenizer);
#endif
cleanup_chunk:
	chunk.memory != NULL ? free(chunk.memory) : (void)0;
cleanup_header:
	curl_easy_reset(channel_curl->handle);
	curl_slist_free_all(channel_curl->header);
	channel_curl->header = NULL;

	return result;
}

channel_op_res_t channel_get_file(channel_t *this, void *data)
{
	channel_curl_t *channel_curl = this->priv;
	int file_handle = 0;
	assert(data != NULL);
	assert(channel_curl->handle != NULL);

	channel_op_res_t result = CHANNEL_OK;
	channel_data_t *channel_data = (channel_data_t *)data;

	if (channel_data->debug) {
		curl_easy_setopt(channel_curl->handle, CURLOPT_VERBOSE, 1L);
	}

	if (((channel_curl->header = curl_slist_append(channel_curl->header, "Content-Type: application/octet-stream")) == NULL) ||
	    ((channel_curl->header = curl_slist_append(channel_curl->header, "Accept: application/octet-stream")) == NULL)) {
		result = CHANNEL_EINIT;
		com_err("Set channel header failed.");
		goto cleanup_header;
	}

	if ((result = channel_set_options(this, channel_data, CHANNEL_GET)) != CHANNEL_OK) {
		com_err("Set channel option failed.");
		goto cleanup_header;
	}

	file_handle = open(channel_data->dest, O_CREAT|O_RDWR|O_TRUNC);
	if (file_handle < 0) {
		com_err("Cannot open :%s %s", channel_data->dest, strerror(errno));
		result = CHANNEL_EIO;
		goto cleanup_header;
	}

	write_callback_t wrdata;
	wrdata.channel_data = channel_data;
	wrdata.output = file_handle;
	result_channel_callback_write_file = CHANNEL_OK;
	if ((curl_easy_setopt(channel_curl->handle, CURLOPT_WRITEFUNCTION, channel_callback_write_file) != CURLE_OK) ||
	    (curl_easy_setopt(channel_curl->handle, CURLOPT_WRITEDATA, &wrdata) != CURLE_OK)) {
		com_err("Cannot setup file writer callback function.");
		result = CHANNEL_EINIT;
		goto cleanup_file;
	}

	unsigned long long int total_bytes_downloaded = 0;
	unsigned char try_count = 0;
	CURLcode curlrc = CURLE_OK;
	do {
		if (try_count > 0) {
			if (channel_data->retries == 0) {
				com_err("Channel get operation failed (%d): '%s'\n", curlrc, curl_easy_strerror(curlrc));
				result = channel_map_curl_error(curlrc);
				goto cleanup_file;
			}

			if (try_count > channel_data->retries) {
				com_err("Channel get operation aborted because of too many failed download attempts (%d).\n", channel_data->retries);
				result = CHANNEL_ELOOP;
				goto cleanup_file;
			}

			com_warn("Channel connection interrupted, trying resume after %llu bytes.", total_bytes_downloaded);
			if (curl_easy_setopt(channel_curl->handle, CURLOPT_RESUME_FROM_LARGE, total_bytes_downloaded) != CURLE_OK) {
				com_err("Could not set Channel resume seek (%d): %s\n", curlrc, curl_easy_strerror(curlrc));
				result = channel_map_curl_error(curlrc);
				goto cleanup_file;
			}
			com_info("Channel sleeps for %d seconds now.", channel_data->retry_sleep);
			if (sleep(channel_data->retry_sleep) > 0) {
				com_info("Channel's sleep got interrupted, retrying nonetheless now.");
			}
			com_info("Channel awakened from sleep.");
		}

		curlrc = curl_easy_perform(channel_curl->handle);
		result = channel_map_curl_error(curlrc);
		if ((result != CHANNEL_OK) && (result != CHANNEL_EAGAIN)) {
			com_err("Channel operation returned error (%d): '%s'", curlrc, curl_easy_strerror(curlrc));
			goto cleanup_file;
		}

		double bytes_downloaded;
		CURLcode resdlprogress = curl_easy_getinfo(channel_curl->handle, CURLINFO_SIZE_DOWNLOAD, &bytes_downloaded);
		if (resdlprogress != CURLE_OK) {
			com_err("Channel does not report bytes downloaded (%d): %s\n", resdlprogress, curl_easy_strerror(resdlprogress));
			result = channel_map_curl_error(resdlprogress);
			goto cleanup_file;
		}
		total_bytes_downloaded += bytes_downloaded;

	} while (++try_count && (result != CHANNEL_OK));
	channel_log_effective_url(this);
	com_warn("Channel downloaded %llu bytes ~ %llu MiB.", total_bytes_downloaded, total_bytes_downloaded / 1024 / 1024);
	long http_response_code;
	if ((result = channel_map_http_code(this, &http_response_code)) != CHANNEL_OK) {
		com_err("Channel operation returned HTTP error code %ld.", http_response_code);
		goto cleanup_file;
	}
	if (channel_data->debug) {
		com_info("Channel operation returned HTTP status code %lx.", http_response_code);
	}

	if (result_channel_callback_write_file != CHANNEL_OK) {
		result = CHANNEL_EIO;
		goto cleanup_file;
	}
cleanup_file:
	if (file_handle && fsync(file_handle) && close(file_handle) != 0) {
		com_err("Channel error while closing download target handle: %s", strerror(errno));
	}
cleanup_header:
	curl_easy_reset(channel_curl->handle);
	curl_slist_free_all(channel_curl->header);
	channel_curl->header = NULL;

	return result;
}

/*this transplant from swupdate corelib channel_curl.c if you want complete this£¬you can reference that*/
channel_t *channel_new(void)
{
	channel_t *newchan = (channel_t *)calloc(1, sizeof(*newchan) +
				sizeof(channel_curl_t));

	if (newchan) {
		newchan->priv = (void *)newchan +  sizeof(*newchan);
		newchan->open = &channel_open;
		newchan->close = &channel_close;
		newchan->get = &channel_get;
		newchan->get_file = &channel_get_file;
	}

	return newchan;
}
