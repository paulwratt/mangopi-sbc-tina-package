/**********************
 *      includes
 **********************/
#include "moc_ota.h"
#include "ui_ota.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "DiskManager.h"


/**********************
 *       variables
 **********************/
typedef struct
{
	ota_ui_t ui;
} ota_para_t;
static ota_para_t para;
static hotplug_message_focus_win_t RegisterInfo;

/**********************
 *  functions
 **********************/
static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
//		destory_page(PAGE_OTA);
//		create_page(PAGE_HOME);
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

static void *progress_ota(void *arg)
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
		if ((msg.cur_percent >= 100) && (0 == strncmp(msg.cur_image, "rootfs", strlen("rootfs")))) {
			char command[64] = {0};

			lv_label_set_text(para.ui.prompt_lable, "will jump to normal system, please wait!");
			sleep(2);
			sprintf(command, "fw_setenv boot_partition boot");
			system(command);
			sprintf(command, "rm -rf %s", getenv("swu_software"));
			system(command);
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
	printf("%s %d %s command:%s\n", __FILE__, __LINE__, __func__, command);
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
		char set_ota_cmd[1024];
		sprintf(set_ota_cmd, "fw_setenv swu_software %s", file_path);
		system(set_ota_cmd);
		return 0;
	}
	return -1;
}

static void ota_disk_hotplug_callback(DiskInfo_t *DiskInfo) {
	if (DiskInfo->operate == MEDIUM_PLUGOUT && strncmp(getenv("swu_software"), DiskInfo->MountPoint, strlen(DiskInfo->MountPoint)) == 0)
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
	pthread_t progress_pthread;
	char command[2048] = {0};

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
	ota_auto_ui_create(&para.ui);
	strcpy(RegisterInfo.Cur_Win, "ota_ui");
	RegisterInfo.CallBackFunction = ota_disk_hotplug_callback;
	DiskManager_Register(&RegisterInfo);
	ota_auto_ui_create(&para.ui);
	sprintf(command, "swupdate -i %s -e stable,%s -k /etc/swupdate_public.pem &", getenv("swu_software"), getenv("swu_mode"));
	printf("%s %d %s command:%s\n", __FILE__, __LINE__, __func__, command);
	system(command);
	ret = pthread_create(&progress_pthread, NULL, (void *)progress_ota, NULL);
	if (ret < 0) {
		printf("%s %d pthread_create error\n", __FILE__, __LINE__);
		return -1;
	}
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

static page_interface_t page_ota =
{
	.ops =
	{
		create_ota,
		destory_ota,
		show_ota,
		hide_ota,
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
