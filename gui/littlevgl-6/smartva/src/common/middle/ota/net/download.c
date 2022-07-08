#include "download.h"
#include "smt_config.h"
struct download_param {
	channel_t *channel;
	channel_data_t *channel_data;
};

static void *download(struct download_param *param) {
	com_info("file download begin:\n");
	com_info("url:%s\n", param->channel_data->url);
	com_info("dest:%s\n", param->channel_data->dest);
	if (param->channel->open(param->channel, param->channel_data) != CHANNEL_OK) {
		com_info("param->channel->open failed\n" );
		goto failed;
	}
	channel_op_res_t chanresult = param->channel->get_file(param->channel, param->channel_data);
	if (chanresult != CHANNEL_OK) {
		com_info("param->channel->get_file failed\n" );
		goto failed;
	}
	param->channel->close(param->channel);
failed:
	free(param->channel);
	free(param->channel_data->dest);
	free(param->channel_data->url);
	free(param->channel_data);
	free(param);
	return NULL;
}

static void *download_thread(void *arg) {
	int is_ota = 0;
	struct download_param *param = (struct download_param *)arg;
	extern char *get_ota_file_path(void);

	if (0 == strncmp(param->channel_data->dest, get_ota_file_path(), strlen(get_ota_file_path()))) {
		is_ota = 1;
	}
	download(param);
	if (is_ota) {
		extern int ota_check_file(char *file_path);
		extern void switch_page(page_id_t old_id, page_id_t new_id);
		extern page_id_t current_page(void);

		if (ota_check_file(get_ota_file_path()) == 0) {
			switch_page(current_page(), PAGE_OTA);
		}
	}
	return NULL;
}
int download_from_url(const char *url, const char *dest_path, bool wait)
{
	struct download_param *param = NULL;
	channel_data_t *channel_options = NULL;

	channel_t *channel = channel_new();
	if (!channel) {
		com_err("%s fail\n", __func__);
		return -1;
	}

	channel_options = malloc(sizeof(channel_data_t));
	memset(channel_options, 0x00, sizeof(channel_data_t));
	channel_options->source = SOURCE_DOWNLOADER;
	channel_options->debug = false;
	channel_options->retries = DL_DEFAULT_RETRIES;
	channel_options->low_speed_timeout = DL_LOWSPEED_TIME;
	channel_options->url = malloc(strlen(url) + 1);
	if (channel_options->url) {
		memset(channel_options->url, 0x00, strlen(url) + 1);
		strncpy(channel_options->url, url, strlen(url));
	}
	channel_options->dest = malloc(strlen(dest_path) + 1);
	if (channel_options->dest) {
		memset(channel_options->dest, 0x00, strlen(dest_path) + 1);
		strncpy(channel_options->dest, dest_path, strlen(dest_path));
	}

	param = malloc(sizeof(struct download_param));
	memset(param, 0x00, sizeof(struct download_param));
	param->channel = channel;
	param->channel_data = channel_options;
	if (!wait) {
		int ret = 0;
		pthread_t wait_pthread;
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		ret = pthread_create(&wait_pthread, &attr, &download_thread, (void *)param);
		pthread_attr_destroy(&attr);
		if (ret < 0){
			com_info("pthread_create error\n");
			return -1;
		}

	} else {
		download(param);
	}
	return 0;
}
