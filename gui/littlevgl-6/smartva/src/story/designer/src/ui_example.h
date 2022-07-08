#ifndef __UI_EXAMPLE_H__
#define __UI_EXAMPLE_H__

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
	lv_obj_t *cont_hbar;
	lv_obj_t *img_hbar_power;
	lv_obj_t *img_hbar_wifi;
	lv_obj_t *label_hbar_timer;
	lv_obj_t *btn_hbar_return;
	lv_obj_t *img_hbar_return;
	lv_obj_t *btn_hbar_home;
	lv_obj_t *img_hbar_home;
	lv_obj_t *image_1;
} example_ui_t;


/**********************
 * functions
 **********************/
void example_auto_ui_create(example_ui_t *ui);
void example_auto_ui_destory(example_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_EXAMPLE_H__*/
