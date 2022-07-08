/**********************
 *      includes
 **********************/
#include "moc_sound.h"
#include "ui_sound.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"

#include "recorder_int.h"
#include "media_mixture.h"
#include "DiskManager.h"
#include "common.h"
#include "app_config_interface.h"

/**********************
 *       variables
 **********************/
typedef struct
{
	sound_ui_t ui;
} sound_para_t;
static sound_para_t para;

static recorder_t *recorder;
static hotplug_message_focus_win_t *RegisterInfo = NULL;
static DiskInfo_t Disk_HotPlugMsg;
static DiskInfo_t Disk_HotPlugTf;
static DiskInfo_t *record_disk;

static struct timespec rec_time;
static float rec_timing = 0;
static struct timespec count_time;
static float c_time = 0;

static char *old_mount_point;
/**********************
 *  functions
 **********************/
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

static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_SOUND);
		//create_page(PAGE_HOME);
		switch_page(PAGE_SOUND, PAGE_HOME);
	}
}

static void media_delete_event(lv_obj_t * btn, lv_event_t event)
{

	if (event == LV_EVENT_CLICKED){
		lv_obj_set_hidden(para.ui.de_label, !lv_obj_get_hidden(para.ui.de_label));
	}
}
static void media_list_event(lv_obj_t * btn, lv_event_t event)
{
	int index;

	index = lv_list_get_btn_index(para.ui.media_list, btn);

	if (event == LV_EVENT_CLICKED){
		if(!lv_obj_get_hidden(para.ui.de_label)){
			media_delete_list_file(para.ui.media_list, index);
			return;
		}
		media_ui_send_event(MEDIA_PLAY_EVENT, NULL, index);
	}
}

static void media_volume_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();

	if(event != LV_EVENT_PRESSED){
		return;
	}
	if(lv_btn_get_state(btn) == LV_BTN_STYLE_PR){
		media_ui_send_event(MEDIA_SET_VOLUME_EVENT, NULL, 0);
		lv_bar_set_value(para.ui.volume_bar, 0, LV_ANIM_OFF);

	}
	if(lv_btn_get_state(btn) == LV_BTN_STYLE_TGL_PR){
		media_ui_send_event(MEDIA_SET_VOLUME_EVENT, NULL, player_ui->media_cfg.volume);
		lv_bar_set_value(para.ui.volume_bar, player_ui->media_cfg.volume, LV_ANIM_OFF);
	}
}

static void record_sound_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t *player_ui = media_get_player_data();

	if (event == LV_EVENT_CLICKED){

		if(!record_disk){
			com_err("no disk!!!");
			return;
		}
		if(lv_btn_get_state(btn) == LV_BTN_STYLE_REL){
			recorder_send_cmd(recorder, AUDIO_STOP_CMD, 0, 0);
			media_ui_send_event(MEDIA_UPDATE_LIST_EVENT, NULL, 0);
		}
		if(lv_btn_get_state(btn) == LV_BTN_STYLE_TGL_REL){
			if(tplayer_get_status(player_ui->tplayer) == PLAY_STATUS){
				media_ui_send_event(MEDIA_PAUSE_EVENT, NULL, 0);
			}
			while(tplayer_get_status(player_ui->tplayer) == PLAY_STATUS){
				usleep(10000);
			}
			recorder_send_cmd(recorder, AUDIO_START_CMD, 0, 0);
			rec_timing = 0;
			get_diff_time(&rec_time, true);
			media_set_time(para.ui.label_time, 0);
		}
	}
}

static char audio_mount_path[256];
static void sound_disk_manager_callback(DiskInfo_t *DiskInfo)
{
	player_ui_t *player_ui = media_get_player_data();

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
				goto end;
			}
			if(DiskInfo->MediaType == record_disk->MediaType){
				if(DiskInfo->MediaType == MEDIUM_USB_MASSSTORAGE && Disk_HotPlugTf.operate == MEDIUM_PLUGIN){
					record_disk = &Disk_HotPlugTf;
				}else if(DiskInfo->MediaType == MEDIUM_SD_CARD && Disk_HotPlugMsg.operate == MEDIUM_PLUGIN){
					record_disk = &Disk_HotPlugMsg;
				}else{
					record_disk = NULL;
					goto end;
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
		recorder_set_mount_path(recorder, record_disk->MountPoint);
		get_recorder_path(recorder, 0, 2);
		memset(audio_mount_path, 0, sizeof(audio_mount_path));
		memcpy(audio_mount_path, recorder->audio_path, media_get_path_to_name(recorder->audio_path) - recorder->audio_path);
		memset(player_ui->play_info.root_path, 0x00, sizeof(player_ui->play_info.root_path));
		memcpy(player_ui->play_info.root_path, audio_mount_path, strlen(audio_mount_path));
		com_info("mount path=%s, player mount point = %s", recorder->mount_path, player_ui->play_info.root_path);
	}
end:
	return;
}

static void media_sound_ui_callback(void *ui_player, media_event_t event, void *param)
{
	media_file_list_t *media_list = NULL;
	player_ui_t * player_ui = (player_ui_t *)ui_player;

	if(player_ui == NULL){
		return;
	}
	switch(event){
		case MEDIA_IDLE_EVENT:
			if(tplayer_get_status(player_ui->tplayer) == PLAY_STATUS){
				media_set_progressbar(para.ui.progressbar, player_ui->play_info.time, player_ui->play_info.nDurationSec);
				media_set_time(para.ui.total_time, player_ui->play_info.nDurationSec);
				media_set_time(para.ui.curr_time, player_ui->play_info.time);
			}

			if(recorder->dv_core[0].record_sta == RECORD_START){
				rec_timing += get_diff_time(&rec_time, true);
				media_set_time(para.ui.label_time, (unsigned int)rec_timing * 1000);
			}

			if(!lv_obj_get_hidden(para.ui.warn_info)){
				c_time += get_diff_time(&count_time, true);
				if(c_time > 3){
					lv_obj_set_hidden(para.ui.warn_info, 1);
				}
			}
			if(!record_disk){
				if(current_page() == PAGE_SOUND){
					//destory_page(PAGE_SOUND);
					//create_page(PAGE_HOME);
					switch_page(PAGE_SOUND, PAGE_HOME);
				}
			}
			break;
		case MEDIA_UPDATE_LIST_EVENT:
			media_list = media_get_file_list(RAT_MEDIA_TYPE_AUDIO);
			if (media_list) {
				media_unload_file(player_ui->media_list);
				player_ui->media_list = media_list;
				media_update_file_list(para.ui.media_list, media_list, media_list_event);
				media_set_list_focus(para.ui.media_list, player_ui->media_list->total_num-1);
				media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			}
			break;
		case MEDIA_PLAY_EVENT:
			if(recorder_get_status(recorder, 0) == RECORD_START){
				lv_obj_set_hidden(para.ui.warn_info, 0);
				get_diff_time(&count_time, true);
				c_time = 0;
				lv_btn_set_state(para.ui.play, LV_BTN_STATE_REL);
				break;
			}
			lv_btn_set_state(para.ui.play, LV_BTN_STATE_TGL_REL);
			media_set_list_focus(para.ui.media_list, player_ui->play_info.index);
			break;
		case MEDIA_PAUSE_EVENT:
			lv_btn_set_state(para.ui.play, LV_BTN_STATE_REL);
			break;
		case MEDIA_PLAY_COMPLETE_EVENT:
			lv_btn_set_state(para.ui.play, LV_BTN_STATE_REL);
			lv_slider_set_value(para.ui.progressbar, lv_slider_get_max_value(para.ui.progressbar), LV_ANIM_OFF);
			media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			break;
		default:
			media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			break;
	}
}
static int create_sound(void)
{
	player_ui_t *player_ui = media_get_player_data();

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
	back_btn_style.body.opa = 64;
	back_btn_style.body.border.opa = 0;
	lv_btn_set_style(back_btn, LV_BTN_STYLE_REL, &back_btn_style);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_PR, &back_btn_style);

	Disk_HotPlugMsg.MediaType = MEDIUM_USB_MASSSTORAGE;
	Disk_HotPlugMsg.operate = MEDIUM_PLUGOUT;
	Disk_HotPlugTf.MediaType = MEDIUM_SD_CARD;
	Disk_HotPlugTf.operate = MEDIUM_PLUGOUT;
	RegisterInfo = malloc(sizeof(hotplug_message_focus_win_t));
	if (RegisterInfo != NULL) {
		memset(RegisterInfo, 0x00, sizeof(hotplug_message_focus_win_t));
		RegisterInfo->CallBackFunction = sound_disk_manager_callback;
		strcpy(RegisterInfo->Cur_Win, "sound_ui");
		DiskManager_Register(RegisterInfo);
	}

	sound_auto_ui_create(&para.ui);
	lv_obj_set_event_cb(para.ui.take, record_sound_event);
	lv_obj_set_event_cb(para.ui.play, media_play_event);
	lv_obj_set_event_cb(para.ui.volume_bar, media_volume_bar_event);
	lv_obj_set_event_cb(para.ui.volume, media_volume_event);
	lv_obj_set_event_cb(para.ui.delete, media_delete_event);

	if(tplayer_get_status(player_ui->tplayer) == PLAY_STATUS){
		media_ui_send_event(MEDIA_PAUSE_EVENT, NULL, 0);
	}

	recorder = recorder_pthread_create();
	if(!recorder || !record_disk){
		return -1;
	}

	recorder_set_mount_path(recorder, record_disk->MountPoint);

	old_mount_point = player_ui->play_info.root_path;
	memset(audio_mount_path, 0, sizeof(audio_mount_path));
	get_recorder_path(recorder, 0, 2);
	memcpy(audio_mount_path, recorder->audio_path, media_get_path_to_name(recorder->audio_path) - recorder->audio_path);
	memset(player_ui->play_info.root_path, 0x00, sizeof(player_ui->play_info.root_path));
	memcpy(player_ui->play_info.root_path, audio_mount_path, strlen(audio_mount_path));
	com_info("mount path=%s, player mount point = %s", recorder->mount_path, player_ui->play_info.root_path);

	player_ui->play_info.fileType = RAT_MEDIA_TYPE_AUDIO;
	player_ui->list_loop = 0;
	media_func_register(EXT_MUSIC_SCENE, media_sound_ui_callback);
	lv_bar_set_value(para.ui.volume_bar, player_ui->media_cfg.volume, LV_ANIM_OFF);
	media_ui_send_event(MEDIA_UPDATE_LIST_EVENT, NULL, 0);

	return 0;
}

static int destory_sound(void)
{
	player_ui_t *player_ui = media_get_player_data();

	//recorder_send_cmd(recorder, AUDIO_EXIT_CMD, 0, 0);
	if(recorder){
		recorder_pthread_destory(recorder);
		recorder = NULL;
		if(old_mount_point){
			memset(player_ui->play_info.root_path, 0x00, sizeof(player_ui->play_info.root_path));
			memcpy(player_ui->play_info.root_path, old_mount_point, strlen(old_mount_point));
		}
		media_func_unregister(EXT_MUSIC_SCENE, 0);
		old_mount_point = NULL;
	}
	if (RegisterInfo != NULL) {
		DiskManager_UnRegister(RegisterInfo);
	}
	sound_auto_ui_destory(&para.ui);
	lv_obj_del(para.ui.cont);
	return 0;
}

static int show_sound(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);

	return 0;
}

static int hide_sound(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);

	return 0;
}

static int msg_proc_sound(MsgDataInfo *msg)
{
	return 0;
}

static page_interface_t page_sound =
{
	.ops =
	{
		create_sound,
		destory_sound,
		show_sound,
		hide_sound,
		msg_proc_sound,
	},
	.info =
	{
		.id         = PAGE_SOUND,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_SOUND(void)
{
	reg_page(&page_sound);
}
