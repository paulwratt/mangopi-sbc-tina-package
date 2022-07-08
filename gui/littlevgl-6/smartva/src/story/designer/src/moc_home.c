/**********************
 *      includes
 **********************/
#include "moc_home.h"
#include "ui_home.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include <stdio.h>
#include "media_mixture.h"
#include "common.h"
#include "bs_widget.h"
#include "va_power.h"

/**********************
 *       variables
 **********************/
typedef struct
{
	int id;

	lv_obj_t *img_bgd;
	lv_task_t *home_tid;
	int dialog_showtime;
} home_moc_t;

typedef enum
{
	DIALOG_NO_DISK,
	VIDEO_IS_NULL,
	MUSIC_IS_NULL,
} dialog_con_t;

typedef struct
{
	home_ui_t ui;
	home_moc_t moc;
} home_para_t;
static home_para_t para;

/**********************
 *  functions
 **********************/
static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_HOME);
		//create_page(PAGE_HOME);
		switch_page(PAGE_HOME, PAGE_HOME);
	}
}

static void btn_hbar_return_event_cb(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_HOME);
		//create_page(PAGE_HOME);
		switch_page(PAGE_HOME, PAGE_HOME);
	}
}

static void home_show_dialog(uint16_t time, dialog_con_t con)
{
	if(para.moc.dialog_showtime == 0)
	{
		lv_obj_set_hidden(para.ui.cont_dialog, false);
		para.moc.dialog_showtime = time;
		switch(con)
		{
			case DIALOG_NO_DISK:
			#if CONFIG_FONT_ENABLE
			lv_label_set_text(para.ui.label_dialog,  get_text_by_id(LANG_HOME_INSERT_DISK));
			#else
			lv_label_set_text(para.ui.label_dialog,  "no disk insert!");
			#endif
			lv_obj_set_hidden(para.ui.label_dialog, false);
			break;
			case VIDEO_IS_NULL:
			#if CONFIG_FONT_ENABLE
			lv_label_set_text(para.ui.label_dialog,  get_text_by_id(LANG_HOME_NO_MOVIE));
			#else
			lv_label_set_text(para.ui.label_dialog,  "video is null!");
			#endif
			lv_obj_set_hidden(para.ui.label_dialog, false);
			break;
			case MUSIC_IS_NULL:
			#if CONFIG_FONT_ENABLE
			lv_label_set_text(para.ui.label_dialog,  get_text_by_id(LANG_HOME_NO_AUDIO));
			#else
			lv_label_set_text(para.ui.label_dialog,  "music is null!");
			#endif
			lv_obj_set_hidden(para.ui.label_dialog, false);
			break;
			default:
			com_info("no this con !");
			break;
		}
	}
}

static void page_change_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t *player_ui = media_get_player_data();

	if (event == LV_EVENT_CLICKED) {
		if (btn == para.ui.image_button_1 || btn == para.ui.image_button_2){
			if (DiskManager_GetDiskNum() == 0) {
				com_warn("usb don't insert");
				home_show_dialog(3, DIALOG_NO_DISK);
				return;
			}
		}

		if (btn == para.ui.image_button_1) {
			if (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
				tplayer_stop(player_ui->tplayer);
				while (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
					printf("Wait stop finish [L=%d, F=%s]:\n", __LINE__, __FUNCTION__);
					usleep(10000);
				}
			}
			switch_page(PAGE_HOME, PAGE_MOVIE);
		} else if(btn == para.ui.image_button_2) {
			switch_page(PAGE_HOME, PAGE_MUSIC);
		}
		else if(btn == para.ui.image_button_3) {
			switch_page(PAGE_HOME, PAGE_CAMERA);
			com_info("\n");
		} else if(btn == para.ui.image_button_4) {
			/*
			if (create_page(PAGE_SOUND) < 0) {
				destory_page(PAGE_SOUND);
			} else {
				destory_page(PAGE_HOME);
			}
			*/
			switch_page(PAGE_HOME, PAGE_SOUND);
			com_info("\n");
		} else if(btn == para.ui.image_button_5) {
			if (DiskManager_GetDiskNum() != 0) {
				//destory_page(PAGE_HOME);
				//create_page(PAGE_PHOTO);
				switch_page(PAGE_HOME, PAGE_PHOTO);
			} else {
				com_warn("usb don't insert");
				home_show_dialog(3, DIALOG_NO_DISK);
			}
			com_info("\n");
		} else if(btn == para.ui.image_button_6) {
			//destory_page(PAGE_HOME);
			//create_page(PAGE_WLAN_SET1);
			switch_page(PAGE_HOME, PAGE_WLAN_SET1);
			com_info("\n");
		} else if(btn == para.ui.image_button_7) {
			if (DiskManager_GetDiskNum() != 0) {
				//destory_page(PAGE_HOME);
				//create_page(PAGE_EXPLORER);
				switch_page(PAGE_HOME, PAGE_EXPLORER);
			} else {
				com_info("\n");
				home_show_dialog(3, DIALOG_NO_DISK);
			}
		}
		else if(btn == para.ui.image_button_8)
		{
			com_info("\n");
			//destory_page(PAGE_HOME);
			//create_page(PAGE_SETTING);
			switch_page(PAGE_HOME, PAGE_SETTING);
			com_info("\n");
		}
		else if(btn == para.ui.image_button_9)
		{
			//destory_page(PAGE_HOME);
			//create_page(PAGE_SLIDE_HOME);
			//switch_page(PAGE_HOME, PAGE_SLIDE_HOME);
			anim_switch_page(PAGE_HOME, PAGE_SLIDE_HOME);
		}
		#if 0
		else if(btn == para.ui.image_button_10)
		{
			destory_page(PAGE_HOME);
			create_page(PAGE_CAMERA);
			com_info("\n");
		}
		#endif
	}
}

static void wifi_signal_update(void)
{
	wifi_data_t wifi;
	get_wifi_data(&wifi);
	static int flag = 0;
	if(flag == wifi.rssi)
		return;
	flag = wifi.rssi;
	if(wifi.is_on && wifi.is_connected)
	{
		//com_info("wifi signal : %d   pic=%x",wifi.rssi,get_img_by_level(wifi.rssi));
		lv_img_set_src(para.ui.img_hbar_wifi, get_img_by_level(wifi.rssi));
	}
	else
	{
		lv_img_set_src(para.ui.img_hbar_wifi, (void *)get_img_by_level(-1));
		//com_info("wifi no signal ..");
	}
}

static void home_update_task(lv_task_t * task)
{
	char buf[16];
	struct tm time;

	//printf("[L=%d, F=%s]:\n", __LINE__, __FUNCTION__);

	va_rtc_get_local_time(&time);
	//printf("tm_hour=%d, tm_min=%d, tm_sec=%d\n", time.tm_hour, time.tm_min, time.tm_sec);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%02d:%02d", time.tm_hour, time.tm_min);
	lv_label_set_text(para.ui.label_hbar_timer, buf);

#if 0
	int level;

	level = va_power_get_battery_level();
	switch(level){
	case POWER_LEVEL_0:
		lv_img_set_src(para.ui.img_hbar_power, LV_SYMBOL_BATTERY_EMPTY);
		break;
	case POWER_LEVEL_1:
		lv_img_set_src(para.ui.img_hbar_power, LV_SYMBOL_BATTERY_1);
		break;
	case POWER_LEVEL_2:
		lv_img_set_src(para.ui.img_hbar_power, LV_SYMBOL_BATTERY_2);
		break;
	case POWER_LEVEL_3:
		lv_img_set_src(para.ui.img_hbar_power, LV_SYMBOL_BATTERY_2);
		break;
	case POWER_LEVEL_4:
		lv_img_set_src(para.ui.img_hbar_power, LV_SYMBOL_BATTERY_3);
		break;
	case POWER_LEVEL_5:
		lv_img_set_src(para.ui.img_hbar_power, LV_SYMBOL_BATTERY_FULL);
		break;
	default:
		debug_info("not know the type.\n");
		break;
	}
#endif
	wifi_signal_update();
	if(para.moc.dialog_showtime)
	{
		para.moc.dialog_showtime--;
		if(para.moc.dialog_showtime <= 0)
		{
			lv_obj_set_hidden(para.ui.cont_dialog, true);
		}
	}
}
#if CONFIG_FONT_ENABLE
static void home_label_text_init(home_para_t *para)
{
	static lv_style_t style0_label_1;
	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.line_space = 2;
	style0_label_1.text.font = get_font_lib()->msyh_16;
	lv_label_set_text(para->ui.label_1, get_text_by_id(LANG_HOME_MOVIE));
	lv_label_set_text(para->ui.label_2, get_text_by_id(LANG_HOME_MUSIC));
	lv_label_set_text(para->ui.label_3, get_text_by_id(LANG_HOME_CAMERA));
	lv_label_set_text(para->ui.label_4, get_text_by_id(LANG_HOME_RECORD));
	lv_label_set_text(para->ui.label_5, get_text_by_id(LANG_HOME_PHOTO));
	lv_label_set_text(para->ui.label_6, get_text_by_id(LANG_HOME_WLAN));
	lv_label_set_text(para->ui.label_7, get_text_by_id(LANG_HOME_EXPLORER));
	lv_label_set_text(para->ui.label_8, get_text_by_id(LANG_HOME_SETING));
	lv_label_set_text(para->ui.label_9, get_text_by_id(LANG_HOME_SHOME));
	lv_label_set_text(para->ui.label_dialog, get_text_by_id(LANG_HOME_FM_PLAY));

	lv_label_set_style(para->ui.label_1, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_2, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_3, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_4, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_5, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_6, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_7, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_8, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_9, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_dialog, LV_LABEL_STYLE_MAIN, &style0_label_1);
}
#endif
void home_moc_create(home_para_t *para)
{
	#if CONFIG_FONT_ENABLE
	home_label_text_init(para);
	#endif
	background_photo_init();
	para->moc.img_bgd = lv_img_create(para->ui.cont, NULL);
	lv_obj_set_pos(para->moc.img_bgd, 0, 0);
	lv_obj_set_size(para->moc.img_bgd, 800, 480);
	lv_img_set_src(para->moc.img_bgd, get_background_photo());
	lv_obj_move_background(para->moc.img_bgd);

	// hbarË¢ÐÂ
	para->moc.home_tid = lv_task_create(home_update_task, 1000, LV_TASK_PRIO_LOW, NULL);
	lv_task_ready(para->moc.home_tid);

	lv_obj_set_event_cb(para->ui.image_button_1, page_change_event);
	lv_obj_set_event_cb(para->ui.image_button_2, page_change_event);
	lv_obj_set_event_cb(para->ui.image_button_3, page_change_event);
	lv_obj_set_event_cb(para->ui.image_button_4, page_change_event);
	lv_obj_set_event_cb(para->ui.image_button_5, page_change_event);
	lv_obj_set_event_cb(para->ui.image_button_6, page_change_event);
	lv_obj_set_event_cb(para->ui.image_button_7, page_change_event);
	lv_obj_set_event_cb(para->ui.image_button_8, page_change_event);
	lv_obj_set_event_cb(para->ui.image_button_9, page_change_event);
	//lv_obj_set_event_cb(para->ui.image_button_10, page_change_event);

	// °´¼ü
	#if USE_SUNXI_KEY
	lv_group_t *g = get_key_group();
	if(g != NULL) {
		lv_group_add_obj(g, para->ui.btn_hbar_return);
		lv_group_add_obj(g, para->ui.btn_hbar_home);
		lv_group_add_obj(g, para->ui.image_button_1);
		lv_group_add_obj(g, para->ui.image_button_2);
		lv_group_add_obj(g, para->ui.image_button_3);
		lv_group_add_obj(g, para->ui.image_button_4);
		lv_group_add_obj(g, para->ui.image_button_5);
		lv_group_add_obj(g, para->ui.image_button_6);
		lv_group_add_obj(g, para->ui.image_button_7);
		lv_group_add_obj(g, para->ui.image_button_8);
		lv_group_add_obj(g, para->ui.image_button_9);
#if 0
		lv_group_add_obj(g, para->ui.image_button_10);
#endif
	}
	#endif

	lv_obj_set_event_cb(para->ui.btn_hbar_return, btn_hbar_return_event_cb);
	lv_obj_set_event_cb(para->ui.btn_hbar_home, btn_hbar_return_event_cb);

}

void home_moc_destory(home_para_t *para)
{
	#if USE_SUNXI_KEY
	lv_group_t *g = get_key_group();
	if(g != NULL) {
		lv_group_remove_all_objs(g);
	}
	#endif

	lv_task_del(para->moc.home_tid);
	background_photo_uninit();
}

static int create_home(void)
{
	memset(&para, 0, sizeof(home_para_t));
	para.ui.cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(para.ui.cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color = LV_COLOR_BLUE;
	cont_style.body.grad_color = LV_COLOR_BLUE;
	lv_cont_set_style(para.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
	lv_cont_set_layout(para.ui.cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(para.ui.cont, LV_FIT_NONE);

	#if 0
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
	#endif

	home_auto_ui_create(&para.ui);
	home_moc_create(&para);

	return 0;
}

static int destory_home(void)
{
	home_moc_destory(&para);
	home_auto_ui_destory(&para.ui);
	lv_obj_del(para.ui.cont);
	return 0;
}

static int show_home(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);

	return 0;
}

static int hide_home(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);

	return 0;
}

static int msg_proc_home(MsgDataInfo *msg)
{
	return 0;
}

static int b_anim_end = 0;

static void home_anim_end(lv_anim_t * a)
{
	b_anim_end = 1;
}

static int anim_home(page_id_t old_page, page_id_t new_page)
{
	lv_anim_t a;
	b_anim_end = 0;
	
	if (old_page == PAGE_HOME && new_page == PAGE_SLIDE_HOME)
	{
	    a.var = para.ui.cont;
	    a.start = 0;
	    a.end = -LV_HOR_RES_MAX;
	    a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_x;
	    a.path_cb = lv_anim_path_linear;
	    a.ready_cb = home_anim_end;
	    a.act_time = 0;
	    a.time = 1000;
	    a.playback = 0;
	    a.playback_pause = 0;
	    a.repeat = 0;
	    a.repeat_pause = 0;
	    lv_anim_create(&a);
	}
	else if (old_page == PAGE_SLIDE_HOME && new_page == PAGE_HOME)
	{
	    a.var = para.ui.cont;
	    a.start = -LV_HOR_RES_MAX;
	    a.end = 0;
	    a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_x;
	    a.path_cb = lv_anim_path_linear;
	    a.ready_cb = home_anim_end;
	    a.act_time = 0;
	    a.time = 1000;
	    a.playback = 0;
	    a.playback_pause = 0;
	    a.repeat = 0;
	    a.repeat_pause = 0;
	    lv_anim_create(&a);
	}
	else
	{
		com_warn("\n");
	}
	
	return 0;
}

static int is_anim_end_home(void)
{
	return b_anim_end;
}

static page_interface_t page_home =
{
	.ops =
	{
		create_home,
		destory_home,
		show_home,
		hide_home,
		msg_proc_home,
		anim_home,
		is_anim_end_home,
	},
	.info =
	{
		.id         = PAGE_HOME,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_HOME(void)
{
	reg_page(&page_home);
}
