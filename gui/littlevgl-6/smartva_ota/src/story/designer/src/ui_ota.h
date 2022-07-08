#ifndef __UI_OTA_H__
#define __UI_OTA_H__

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
	lv_obj_t *progressbar;
	lv_obj_t *label_progress;
	lv_obj_t *prompt_lable;
} ota_ui_t;


/**********************
 * functions
 **********************/
void ota_auto_ui_create(ota_ui_t *ui);
void ota_auto_ui_destory(ota_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_OTA_H__*/
