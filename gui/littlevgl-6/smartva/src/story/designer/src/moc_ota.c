/**********************
 *      includes
 **********************/
#include "moc_ota.h"
#include "ui_ota.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "va_ota_conf.h"
#include "DiskManager.h"

#define OTA_CHECK_URL_0 "http://172.25.11.206:8090/ota_config_V_0_1.conf"
#define OTA_CHECK_0_IP "172.25.11.206"
#define OTA_CHECK_URL_1 "http://172.20.10.174:8090/ota_config_V_0_1.conf"
#define OTA_CHECK_1_IP "172.20.10.174"

#define OTA_CONFIG_PATH "/tmp/ota_config.conf"
#define OTA_FIRMWARE_PATH "/mnt/UDISK/test_ota.bin"
/**********************
 *       variables
 **********************/
typedef struct
{
	ota_ui_t ui;
} ota_para_t;
static ota_para_t para;
static char ota_firmware_path[1024];
static bool ota_ing = 0;
static hotplug_message_focus_win_t *RegisterInfo = NULL;

void set_ota_status(bool status)
{
	ota_ing = status;
}
int get_ota_status(void)
{
	return ota_ing;
}
char *get_ota_file_path(void)
{
	return ota_firmware_path;
}
void set_ota_file_path(char *path)
{
	memset(ota_firmware_path, 0x00, sizeof(ota_firmware_path));
	strncpy(ota_firmware_path, path, strlen(path));
}
static int net_is_ok(char *ip) {
	char command[128] = {0};
	FILE   *stream;

	sprintf(command, "ping %s -c 4", ip);
	stream = popen(command, "r");
	if(!stream) {
		return -1;
	}
	while (fgets(command, sizeof(command), stream)) {
		if (strstr(command, "100% packet loss") != NULL || strstr(command, "Network is unreachable") != NULL) {
			pclose(stream);
			return 0;
		}
	}
	pclose(stream);

	return 1;
}
/**********************
 *  functions
 **********************/
static __s32 ota_check_thread_run = 0;
static void* ota_check_thread(void *arg)
{
	char str[64] = {0};
	extern int download_from_url(const char *url, const char *dest_path, bool wait);
	extern int ota_check_file(char *file_path);

	ota_check_thread_run = 1;
	if (get_ota_status()) {
		printf("return ota_check\n");
		ota_check_thread_run = 0;
		pthread_exit(NULL);
	}
	if (net_is_ok(OTA_CHECK_0_IP)) {
		download_from_url(OTA_CHECK_URL_0, OTA_CONFIG_PATH, 1);
	} else if (net_is_ok(OTA_CHECK_1_IP)) {
		download_from_url(OTA_CHECK_URL_1, OTA_CONFIG_PATH, 1);
	} else {
		printf("service not attach\n");
		return 0;
	}
	va_ota_param_init(OTA_CONFIG_PATH);
	va_ota_read_string_type_param("ota_config","version", str, sizeof(str));
	printf("ota_config_version:%s\n", str);
	va_ota_read_string_type_param("ota_config","test_mode", str, sizeof(str));
	printf("test_mode:%s\n", str);
	if (0 != strncmp(str, "always", strlen("always")))
	{
		va_ota_param_uninit();
		ota_check_thread_run = 0;
		pthread_exit(NULL);
	}

	va_ota_read_string_type_param("ota_config","firmware_path", str, sizeof(str));
	printf("ota_config_firmware_path:%s\n", str);
	set_ota_status(1);
	set_ota_file_path(OTA_FIRMWARE_PATH);
	download_from_url(str, get_ota_file_path(), 1);
	if (ota_check_file(get_ota_file_path()) == 0) {
		switch_page(current_page(), PAGE_OTA);
	}
	va_ota_param_uninit();
	pthread_exit(NULL);
	ota_check_thread_run = 0;
}
void ota_check(void) {
	if (ota_check_thread_run == 0) {
		int ret = 0;
		pthread_t wait_pthread;
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, 0x4000);
		ret = pthread_create(&wait_pthread, &attr, &ota_check_thread, (void *)NULL);
		pthread_attr_destroy(&attr);
		if (ret < 0){
			com_info("pthread_create error\n");
			return ;
		}
	} else {
		return ;
	}
}
static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		destory_page(PAGE_OTA);
		create_page(PAGE_HOME);
	}
}
void ota_set_progressbar(unsigned int progress, char *image_name)
{
	ota_ui_t *ui = &para.ui;
	char label_str[8] = {0};
	char prompt[64] = {0};

	sprintf(label_str, "%d ", progress);
	sprintf(prompt, "Updating %s ...", image_name);
	lv_slider_set_value(ui->progressbar, progress, LV_ANIM_OFF);
	lv_label_set_text(ui->label_progress, label_str);
	lv_label_set_text(ui->prompt_lable, prompt);
}
void ota_set_prompt(char *str)
{
	ota_ui_t *ui = &para.ui;
	lv_label_set_text(ui->prompt_lable, str);
}

static void *progress_ota(void)
{
	int connfd;
	struct progress_msg msg;
	char psplash_pipe_path[256];
	int psplash_ok = 0;
	const char *tmpdir = NULL;
	unsigned int curstep = 0;
	unsigned int percent = 0;
	char bar[60];
	unsigned int filled_len;
//	int opt_c = 0;
//	int opt_w = 0;
//	int opt_r = 0;
//	int opt_p = 0;
	RECOVERY_STATUS status = IDLE;

	connfd = -1;
	tmpdir = getenv("TMPDIR");
	if (!tmpdir)
		tmpdir = "/tmp";
	snprintf(psplash_pipe_path, sizeof(psplash_pipe_path), "%s/psplash_fifo", tmpdir);
	while (1) {
		if (connfd < 0) {
			fprintf(stdout, "connect to swupdate\n\n");
			connfd = progress_ipc_connect(1);
		} else {
			fprintf(stdout, "Not to connect swupdate\n\n");
		}

		if (progress_ipc_receive(&connfd, &msg) == -1) {
			continue;
		}

		if (msg.infolen > 0)
			fprintf(stdout, "INFO : %s\n\n", msg.info);

//		if (!psplash_ok && opt_p) {
//			psplash_ok = psplash_init(psplash_pipe_path);
//		}

		if ((msg.cur_step != curstep) && (curstep != 0))
			fprintf(stdout, "\n");

		filled_len = sizeof(bar) * msg.cur_percent / 100;
		if (filled_len > sizeof(bar))
			filled_len = sizeof(bar);

		memset(bar,'=', filled_len);
		memset(&bar[filled_len], '-', sizeof(bar) - filled_len);

//		fprintf(stdout, "[ %.60s ] %d of %d %d%% (%s)\r", bar, msg.cur_step, msg.nsteps, msg.cur_percent, msg.cur_image);
//		fflush(stdout);
		ota_set_progressbar(msg.cur_percent, msg.cur_image);
		if (msg.cur_percent >= 100) {
			lv_label_set_text(para.ui.prompt_lable, "will jump to recovery system, please wait!");
			sleep(2);
			system("reboot");
			return 0;
		}
		if (psplash_ok && ((msg.cur_step != curstep) || (msg.cur_percent != percent))) {
//			psplash_progress(psplash_pipe_path, &msg);
			curstep = msg.cur_step;
			percent = msg.cur_percent;
		}

		switch (msg.status) {
		case SUCCESS:
		case FAILURE:
//			fprintf(stdout, "\n\n");
//			fprintf(stdout, "%s !\n", msg.status == SUCCESS
//							  ? "SUCCESS"
//							  : "FAILURE");
//			if (psplash_ok)
//				psplash_progress(psplash_pipe_path, &msg);
			psplash_ok = 0;
			if ((msg.status == SUCCESS)) {
//				sleep(5);
//				if (system("reboot") < 0) { /* It should never happen */
//					fprintf(stdout, "Please reset the board.\n");
//				}
			}
			break;
		case DONE:
//			fprintf(stdout, "\nDONE.\n");
			break;
		default:
			break;
		}

		status = msg.status;
	}
}
int ota_verify_file(char *file_path)
{
	char command[1024] = {0};
	FILE   *stream;

	sprintf(command, "swupdate -i %s -k /etc/swupdate_public.pem -c ", file_path);
	stream = popen(command, "r");
	if(!stream) {
		return -1;
	}
	while (fgets(command, sizeof(command), stream)) {
		if (strstr(command, "successfully checked") != NULL) {
			pclose(stream);
			return 0;
		}
	}
	pclose(stream);
    return -1;
}

int ota_check_file(char *file_path) {
	if (ota_verify_file(file_path) == 0) {
		strncpy(ota_firmware_path, file_path, strlen(file_path));
		return 0;
	}
	return -1;
}
static void ota_disk_hotplug_callback(DiskInfo_t *DiskInfo) {
	if (DiskInfo->operate == MEDIUM_PLUGOUT && strncmp(ota_firmware_path, DiskInfo->MountPoint, strlen(DiskInfo->MountPoint)) == 0)
	{
		system("killall swupdate");
		ota_set_prompt("Disk PlugOut Will Reboot");
		sleep(2);
		system("reboot");
	}
}


static int create_ota(void)
{
	int ret = 0;
	char command[2048] = {0};
	pthread_t progress_pthread;
	pthread_attr_t attr;

	para.ui.cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(para.ui.cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color = LV_COLOR_BLUE;
	cont_style.body.grad_color = LV_COLOR_BLUE;
	lv_cont_set_style(para.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
	lv_cont_set_layout(para.ui.cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(para.ui.cont, LV_FIT_NONE);

	static lv_style_t back_btn_style;
	lv_style_copy(&back_btn_style, &lv_style_pretty);
	back_btn_style.text.font = &lv_font_roboto_28;
	lv_obj_t *back_btn = lv_btn_create(para.ui.cont, NULL);
	lv_obj_align(back_btn, para.ui.cont, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_t *back_lable = lv_label_create(back_btn, NULL);
	lv_label_set_text(back_lable, LV_SYMBOL_LEFT);
	lv_obj_set_event_cb(back_btn, back_btn_event);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_REL, &back_btn_style);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_PR, &back_btn_style);
	sprintf(command, "fw_setenv swu_software %s", ota_firmware_path);
	system(command);
	ota_auto_ui_create(&para.ui);

	RegisterInfo = malloc(sizeof(hotplug_message_focus_win_t));
	if (RegisterInfo != NULL) {
		memset(RegisterInfo, 0x00, sizeof(hotplug_message_focus_win_t));
		RegisterInfo->CallBackFunction = ota_disk_hotplug_callback;
		strcpy(RegisterInfo->Cur_Win, "ota_ui");
		DiskManager_Register(RegisterInfo);
	}
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x4000);
	ret = pthread_create(&progress_pthread, &attr, (void *)progress_ota, NULL);
	pthread_attr_destroy(&attr);
	if (ret < 0) {
		printf("%s %d pthread_create error\n", __FILE__, __LINE__);
		return -1;
	}
	memset(command, 0x00, sizeof(command));
	sprintf(command, "nice -n 19 swupdate -i %s -e stable,upgrade_recovery -k /etc/swupdate_public.pem &", ota_firmware_path);
	system(command);
	//system("reboot");
	return 0;
}

static int destory_ota(void)
{
	ota_auto_ui_destory(&para.ui);
	lv_obj_del(para.ui.cont);

	return 0;
}

static int show_ota(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);

	return 0;
}

static int hide_ota(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);

	return 0;
}

static int msg_proc_ota(MsgDataInfo *msg)
{
	return 0;
}

static page_interface_t page_ota =
{
	.ops =
	{
		create_ota,
		destory_ota,
		show_ota,
		hide_ota,
		msg_proc_ota,
	},
	.info =
	{
		.id         = PAGE_OTA,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_OTA(void)
{
	reg_page(&page_ota);
}
