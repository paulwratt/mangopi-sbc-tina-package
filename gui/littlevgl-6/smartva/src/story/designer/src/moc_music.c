/**********************
 *      includes
 **********************/
#include "moc_music.h"
#include "ui_music.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "media_load_file.h"
#include "player_int.h"
#include "media_mixture.h"
#include "media_ui_mixture.h"
#include "music_lrc.h"
#include "media_spectrum.h"
#include "va_image.h"
#include "common.h"
#include "bs_widget.h"
#include "app_config_interface.h"

/**********************
 *       variables
 **********************/
typedef struct
{
	music_ui_t ui;
	lv_obj_t* mbox_file_is_null;
	lv_obj_t* mbox_reminder;
} music_para_t;
static music_para_t para;

/**********************
 *  functions
 **********************/

static music_lrc_info lrc_info;

#define SPECTRUM_NUM 19
lv_obj_t *spectrum_ui[SPECTRUM_NUM];
void *spectrum1_particle_png = NULL;

lv_style_t * lv_style_lrc;
lv_style_t lv_style_default_lrc;
lv_style_t lv_style_line_lrc;

#define min(x, y)   ((x) <= (y) ? (x) : (y))
#define max(x, y)   ((x) <= (y) ? (y) : (x))

static int media_load_lrc(player_ui_t * player_ui)
{

	FILE *lrc = NULL;
	char lrc_name[FILE_NAME_LEN];
	char lrc_path[FILE_PATH_LEN];
	char name[96];
	char suffix[] = "lrc";
	char *file_name;
	int i;
	int ret;

	memset(&lrc_info, 0, sizeof(lrc_info));
//match lrc name
	file_name = player_ui->play_info.filename;
	memset(lrc_name, 0, sizeof(lrc_name));
	memset(name, 0, sizeof(name));
	for(i=0; i<sizeof(name); i++ ){
		if(file_name[i] == '.'){
			break;
		}
		name[i] = file_name[i];
	}
	if(i == FILE_NAME_LEN-1){
		goto END;
	}
	sprintf(lrc_name, "%s.%s", name, suffix);
	com_info("match lrc name is %s", lrc_name);

//find lrc
	memset(lrc_path, 0, sizeof(lrc_path));
	ret = find_music_lrc(player_ui->play_info.root_path, lrc_name, lrc_path);
	if(ret < 0){
		com_warn("don't find lrc:%s",lrc_name);
		goto END;
	}
//load lrc
	com_info("open %s", lrc_path);
	lrc = fopen(lrc_path, "rb");
	if(!lrc){
		goto END;
	}
	fseek(lrc, 0, SEEK_END);
	lrc_info.lrc_size = ftell(lrc);
	fseek(lrc, 0, SEEK_SET);

	if(lrc_info.lrc_size <=0 ){
		com_err("lrc size = %d", lrc_info.lrc_size);
		goto END;
	}

	lrc_info.lrc_cache = malloc(lrc_info.lrc_size);
	if(!lrc_info.lrc_cache){
		com_err("lrc malloc failed!!!");
		goto END;
	}

	ret = fread(lrc_info.lrc_cache, 1, lrc_info.lrc_size, lrc);
	if(ret != lrc_info.lrc_size){
		com_err("lrc read failed!!!");
		free(lrc_info.lrc_cache);
		goto END;
	}

	lrc_info.start = parse_lrc_get_tag(lrc_info.lrc_cache, &lrc_info);
	if(!lrc_info.start){
		goto END;
	}
	lrc_info.end = lrc_info.lrc_cache + lrc_info.lrc_size;
	lrc_info.list_len = parse_lrc_get_time_list(&lrc_info);
	if(lrc_info.lrc_cache){
		free(lrc_info.lrc_cache);
	}
	lv_ta_set_one_line(para.ui.lrc, true);
	lv_ta_set_text(para.ui.lrc, "");

#if 0
//show lrc
	unsigned int curr_line = 0;
	unsigned int time_list[256];
	char text_list[256][STRLEN];
//resort by time
	curr_line = parse_lrc_get_current_line(0, &lrc_info);
	for(i = 0; i < lrc_info.list_len; i++){
		time_list[i] = lrc_info.time_list[curr_line];
		memcpy(text_list[i], lrc_info.text_list[curr_line], STRLEN);
		curr_line = parse_lrc_get_next_line(curr_line, &lrc_info);
	}

	if(lv_list_get_size(para.ui.lrc_list) > 0){
		lv_list_clean(para.ui.lrc_list);
	}

	for(i = 0; i < lrc_info.list_len; i++){
		lrc_info.time_list[i] = time_list[i];
		memcpy(lrc_info.text_list[i], text_list[i], STRLEN);

		lv_list_add_btn(para.ui.lrc_list, NULL, lrc_info.text_list[i]);
		//lv_ta_add_text(para.ui.lrc, lrc_info.text_list[i]);
	}
	lv_obj_t *focus_btn;
	for(i = 0; i < lrc_info.list_len; i++){
		focus_btn = media_get_focus_btn(para.ui.lrc_list, i);
		lv_label_set_align(lv_list_get_btn_label(focus_btn), LV_LABEL_ALIGN_CENTER);
	}

	lv_style_lrc = lv_label_get_style(lv_list_get_btn_label(focus_btn), LV_LABEL_STYLE_MAIN);
	memcpy(&lv_style_default_lrc, lv_style_lrc , sizeof(lv_style_default_lrc));
	memcpy(&lv_style_line_lrc, lv_style_lrc , sizeof(lv_style_line_lrc));
	lv_style_line_lrc.text.color = lv_color_hex(0xffff7f);
	lv_style_line_lrc.text.sel_color = lv_color_hex(0x5596d8);
	lv_style_line_lrc.text.font = &lv_font_roboto_28;

#endif

	if(lrc)
		fclose(lrc);
	return 1;
END:
	if(lrc)
		fclose(lrc);
	return 0;
}

static void media_update_lrc(int time)
{
#if	0
	static unsigned int curr_line = 0;
	static lv_obj_t *focus_btn = NULL;

	for(int i=0; i < lrc_info.list_len; i++){
		if(time > lrc_info.time_list[i]){
			curr_line = i;
			break;
		}
	}

	if(!focus_btn){
		lv_label_set_style(lv_list_get_btn_label(focus_btn), LV_LABEL_STYLE_MAIN, &lv_style_default_lrc);
	}

	focus_btn = media_get_focus_btn(para.ui.lrc_list, curr_line);
	lv_label_set_style(lv_list_get_btn_label(focus_btn), LV_LABEL_STYLE_MAIN, &lv_style_line_lrc);


	lv_list_focus(focus_btn, LV_ANIM_OFF);
	//lv_btn_set_state(focus_btn, LV_BTN_STATE_REL);
#else
	unsigned int curr_line = 0;
	static unsigned int last_line = 0;

	curr_line = parse_lrc_get_current_line(time, &lrc_info);
	if(curr_line != last_line){
		last_line = curr_line;
		com_info("time=%d %d: %s ",time, lrc_info.time_list[curr_line], lrc_info.text_list[curr_line]);
		lv_ta_set_text(para.ui.lrc, lrc_info.text_list[curr_line]);
		//lv_ta_add_text(para.ui.lrc, lrc_info.text_list[last_line]);
	}
#endif
}

static void mbox_event_cb(lv_obj_t *obj, lv_event_t event)
{
	if (obj == para.mbox_file_is_null)
	{
		if(event == LV_EVENT_DELETE)
		{
            switch_page(PAGE_MUSIC, PAGE_HOME);
            lv_obj_set_click(lv_layer_top(), false);
		}
	}
}

static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_MUSIC);
		//create_page(PAGE_HOME);
		switch_page(PAGE_MUSIC, PAGE_HOME);
	}
}

static void media_list_event(lv_obj_t * btn, lv_event_t event)
{
	int index;
	index = lv_list_get_btn_index(para.ui.media_list, btn);
	if (event == LV_EVENT_CLICKED){
		player_ui_t *player_ui = media_get_player_data();

		rat_npl_set_cur(player_ui->media_list->media_hrat, index);
		media_ui_send_event(MEDIA_PLAY_EVENT, NULL, index);
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
		com_info("play state %d", lv_btn_get_state(para.ui.btn_play));
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

static void btn_online_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		media_set_online_mode(1);
	}
}

static void menu_page_show_submenu(lv_obj_t * obj)
{
	lv_obj_set_hidden(para.ui.list_sound_effect, true);
	lv_obj_set_hidden(para.ui.list_song_information, true);
	lv_obj_set_hidden(para.ui.ab_page, true);
	if (NULL != obj)
		lv_obj_set_hidden(obj, false);
}

static void btn_menu_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		lv_obj_set_hidden(para.ui.page_menu, false);
	}
}

static void btn_page_menu_exit_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		lv_obj_set_hidden(para.ui.page_menu, true);
		menu_page_show_submenu(NULL);
	}
}

static void btn_list_sound_effect_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		menu_page_show_submenu(para.ui.list_sound_effect);
	}
}

static void btn_sound_eq_event(lv_obj_t * btn, lv_event_t event)
{
	int index;
	index = lv_list_get_btn_index(para.ui.list_sound_effect, btn);
	if (event == LV_EVENT_CLICKED){
		media_ui_send_event(MEDIA_SET_AUDIO_EQ, NULL, index);
		lv_obj_set_hidden(para.ui.list_sound_effect, true);
	}
}

static void btn_list_song_information_exit_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		lv_obj_set_hidden(para.ui.list_song_information, true);
	}
}

static void btn_list_song_information_event(lv_obj_t * btn, lv_event_t event)
{
#if CONFIG_FONT_ENABLE
	static lv_style_t style0_label_1;
	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.font = get_font_lib()->msyh_16;
#endif

	lv_obj_t *list_bnt;
	lv_obj_t *list_label;
	char info[74] = {0};
	char time_str[64] = {0};
	char size_str[64] = {0};

	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();

	if (event == LV_EVENT_CLICKED){
		if (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS){
			menu_page_show_submenu(para.ui.list_song_information);
			lv_list_clean(para.ui.list_song_information);

			//size
			file_size_int_to_string(player_ui->tplayer->mMediaInfo->nFileSize, size_str);
			#if CONFIG_FONT_ENABLE
			sprintf (info, "%s: %s", get_text_by_id(LANG_MUSIC_SIZE), size_str);
			#else
			sprintf (info, "%s: %s", "size", size_str);
			#endif
			list_bnt = lv_list_add_btn(para.ui.list_song_information, NULL, info);
			list_label = lv_list_get_btn_label(list_bnt);
			lv_label_set_long_mode(list_label,LV_LABEL_LONG_EXPAND);
#if CONFIG_FONT_ENABLE
			lv_list_set_style(list_label, LV_LABEL_STYLE_MAIN, &style0_label_1);
#endif
			//time
			time_int_to_string(player_ui->tplayer->mMediaInfo->nDurationMs, time_str);
#if CONFIG_FONT_ENABLE
			sprintf (info, "%s: %s", get_text_by_id(LANG_MUSIC_TIME), time_str);
#else
			sprintf (info, "%s: %s", "time", time_str);
#endif
			list_bnt = lv_list_add_btn(para.ui.list_song_information, NULL, info);
			list_label = lv_list_get_btn_label(list_bnt);
			lv_label_set_long_mode(list_label,LV_LABEL_LONG_EXPAND);
#if CONFIG_FONT_ENABLE
			lv_list_set_style(list_label, LV_LABEL_STYLE_MAIN, &style0_label_1);
#endif
			//album
			if (NULL == player_ui->tplayer->mMediaInfo->Id3Info->album){
#if CONFIG_FONT_ENABLE
				sprintf (info, "%s: %s", get_text_by_id(LANG_MUSIC_ALBUM), player_ui->tplayer->mMediaInfo->Id3Info->album);
#else
				sprintf (info, "%s: %s", "album", player_ui->tplayer->mMediaInfo->Id3Info->album);
#endif
				list_bnt = lv_list_add_btn(para.ui.list_song_information, NULL, info);
				list_label = lv_list_get_btn_label(list_bnt);
				lv_label_set_long_mode(list_label, LV_LABEL_LONG_EXPAND);
#if CONFIG_FONT_ENABLE
				lv_list_set_style(list_label, LV_LABEL_STYLE_MAIN, &style0_label_1);
#endif
			}

			//artist
			if (NULL == player_ui->tplayer->mMediaInfo->Id3Info->artist){
			#if CONFIG_FONT_ENABLE
				sprintf (info, "%s: %s", get_text_by_id(LANG_MUSIC_ARTIST), player_ui->tplayer->mMediaInfo->Id3Info->artist);
			#else
				sprintf (info, "%s: %s",  "artist", player_ui->tplayer->mMediaInfo->Id3Info->artist);
			#endif
				list_bnt = lv_list_add_btn(para.ui.list_song_information, NULL, info);
				list_label = lv_list_get_btn_label(list_bnt);
				lv_label_set_long_mode(list_label, LV_LABEL_LONG_EXPAND);
#if CONFIG_FONT_ENABLE
				lv_list_set_style(list_label, LV_LABEL_STYLE_MAIN, &style0_label_1);
#endif
			}

			//ESC
			#if CONFIG_FONT_ENABLE
			list_bnt = lv_list_add_btn(para.ui.list_song_information, NULL, get_text_by_id(LANG_MUSIC_ESC));
			#else
			list_bnt = lv_list_add_btn(para.ui.list_song_information, NULL,  "ESC");
			#endif
			list_label = lv_list_get_btn_label(list_bnt);
			lv_label_set_long_mode(list_label, LV_LABEL_LONG_EXPAND);
			lv_obj_set_event_cb(list_bnt, btn_list_song_information_exit_event);
#if CONFIG_FONT_ENABLE
			lv_list_set_style(list_label, LV_LABEL_STYLE_MAIN, &style0_label_1);
#endif
		}
	}
}

static void btn_button_ab_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		menu_page_show_submenu(para.ui.ab_page);
	}
}

static void btn_ab_ok_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();

	player_ui->ab_play.a_point = media_bar_value_to_time(para.ui.slider_a);
	player_ui->ab_play.b_point = media_bar_value_to_time(para.ui.slider_b);
	player_ui->ab_play.loop = 1;
	player_ui->ab_play.enable = 1;
	lv_obj_set_hidden(para.ui.ab_page, true);
}

static void btn_ab_esc_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		lv_obj_set_hidden(para.ui.ab_page, true);
	}
}

static void slider_a_event(lv_obj_t * sli, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		media_set_time(para.ui.label_slider_a,  media_bar_value_to_time(para.ui.slider_a));
	}
}

static void slider_b_event(lv_obj_t * sli, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED){
		media_set_time(para.ui.label_slider_b, media_bar_value_to_time(para.ui.slider_b));
	}
}

static void file_source_event(lv_obj_t * sli, lv_event_t event)
{
	DiskInfo_t *dskinfo = NULL;
	media_file_list_t	*media_list = NULL;
	int disknum = 0,  index = 0;
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();

	if (event == LV_EVENT_CLICKED) {
		disknum = DiskManager_GetDiskNum();
		if (disknum < 2) {
			com_info("do not found other disk!");
			goto notfound;
			return;
		}

		for (index = 0; index < disknum; index++) {
			int scan_type_list[1] = {RAT_MEDIA_TYPE_AUDIO};
			dskinfo = DiskManager_GetDiskInfoByIndex(index);
			if (strcmp(dskinfo->MountPoint, player_ui->play_info.root_path) == 0) {
				com_info("same disk, skip it!");
				continue;
			}

			media_list = meida_scan_disk_file(scan_type_list, 1, NULL, dskinfo);
			if (NULL != media_list) {
				com_info("%s found media file!", dskinfo->MountPoint);
				goto found;
			}
		}
		com_info("not found any media file in all disk!");
notfound:
#if CONFIG_FONT_ENABLE
		para.mbox_file_is_null = media_mbox_create(get_text_by_id(LANG_MUSIC_NO_FILE), 1500, mbox_event_cb);
#else
		para.mbox_file_is_null = media_mbox_create("music file is null!", 1500, mbox_event_cb);
#endif
		return;
found:
		if(tplayer_get_status(player_ui->tplayer) == PLAY_STATUS){
			tplayer_stop(player_ui->tplayer);
			lv_btn_set_state(para.ui.btn_play, LV_BTN_STYLE_REL);
			while (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
				printf("Wait stop finish [L=%d, F=%s]:\n", __LINE__, __FUNCTION__);
				usleep(10000);
			}
		}
		tplayer_exit(player_ui->tplayer);
		media_unload_file(player_ui->media_list);

		media_init_playinfo(player_ui->media_list, &player_ui->break_tag);
		memset(&player_ui->play_info, 0x00, sizeof(play_info_t));
		memcpy(&player_ui->play_info, &player_ui->break_tag, sizeof(play_info_t));
		media_update_file_list(para.ui.media_list, player_ui->media_list, media_list_event);
		tplayer_init(player_ui->tplayer, 1);
	}
}

void music_spectrum_ui_set_value(int *value)
{
#define SPECTRUM_MAX_HEIGHT 200
#define SPECTRUM_MIN_HEIGHT 10
#define SPECTRUM_MAX_VALUE	1000

	unsigned int spectrum_value;
	int x = 5, y = 230, w = 20, tone = 0, i = 0;
#if 1
	int music_tone[]={262, 294, 330, 349, 392, 440, 494,
					523, 587, 659, 698, 784, 880, 988,
					1046, 1175, 1318, 1397, 1568, 1760, 1967};
#else
	int music_tone[]={0, 1, 2, 3, 4, 5, 6,
				7, 8, 9, 10, 11, 12, 13,
				14, 15, 16, 17, 18, 19, 20};
#endif

	if(!value){
		return;
	}

	for(i = 0; i < sizeof(music_tone) / sizeof(int); i++){
		if(i >= SPECTRUM_NUM){
			break;
		}
		tone = music_tone[i];
		if(tone > media_get_spectrum_size()){
			tone = media_get_spectrum_size()-1;
		}
		spectrum_value = value[tone] * SPECTRUM_MAX_HEIGHT / SPECTRUM_MAX_VALUE;
		//printf("%u ", value[tone]);
		spectrum_value = max(spectrum_value, SPECTRUM_MIN_HEIGHT);
		spectrum_value = min(spectrum_value, SPECTRUM_MAX_HEIGHT);
		lv_obj_set_pos(spectrum_ui[i], x+i*(w+5), y - spectrum_value);
		lv_obj_set_size(spectrum_ui[i], w, (int)spectrum_value);
	}

	//printf("\n");
}

static void media_player_ui_callback(void *ui_player, media_event_t event, void *param)
{
	player_ui_t * player_ui = (player_ui_t *)ui_player;
	media_file_list_t *media_file_list = NULL;

	if (player_ui == NULL) {
		return;
	}
	switch (event) {
		case MEDIA_IDLE_EVENT:
			if(tplayer_get_status(player_ui->tplayer) == PLAY_STATUS){
				media_set_progressbar(para.ui.progressbar, player_ui->play_info.time, player_ui->play_info.nDurationSec);
				//com_info("player_ui->play_info.time=%d, layer_ui->play_info.nDurationSec=%d",player_ui->play_info.time, player_ui->play_info.nDurationSec);
				media_set_time(para.ui.total_time, player_ui->play_info.nDurationSec);
				media_set_time(para.ui.curr_time, player_ui->play_info.time);
				music_spectrum_ui_set_value(media_get_spectrum());
				if (lrc_info.list_len) {
					media_update_lrc(player_ui->play_info.time);
				}
			}

			media_A_to_B_play(&player_ui->ab_play);
			break;
		case MEDIA_UPDATE_LIST_EVENT:
			media_file_list = media_get_file_list(RAT_MEDIA_TYPE_AUDIO);
			if (media_file_list!= NULL) {
				media_unload_file(player_ui->media_list);
				player_ui->media_list = media_file_list;
				media_update_file_list(para.ui.media_list, media_file_list, media_list_event);
				media_set_list_focus(para.ui.media_list, player_ui->play_info.index);
				media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			}
			break;
		case MEDIA_PLAY_EVENT:
			lv_label_set_text(para.ui.file_name, player_ui->play_info.filename);
			lv_btn_set_state(para.ui.btn_play, LV_BTN_STATE_TGL_REL);
			lv_obj_set_hidden(para.ui.lrc_no, true);
			media_set_list_focus(para.ui.media_list, player_ui->play_info.index);
			lv_obj_set_hidden(para.ui.download, false);
			media_ui_send_event(MEDIA_DOWNLOAD_EVENT, NULL, 0);
			break;
		case MEDIA_PAUSE_EVENT:
			lv_btn_set_state(para.ui.btn_play, LV_BTN_STATE_REL);
			break;
		case MEDIA_DOWNLOAD_EVENT:
			if(!media_downloading(para.ui.download)){
				lv_obj_set_hidden(para.ui.download, true);
				media_ui_send_event(MEDIA_LOAD_LRC_EVENT, NULL, 0);
			}
			break;
		case MEDIA_LOAD_LRC_EVENT:
			if(!media_load_lrc(player_ui)){
				lv_obj_set_hidden(para.ui.lrc_no, false);
			}
			media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			break;
		case MEDIA_LIST_LOOP_EVENT:
			media_ab_set_enable(0);
			lv_list_up(para.ui.media_list);
			break;
		case MEDIA_PLAY_COMPLETE_EVENT:
			lv_btn_set_state(para.ui.btn_play, LV_BTN_STATE_REL);
			if(lv_bar_get_value(para.ui.progressbar) < lv_bar_get_max_value(para.ui.progressbar)){
				lv_bar_set_value(para.ui.progressbar, lv_bar_get_max_value(para.ui.progressbar), LV_ANIM_OFF);
			}

			media_ui_send_event(MEDIA_PLAY_EVENT, NULL, rat_npl_get_next(player_ui->media_list->media_hrat));
			media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			break;
		default:
			media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
			break;
	}
}

static void media_play_mode_event(lv_obj_t * btn, lv_event_t event)
{
	if(event == LV_EVENT_CLICKED){
		para.mbox_reminder = create_mbox_reminder_playmode_change(para.ui.play_mode);
	}
}

#if CONFIG_FONT_ENABLE
static void music_label_text_init(music_para_t *para)
{
	static lv_style_t style0_label_1;
	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.line_space = 2;
	style0_label_1.text.font = get_font_lib()->msyh_16;

	lv_label_set_text(para->ui.label_online, get_text_by_id(LANG_MUSIC_ONLINE));
	lv_label_set_text(para->ui.menu_label, get_text_by_id(LANG_MUSIC_MENU));
	lv_label_set_text(para->ui.label_sound_effect, get_text_by_id(LANG_MUSIC_EFFECT));
	lv_label_set_text(para->ui.label_AB_OK, get_text_by_id(LANG_MUSIC_OK));
	lv_label_set_text(para->ui.label_AB_ESC, get_text_by_id(LANG_MUSIC_ESC));
	lv_label_set_text(para->ui.label_song_information, get_text_by_id(LANG_MUSIC_INFO));
	lv_label_set_text(para->ui.label_menu_esc, get_text_by_id(LANG_MUSIC_ESC));

	lv_label_set_style(para->ui.label_online, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.menu_label, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_sound_effect, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_AB_ESC, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_AB_OK, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_song_information, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_menu_esc,LV_LABEL_STYLE_MAIN, &style0_label_1);
}
#endif

void create_audio_eq_list(lv_obj_t *list, lv_event_cb_t event_cb)
{
	char* qeName[9] = {"normal", "dbb", "pop", "rock", "classic", "jazz", "vocal", "dance", "soft"};
	char eqLableName[10];
	lv_obj_t *btn;
	int i;

	lv_list_clean(list);
	for (i = 0; i < sizeof(qeName)/sizeof(qeName[0]); i++) {
		sprintf(eqLableName, "%s", qeName[i]);
		lv_list_add_btn(list, NULL, eqLableName);
	}

	btn = lv_list_get_next_btn(list, NULL);
	for (i = 0; i < sizeof(qeName)/sizeof(qeName[0]); i++) {
		lv_obj_set_event_cb(btn, event_cb);
		btn = lv_list_get_next_btn(list, btn);
	}
}

static int create_music(void)
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
	lv_obj_t *back_btn = lv_btn_create(para.ui.cont, NULL);
	lv_obj_align(back_btn, para.ui.cont, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_t *back_lable = lv_label_create(back_btn, NULL);
	lv_label_set_text(back_lable, LV_SYMBOL_LEFT);
	back_btn_style.body.opa = 64;
	back_btn_style.body.border.opa = 0;
	lv_obj_set_event_cb(back_btn, back_btn_event);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_REL, &back_btn_style);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_PR, &back_btn_style);

	music_auto_ui_create(&para.ui);
#if CONFIG_FONT_ENABLE
    music_label_text_init(&para);
#endif

#ifdef LV_USE_IMG
	spectrum1_particle_png = (void *)parse_image_from_file(LV_IMAGE_PATH"particle.png");
	int x = 5, y = 230, w = 20, h=9;
	for (int i = 0; i < SPECTRUM_NUM; i++) {
		spectrum_ui[i] = lv_img_create(para.ui.spectrum, NULL);
		lv_obj_set_pos(spectrum_ui[i], x+i*(w+5), y);
		lv_obj_set_size(spectrum_ui[i], w, h);
		lv_img_set_src(spectrum_ui[i], spectrum1_particle_png);
	}
#endif // LV_USE_IMG
	lv_obj_set_event_cb(para.ui.online, btn_online_event);
	lv_obj_set_event_cb(para.ui.btn_play, media_play_event);
	lv_obj_set_event_cb(para.ui.next, media_next_last_event);
	lv_obj_set_event_cb(para.ui.last, media_next_last_event);
	lv_obj_set_event_cb(para.ui.volume_bar, media_volume_bar_event);
	lv_obj_set_event_cb(para.ui.volume, media_volume_event);
	lv_obj_set_event_cb(para.ui.progressbar, media_progressbar_event);
	lv_obj_set_event_cb(para.ui.button_menu, btn_menu_event);
	lv_obj_set_event_cb(para.ui.button_page_menu_exit, btn_page_menu_exit_event);
	lv_obj_set_event_cb(para.ui.button_sound_effect, btn_list_sound_effect_event);
	lv_obj_set_event_cb(para.ui.button_song_information, btn_list_song_information_event);
	lv_obj_set_event_cb(para.ui.button_ab, btn_button_ab_event);
	lv_obj_set_event_cb(para.ui.button_ab_ok, btn_ab_ok_event);
	lv_obj_set_event_cb(para.ui.button_ab_esc, btn_ab_esc_event);
	lv_obj_set_event_cb(para.ui.slider_a, slider_a_event);
	lv_obj_set_event_cb(para.ui.slider_b, slider_b_event);
	lv_obj_set_event_cb(para.ui.button_file_source, file_source_event);
	create_audio_eq_list(para.ui.list_sound_effect, btn_sound_eq_event);

	/*get breaktag*/
	if (player_ui->clicked_form_explorer == 0) {
		media_func_get_breaktag(MUSIC_SCENE, &player_ui->break_tag);
	}
	player_ui->clicked_form_explorer = 0;
	/*get audio file list*/
	if (player_ui->media_list == NULL) {
		player_ui->media_list = media_get_file_list(RAT_MEDIA_TYPE_AUDIO);
	}
	if (player_ui->media_list == NULL || player_ui->media_list->total_num <= 0) {
#if CONFIG_FONT_ENABLE
		para.mbox_file_is_null = media_mbox_create(get_text_by_id(LANG_MUSIC_NO_FILE), 1500, mbox_event_cb);
#else
		para.mbox_file_is_null = media_mbox_create("music file is null!", 1500, mbox_event_cb);
#endif
		return -1;
	}
	/*update ui audio file list*/
	media_update_file_list(para.ui.media_list, player_ui->media_list, media_list_event);
	if (media_get_playinfo_by_breakpoint(player_ui->media_list, &player_ui->break_tag) == 0) {
		player_ui->break_vaild = 1;
	}
	player_ui->auto_play_enable = 1;
	memset(&player_ui->play_info, 0x00, sizeof(play_info_t));
	memcpy(&player_ui->play_info, &player_ui->break_tag, sizeof(play_info_t));
	media_set_play_file_index(player_ui->media_list, player_ui->play_info.index);

	media_func_register(MUSIC_SCENE, media_player_ui_callback);
	/*set music play mode*/
	media_config_init(para.ui.play_mode, para.ui.volume_bar, media_play_mode_event);
	if (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
		lv_btn_set_state(para.ui.btn_play, LV_BTN_STATE_TGL_REL);
		lv_label_set_text(para.ui.file_name, player_ui->play_info.filename);
		media_set_list_focus(para.ui.media_list, player_ui->play_info.index);
		media_set_progressbar(para.ui.progressbar, player_ui->play_info.time, player_ui->play_info.nDurationSec);
		media_set_time(para.ui.total_time, player_ui->play_info.nDurationSec);
		media_set_time(para.ui.curr_time, player_ui->play_info.time);
		goto END;
	}

	if (player_ui->break_vaild) {
		player_ui->break_vaild = 0;
		media_set_progressbar(para.ui.progressbar, player_ui->break_tag.offset, player_ui->break_tag.nDurationSec);
		media_set_time(para.ui.total_time, player_ui->break_tag.nDurationSec);
		media_set_time(para.ui.curr_time, player_ui->break_tag.offset);
		media_set_list_focus(para.ui.media_list, player_ui->break_tag.index);
		lv_label_set_text(para.ui.file_name, player_ui->break_tag.filename);
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
END:
	return 0;
}

static int destory_music(void)
{
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();
//to support to play music in backstage.
	media_config_deinit(MUSIC_SCENE);
	if (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
		media_func_unregister(MUSIC_SCENE, 1);
		{
            tplayer_exit(player_ui->tplayer);
            media_unload_file(player_ui->media_list);
            memset(player_ui->scene_name, 0, sizeof(player_ui->scene_name));
		}
	} else {
		media_func_unregister(MUSIC_SCENE, 0);
	}

	media_play_mod_icon_destory();
	music_auto_ui_destory(&para.ui);
	free_image(spectrum1_particle_png);
	lv_obj_del(para.ui.cont);
	return 0;
}

static int show_music(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);
	return 0;
}

static int hide_music(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);

	return 0;
}

static int msg_proc_music(MsgDataInfo *msg)
{
	return 0;
}

static page_interface_t page_music =
{
	.ops =
	{
		create_music,
		destory_music,
		show_music,
		hide_music,
		msg_proc_music,
	},
	.info =
	{
		.id         = PAGE_MUSIC,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_MUSIC(void)
{
	reg_page(&page_music);
}
