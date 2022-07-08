#ifndef __UI_PHOTO_H__
#define __UI_PHOTO_H__

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
	lv_obj_t *back;
	lv_obj_t *file_name;
	lv_obj_t *image_button_preview;
	lv_obj_t *button_setting_interface;
	lv_obj_t *container_control;
	lv_obj_t *image_button_play_start;
	lv_obj_t *image_button_rotate_left;
	lv_obj_t *image_button_rotate_right;
	lv_obj_t *image_button_scaler_down;
	lv_obj_t *image_button_scaler_up;
	lv_obj_t *button_show_effect;
	lv_obj_t *label_effect;
	lv_obj_t *button_show_speed;
	lv_obj_t *label_speed;
	lv_obj_t *button_info;
	lv_obj_t *label_info;
	lv_obj_t *container_sel_effect;
	lv_obj_t *button_matrix_effect;
	lv_obj_t *image_set_effect;
	lv_obj_t *container_sel_speed;
	lv_obj_t *button_matrix_speed;
	lv_obj_t *image_set_speed;
	lv_obj_t *container_dialog;
	lv_obj_t *container_1;
	lv_obj_t *button_1;
	lv_obj_t *label_dialog_ok;
	lv_obj_t *label_dialog_info;
	lv_obj_t *container_info;
	lv_obj_t *image_get_info;
	lv_obj_t *label_title_filename;
	lv_obj_t *label_title_file_size;
	lv_obj_t *label_title_w_h;
	lv_obj_t *label_filename;
	lv_obj_t *label_file_size;
	lv_obj_t *label_w_h;
	lv_obj_t *label_title_file_time;
	lv_obj_t *label_file_time;
} photo_ui_t;


/**********************
 * functions
 **********************/
void photo_auto_ui_create(photo_ui_t *ui);
void photo_auto_ui_destory(photo_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_PHOTO_H__*/
