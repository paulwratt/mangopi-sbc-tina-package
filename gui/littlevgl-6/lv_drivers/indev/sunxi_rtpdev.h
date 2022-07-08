/**
 * @file: sunxi_rtpdev.h
 * @autor: liujiaming
 * @url: liujiaming@allwinnertech.com
 */
#ifndef SUNXI_RTPDEV_H
#define SUNXI_RTPDEV_H

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

#ifdef __RTP_USE__

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/**
 * Initialize the rtpdev interface
 */
void rtpdev_init(void);

/**
 * unitialize the rtpdev interface
 */
void rtpdev_uninit(void);

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool rtpdev_read(lv_indev_drv_t * drv, lv_indev_data_t * data);



#endif /* __RTP_USE__ */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SUNXI_RTPDEV_H */
