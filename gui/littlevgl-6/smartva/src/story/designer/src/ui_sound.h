#ifndef __UI_SOUND_H__
#define __UI_SOUND_H__

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
	lv_obj_t *take;
	lv_obj_t *label_time;
	lv_obj_t *media_list;
	lv_obj_t *container_1;
	lv_obj_t *play;
	lv_obj_t *progressbar;
	lv_obj_t *volume_bar;
	lv_obj_t *curr_time;
	lv_obj_t *total_time;
	lv_obj_t *volume;
	lv_obj_t *warn_info;
	lv_obj_t *delete;
	lv_obj_t *de_label;
} sound_ui_t;


/**********************
 * functions
 **********************/
void sound_auto_ui_create(sound_ui_t *ui);
void sound_auto_ui_destory(sound_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_SOUND_H__*/
