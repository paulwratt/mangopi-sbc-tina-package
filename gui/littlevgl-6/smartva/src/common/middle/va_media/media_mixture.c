#include <pthread.h>
#include "player_int.h"
#include "media_load_file.h"
#include "media_mixture.h"
#include "media_ui_mixture.h"
#include "DiskManager.h"
#include "rat_npl.h"
#include "media_spectrum.h"
#include "common.h"
#include "app_config_interface.h"
#include "moc_photo.h"

#define		LOCK
#ifdef LOCK
#define	MUTEX_LOCK(mtx)			if(pthread_mutex_lock(&mtx)){\
									printf("%s:%d mutex lock fail\n",__func__,__LINE__);\
								}
#define	MUTEX_UNLOCK(mtx)		if(pthread_mutex_unlock(&mtx)){\
									printf("%s:%d mutex unlock fail\n",__func__,__LINE__);\
								}
#else
#define MUTEX_LOCK(mtx)
#define MUTEX_UNLOCK(mtx)
#endif
#define ISNULL(x) if(!x){return 0;}
enum {
	BREAKPOINT_SCENE_MUSIC = 0,
	BREAKPOINT_SCENE_MOVIE,
	BREAKPOINT_PLAY_SCENE_NUM
};
enum {
	AUTO_PLAY_SCENE_MUSIC = 0,
	AUTO_PLAY_SCENE_MOVIE,
	AUTO_PLAY_SCENE_PICTURE,
	AUTO_PLAY_SCENE_NUM
};

/*************************media ui common api*********************************/
void media_set_list_focus(lv_obj_t *list, int index)
{
	lv_obj_t *focus_btn;

	focus_btn = lv_list_get_next_btn(list, NULL);
	for(int i = 0; i < index; i++){
		focus_btn = lv_list_get_next_btn(list, focus_btn);
	}
	lv_btn_set_state(focus_btn, LV_BTN_STATE_REL);
	lv_list_set_btn_selected(list, focus_btn);
}

void media_delete_list_file(lv_obj_t *list, int index)
{
	char *path;
	char cmd[64]={0};
	int list_size;
	lv_obj_t *focus_btn;
	player_ui_t *player_ui = media_get_player_data();

	path = media_get_file_path(player_ui->media_list, index);
	com_info("file = %s", path);
	sprintf(cmd, "rm %s", path);
	system(cmd);

	list_size = lv_list_get_size(list);
	if(list_size < 2){
		focus_btn = lv_list_get_next_btn(list, NULL);
		for(int i = 0; i < index; i++){
			focus_btn = lv_list_get_next_btn(list, focus_btn);
		}
		lv_label_set_text(lv_list_get_btn_label(focus_btn), "");
	}else{
		lv_list_remove(list, index);
	}
	media_ui_send_event(MEDIA_UPDATE_LIST_EVENT, NULL, 0);
}

void media_play_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t *player_ui = media_get_player_data();
	if (event != LV_EVENT_PRESSED) {
		return;
	}

	if (lv_btn_get_state(btn) == LV_BTN_STYLE_PR) {
		media_ui_send_event(MEDIA_PLAY_EVENT, NULL, media_get_play_file_index(player_ui->media_list));
	}

	if (lv_btn_get_state(btn) == LV_BTN_STYLE_TGL_PR) {
		media_ui_send_event(MEDIA_PAUSE_EVENT, NULL, media_get_play_file_index(player_ui->media_list));
	}
}

void media_volume_bar_event(lv_obj_t * btn, lv_event_t event)
{
	player_ui_t *player_ui = media_get_player_data();

	if (event == LV_EVENT_CLICKED) {
		player_ui->media_cfg.volume = lv_slider_get_value(btn);
		media_ui_send_event(MEDIA_SET_VOLUME_EVENT, NULL, player_ui->media_cfg.volume);
	}
}

int media_bar_value_to_time(lv_obj_t * btn)
{
	player_ui_t * player_ui = (player_ui_t *)media_get_player_data();
	return lv_slider_get_value(btn) * player_ui->play_info.nDurationSec / lv_slider_get_max_value(btn);
}

void media_progressbar_event(lv_obj_t * btn, lv_event_t event)
{
	int duration;
    int value;
	int seekvalue;
	player_ui_t *player_ui = media_get_player_data();

	if (event == LV_EVENT_CLICKED) {
		value = lv_slider_get_value(btn);
		tplayer_get_duration(player_ui->tplayer, &duration);
		if (duration <= 0 || tplayer_get_status(player_ui->tplayer) < PREPARED_STATUS) {
			return;
		}
        seekvalue = value * duration / lv_slider_get_max_value(btn);
		media_ui_send_event(MEDIA_SEEKTO_EVENT, NULL, seekvalue);
	}
}

void media_set_progressbar(lv_obj_t * btn, unsigned int time, unsigned int nDurationMs)
{
	player_ui_t *player_ui = media_get_player_data();
	int barValue = 0;

	if(player_ui->break_tag.nDurationSec)
	{
		barValue = lv_slider_get_max_value(btn) * time / nDurationMs;
		lv_slider_set_value(btn, barValue, LV_ANIM_OFF);
	}
	else
	{
		lv_slider_set_value(btn, 0, LV_ANIM_OFF);
	}
}

void file_size_int_to_string(long int size_int, char *size_str)
{
	char *unit[5] = {"B", "KB", "MB", "GM", "TB"};
	unsigned char convert_count = 0;
	int long num;
	char num_string[10];
	int point;

	num = size_int;
	while (num /= 1000)
		convert_count++;

	memset(num_string, 0, sizeof(num_string));
	sprintf (num_string, "%ld", size_int);
	point = strlen(num_string) - (convert_count * 3);
	strncpy(size_str, num_string, point);
	strncat(size_str, ". ", 1);
	strncat(size_str, num_string+point, 1);
	strcat(size_str, unit[convert_count]);
}

void time_int_to_string(unsigned int int_time, char *time_str)
{
	int hours, minutes, second;

	second = int_time;
	minutes = second / 60;
	hours = minutes / 60;
	sprintf(time_str, "%02d:%02d:%02d", hours % 24, minutes % 60, second % 60);
}

void media_set_time(lv_obj_t *time_lable, unsigned int time)
{
	char time_str[64];
	time_int_to_string(time, time_str);
	lv_label_set_text(time_lable, time_str);
}

int media_downloading(lv_obj_t * donwnload)
{
	static int count = 0;
	player_ui_t *player_ui = media_get_player_data();

	if(tplayer_get_status(player_ui->tplayer) < PREPARED_STATUS){
		count++;
		if(count == 10){
			lv_label_set_text(donwnload, "downloading .");
		}else if(count == 20){
			lv_label_set_text(donwnload, "downloading . .");
		}else if(count == 30){
			lv_label_set_text(donwnload, "downloading . . .");
			count = 0;
		}
		return -1;
	}
	return 0;
}

void media_ab_set_enable(int on_off)
{
	player_ui_t *player_ui = media_get_player_data();
	player_ui->ab_play.enable = on_off;
}

int media_A_to_B_play(media_ab_t *ab)
{
	player_ui_t *player_ui = media_get_player_data();
	static int status = 0;

	if(!player_ui->ab_play.enable){
		status = 0;
		return 0;
	}

	if(ab->a_point >= ab->b_point){
		com_err("media ui AB set error !!!");
		return -1;
	}

	switch(status){
		case 0:
			media_ui_send_event(MEDIA_PLAY_EVENT, NULL, media_get_play_file_index(player_ui->media_list));
			media_ui_send_event(MEDIA_SEEKTO_EVENT, NULL, ab->a_point);
			status++;
			break;
		case 1:
			//com_info("time = %d ab->b_point = %d!!!", player_ui->play_info.time, ab->b_point);
			if(player_ui->play_info.time >= ab->b_point){
				if(ab->loop){
					if(tplayer_get_status(player_ui->tplayer) != SEEKTO_STATUS){
						media_ui_send_event(MEDIA_SEEKTO_EVENT, NULL, ab->a_point);
					}
				}else{
					status = 0;
				}
			}
			break;
		default:
			break;
	}

	return 0;
}

/*******************************
		breakpoint api
********************************/
//static void media_breakpoint_record_mountpoint(void)
//{
//	player_ui_t *player_ui = media_get_player_data();
//	memset(player_ui->break_tag.root_path, 0x00, strlen(player_ui->break_tag.root_path));
//    memcpy(player_ui->break_tag.root_path, player_ui->play_info.root_path,
//            strlen(player_ui->play_info->root_path));
//    write_string_type_param(PUBLIC_SCENE, BREAK_TAG_ROOT_PATH, player_ui->break_tag.root_path,
//	        strlen(player_ui->break_tag.root_path));

//	com_info("player_ui->break_tag.mount_point = %s", player_ui->break_tag.root_path);
//}

/**************************media event********************************/

//static void media_breakpoint_record_path(void)
//{
//	player_ui_t *player_ui = media_get_player_data();

//	player_ui->break_tag.fileType = player_ui->media_list->media_type;
//	player_ui->break_tag.index = media_get_play_file_index(player_ui->media_list);
//	memset(player_ui->break_tag.filename, 0x00, sizeof(player_ui->break_tag.filename));
//	memset(player_ui->break_tag.path, 0x00, sizeof(player_ui->break_tag.path));
//	memset(player_ui->break_tag.filename, 0x00, sizeof(player_ui->break_tag.filename));

//	"%s", player_ui->play_info.filename);
//	memcpy(player_ui->break_tag.path, player_ui->play_info.path,
//    media_get_path_to_name(player_ui->play_info.path) - player_ui->play_info.path);
//}

static void media_breakpoint_record_time(int offset)
{
	player_ui_t *player_ui = media_get_player_data();

	player_ui->break_tag.offset = offset;
	player_ui->break_tag.nDurationSec = (player_ui->tplayer->mMediaInfo->nDurationMs) / 1000;
	// player_ui->media_cfg.breaktag = player_ui->break_tag;
}

void media_func_get_breaktag(char *player_name, play_info_t *breaktag)
{
	play_info_t *breaktag_buf = NULL;

	if (breaktag == NULL) {
		com_err("param error\n");
		return ;
	}

	breaktag_buf = breaktag;
	memset(breaktag_buf, 0x00, sizeof(play_info_t));
	read_string_type_param(player_name, BREAK_TAG_ROOT_PATH, breaktag_buf->root_path,
	                        sizeof(breaktag_buf->root_path));
	read_string_type_param(player_name, PATH, breaktag_buf->path,
	                        sizeof(breaktag_buf->path));
	read_string_type_param(player_name, FILENAME, breaktag_buf->filename,
	                        sizeof(breaktag_buf->filename));
	read_int_type_param(player_name, FILETYPE, (int *)&breaktag_buf->fileType);
	read_int_type_param(player_name, OFFSET, &breaktag_buf->offset);
	read_int_type_param(player_name, INDEX, &breaktag_buf->index);
	read_int_type_param(player_name, DURATION, &breaktag_buf->nDurationSec);
#if 0
    com_info("%s\n", player_ui->media_cfg.breaktag.mount_point);
	com_info("%s\n", player_ui->media_cfg.breaktag.path);
	com_info("%s\n", player_ui->media_cfg.breaktag.filename);
	com_info("%d\n", player_ui->media_cfg.breaktag.fileType);
	com_info("%d\n", player_ui->media_cfg.breaktag.offset);
	com_info("%d\n", player_ui->media_cfg.breaktag.index);
	com_info("%d\n", player_ui->media_cfg.breaktag.nDurationSec);
#endif

}

void media_func_set_breaktag(char *player_name)
{
	player_ui_t *player_ui = media_get_player_data();
	play_info_t *break_tag = NULL;

	break_tag = &player_ui->break_tag;

	write_string_type_param(player_name, BREAK_TAG_ROOT_PATH, break_tag->root_path,
						strlen(break_tag->root_path));
	write_string_type_param(player_name, PATH, break_tag->path,
						strlen(break_tag->path));
	write_string_type_param(player_name, FILENAME, break_tag->filename,
						strlen(break_tag->filename));
	write_int_type_param(player_name, FILETYPE, break_tag->fileType);
	write_int_type_param(player_name, OFFSET, break_tag->offset);
	write_int_type_param(player_name, INDEX, break_tag->index);
	write_int_type_param(player_name, DURATION, break_tag->nDurationSec);

    com_info("root_path		:%s", break_tag->root_path);
	com_info("volume		:%d", player_ui->media_cfg.volume);
	com_info("loop			:%d", player_ui->media_cfg.loop);
	com_info("path			:%s", break_tag->path);
	com_info("filename		:%s", break_tag->filename);
	com_info("fileType		:%d", break_tag->fileType);
	com_info("offset		:%d", break_tag->offset);
	com_info("index			:%d", break_tag->index);
	com_info("nDurationSec	:%d", break_tag->nDurationSec);
}

void media_init_playinfo(media_file_list_t *media_file_list, play_info_t *breaktag)
{
	int ret = 0;
	char path[1024] = {0};

	memset(breaktag, 0x00, sizeof(play_info_t));
	com_info("root_path:%s", ((__rat_list_t *)media_file_list->media_hrat)->str_path);
	strcpy(breaktag->root_path, ((__rat_list_t *)media_file_list->media_hrat)->str_path);
	memset(path, 0x00, sizeof(path));
	printf("breaktag->index:%d", breaktag->index);
	ret = rat_npl_index2file(media_file_list->media_hrat, breaktag->index, path);
	if (ret != EPDK_OK) {
		com_info("rat_npl_index2file fail!");
	}
	com_info("path:%s", path);
	strcpy(breaktag->path, path);
	com_info("filename:%s", media_get_path_to_name(path));
	strcpy(breaktag->filename, media_get_path_to_name(path));
	breaktag->fileType = ((__rat_list_t *)media_file_list->media_hrat)->media_type;
	com_info("fileType:%d", breaktag->fileType);
	return ;
}
int media_get_playinfo_by_breakpoint(media_file_list_t *media_file_list, play_info_t *breaktag)
{
	int i = 0, ret = 0;
	char path[1024] = {0};

	if (media_file_list == NULL || media_file_list->total_num <= 0 || breaktag == NULL) {
		com_info("param error");
		return 0;
	}
	if (strcmp(breaktag->root_path, ((__rat_list_t *)media_file_list->media_hrat)->str_path) != 0) {
		com_info("breaktag->root_path:%s", breaktag->root_path);
		com_info("str_path:%s", ((__rat_list_t *)media_file_list->media_hrat)->str_path);
		goto error;
	}

	if (breaktag->index >= 0 && breaktag->index < media_file_list->total_num) {
		ret = rat_npl_index2file(media_file_list->media_hrat, breaktag->index, path);
		if (ret != EPDK_OK) {
			com_info("rat_npl_index2file fail!");
		}
		if (ret == EPDK_OK && !strcmp(breaktag->filename, media_get_path_to_name(path))) {
			goto FOUND;
		}
	}

	for (i = 0; i < media_file_list->total_num; i++) {
		memset(path, 0x00, sizeof(path));
		ret = rat_npl_index2file(media_file_list->media_hrat, i, path);
		if (ret != EPDK_OK) {
			com_info("rat_npl_index2file fail!");
		}

		if (ret == EPDK_OK && !strcmp(breaktag->filename, media_get_path_to_name(path))) {
			breaktag->index = i;
			goto FOUND;
		}
	}
error:
	com_info("don't find break point(%s)", breaktag->filename);
	media_init_playinfo(media_file_list, breaktag);
	return -1;
FOUND:
	com_info("break_filename = %s", breaktag->filename);
	return 0;
}
/*******************************
		breakpoint api end
********************************/
int media_update_file_list(lv_obj_t * list, media_file_list_t	*media_list, lv_event_cb_t event_cb)
{
	int index = 0;
	const void *img_src = NULL;
	lv_obj_t *btn = NULL;

	btn = lv_list_get_next_btn(list, NULL);
	img_src = lv_img_get_src(lv_list_get_btn_img(btn));
	lv_list_clean(list);
	com_info("str_path:%s", ((__rat_list_t *)media_list->media_hrat)->str_path);
	for (index = 0; index < media_list->total_num; index++) {
		int ret = 0;
		char file_name[512] = {0};
		char path[1024] = {0};

		memset(path, 0x00, sizeof(path));
		ret = rat_npl_index2file(media_list->media_hrat, index, path);
		if (ret != EPDK_OK) {
			com_info("rat_npl_index2file fail!\n");
		}
		sprintf(file_name, "%d.%s", index + 1, media_get_path_to_name(path));
		btn = lv_list_add_btn(list, img_src, file_name);
		lv_obj_set_event_cb(btn, event_cb);
	}
	return 0;
}

int media_config_init(lv_obj_t *play_mode, lv_obj_t *volume_bar, lv_event_cb_t event_cb) {
	player_ui_t *player_ui = media_get_player_data();

	com_info("%s set config!\n", player_ui->scene_name);
	read_int_type_param(player_ui->scene_name, PLAYMODE, (int *)&player_ui->media_cfg.play_mode);
	read_int_type_param(player_ui->scene_name, LOOP, (int *)&player_ui->media_cfg.loop);
	read_int_type_param(PUBLIC_SCENE, VOLUME, (int *)&player_ui->media_cfg.volume);

	rat_npl_set_play_mode(player_ui->media_list->media_hrat, player_ui->media_cfg.play_mode);
	media_update_playmode_btn(play_mode, player_ui->media_cfg.play_mode);
	lv_obj_set_event_cb(play_mode, event_cb);
	lv_bar_set_value(volume_bar, player_ui->media_cfg.volume, LV_ANIM_OFF);
	player_ui->list_loop = 0;
	return 0;
}

int media_config_deinit(char *scene_name) {
	player_ui_t *player_ui = media_get_player_data();
	if (scene_name == NULL) {
		com_info("param err\n");
		return 0;
	}
	write_int_type_param(scene_name, PLAYMODE, player_ui->media_cfg.play_mode);
	write_int_type_param(scene_name, LOOP, player_ui->media_cfg.loop);
	write_int_type_param(PUBLIC_SCENE, VOLUME, player_ui->media_cfg.volume);
	return 0;
}

static int media_set_play_param(void)
{
	player_ui_t *player_ui = media_get_player_data();

	player_ui->play_info.index = media_get_play_file_index(player_ui->media_list);
	memset(player_ui->play_info.path, 0x00, sizeof(player_ui->play_info.path));
	memcpy(player_ui->play_info.path, media_get_file_path(player_ui->media_list, player_ui->play_info.index), sizeof(player_ui->play_info.path));
	memset(player_ui->play_info.filename, 0x00, sizeof(player_ui->play_info.filename));
	memcpy(player_ui->play_info.filename, media_get_path_to_name(player_ui->play_info.path), sizeof(player_ui->play_info.filename));

	if (strlen(player_ui->play_info.path) == 0) {
		com_err("playing file path error");
		return -1;
	}
	com_info("start playing %s\n", player_ui->play_info.path);
	media_func_set_breaktag(player_ui->scene_name);
	if (tplayer_play_url(player_ui->tplayer, player_ui->play_info.path) < 0) {
		com_err("tplayer_play_url fail!");
		return -1;
	}

    if (0 == strcmp(player_ui->scene_name, MOVIE_SCENE)) {
        tplayer_set_displayrect(player_ui->tplayer, 0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    }
	return 0;
}

static void media_play_file(int index)
{
	int player_type = 0;
	player_ui_t *player_ui = media_get_player_data();

	if (index == -1) {
		com_err("param error index:%d", index);
		return;
	}
	media_set_play_file_index(player_ui->media_list, index);
	if (0 == strcmp(player_ui->scene_name, MOVIE_SCENE)) {
        player_type = CEDARX_PLAYER;
	} else if (0 == strcmp(player_ui->scene_name, MUSIC_SCENE)) {
        player_type = AUDIO_PLAYER;
	}

	//continue play current file
	if (tplayer_get_status(player_ui->tplayer) == STOP_STATUS && player_ui->play_info.index == index) {
		if (tplayer_play_url(player_ui->tplayer, player_ui->play_info.path) < 0) {
			return;
		}

        if (player_type == CEDARX_PLAYER) {
            tplayer_set_displayrect(player_ui->tplayer, 0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX);
        }

        tplayer_play(player_ui->tplayer);
        media_ui_send_event(MEDIA_SEEKTO_EVENT, NULL, player_ui->play_info.offset);
	} else if (tplayer_get_status(player_ui->tplayer) == PAUSE_STATUS) {
		tplayer_play(player_ui->tplayer);
	} else {//to change play file
		if (media_set_play_param() < 0) {
			com_err("media_set_play_param fail!");
			return;
		}
		tplayer_play(player_ui->tplayer);
	}
	media_ui_send_event(MEDIA_SET_VOLUME_EVENT, NULL, player_ui->media_cfg.volume);
}

int media_external_click(rat_media_type_t media_type, char *file_path, DiskInfo_t *DiskInfo)
{
	int page_id;
	player_ui_t *player_ui = media_get_player_data();

	if (media_type == RAT_MEDIA_TYPE_AUDIO){
		page_id = PAGE_MUSIC;
	} else if (media_type == RAT_MEDIA_TYPE_VIDEO) {
		page_id = PAGE_MOVIE;
	} else if (media_type == RAT_MEDIA_TYPE_PIC) {
		page_id = PAGE_PHOTO;
	} else {
		com_err("don't support this file(media_type = %d)!!!", media_type);
		return 0;
	}

	if(page_id == PAGE_PHOTO){
		photo_user_data_t *user_data = NULL;
		media_file_list_t *media_file_list = NULL;

		user_data = get_page_user_data(PAGE_PHOTO);
		if(user_data == NULL){
			user_data = malloc(sizeof(photo_user_data_t));
			if(user_data == NULL){
				return -1;
			}
			memset(user_data,0,sizeof(photo_user_data_t));
			set_page_user_data(PAGE_PHOTO,user_data);
		}

		user_data->is_auto_play = 1;
		media_file_list = media_load_file(RAT_MEDIA_TYPE_PIC,DiskInfo->MountPoint);
		if (NULL != media_file_list && media_file_list->total_num > 0) {
			user_data->index = rat_npl_file2index(media_file_list->media_hrat,file_path);
			media_unload_file(player_ui->media_list);
		}
		memset(user_data->mountpoint,0,sizeof(user_data->mountpoint));
		memcpy(user_data->mountpoint,DiskInfo->MountPoint,strlen(DiskInfo->MountPoint));
		memset(user_data->filename,0,sizeof(user_data->filename));
		memcpy(user_data->filename,file_path,strlen(file_path));

		switch_page(current_page(), page_id);
	}else{
		// if current play status, stop it
		if (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
			tplayer_stop(player_ui->tplayer);
			while (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
				com_info("Wait stop finish");
				usleep(10000);
			}
			tplayer_exit(player_ui->tplayer);
			memset(&player_ui->play_info, 0x00, sizeof(play_info_t));
			media_unload_file(player_ui->media_list);
			player_ui->media_list = NULL;
			memset(player_ui->scene_name, 0, sizeof(player_ui->scene_name));
		}
		memset(player_ui->break_tag.root_path, 0x00, sizeof(player_ui->break_tag.root_path));
		strcpy(player_ui->break_tag.root_path, DiskInfo->MountPoint);
		memset(player_ui->break_tag.path, 0x00, sizeof(player_ui->break_tag.path));
		strcpy(player_ui->break_tag.path, file_path);
		memset(player_ui->break_tag.filename, 0x00, sizeof(player_ui->break_tag.filename));
		strcpy(player_ui->break_tag.filename, media_get_path_to_name(file_path));
		player_ui->break_tag.fileType = media_type;
		player_ui->break_tag.index = -1;
		player_ui->break_tag.nDurationSec = 0;
		player_ui->break_tag.offset = 0;
		player_ui->clicked_form_explorer = 1;
		player_ui->break_vaild = 0;

		switch_page(current_page(), page_id);
	}

	return 0;
}

media_file_list_t *meida_scan_disk_file(int *scan_type_list, int count, int *media_type, DiskInfo_t *DiskInfo)
{
	int index = 0;
	media_file_list_t *media_file_list = NULL;

	for (index = 0; index < count; index++) {
		media_file_list = media_load_file(scan_type_list[index], DiskInfo->MountPoint);
		if (NULL != media_file_list && media_file_list->total_num > 0) {
			if (media_type) {
				*media_type = (int)*(scan_type_list + index);
			}
			return media_file_list;
		}
		media_unload_file(media_file_list);
	}
	return NULL;
}

media_file_list_t *media_get_file_list(rat_media_type_t fileType)
{
	int disk_num = 0, index = 0, cur_play_path_len = 0, scan_type_list[1] = {fileType};
	media_file_list_t *media_list = NULL;
	player_ui_t *player_ui = media_get_player_data();
	char *root_path = NULL;

	disk_num = DiskManager_GetDiskNum();
	root_path = player_ui->break_tag.root_path;
	cur_play_path_len = strlen(root_path);

	if (cur_play_path_len != 0) {
		for (index = 0; index < disk_num; index++) {
			DiskInfo_t *disk_tmp = NULL;

			disk_tmp = DiskManager_GetDiskInfoByIndex(index);
			com_info("cur_play_path:%s\n", root_path);
			com_info("MountPoint:%s\n", disk_tmp->MountPoint);
			if (strncmp(root_path, disk_tmp->MountPoint, strlen(disk_tmp->MountPoint)) == 0) {
				media_list = meida_scan_disk_file(scan_type_list, 1, NULL, disk_tmp);
				if (NULL != media_list) {
					com_err("in %s found file fileType:%d total_num:%d", disk_tmp->MountPoint, fileType, media_list->total_num);
					return media_list;
				}
				break;
			}
		}
	}


	com_info("disk_num:%d", disk_num);
	for (index = 0; index < disk_num; index++) {
		DiskInfo_t *disk_tmp = NULL;

		disk_tmp = DiskManager_GetDiskInfoByIndex(index);
		com_info("cur_play_path:%s", root_path);
		com_info("MountPoint:%s", disk_tmp->MountPoint);
		if (cur_play_path_len == 0 || strncmp(root_path, disk_tmp->MountPoint, strlen(disk_tmp->MountPoint)) != 0) {
			media_list = meida_scan_disk_file(scan_type_list, 1, NULL, disk_tmp);
			if (NULL != media_list) {
				com_info("in %s found file fileType:%d total_num:%d", disk_tmp->MountPoint, fileType, media_list->total_num);
				return media_list;
			}
		}
	}
	return NULL;
}

static void media_disk_plugin(DiskInfo_t *DiskInfo)
{
	media_file_list_t *media_file_list = NULL;
	player_ui_t *player_ui = media_get_player_data();
	int media_type = 0;
	page_id_t cur_page = 0;
	int scan_type_list[3] = {RAT_MEDIA_TYPE_VIDEO, RAT_MEDIA_TYPE_AUDIO, RAT_MEDIA_TYPE_PIC};
	play_info_t breaktag;
	char scene_name[64];

	cur_page = current_page();
	if (cur_page == PAGE_EXPLORER) {
		com_info("curren page PAGE_EXPLORER skip plugin message!");
		return ;
	} else if (cur_page == PAGE_MUSIC) {
		scan_type_list[0] = RAT_MEDIA_TYPE_AUDIO;
		scan_type_list[1] = RAT_MEDIA_TYPE_VIDEO;
		scan_type_list[2] = RAT_MEDIA_TYPE_PIC;
	} else if (cur_page == PAGE_PHOTO) {
		scan_type_list[0] = RAT_MEDIA_TYPE_PIC;
		scan_type_list[1] = RAT_MEDIA_TYPE_VIDEO;
		scan_type_list[2] = RAT_MEDIA_TYPE_AUDIO;
	}
	media_file_list = meida_scan_disk_file(scan_type_list, sizeof(scan_type_list)/sizeof(scan_type_list[0]), &media_type, DiskInfo);
	if (media_file_list != NULL) {
		if (RAT_MEDIA_TYPE_AUDIO == media_type) {
			strcpy(scene_name, MUSIC_SCENE);
		} else if (RAT_MEDIA_TYPE_VIDEO == media_type) {
			strcpy(scene_name, MOVIE_SCENE);
		} else {
			strcpy(scene_name, PHOTO_SCENE);
		}
		goto filelist_found;
	} else {
		goto done;
	}

filelist_found:
	if (RAT_MEDIA_TYPE_PIC == media_type) {
		photo_user_data_t *user_data = NULL;

		user_data = get_page_user_data(PAGE_PHOTO);
		if(user_data == NULL){
			user_data = malloc(sizeof(photo_user_data_t));
			if(user_data == NULL){
				return ;
			}
			memset(user_data,0,sizeof(photo_user_data_t));
			set_page_user_data(PAGE_PHOTO,user_data);
		}

		user_data->is_auto_play = 1;
		memset(user_data->mountpoint,0,sizeof(user_data->mountpoint));
		memcpy(user_data->mountpoint,DiskInfo->MountPoint,strlen(DiskInfo->MountPoint));
		memset(user_data->filename,0,sizeof(user_data->filename));
		user_data->index = -1;

		switch_page(current_page(), PAGE_PHOTO);

	} else {
		media_func_get_breaktag(scene_name, &breaktag);
		if (media_get_playinfo_by_breakpoint(media_file_list, &breaktag) < 0) {
			memset(&player_ui->break_tag, 0x00, sizeof(play_info_t));
			memcpy(&player_ui->break_tag, &breaktag, sizeof(play_info_t));
			media_func_set_breaktag(scene_name);
		}
		if (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
			tplayer_stop(player_ui->tplayer);
			while (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS) {
				com_info("Wait stop finish\n");
				usleep(10000);
			}
			tplayer_exit(player_ui->tplayer);
			memset(&player_ui->play_info, 0x00, sizeof(play_info_t));
			media_unload_file(player_ui->media_list);
			player_ui->media_list = NULL;
			memset(player_ui->scene_name, 0, sizeof(player_ui->scene_name));
		}
		if (RAT_MEDIA_TYPE_AUDIO == media_type) {
			switch_page(current_page(), PAGE_MUSIC);
		} else if (RAT_MEDIA_TYPE_VIDEO == media_type) {
			switch_page(current_page(), PAGE_MOVIE);
		}
	}
	media_unload_file(media_file_list);
done:
	return;
}
//disk is pulled out
static void media_disk_plugout(DiskInfo_t *DiskInfo)
{
	player_ui_t *player_ui = media_get_player_data();

	if(current_page() == PAGE_MOVIE || current_page() == PAGE_MUSIC)
	{
		if ((strlen(player_ui->play_info.root_path) == strlen(DiskInfo->MountPoint))
			&& 0 == strcmp(player_ui->play_info.root_path, DiskInfo->MountPoint))
		{
			playerStatus PlayerStatus = EXIT_STATUS;
			//it will exit if music play in backstage.
			tplayer_stop(player_ui->tplayer);
			PlayerStatus = tplayer_get_status(player_ui->tplayer);
			while (PlayerStatus != STOP_STATUS && PlayerStatus != EXIT_STATUS) {
				usleep(10000);
				PlayerStatus = tplayer_get_status(player_ui->tplayer);
			}
			tplayer_exit(player_ui->tplayer);
			media_unload_file(player_ui->media_list);
			switch_page(current_page(), PAGE_HOME);
			goto end;

		}
	}
	else if(current_page() == PAGE_PHOTO)
	{
		int ret;
		char mountpoint[MOUNT_PATH_MAX_LENGTH];
		ret = read_string_type_param(PHOTO_SCENE, "mountpoint",mountpoint,sizeof(mountpoint));

		if (ret < 0 || 0 == strcmp(mountpoint, DiskInfo->MountPoint))
		{
			switch_page(current_page(), PAGE_HOME);
		}
	}

end:
	return;
}

void media_set_online_mode(int enable)
{
	player_ui_t *player_ui = media_get_player_data();

	player_ui->online_mode = enable;
}

static int media_tplayer_callback(void* pUserData, int msg, int param0, void* param1)
{
	//player_context_t* pPlayer = (player_context_t*)pUserData;
	player_ui_t *player_ui = media_get_player_data();
	int video_width;
	int video_height;
	switch(msg){
		case TPLAYER_NOTIFY_PREPARED:
			if(current_page() == PAGE_MUSIC){
				media_spectrum_destroy();
			}
			break;
		case TPLAYER_NOTIFY_PLAYBACK_COMPLETE:
			if (player_ui->list_loop) {
				media_ui_send_event(MEDIA_LIST_LOOP_EVENT, NULL, 0);
			} else {
				media_ui_send_event(MEDIA_PLAY_COMPLETE_EVENT, NULL, 0);
			}
			break;
		case TPLAYER_NOTIFY_MEDIA_ERROR:
			if(player_ui->list_loop){
				media_ui_send_event(MEDIA_LIST_LOOP_EVENT, NULL, 0);
			}
			break;
		case TPLAYER_NOTIFY_AUDIO_FRAME:
			if(player_ui->play_info.fileType == RAT_MEDIA_TYPE_AUDIO){
				media_spectrum_init((AudioPcmData *)param1);
			}
			break;
		case TPLAYER_NOTIFY_MEDIA_VIDEO_SIZE:
		    if (STOP_STATUS == tplayer_get_status(player_ui->tplayer))
		    {
                video_width = ((int *)param1)[0];
                video_height = ((int *)param1)[1];
                com_info("width = %d height = %d\r\n", video_width, video_height);
                if (video_width > 1280 && video_height > 720)
                {
                    com_info("more than 720P\r\n");
                    player_ui->callback(player_ui, MEDIA_MBOX_EVENT, 0);
                }
		    }
			break;
		default:
			break;
	}
	return 0;
}
static pthread_cond_t disk_hotplug_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t disk_hotplug_mutex;
static hotplug_message_focus_win_t *RegisterInfo = NULL;
static void disk_manager_callback(DiskInfo_t *DiskInfo);
static void media_disk_hotplug_init(void)
{
	pthread_mutex_init(&disk_hotplug_mutex, NULL);
	pthread_cond_init(&disk_hotplug_cond, NULL);

	RegisterInfo = malloc(sizeof(hotplug_message_focus_win_t));
	if (RegisterInfo != NULL) {
		memset(RegisterInfo, 0x00, sizeof(hotplug_message_focus_win_t));
		RegisterInfo->CallBackFunction = disk_manager_callback;
		strcpy(RegisterInfo->Cur_Win, "media_ui");
		DiskManager_Register(RegisterInfo);
	}
}
static void media_disk_hotplug_deinit(void)
{
	if (RegisterInfo != NULL) {
		DiskManager_UnRegister(RegisterInfo);
	}
	pthread_mutex_destroy(&disk_hotplug_mutex);
	pthread_cond_destroy(&disk_hotplug_cond);
}

static void media_disk_hotplug_down(void)
{
	pthread_mutex_lock(&disk_hotplug_mutex);
	pthread_cond_wait(&disk_hotplug_cond, &disk_hotplug_mutex);
	pthread_mutex_unlock(&disk_hotplug_mutex);
}
static void media_disk_hotplug_up(void)
{
	pthread_mutex_lock(&disk_hotplug_mutex);
	pthread_cond_signal(&disk_hotplug_cond);
	pthread_mutex_unlock(&disk_hotplug_mutex);
}
/*there maybe have bug, when prev message not finish, next message come*/
static void disk_manager_callback(DiskInfo_t *DiskInfo)
{
//	player_ui_t *player_ui = media_get_player_data();
#ifdef DEBUG_MEDIA_ENABLE
	com_info("DiskInfo->DeviceName=%s\n", DiskInfo->DeviceName);
	com_info("DiskInfo->MountPoint=%s\n", DiskInfo->MountPoint);
	com_info("DiskInfo->operate=%d\n", (int)DiskInfo->operate);
	com_info("DiskInfo->MediaType=%d\n", (int)DiskInfo->MediaType);
	com_info("DiskInfo->Major=%d\n", (int)DiskInfo->Major);
	com_info("DiskInfo->Minor=%d\n", (int)DiskInfo->Minor);
#endif
	//if disk  pullout when it is using, it will mount failed when disk is inserted again.
	if (DiskInfo->operate == MEDIUM_PLUGOUT) {
		media_ui_send_event(MEDIA_DISK_PULLOUT_EVENT, DiskInfo, 0);
		media_disk_hotplug_down();
	} else if (DiskInfo->operate == MEDIUM_PLUGIN) {
		media_ui_send_event(MEDIA_DISK_INSERT_EVENT, DiskInfo, 0);
		media_disk_hotplug_down();
	}
}
void media_ui_send_event(media_event_t event, void *param0, int param1)
{
	player_ui_t *player_ui = media_get_player_data();

	media_event_param_t *event_param = malloc(sizeof(media_event_param_t));
	event_param->event = event;
	event_param->param[0] = (long int)param0;
	event_param->param[1] = param1;
	__db_list_put_tail(player_ui->event_head, event_param);
	com_info("media ui send event = %d ", event_param->event);
}

static void media_ui_task(struct _lv_task_t *param)
{
	player_ui_t *player_ui = (player_ui_t *)param->user_data;
	media_event_param_t *event_param = NULL;
	DiskInfo_t *disk_info;

	/*default event MEDIA_IDLE_EVENT, so when event_head is empty do not return*/
	event_param = (struct media_event_param_t *)__db_list_pop(player_ui->event_head);
	if (NULL != event_param) {
		memset(&player_ui->event_param, 0x00, sizeof(struct media_event_param_t));
		memcpy(&player_ui->event_param, event_param, sizeof(struct media_event_param_t));
		free(event_param);
		com_info("media ui get event = %d ", player_ui->event_param.event);
	}

	switch(player_ui->event_param.event){
		case MEDIA_IDLE_EVENT:
        {
            if (tplayer_get_status(player_ui->tplayer) == PLAY_STATUS)
            {
                tplayer_get_current_pos(player_ui->tplayer, &player_ui->play_info.time);
                player_ui->play_info.nDurationSec = (player_ui->tplayer->mMediaInfo->nDurationMs) / 1000;
                media_breakpoint_record_time(player_ui->play_info.time);
            }
        break;
        }
		case MEDIA_PREPARE_EVENT:
        {
            media_set_play_file_index(player_ui->media_list, player_ui->event_param.param[1]);
            media_set_play_param();
            tplayer_set_looping(player_ui->tplayer, !player_ui->list_loop);
            media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
        break;
        }
		case MEDIA_PLAY_EVENT:
        {
            //to support to play music in backstage.
            media_play_file(player_ui->event_param.param[1]);
            if(current_page() != PAGE_MUSIC && current_page() != PAGE_MOVIE)
            {
                media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
            }
        break;
        }
		case MEDIA_PAUSE_EVENT:
        {
            tplayer_pause(player_ui->tplayer);
            media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
        break;
        }
		case MEDIA_STOP_EVENT:
        {
            tplayer_stop(player_ui->tplayer);
            media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
        break;
        }
		case MEDIA_SET_VOLUME_EVENT:
        {
            tplayer_volume(player_ui->tplayer, player_ui->event_param.param[1]);        // ???
            write_int_type_param(PUBLIC_SCENE, "volume", player_ui->event_param.param[1]);
            media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
        break;
        }
		case MEDIA_SEEKTO_EVENT:
        {
            tplayer_seekto(player_ui->tplayer, player_ui->event_param.param[1]);
            if(tplayer_get_status(player_ui->tplayer) == SEEKTO_STATUS)
            {
                media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
            }
        break;
        }
		case MEDIA_LIST_LOOP_EVENT:
        {
            media_ui_send_event(MEDIA_PLAY_EVENT, NULL, media_get_play_file_index(player_ui->media_list) + 1);
        break;
        }
		case MEDIA_SET_AUDIO_EQ:
        {
            tplayer_set_audio_eq(player_ui->tplayer, player_ui->event_param.param[1]);
        break;
        }
		case MEDIA_DISK_INSERT_EVENT:
        {
            disk_info = (DiskInfo_t *)player_ui->event_param.param[0];
            if (current_page() != PAGE_CAMERA) {
                media_disk_plugin(disk_info);
            }
            media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
            media_disk_hotplug_up();
        break;
        }
		case MEDIA_DISK_PULLOUT_EVENT:
        {
            disk_info = (DiskInfo_t *)player_ui->event_param.param[0];
            com_info("MEDIA_DISK_PULLOUT_EVENT %s\n", disk_info->MountPoint);
            media_disk_plugout(disk_info);
            media_ui_send_event(MEDIA_IDLE_EVENT, NULL, 0);
            media_disk_hotplug_up();
	        break;
        }
		default:
        {
        break;
        }
	}

	if(player_ui->callback){
		player_ui->callback(param->user_data, player_ui->event_param.event, (void *)player_ui->event_param.param);
	}

}

static player_ui_t *player_data;

player_ui_t *media_get_player_data(void)
{
	return player_data;
}

player_ui_t *media_init(void)
{
	player_ui_t *player_ui;

	player_ui = (player_ui_t *)malloc(sizeof(player_ui_t));
	if(player_ui == NULL){
		com_err("player_ui malloc failed!\r\n");
		goto end;
	}
	player_data = player_ui;
	memset(player_ui, 0, sizeof(player_ui_t));
	player_ui->tplayer = tplayer_pthread_create();
	if(!player_ui->tplayer){
		com_err("tplayer create failed!\r\n");
		goto end;
	}
	tplayer_set_callback(player_ui->tplayer, media_tplayer_callback);

	media_disk_hotplug_init();

	player_ui->event_head = db_list_create("media_event", 0);
	if (!player_ui->event_head) {
		com_err("event head create failed!\r\n");
		goto end;
	}
	player_ui->lv_task = lv_task_create(media_ui_task, 30, LV_TASK_PRIO_LOW, (void *)player_ui);
	if(!player_ui->lv_task){
		com_err("lv task create failed!\r\n");
		goto end;
	}

	/*
	{
		DiskInfo_t DiskInfo_test;
		memset(&DiskInfo_test, 0, sizeof(DiskInfo_test));
		memcpy(DiskInfo_test.DeviceName, "/dev/sda1", strlen("/dev/sda1"));
		memcpy(DiskInfo_test.MountPoint, "/mnt/exUDISK", strlen("/mnt/exUDISK"));
		DiskInfo_test.operate = MEDIUM_PLUGIN;
		DiskInfo_test.MediaType = MEDIA_USB_MASSSTORAGE;
		DiskInfo_test.Major = 56;
		DiskInfo_test.Minor = 49;
		disk_manager_callback_test(&DiskInfo_test);
	}
	*/

end:
	return player_ui;
}

void media_uninit(player_ui_t *player_ui)
{
	lv_task_del(player_ui->lv_task);
	media_disk_hotplug_deinit();
	tplayer_pthread_destory(player_ui->tplayer);
	__db_list_destory(player_ui->event_head);
	free(player_ui);
	player_data = NULL;
}

void media_func_register(char *player_name, media_ui_callback callback)
{
    int player_type = 0;
	player_ui_t *player_ui = media_get_player_data();
	memcpy(player_ui->scene_name, player_name, strlen(player_name));
	if (0 == strcmp(player_name, MOVIE_SCENE))
	{
        player_type = CEDARX_PLAYER;
	}
	else if (0 == strcmp(player_name, MUSIC_SCENE))
	{
        player_type = AUDIO_PLAYER;
	}
	memset(&player_ui->media_cfg, 0, sizeof(player_ui->media_cfg));
	player_ui->callback = callback;
	tplayer_init(player_ui->tplayer, player_type);
}

//en_back : 1: support play in backstage
void media_func_unregister(char *player_name, bool en_back)
{
	player_ui_t *player_ui = media_get_player_data();
	if(strcmp(player_ui->scene_name, player_name) == 0){
		player_ui->callback = NULL;
		media_spectrum_destroy();
		media_func_set_breaktag(player_name);
		if (!en_back) {
			tplayer_exit(player_ui->tplayer);
			memset(&player_ui->play_info, 0x00, sizeof(play_info_t));
			media_unload_file(player_ui->media_list);
			player_ui->media_list = NULL;
			memset(player_ui->scene_name, 0, sizeof(player_ui->scene_name));
		}
	}
}
