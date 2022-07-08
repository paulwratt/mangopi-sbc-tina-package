#ifndef __UI_HOME_H__
#define __UI_HOME_H__

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
	lv_obj_t *image_button_1;
	lv_obj_t *image_button_2;
	lv_obj_t *image_button_3;
	lv_obj_t *image_button_4;
	lv_obj_t *image_button_5;
	lv_obj_t *image_button_6;
	lv_obj_t *image_button_7;
	lv_obj_t *image_button_8;
	lv_obj_t *label_1;
	lv_obj_t *label_2;
	lv_obj_t *label_3;
	lv_obj_t *label_4;
	lv_obj_t *label_5;
	lv_obj_t *label_6;
	lv_obj_t *label_7;
	lv_obj_t *label_8;
	lv_obj_t *cont_hbar;
	lv_obj_t *img_hbar_power;
	lv_obj_t *img_hbar_wifi;
	lv_obj_t *label_hbar_timer;
	lv_obj_t *btn_hbar_return;
	lv_obj_t *img_hbar_return;
	lv_obj_t *btn_hbar_home;
	lv_obj_t *img_hbar_home;
	lv_obj_t *image_button_9;
	lv_obj_t *label_9;
	lv_obj_t *cont_dialog;
	lv_obj_t *label_dialog;
} home_ui_t;


/**********************
 * functions
 **********************/
void home_auto_ui_create(home_ui_t *ui);
void home_auto_ui_destory(home_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_HOME_H__*/
