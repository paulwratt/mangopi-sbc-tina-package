#include "smt_config.h"
#include "media_ui_mixture.h"

static ui_image_t play_mod_iamge[RAT_PLAY_MODE_MAX] = {
											{NULL, LV_IMAGE_PATH"all_cycle.png"},
											{NULL, LV_IMAGE_PATH"single_cycle.png"},
											{NULL, LV_IMAGE_PATH"sequential_play.png"},
											{NULL, LV_IMAGE_PATH"single_play.png"},
											{NULL, LV_IMAGE_PATH"random_play.png"}
										};
#if CONFIG_FONT_ENABLE
static int playmode_string_id[RAT_PLAY_MODE_MAX] = {LANG_MOVIE_PLAY_ONE, LANG_MOVIE_CYC_ONE, LANG_MOVIE_CYC_ALL, LANG_MOVIE_CYC_SEQ, LANG_MOVIE_RANDOM};
#else
static char *playmode_string[RAT_PLAY_MODE_MAX] = {"ONLY_ONCE", "ROTATE_ONE", "ROTATE_ALL", "SEQUENCE", "RANDOM"};
#endif

lv_obj_t* media_mbox_create(const char* message, unsigned int time, lv_event_cb_t event_cb)
{
#if CONFIG_FONT_ENABLE
    static lv_style_t mbox_style;
#endif

	lv_obj_t* mbox = NULL;
	lv_obj_set_click(lv_layer_top(), true);
	mbox = (lv_obj_t*)lv_mbox_create(lv_layer_top(), NULL);

#if CONFIG_FONT_ENABLE
	lv_style_copy(&mbox_style, &lv_style_pretty);
    mbox_style.text.font = get_font_lib()->msyh_16;
    lv_mbox_set_style(mbox, LV_MBOX_STYLE_BG, &mbox_style);
#endif

	lv_mbox_set_text(mbox, message);
	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(mbox, event_cb);
	lv_mbox_start_auto_close(mbox, time);

	return mbox;
}

void media_play_mod_icon_destory(void)
{
	free_image_buff_form_list(play_mod_iamge, sizeof(play_mod_iamge)/sizeof(play_mod_iamge[0]));
}

int media_update_playmode_btn(lv_obj_t*btn_obj, rat_play_mode_e play_mode) {
	void *image_buff = NULL;
	lv_btn_state_t state = 0;

	if (btn_obj == NULL || play_mode < RAT_PLAY_MODE_ONLY_ONCE || play_mode > RAT_PLAY_MODE_RANDOM) {
		com_err("param error");
		return -1;
	}

	image_buff = get_image_buff_form_list(play_mod_iamge, sizeof(play_mod_iamge)/sizeof(play_mod_iamge[0]), play_mode);
	for (state = LV_BTN_STATE_REL; state <= LV_BTN_STATE_TGL_PR; state++) {
		lv_imgbtn_set_src(btn_obj, state, image_buff);
	}
	return 0;
}

static void mbox_reminder_playmode_change_event(lv_obj_t * btn, lv_event_t event)
{
	if(event == LV_EVENT_DELETE){
		lv_obj_set_click(lv_layer_top(), false);
	}
}

lv_obj_t *create_mbox_reminder_playmode_change(lv_obj_t*btn_obj)
{
	lv_obj_t *obj = NULL;
	player_ui_t *player_ui = media_get_player_data();

	if (NULL == btn_obj) {
		com_err("param error");
		return NULL;
	}

	player_ui->media_cfg.play_mode = (player_ui->media_cfg.play_mode >= RAT_PLAY_MODE_RANDOM) ? RAT_PLAY_MODE_ONLY_ONCE : player_ui->media_cfg.play_mode+1;
	com_info("play_mode = %d", player_ui->media_cfg.play_mode);
	rat_npl_set_play_mode(player_ui->media_list->media_hrat, player_ui->media_cfg.play_mode);
	media_update_playmode_btn(btn_obj, player_ui->media_cfg.play_mode);
#if CONFIG_FONT_ENABLE
	obj = media_mbox_create(get_text_by_id(playmode_string_id[player_ui->media_cfg.play_mode]), 1500, mbox_reminder_playmode_change_event);
#else
	obj = media_mbox_create(playmode_string[player_ui->media_cfg.play_mode], 1500, mbox_reminder_playmode_change_event);
#endif
	return obj;
}
