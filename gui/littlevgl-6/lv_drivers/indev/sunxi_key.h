/**
 * @file: sunxi_key.h
 * @autor: huangyixiu
 * @url: huangyixiu@allwinnertech.com
 */
#ifndef SUNXI_KEY_H
#define SUNXI_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifndef LV_DRV_NO_CONF
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "../../lv_drv_conf.h"
#endif
#endif

#if USE_SUNXI_KEY

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

typedef void (*p_keydev_callback)(uint32_t key, lv_indev_state_t state);
void keydev_init(void);
void keydev_uninit(void);
bool keydev_read(lv_indev_drv_t * drv, lv_indev_data_t * data);
void keydev_register_hook(p_keydev_callback func);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
