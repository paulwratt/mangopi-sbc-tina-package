#ifndef __UI_SETTING_H__
#define __UI_SETTING_H__

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
	lv_obj_t *line_1;
	lv_obj_t *line_2;
	lv_obj_t *label_setting;
	lv_obj_t *list_1;
	lv_obj_t *label_10;
	lv_obj_t *line_5;
	lv_obj_t *infomation_cont;
	lv_obj_t *container_1;
	lv_obj_t *label_sdk_version;
	lv_obj_t *label_sdk_val;
	lv_obj_t *label_os_version;
	lv_obj_t *label_os_val;
	lv_obj_t *label_ui_version;
	lv_obj_t *label_ui_val;
	lv_obj_t *line_19;
	lv_obj_t *line_20;
	lv_obj_t *container_3;
	lv_obj_t *label_screen_info;
	lv_obj_t *label_screen_val;
	lv_obj_t *button_def_setting;
	lv_obj_t *label_btn_def_setting;
	lv_obj_t *cont_dialog;
	lv_obj_t *label_dialog;
	lv_obj_t *image_btn_return;
	lv_obj_t *img_sett_return;
	lv_obj_t *display_cont;
	lv_obj_t *cont_backlight;
	lv_obj_t *slider_backlight;
	lv_obj_t *label_backlight;
	lv_obj_t *label_num_backlight;
	lv_obj_t *cont_enhance;
	lv_obj_t *label_enhance;
	lv_obj_t *line_7;
	lv_obj_t *switch_enhance;
	lv_obj_t *label_saturation;
	lv_obj_t *label_contrast;
	lv_obj_t *label_bright;
	lv_obj_t *slider_saturation;
	lv_obj_t *slider_contrast;
	lv_obj_t *slider_bright;
	lv_obj_t *line_17;
	lv_obj_t *line_18;
	lv_obj_t *label_num_saturation;
	lv_obj_t *label_num_contrast;
	lv_obj_t *label_num_bright;
	lv_obj_t *general_cont;
	lv_obj_t *label_volume;
	lv_obj_t *slider_volume;
	lv_obj_t *button_backlight;
	lv_obj_t *label_btn_backlight;
	lv_obj_t *label_volume_num;
	lv_obj_t *cont_time_set;
	lv_obj_t *image_hour1;
	lv_obj_t *image_hour2;
	lv_obj_t *image_min1;
	lv_obj_t *image_min2;
	lv_obj_t *button_hour_up;
	lv_obj_t *label_hour_up;
	lv_obj_t *button_hour_down;
	lv_obj_t *label_hour_down;
	lv_obj_t *button_min_up;
	lv_obj_t *label_min_up;
	lv_obj_t *button_min_down;
	lv_obj_t *label_min_down;
	lv_obj_t *image_time_dot;
} setting_ui_t;


/**********************
 * functions
 **********************/
void setting_auto_ui_create(setting_ui_t *ui);
void setting_auto_ui_destory(setting_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_SETTING_H__*/
