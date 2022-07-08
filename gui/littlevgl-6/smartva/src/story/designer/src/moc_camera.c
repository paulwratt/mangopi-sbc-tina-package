/**********************
 *      includes
 **********************/
#include "moc_camera.h"
#include "ui_camera.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "recorder_int.h"
#include "media_mixture.h"
#include "DiskManager.h"

/**********************
 *       variables
 **********************/
typedef struct
{
	camera_ui_t ui;
} camera_para_t;
static camera_para_t para;


/**********************
 *  functions
 **********************/
static recorder_t *recorder;
static lv_task_t *camera_task;
static hotplug_message_focus_win_t *RegisterInfo = NULL;
static DiskInfo_t Disk_HotPlugMsg;
static DiskInfo_t Disk_HotPlugTf;
static DiskInfo_t *record_disk;

static void clean_screen(camera_ui_t *ui)
{
	// open tplayerdemo || willow					// 打开视频或则图片图层，pipe要是0

	lv_obj_t *scn = lv_scr_act();					// 清屏幕
	static lv_style_t scn_style;
	lv_style_copy(&scn_style, &lv_style_plain);
	scn_style.body.main_color.full = 0x00000000;
	scn_style.body.grad_color.full = 0x00000000;
	lv_obj_set_style(scn, &scn_style);

	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color.full = 0x00000000;		// 清cont
	cont_style.body.grad_color.full = 0x00000000;
	lv_cont_set_style(ui->cont, LV_CONT_STYLE_MAIN, &cont_style);
}

static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_CAMERA);
		//create_page(PAGE_HOME);
		switch_page(PAGE_CAMERA, PAGE_HOME);
	}
}

static struct timespec rec_time;
static float rec_timing = 0;
static struct timespec count_time;
static float c_time = 0;

static float get_diff_time(struct timespec * start , bool update)
{
	float dt;
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC,&now);
	dt = (float)(now.tv_sec  - start->tv_sec);
	dt += (float)(now.tv_nsec - start->tv_nsec) * 1e-9;

	if(update == true){
        start->tv_sec = now.tv_sec;
        start->tv_nsec = now.tv_nsec;
	}

	return dt;
}

static void set_take_mode(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		if(!record_disk){
			com_warn("no disk!!!");
			lv_obj_set_hidden(para.ui.disk_info, 0);
			get_diff_time(&count_time, true);
			c_time = 0;
			return;
		}

		if(recorder->dv_core[0].record_sta == RECORD_START){
			lv_btn_set_state(btn, LV_BTN_STYLE_TGL_REL);
			return;
		}
		if(lv_btn_get_state(btn) == LV_BTN_STYLE_REL){
			lv_obj_set_hidden(para.ui.label_mode, 1);
			lv_imgbtn_set_toggle(para.ui.take, false);
		}
		if(lv_btn_get_state(btn) == LV_BTN_STYLE_TGL_REL){
			lv_label_set_text(para.ui.label_mode, "recording...");
			lv_imgbtn_set_toggle(para.ui.take, true);
			lv_obj_set_hidden(para.ui.label_mode, 0);
		}
	}

}

static void take_picture_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		if(!record_disk){
			com_warn("no disk!!!");
			lv_obj_set_hidden(para.ui.disk_info, 0);
			get_diff_time(&count_time, true);
			c_time = 0;
			return;
		}
		if(lv_btn_get_state(para.ui.mode) == LV_BTN_STYLE_REL){
			recorder_send_cmd(recorder, TAKE_PICTURE_CMD, 0, 0);
		}

		if(lv_btn_get_state(para.ui.mode) == LV_BTN_STYLE_TGL_REL){
			if(recorder->dv_core[0].record_sta == RECORD_START){
				recorder_send_cmd(recorder, RECORDER_STOP_CMD, 0, 0);
				recorder_send_cmd(recorder, PREVIEW_DISPLAY_ENABLE_CMD, 0, 1);
				recorder_send_cmd(recorder, CAMRERA_INIT_CMD, 0, 0);
				recorder_send_cmd(recorder, PREVIEW_START_CMD, 0, 0);
			}else if(recorder->dv_core[0].record_sta == RECORD_STOP){
				recorder_send_cmd(recorder, PREVIEW_DISPLAY_ENABLE_CMD, 0, 0);
				recorder_send_cmd(recorder, PREVIEW_STOP_CMD, 0, 0);
				recorder_send_cmd(recorder, CAMRERA_INIT_CMD, 0, 0);
				recorder_send_cmd(recorder, RECORDER_START_CMD, 0, 0);
				rec_timing = 0;
				get_diff_time(&rec_time, true);
			}
		}
	}
}

static void camera_disk_manager_callback(DiskInfo_t *DiskInfo)
{
	if(DiskInfo->MediaType == MEDIUM_USB_MASSSTORAGE){
		memset(&Disk_HotPlugMsg, 0x00, sizeof(DiskInfo_t));
		memcpy(&Disk_HotPlugMsg, DiskInfo, sizeof(DiskInfo_t));
	}
	if(DiskInfo->MediaType == MEDIUM_SD_CARD){
		memset(&Disk_HotPlugTf, 0x00, sizeof(DiskInfo_t));
		memcpy(&Disk_HotPlugTf, DiskInfo, sizeof(DiskInfo_t));
	}
	switch(DiskInfo->operate){
		case MEDIUM_PLUGOUT:
			if(!record_disk){
				com_err("no disk!!!");
				break;
			}
			if(DiskInfo->MediaType == record_disk->MediaType){
				if(DiskInfo->MediaType == MEDIUM_USB_MASSSTORAGE && Disk_HotPlugTf.operate == MEDIUM_PLUGIN){
					record_disk = &Disk_HotPlugTf;
				}else if(DiskInfo->MediaType == MEDIUM_SD_CARD && Disk_HotPlugMsg.operate == MEDIUM_PLUGIN){
					record_disk = &Disk_HotPlugMsg;
				}else{
					record_disk = NULL;
				}
			}
			break;
		case MEDIUM_PLUGIN:
			if(!record_disk){
				if(DiskInfo->MediaType == MEDIUM_USB_MASSSTORAGE){
					record_disk = &Disk_HotPlugMsg;
				}
				if(DiskInfo->MediaType == MEDIUM_SD_CARD){
					record_disk = &Disk_HotPlugTf;
				}
			}
			break;
		default:
			break;
	}
	if(recorder){
		if(record_disk){
			recorder_set_mount_path(recorder, record_disk->MountPoint);
		}else{
			recorder_set_mount_path(recorder, "/tmp");
		}
		com_info("mount path=%s", recorder->mount_path);
	}
}

static void camera_ui_task(struct _lv_task_t *param)
{
	if(recorder->dv_core[0].record_sta == RECORD_START){
		rec_timing += get_diff_time(&rec_time, true);
		media_set_time(para.ui.label_time, rec_timing * 1000);
	}
	if(!lv_obj_get_hidden(para.ui.disk_info)){
		c_time += get_diff_time(&count_time, true);
		if(c_time > 3){
			lv_obj_set_hidden(para.ui.disk_info, 1);
		}
	}
}

static int create_camera(void)
{
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
#if 0
	lv_obj_t *back_btn = lv_btn_create(para.ui.cont, NULL);
	lv_obj_align(back_btn, para.ui.cont, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_t *back_lable = lv_label_create(back_btn, NULL);
	lv_label_set_text(back_lable, LV_SYMBOL_LEFT);
	lv_obj_set_event_cb(back_btn, back_btn_event);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_REL, &back_btn_style);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_PR, &back_btn_style);
#endif
	Disk_HotPlugMsg.MediaType = MEDIUM_USB_MASSSTORAGE;
	Disk_HotPlugMsg.operate = MEDIUM_PLUGOUT;
	Disk_HotPlugTf.MediaType = MEDIUM_SD_CARD;
	Disk_HotPlugTf.operate = MEDIUM_PLUGOUT;
	RegisterInfo = malloc(sizeof(hotplug_message_focus_win_t));
	if (RegisterInfo != NULL) {
		memset(RegisterInfo, 0x00, sizeof(hotplug_message_focus_win_t));
		RegisterInfo->CallBackFunction = camera_disk_manager_callback;
		strcpy(RegisterInfo->Cur_Win, "camera_ui");
		DiskManager_Register(RegisterInfo);
	}
	camera_auto_ui_create(&para.ui);
	lv_obj_set_event_cb(para.ui.back, back_btn_event);
	lv_obj_set_event_cb(para.ui.take, take_picture_event);
	lv_obj_set_event_cb(para.ui.mode, set_take_mode);

	recorder = recorder_pthread_create();
	if(!recorder){
		return -1;
	}
	if(record_disk){
		recorder_set_mount_path(recorder, record_disk->MountPoint);
	}else{
		recorder_set_mount_path(recorder, "/tmp");
	}
	com_info("mount path=%s", recorder->mount_path);

	recorder_send_cmd(recorder, CAMRERA_INIT_CMD, 0, 0);
	recorder_send_cmd(recorder, PREVIEW_START_CMD, 0, 0);
	clean_screen(&para.ui);
	lv_obj_set_hidden(para.ui.label_mode, 1);
	camera_task = lv_task_create(camera_ui_task, LV_INDEV_DEF_LONG_PRESS_REP_TIME, LV_TASK_PRIO_LOW, (void *)recorder);

	return 0;
}

static int destory_camera(void)
{
	camera_auto_ui_destory(&para.ui);
	lv_obj_del(para.ui.cont);
	lv_task_del(camera_task);
	recorder_pthread_destory(recorder);
	if (RegisterInfo != NULL) {
		DiskManager_UnRegister(RegisterInfo);
	}
	recorder = NULL;
	return 0;
}

static int show_camera(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);

	return 0;
}

static int hide_camera(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);

	return 0;
}

static int msg_proc_camera(MsgDataInfo *msg)
{
	return 0;
}

static page_interface_t page_camera =
{
	.ops =
	{
		create_camera,
		destory_camera,
		show_camera,
		hide_camera,
		msg_proc_camera,
	},
	.info =
	{
		.id         = PAGE_CAMERA,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_CAMERA(void)
{
	reg_page(&page_camera);
}
