#ifndef __MEDIA_MIXTURE_H__
#define __MEDIA_MIXTURE_H__

#include <pthread.h>
#include "lvgl.h"
#include "page.h"
#include "player_int.h"
#include "media_load_file.h"
#include "DiskManager.h"
#include "rat_npl.h"
//#include "va_param.h"

#define FILE_NAME_LEN 128
#define FILE_PATH_LEN 512

typedef struct play_info_t
{
	char                root_path[MOUNT_PATH_MAX_LENGTH];
	char				filename[FILE_NAME_LEN];
	char				path[FILE_PATH_LEN];
	rat_media_type_t	fileType;
	int					index;
	int					time;
	int				offset;
	int					nDurationSec;
} play_info_t;

typedef struct media_ab_t
{
	int a_point;
	int b_point;
	int loop;
	int enable;
}media_ab_t;

typedef struct media_cfg_t
{
	int loop;
	int volume;
	rat_play_mode_e play_mode;
} media_cfg_t;

typedef enum{
	MEDIA_IDLE_EVENT = 0,
	MEDIA_UPDATE_LIST_EVENT = 1,
	MEDIA_DISK_INSERT_EVENT = 2,
	MEDIA_DISK_PULLOUT_EVENT = 3,
	MEDIA_PREPARE_EVENT = 4,
	MEDIA_PLAY_EVENT = 5,
	MEDIA_PAUSE_EVENT = 6,
	MEDIA_STOP_EVENT = 7,
	MEDIA_SET_VOLUME_EVENT = 8,
	MEDIA_SEEKTO_EVENT = 9,
	MEDIA_PLAY_COMPLETE_EVENT = 10,
	MEDIA_LOAD_LRC_EVENT = 11,
	MEDIA_DOWNLOAD_EVENT = 12,
	MEDIA_LIST_LOOP_EVENT = 13,
	MEDIA_SET_AUDIO_EQ = 14,
	MEDIA_MBOX_EVENT = 15,
} media_event_t;

typedef void (*media_ui_callback)(void *ui_player, media_event_t event, void *param);

typedef struct media_event_param_t{
	media_event_t event;
	long int param[2];
}media_event_param_t;

typedef struct PLAYER_UI_T{
	lv_task_t *			lv_task;
	player_t			*tplayer;

	bool				clicked_form_explorer;
	play_info_t			break_tag;
	bool				break_vaild; //0: don't use

	media_ui_callback	callback;
	bool				list_loop;
	bool				online_mode;

	//ui event
	db_list_t*			event_head;
	media_event_param_t	event_param;

	//the info of media be playing
	char				scene_name[64];
	play_info_t			play_info;
	media_file_list_t	*media_list;
	bool				auto_play_enable;//0: don't play but it is already prepare

	// a b point play
	media_ab_t			ab_play;

	//va_param_ops_t param_ops;
	media_cfg_t			media_cfg;
}player_ui_t;

player_ui_t * media_init(void);
void media_uninit(player_ui_t *player_ui);
player_ui_t *media_get_player_data(void);

void media_func_set_breaktag(char *player_name);
void media_func_get_breaktag(char *player_name, play_info_t *breaktag);
void media_func_register(char *player_name, media_ui_callback callback);
void media_func_unregister(char *player_name, bool en_back);

void media_set_list_focus(lv_obj_t *list, int index);
void media_delete_list_file(lv_obj_t *list, int index);
void media_ui_send_event(media_event_t event, void *param0, int param1);
void media_play_event(lv_obj_t * btn, lv_event_t event);
void media_volume_bar_event(lv_obj_t * btn, lv_event_t event);
void media_progressbar_event(lv_obj_t * btn, lv_event_t event);
int media_downloading(lv_obj_t * donwnload);
media_file_list_t *media_get_file_list(rat_media_type_t fileType);
media_file_list_t *meida_scan_disk_file(int *scan_type_list, int count, int *media_type, DiskInfo_t *DiskInfo);
int media_update_file_list(lv_obj_t * list, media_file_list_t	*media_list, lv_event_cb_t event_cb);
int media_external_click(rat_media_type_t media_type, char *file_path, DiskInfo_t *DiskInfo);
void media_set_online_mode(int enable);
void media_set_progressbar(lv_obj_t * btn, unsigned int time, unsigned int nDurationMs);
void media_set_time(lv_obj_t *time_lable, unsigned int time);
int media_bar_value_to_time(lv_obj_t * btn);
void media_ab_set_enable(int on_off);
int media_A_to_B_play(media_ab_t *ab);
void time_int_to_string(unsigned int int_time, char *time_str);
void file_size_int_to_string(long int size_int, char *size_str);
int media_config_init(lv_obj_t *play_mode, lv_obj_t *volume_bar, lv_event_cb_t event_cb);
int media_get_playinfo_by_breakpoint(media_file_list_t *media_file_list, play_info_t *breaktag);
int media_config_deinit(char *scene_name);
void media_init_playinfo(media_file_list_t *media_file_list, play_info_t *breaktag);
#endif
