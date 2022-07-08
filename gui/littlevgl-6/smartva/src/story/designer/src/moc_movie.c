/**********************
 *      includes
 **********************/
#include "moc_movie.h"
#include "ui_movie.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "media_load_file.h"
#include "player_int.h"
#include "media_mixture.h"
#include "media_ui_mixture.h"
#include "common.h"
#include "bs_widget.h"
#include "default_timer.h"
#include "app_config_interface.h"

/**********************
 *       variables
 **********************/
typedef struct
{
	movie_ui_t ui;
	lv_obj_t* mbox_video_morethan_720P;
	lv_obj_t* mbox_file_is_null;
	lv_obj_t* mbox_play_mod_reminder;
} movie_para_t;
static movie_para_t para;

/**********************
 *  functions
 **********************/
typedef enum{
	NONE_STATUS,
	FULL_STATUS,
	BAR_STATUS,
	LIST_STATUS,
}ui_status_t;

typedef enum{
    CEDAR_VID_WINDOW_NOTCARE = 0x00,        /* 不关心显示比例，以当前设置的比例 */
    CEDAR_VID_WINDOW_ORIGINAL,              /* 以视频原始大小在窗口显示，不能溢出窗口 */
    CEDAR_VID_WINDOW_BESTSHOW,              /* 以视频本身的比例缩放至满窗口显示，视频不变形 */
    CEDAR_VID_WINDOW_FULLSCN,               /* 以铺满的模式满窗口显示视频图像，可能会变形 */
    CEDAR_VID_WINDOW_4R3MODE,               /* 以4:3的模式满窗口显示视频图像，图像会变形 */
    CEDAR_VID_WINDOW_16R9MODE,              /* 以16:9的模式满窗口显示视频图像，图像会变形 */
    CEDAR_VID_WINDOW_USERDEF,               //用户自定义，自己确定视频缩放比例
} vide_window_ratio_mode_t;

typedef enum{
	LEFT_CHANNEL  = 0,
	RIGHT_CHANNEL = 1,
	STEREO         = 2,
}sound_track_t;

ui_status_t ui_status = NONE_STATUS;

static int vide_set_win_ratio(int x, int y, unsigned int width, unsigned int height, vide_window_ratio_mode_t nVidShowMode)
{
	int tmp_wind_x;
	int tmp_wind_y;
	unsigned int tmp_wind_width;
	unsigned int tmp_wind_height;
	player_ui_t *player_ui = media_get_player_data();

    switch (nVidShowMode){
		case CEDAR_VID_WINDOW_ORIGINAL:
			 //show the video with video original size
			tmp_wind_width = player_ui->tplayer->mMediaInfo->pVideoStreamInfo->nWidth;
			tmp_wind_height = player_ui->tplayer->mMediaInfo->pVideoStreamInfo->nHeight;
			if (tmp_wind_width > width){
				tmp_wind_width = width;
			}
			if (tmp_wind_height > height){
				tmp_wind_height = height;
			}
			tmp_wind_x = x + (width - tmp_wind_width) / 2;
			tmp_wind_y = y + (height - tmp_wind_height) / 2;
			break;
		case CEDAR_VID_WINDOW_BESTSHOW:
			tmp_wind_width = player_ui->tplayer->mMediaInfo->pVideoStreamInfo->nWidth;
			tmp_wind_height = player_ui->tplayer->mMediaInfo->pVideoStreamInfo->nHeight;
			tmp_wind_x = x + (width - tmp_wind_width) / 2;
			tmp_wind_y = y + (height - tmp_wind_height) / 2;
			break;
		case CEDAR_VID_WINDOW_FULLSCN:
			//show the video in full screen window
			tmp_wind_x = x;
			tmp_wind_y = y;
			tmp_wind_width = width;
			tmp_wind_height = height;
			break;
		case CEDAR_VID_WINDOW_4R3MODE:
			//show the video with 4:3 mode
			tmp_wind_width = (height * 4) / 3;
			tmp_wind_height = height;
			tmp_wind_x = x + (width - tmp_wind_width) / 2;
			tmp_wind_y = y;
			break;
		case CEDAR_VID_WINDOW_16R9MODE:
			//show the video with 16:9 mode
			tmp_wind_width = width;
			tmp_wind_height = (width * 9) / 16;
			tmp_wind_x = x;
			tmp_wind_y = y + (height - tmp_wind_height) / 2;
			break;
		default:
			tmp_wind_x = x;
			tmp_wind_y = y;
			tmp_wind_width = width;
			tmp_wind_height = height;
			break;
	}
	tplayer_set_displayrect(player_ui->tplayer, tmp_wind_x, tmp_wind_y, tmp_wind_width, tmp_wind_height);

	return 0;
}

static void clean_screen(movie_ui_t *ui)
{
	// open tplayerdemo || willow					// ??????Ƶ????ͼƬͼ?㣬pipeҪ??0

	lv_obj_t *scn = lv_scr_act();					// ????Ļ
	static lv_style_t scn_style;
	lv_style_copy(&scn_style, &lv_style_plain);
	scn_style.body.main_color.full = 0x00000000;
	scn_style.body.grad_color.full = 0x00000000;
	lv_obj_set_style(scn, &scn_style);

	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color.full = 0x00000000;		// ??cont
	cont_style.body.grad_color.full = 0x00000000;
	lv_cont_set_style(ui->cont, LV_CONT_STYLE_MAIN, &cont_style);
}

static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		player_ui_t * player_ui = (player_ui_t *)media_get_player_data();

		if (PLAY_STATUS == player_ui->tplayer->mpstatus || PAUSE_STATUS == player_ui->tplayer->mpstatus){
			media_func_set_breaktag(player_ui->scene_name);
			media_ui_send_event(MEDIA_STOP_EVENT, NULL, 0);

			static lv_style_t cont_style;
			lv_style_copy(&cont_style, &lv_style_scr);
			cont_style.body.main_color = lv_color_hex(0xff557f);
			cont_style.body.grad_color = lv_color_hex(0xff557f);
			lv_cont_set_style(para.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
			lv_obj_set_hidden(para.ui.image_4, false);
			lv_obj_set_hidden(para.ui.media_list, false);
			lv_obj_set_hidden(para.ui.online, false);
			lv_obj_set_hidden(para.ui.image_4, false);
			lv_obj_set_hidden(para.ui.order, true);
			lv_obj_set_hidden(para.ui.file_name, true);
			lv_obj_set_hidden(para.ui.download, true);
			lv_obj_set_hidden(para.ui.container_1, false);
			lv_obj_set_hidden(para.ui.label_file_size, false);
			lv_obj_set_hidden(para.ui.label_file_time, false);
			lv_btn_set_state(para.ui.play, LV_BTN_STATE_REL);
			ui_status = NONE_STATUS;
		}else{
			player_ui->auto_play_enable = 0;
			//destory_page(PAGE_MOVIE);
			//create_page(PAGE_HOME);
			switch_page(PAGE_MOVIE, PAGE_HOME);
		}
	}
}
static void btn_online_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		media_set_online_mode(1);
	}
}
static void switch_to_page(lv_obj_t * obj, bool en)
{
	lv_obj_set_hidden(para.ui.ab_page, true);
	lv_obj_set_hidden(para.ui.ratio_page, true);
	lv_obj_set_hidden(para.ui.sound_track_page, true);
	lv_obj_set_hidden(para.ui.audio_track_list, true);
	lv_obj_set_hidden(obj, en);
}
static void btn_menu_event(lv_obj_t * btn, lv_event_t event)
{
	static int once = 0;
	if (event == LV_EVENT_CLICKED){
		if (once == 0){
			lv_obj_set_hidden(para.ui.menu_page, false);
			once = 1;
		}else{
			switch_to_page(para.ui.menu_page, true);
			once = 0;
		}
	}
}

static void btn_menu_exit_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		switch_to_page(para.ui.menu_page, true);
	}
}

static void btn_ab_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();

	if (event == LV_EVENT_CLICKED){
		player_ui->ab_play.loop = 0;
		player_ui->ab_play.enable = 0;
		media_set_progressbar(para.ui.slider_a, player_ui->ab_play.a_point,
			player_ui->play_info.nDurationSec);
		media_set_progressbar(para.ui.slider_b, player_ui->ab_play.b_point,
			player_ui->play_info.nDurationSec);

		media_set_time(para.ui.label_a, player_ui->ab_play.a_point);
		media_set_time(para.ui.label_b, player_ui->ab_play.b_point);
		switch_to_page(para.ui.ab_page, false);
		media_ui_send_event(MEDIA_PAUSE_EVENT, NULL, 0);
	}
}

static void btn_ab_ok_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();

	if (event == LV_EVENT_CLICKED){
		player_ui->ab_play.a_point = media_bar_value_to_time(para.ui.slider_a);
		player_ui->ab_play.b_point = media_bar_value_to_time(para.ui.slider_b);
		player_ui->ab_play.loop = 1;
		player_ui->ab_play.enable = 1;
		lv_obj_set_hidden(para.ui.ab_page, true);
	}
}

static void btn_ab_esc_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();

	if (event == LV_EVENT_CLICKED){
		player_ui->ab_play.enable = 0;
		lv_obj_set_hidden(para.ui.ab_page, true);
	}
}
static void btn_ratio_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		switch_to_page(para.ui.ratio_page, false);
	}
}

static void btn_video_ratio_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		vide_set_win_ratio(0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX, CEDAR_VID_WINDOW_BESTSHOW);
		lv_obj_set_hidden(para.ui.ratio_page, true);
	}
}

static void btn_screen_ratio_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		vide_set_win_ratio(0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX, CEDAR_VID_WINDOW_FULLSCN);
		lv_obj_set_hidden(para.ui.ratio_page, true);
	}
}

static void btn_original_ratio_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		vide_set_win_ratio(0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX, CEDAR_VID_WINDOW_ORIGINAL);
		lv_obj_set_hidden(para.ui.ratio_page, true);
	}
}

static void btn_43_ratio_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		vide_set_win_ratio(0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX, CEDAR_VID_WINDOW_4R3MODE);
		lv_obj_set_hidden(para.ui.ratio_page, true);
	}
}

static void btn_169_ratio_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		vide_set_win_ratio(0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX, CEDAR_VID_WINDOW_16R9MODE);
		lv_obj_set_hidden(para.ui.ratio_page, true);
	}
}

static void btn_sound_track_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		switch_to_page(para.ui.sound_track_page, false);
	}
}

static void btn_stereo_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		player_ui_t *player_ui = media_get_player_data();
		tplayer_switch_soundchannel(player_ui->tplayer, STEREO);
		lv_obj_set_hidden(para.ui.sound_track_page, true);
	}
}

static void btn_left_channel_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		player_ui_t *player_ui = media_get_player_data();
		tplayer_switch_soundchannel(player_ui->tplayer, LEFT_CHANNEL);
		lv_obj_set_hidden(para.ui.sound_track_page, true);
	}
}

static void btn_right_channel_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		player_ui_t *player_ui = media_get_player_data();
		tplayer_switch_soundchannel(player_ui->tplayer, RIGHT_CHANNEL);
		lv_obj_set_hidden(para.ui.sound_track_page, true);
	}
}

static void btn_audio_track_switch_event(lv_obj_t * btn, lv_event_t event)
{
	int audio_num;
	audio_num = lv_list_get_btn_index(para.ui.audio_track_list, btn);
	if (event == LV_EVENT_CLICKED){
		player_ui_t *player_ui = media_get_player_data();
		tplayer_switch_audio(player_ui->tplayer, audio_num);
		switch_to_page(para.ui.audio_track_list, true);
	}
}

static void btn_audio_track_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t *player_ui = media_get_player_data();
	if (event == LV_EVENT_CLICKED){
		if (player_ui->tplayer->mpstatus >= PREPARED_STATUS){
			lv_obj_t *btn;
			int audio_track_num;
			char btn_name[10];
			int i;

			audio_track_num = player_ui->tplayer->mMediaInfo->nAudioStreamNum;
			lv_list_clean(para.ui.audio_track_list);
			for (i = 0; i < audio_track_num; i++)
			{
				memset(btn_name, 0, sizeof(btn_name));
				sprintf(btn_name, "track-%d", i);
				btn = lv_list_add_btn(para.ui.audio_track_list, NULL, btn_name);
				lv_obj_set_event_cb(btn, btn_audio_track_switch_event);
			}
		}
		switch_to_page(para.ui.audio_track_list, false);
	}
}

static void media_list_event(lv_obj_t * btn, lv_event_t event)
{
	int index;
	static int pre_index = INT_MAX;
	char size_str[25] = {0};
	char size[20] = {0};
	char time_str[25] = {0};
	char time[20] = {0};
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();

	index = lv_list_get_btn_index(para.ui.media_list, btn);
	if (event == LV_EVENT_CLICKED) {
		if (pre_index != index) {
			pre_index = index;

			media_set_list_focus(para.ui.media_list, index);

			media_get_file_time(player_ui->media_list, index, time);
			sprintf(time_str, "time %s", time);
			lv_label_set_text(para.ui.label_file_time, time_str);

			media_get_file_size(player_ui->media_list, index, size);
			sprintf(size_str, "size %s", size);
			lv_label_set_text(para.ui.label_file_size,size_str);
		} else if (pre_index == index) {
			player_ui_t *player_ui = media_get_player_data();

			rat_npl_set_cur(player_ui->media_list->media_hrat, index);
			media_ui_send_event(MEDIA_PLAY_EVENT, NULL, index);
		}
	}
}

static void btn_list_order_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		lv_obj_set_hidden(para.ui.media_list, false);
		lv_obj_set_hidden(para.ui.order, true);
		lv_obj_set_hidden(para.ui.file_name, true);
		lv_obj_set_hidden(para.ui.online, false);

		media_ui_send_event(MEDIA_PAUSE_EVENT, NULL, 0);
		lv_btn_set_state(para.ui.play, LV_BTN_STATE_PR);
		ui_status = LIST_STATUS;
	}
}

static void btn_full_button_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();

	if (event == LV_EVENT_CLICKED){
		switch(ui_status){
			case FULL_STATUS:
				lv_obj_set_hidden(para.ui.media_list, true);
				lv_obj_set_hidden(para.ui.online, true);
				lv_obj_set_hidden(para.ui.order, false);
				lv_obj_set_hidden(para.ui.file_name, false);
				lv_obj_set_hidden(para.ui.container_1, false);
				ui_status = BAR_STATUS;
				break;
			case LIST_STATUS:
				lv_obj_set_hidden(para.ui.media_list, true);
				lv_obj_set_hidden(para.ui.online, true);
				lv_obj_set_hidden(para.ui.order, false);
				lv_obj_set_hidden(para.ui.file_name, false);
				lv_obj_set_hidden(para.ui.container_1, false);
				media_ui_send_event(MEDIA_PLAY_EVENT, NULL, player_ui->play_info.index);
				lv_btn_set_state(para.ui.play, LV_BTN_STATE_TGL_REL);
				ui_status = BAR_STATUS;
				break;
			case BAR_STATUS:
				lv_obj_set_hidden(para.ui.media_list, true);
				lv_obj_set_hidden(para.ui.online, true);
				lv_obj_set_hidden(para.ui.order, true);
				lv_obj_set_hidden(para.ui.container_1, true);
				lv_obj_set_hidden(para.ui.file_name, true);
				switch_to_page(para.ui.menu_page, true);
				ui_status = FULL_STATUS;
				break;
			default:
				break;
		}
	}
}

static void media_next_last_event(lv_obj_t * btn, lv_event_t event)
{
	if(event == LV_EVENT_CLICKED){
		int index = -1;
		player_ui_t *player_ui = media_get_player_data();
		if (para.ui.next == btn)
			index = rat_npl_get_next(player_ui->media_list->media_hrat);
		else if(para.ui.last == btn)
			index = rat_npl_get_prev(player_ui->media_list->media_hrat);
		com_info("play state %d", lv_btn_get_state(para.ui.play));
		media_ab_set_enable(0);
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

static void mbox_event_cb(lv_obj_t *obj, lv_event_t event)
{
	if (obj == para.mbox_video_morethan_720P)
	{
		if(event == LV_EVENT_DELETE)
		{
            player_ui_t *player_ui = media_get_player_data();
			int index = -1;

            index = rat_npl_get_next(player_ui->media_list->media_hrat);
            media_ab_set_enable(0);
            media_ui_send_event(MEDIA_PLAY_EVENT, NULL, index);
            lv_obj_set_click(lv_layer_top(), false);
		}
	}
	else if (obj == para.mbox_file_is_null)
	{
		if(event == LV_EVENT_DELETE)
		{
            switch_page(PAGE_MOVIE, PAGE_HOME);
            lv_obj_set_click(lv_layer_top(), false);
		}
	}
}

static void media_player_ui_callback(void *ui_player, media_event_t event, void *param)
{
	player_ui_t *player_ui = (player_ui_t *)ui_player;
	media_file_list_t *media_list = NULL;

	if(player_ui == NULL){
		return;
	}

	switch(event){
		case MEDIA_IDLE_EVENT:
			if (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
				media_set_progressbar(para.ui.progressbar, player_ui->play_info.time, player_ui->play_info.nDurationSec);
				media_set_time(para.ui.total_time, player_ui->play_info.nDurationSec);
				media_set_time(para.ui.curr_time, player_ui->play_info.time);
			} else if (tplayer_get_status(player_ui->tplayer) == STOP_STATUS){
				media_set_progressbar(para.ui.progressbar, 0, 0);
				media_set_time(para.ui.total_time, 0);
				media_set_time(para.ui.curr_time, 0);
				// printf("%s %d %s play_status:%d\n", __FILE__, __LINE__, __func__, tplayer_get_status(player_ui->tplayer));
			}
/*
			if(lv_obj_get_hidden(para.ui.ab_page) == false){
				media_set_time(para.ui.label_a, media_bar_value_to_time(para.ui.slider_a));
				media_set_time(para.ui.label_b, media_bar_value_to_time(para.ui.slider_b));
			}
			media_A_to_B_play(para.ui.button_ab, &player_ui->ab_play);
*/
			break;
		case MEDIA_UPDATE_LIST_EVENT:
			media_list = media_get_file_list(RAT_MEDIA_TYPE_VIDEO);
			if (media_list != NULL) {
				media_unload_file(player_ui->media_list);
				player_ui->media_list = media_list;
				media_update_file_list(para.ui.media_list, player_ui->media_list, media_list_event);
				media_set_list_focus(para.ui.media_list, player_ui->play_info.index);
				media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			}
			break;
		case MEDIA_PLAY_EVENT:
			lv_label_set_text(para.ui.file_name, player_ui->play_info.filename);
			media_set_list_focus(para.ui.media_list, player_ui->play_info.index);
			clean_screen(&para.ui);
			lv_btn_set_state(para.ui.play, LV_BTN_STATE_TGL_REL);
			lv_obj_set_hidden(para.ui.media_list, true);
			lv_obj_set_hidden(para.ui.online, true);
			lv_obj_set_hidden(para.ui.image_4, true);
			lv_obj_set_hidden(para.ui.label_file_size, true);
			lv_obj_set_hidden(para.ui.label_file_time, true);
			lv_obj_set_hidden(para.ui.order, false);
			lv_obj_set_hidden(para.ui.file_name, false);
			ui_status = BAR_STATUS;
			lv_obj_set_hidden(para.ui.download, false);
			media_ui_send_event(MEDIA_DOWNLOAD_EVENT, NULL, 0);
			break;
		case MEDIA_PAUSE_EVENT:
			lv_btn_set_state(para.ui.play, LV_BTN_STATE_REL);
			break;
		case MEDIA_DOWNLOAD_EVENT:
			if(!media_downloading(para.ui.download)){
				lv_obj_set_hidden(para.ui.download, true);
				media_ui_send_event(MEDIA_LOAD_LRC_EVENT, NULL, 0);
			}
			break;
		case MEDIA_LOAD_LRC_EVENT:
			media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			break;
		case MEDIA_LIST_LOOP_EVENT:
			media_ab_set_enable(0);
			lv_list_up(para.ui.media_list);
			break;
		case MEDIA_PLAY_COMPLETE_EVENT:
			lv_btn_set_state(para.ui.play, LV_BTN_STATE_REL);
			lv_slider_set_value(para.ui.progressbar, lv_slider_get_max_value(para.ui.progressbar), LV_ANIM_OFF);

			player_ui = media_get_player_data();
			media_ui_send_event(MEDIA_PLAY_EVENT, NULL, rat_npl_get_next(player_ui->media_list->media_hrat));
			media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			break;
		case MEDIA_SEEKTO_EVENT:
			media_set_progressbar(para.ui.progressbar, player_ui->break_tag.offset, player_ui->break_tag.nDurationSec);
			media_set_time(para.ui.total_time, player_ui->break_tag.nDurationSec);
			media_set_time(para.ui.curr_time, player_ui->break_tag.offset);
			media_set_list_focus(para.ui.media_list, player_ui->break_tag.index);
			lv_label_set_text(para.ui.file_name, player_ui->break_tag.filename);
			break;
		case MEDIA_MBOX_EVENT:
			para.mbox_video_morethan_720P = media_mbox_create("more than 720P, play next video!", 1500, mbox_event_cb);
			break;
		default:
			media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			break;
	}
}

static void media_play_mode_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		para.mbox_play_mod_reminder = create_mbox_reminder_playmode_change(para.ui.play_mode);
	}
}

#if CONFIG_FONT_ENABLE
static void movie_label_text_init(movie_para_t *para)
{
	static lv_style_t style0_label_1;
	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.line_space = 2;
	style0_label_1.text.font = get_font_lib()->msyh_16;

	lv_label_set_text(para->ui.label_file_size, get_text_by_id(LANG_MOVIE_SIZE));
	lv_label_set_text(para->ui.label_file_time, get_text_by_id(LANG_MOVIE_TIME));
	lv_label_set_text(para->ui.label_online, get_text_by_id(LANG_MOVIE_ONLINE));
	lv_label_set_text(para->ui.label_menu, get_text_by_id(LANG_MOVIE_MENU));
	lv_label_set_text(para->ui.label_menu_esc, get_text_by_id(LANG_MOVIE_ESC));

	lv_label_set_text(para->ui.label_AB_OK, get_text_by_id(LANG_MOVIE_OK));
	lv_label_set_text(para->ui.label_AB_ESC, get_text_by_id(LANG_MOVIE_ESC));

	lv_label_set_text(para->ui.label_ratio, get_text_by_id(LANG_MOVIE_RATIO));
	lv_label_set_text(para->ui.label_ratio_video, get_text_by_id(LANG_MOVIE_VID_RATIO));
	lv_label_set_text(para->ui.label_ratio_screen, get_text_by_id(LANG_MOVIE_SCN_RATIO));
	lv_label_set_text(para->ui.label_ratio_original, get_text_by_id(LANG_MOVIE_ORG_RATIO));

	lv_label_set_text(para->ui.label_sound_channel, get_text_by_id(LANG_MOVIE_SOUND));
	lv_label_set_text(para->ui.label_sound_stereo, get_text_by_id(LANG_MOVIE_STEREO));
	lv_label_set_text(para->ui.label_sound_left, get_text_by_id(LANG_MOVIE_L_SOUND));
	lv_label_set_text(para->ui.label_sound_right, get_text_by_id(LANG_MOVIE_R_SOUND));

	lv_label_set_text(para->ui.label_audio_track, get_text_by_id(LANG_MOVIE_AUDIO_TRACK));

	lv_label_set_style(para->ui.label_file_size, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_file_time, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_online, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_menu, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_AB_ESC, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_AB_OK, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_ratio, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_ratio_video, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_ratio_screen, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_ratio_original, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_ratio_43, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_ratio_169, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_sound_stereo, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_sound_channel, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_sound_left, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_sound_right, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_audio_track, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_menu_esc, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_AB, LV_LABEL_STYLE_MAIN, &style0_label_1);
}
#endif
static int create_movie(void)
{
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();
	memset(&player_ui->ab_play, 0, sizeof(player_ui->ab_play));

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

#if 1
	lv_obj_t *back_btn = lv_btn_create(para.ui.cont, NULL);
	lv_obj_align(back_btn, para.ui.cont, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_t *back_lable = lv_label_create(back_btn, NULL);
	lv_label_set_text(back_lable, LV_SYMBOL_LEFT);
	lv_obj_set_event_cb(back_btn, back_btn_event);
	back_btn_style.body.opa = 0;
	back_btn_style.body.border.opa = 0;
	lv_btn_set_style(back_btn, LV_BTN_STYLE_REL, &back_btn_style);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_PR, &back_btn_style);
#endif
	movie_auto_ui_create(&para.ui);
	lv_obj_move_background(para.ui.full_button);

#if CONFIG_FONT_ENABLE
    movie_label_text_init(&para);
#endif

	//lv_list_set_layout(para.ui.media_list, LV_LAYOUT_ROW_B);
	lv_obj_set_event_cb(para.ui.online, btn_online_event);
	lv_obj_set_event_cb(para.ui.full_button, btn_full_button_event);
	lv_obj_set_event_cb(para.ui.order, btn_list_order_event);
	lv_obj_set_event_cb(para.ui.play, media_play_event);
	lv_obj_set_event_cb(para.ui.next, media_next_last_event);
	lv_obj_set_event_cb(para.ui.last, media_next_last_event);
	lv_obj_set_event_cb(para.ui.volume_bar, media_volume_bar_event);
	lv_obj_set_event_cb(para.ui.volume, media_volume_event);
	lv_obj_set_event_cb(para.ui.progressbar, media_progressbar_event);

	lv_obj_set_event_cb(para.ui.button_menu, btn_menu_event);
	lv_obj_set_event_cb(para.ui.button_exit, btn_menu_exit_event);

	lv_obj_set_event_cb(para.ui.button_ab, btn_ab_event);
	lv_obj_set_event_cb(para.ui.button_ok, btn_ab_ok_event);
	lv_obj_set_event_cb(para.ui.button_esc, btn_ab_esc_event);

	lv_obj_set_event_cb(para.ui.button_ratio, btn_ratio_event);
	lv_obj_set_event_cb(para.ui.button_video_ratio, btn_video_ratio_event);
	lv_obj_set_event_cb(para.ui.button_screen_ratio, btn_screen_ratio_event);
	lv_obj_set_event_cb(para.ui.button_original_ratio, btn_original_ratio_event);
	lv_obj_set_event_cb(para.ui.button_43_ratio, btn_43_ratio_event);
	lv_obj_set_event_cb(para.ui.button_169_ratio, btn_169_ratio_event);

	lv_obj_set_event_cb(para.ui.button_sound_track, btn_sound_track_event);
	lv_obj_set_event_cb(para.ui.button_stereo, btn_stereo_event);
	lv_obj_set_event_cb(para.ui.button_left_channel, btn_left_channel_event);
	lv_obj_set_event_cb(para.ui.button_right_channel, btn_right_channel_event);

	lv_obj_set_event_cb(para.ui.button_audio_track, btn_audio_track_event);
	if (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
		media_ui_send_event(MEDIA_PAUSE_EVENT, NULL, 0);
	}
	auto_close_screen_timer_disable();
	/*get breaktag*/
	if (player_ui->clicked_form_explorer == 0) {
		media_func_get_breaktag(MOVIE_SCENE, &player_ui->break_tag);
	}
	player_ui->clicked_form_explorer = 0;
	/*get video file list*/
	if (player_ui->media_list == NULL) {
		player_ui->media_list = media_get_file_list(RAT_MEDIA_TYPE_VIDEO);
	}
	if (player_ui->media_list == NULL) {
#if CONFIG_FONT_ENABLE
		para.mbox_file_is_null = media_mbox_create(get_text_by_id(LANG_MOVIE_NO_FILE), 1500, mbox_event_cb);
#else
		para.mbox_file_is_null = media_mbox_create("video file is null!", 1500, mbox_event_cb);
#endif
		return -1;
	}
	media_update_file_list(para.ui.media_list, player_ui->media_list, media_list_event);
	if (media_get_playinfo_by_breakpoint(player_ui->media_list, &player_ui->break_tag) == 0) {
		player_ui->break_vaild = 1;
	}
	player_ui->auto_play_enable = 1;
	memset(&player_ui->play_info, 0x00, sizeof(play_info_t));
	memcpy(&player_ui->play_info, &player_ui->break_tag, sizeof(play_info_t));
	media_set_play_file_index(player_ui->media_list, player_ui->play_info.index);

	media_func_register(MOVIE_SCENE, media_player_ui_callback);
	media_config_init(para.ui.play_mode, para.ui.volume_bar, media_play_mode_event);
	if (player_ui->break_vaild) {
		player_ui->break_vaild = 0;
		if (player_ui->auto_play_enable) {
			player_ui->auto_play_enable = 0;
			media_ui_send_event(MEDIA_PLAY_EVENT, NULL, player_ui->break_tag.index);
		} else {
			media_ui_send_event(MEDIA_PREPARE_EVENT, NULL, player_ui->break_tag.index);
		}
		media_ui_send_event(MEDIA_SEEKTO_EVENT, NULL, player_ui->break_tag.offset);
	} else {
		if (player_ui->auto_play_enable) {
			player_ui->auto_play_enable = 0;
			media_ui_send_event(MEDIA_PLAY_EVENT, NULL, media_get_play_file_index(player_ui->media_list));
		}
	}
	ui_status = NONE_STATUS;

	return 0;
}

static int destory_movie(void)
{
	media_config_deinit(MOVIE_SCENE);
	auto_close_screen_timer_enable();
	media_func_unregister(MOVIE_SCENE, 0);
	movie_auto_ui_destory(&para.ui);
	lv_obj_del(para.ui.cont);
	return 0;
}

static int show_movie(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);

	return 0;
}

static int hide_movie(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);
	return 0;
}

static int msg_proc_movie(MsgDataInfo *msg)
{
	return 0;
}

static page_interface_t page_movie =
{
	.ops =
	{
		create_movie,
		destory_movie,
		show_movie,
		hide_movie,
		msg_proc_movie,
	},
	.info =
	{
		.id         = PAGE_MOVIE,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_MOVIE(void)
{
	reg_page(&page_movie);
}
