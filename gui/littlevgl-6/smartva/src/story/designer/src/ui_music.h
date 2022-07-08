#ifndef __UI_MUSIC_H__
#define __UI_MUSIC_H__

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
	lv_obj_t *list_1;
	lv_obj_t *list_2;
	lv_obj_t *list_3;
	lv_obj_t *image_1;
	lv_obj_t *image_2;
	lv_obj_t *container_1;
	lv_obj_t *btn_play;
	lv_obj_t *progressbar;
	lv_obj_t *volume_bar;
	lv_obj_t *curr_time;
	lv_obj_t *total_time;
	lv_obj_t *next;
	lv_obj_t *last;
	lv_obj_t *play_mode;
	lv_obj_t *volume;
	lv_obj_t *button_menu;
	lv_obj_t *menu_label;
	lv_obj_t *file_name;
	lv_obj_t *lrc;
	lv_obj_t *lrc_no;
	lv_obj_t *online;
	lv_obj_t *label_online;
	lv_obj_t *lrc_list;
	lv_obj_t *spectrum;
	lv_obj_t *download;
	lv_obj_t *ab_page;
	lv_obj_t *label_1;
	lv_obj_t *slider_a;
	lv_obj_t *label_slider_a;
	lv_obj_t *label_2;
	lv_obj_t *slider_b;
	lv_obj_t *label_slider_b;
	lv_obj_t *button_ab_ok;
	lv_obj_t *label_AB_OK;
	lv_obj_t *button_ab_esc;
	lv_obj_t *label_AB_ESC;
	lv_obj_t *page_menu;
	lv_obj_t *button_sound_effect;
	lv_obj_t *label_sound_effect;
	lv_obj_t *button_song_information;
	lv_obj_t *label_song_information;
	lv_obj_t *button_ab;
	lv_obj_t *label_AB;
	lv_obj_t *button_page_menu_exit;
	lv_obj_t *label_menu_esc;
	lv_obj_t *list_sound_effect;
	lv_obj_t *list_song_information;
	lv_obj_t *button_file_source;
	lv_obj_t *label_file_source;
} music_ui_t;


/**********************
 * functions
 **********************/
void music_auto_ui_create(music_ui_t *ui);
void music_auto_ui_destory(music_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_MUSIC_H__*/
