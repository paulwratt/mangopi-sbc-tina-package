#ifndef __UI_MOVIE_H__
#define __UI_MOVIE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *      includes
 **********************/
#include "lvgl.h"


/**********************
 *       variables
 **********************/
typedef struct
{
	uint8_t id;
	lv_obj_t *cont;
	lv_obj_t *media_list;
	lv_obj_t *image_4;
	lv_obj_t *container_1;
	lv_obj_t *play;
	lv_obj_t *progressbar;
	lv_obj_t *volume_bar;
	lv_obj_t *curr_time;
	lv_obj_t *total_time;
	lv_obj_t *next;
	lv_obj_t *last;
	lv_obj_t *play_mode;
	lv_obj_t *volume;
	lv_obj_t *button_menu;
	lv_obj_t *label_menu;
	lv_obj_t *file_name;
	lv_obj_t *download;
	lv_obj_t *order;
	lv_obj_t *full_button;
	lv_obj_t *online;
	lv_obj_t *label_online;
	lv_obj_t *ab_page;
	lv_obj_t *slider_a;
	lv_obj_t *slider_b;
	lv_obj_t *button_ok;
	lv_obj_t *label_AB_OK;
	lv_obj_t *label_1;
	lv_obj_t *label_2;
	lv_obj_t *label_a;
	lv_obj_t *label_b;
	lv_obj_t *text_area_1;
	lv_obj_t *button_esc;
	lv_obj_t *label_AB_ESC;
	lv_obj_t *menu_page;
	lv_obj_t *button_ab;
	lv_obj_t *label_AB;
	lv_obj_t *button_ratio;
	lv_obj_t *label_ratio;
	lv_obj_t *button_sound_track;
	lv_obj_t *label_sound_channel;
	lv_obj_t *button_audio_track;
	lv_obj_t *label_audio_track;
	lv_obj_t *button_exit;
	lv_obj_t *label_menu_esc;
	lv_obj_t *ratio_page;
	lv_obj_t *button_video_ratio;
	lv_obj_t *label_ratio_video;
	lv_obj_t *button_screen_ratio;
	lv_obj_t *label_ratio_screen;
	lv_obj_t *button_original_ratio;
	lv_obj_t *label_ratio_original;
	lv_obj_t *button_43_ratio;
	lv_obj_t *label_ratio_43;
	lv_obj_t *button_169_ratio;
	lv_obj_t *label_ratio_169;
	lv_obj_t *sound_track_page;
	lv_obj_t *button_stereo;
	lv_obj_t *label_sound_stereo;
	lv_obj_t *button_left_channel;
	lv_obj_t *label_sound_left;
	lv_obj_t *button_right_channel;
	lv_obj_t *label_sound_right;
	lv_obj_t *label_file_size;
	lv_obj_t *label_file_time;
	lv_obj_t *audio_track_list;
} movie_ui_t;


/**********************
 * functions
 **********************/
void movie_auto_ui_create(movie_ui_t *ui);
void movie_auto_ui_destory(movie_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_MOVIE_H__*/
