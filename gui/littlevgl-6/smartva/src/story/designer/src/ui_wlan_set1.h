#ifndef __UI_WLAN_SET1_H__
#define __UI_WLAN_SET1_H__

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
	lv_obj_t *container_bg;
	lv_obj_t *cont_hbar;
	lv_obj_t *img_hbar_power;
	lv_obj_t *img_hbar_wifi;
	lv_obj_t *label_hbar_timer;
	lv_obj_t *btn_hbar_return;
	lv_obj_t *img_hbar_return;
	lv_obj_t *btn_hbar_home;
	lv_obj_t *img_hbar_home;
	lv_obj_t *label_1;
	lv_obj_t *list_1;
	lv_obj_t *container_1;
	lv_obj_t *keyboard_1;
	lv_obj_t *label_2;
	lv_obj_t *text_area_1;
	lv_obj_t *button_1;
	lv_obj_t *label_3;
	lv_obj_t *button_2;
	lv_obj_t *label_4;
	lv_obj_t *label_5;
	lv_obj_t *container_mark;
} wlan_set1_ui_t;


/**********************
 * functions
 **********************/
void wlan_set1_auto_ui_create(wlan_set1_ui_t *ui);
void wlan_set1_auto_ui_destory(wlan_set1_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_WLAN_SET1_H__*/
