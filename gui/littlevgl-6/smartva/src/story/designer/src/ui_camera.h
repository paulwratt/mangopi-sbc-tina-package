#ifndef __UI_CAMERA_H__
#define __UI_CAMERA_H__

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
	lv_obj_t *back;
	lv_obj_t *page_1;
	lv_obj_t *take;
	lv_obj_t *menu;
	lv_obj_t *label_time;
	lv_obj_t *mode;
	lv_obj_t *label_mode;
	lv_obj_t *disk_info;
} camera_ui_t;


/**********************
 * functions
 **********************/
void camera_auto_ui_create(camera_ui_t *ui);
void camera_auto_ui_destory(camera_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_CAMERA_H__*/
