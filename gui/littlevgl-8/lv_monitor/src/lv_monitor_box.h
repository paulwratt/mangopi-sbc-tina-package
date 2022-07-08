/**
 * @file lv_monitor_box.h
 *
 */

#ifndef LV_MONITOR_BOX_H
#define LV_MONITOR_BOX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/src/lv_conf_internal.h"
#include "lvgl/src/core/lv_obj.h"
#include "lvgl/src/extra/widgets/meter/lv_meter.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

enum {
    LV_MONITOR_BOX_CPU_TEST,
    LV_MONITOR_BOX_HARDWARE_TEST,
    LV_MONITOR_BOX_DDR_TEST,
    LV_MONITOR_BOX_VIDEO_TEST,
    LV_MONITOR_BOX_CAMERA_TEST,
    LV_MONITOR_BOX_NETWORK_TEST,
};

enum {
    LV_MONITOR_BOX_DDR_TOTAL,
    LV_MONITOR_BOX_DDR_RISCV,
    LV_MONITOR_BOX_DDR_CPU,
    LV_MONITOR_BOX_DDR_CE,
    LV_MONITOR_BOX_DDR_CSI,
    LV_MONITOR_BOX_DDR_ISP,
    LV_MONITOR_BOX_DDR_DE0,
    LV_MONITOR_BOX_DDR_DE1,
    LV_MONITOR_BOX_DDR_DI,
    LV_MONITOR_BOX_DDR_DMA,
    LV_MONITOR_BOX_DDR_IOMMU,
    LV_MONITOR_BOX_DDR_MAHB,
    LV_MONITOR_BOX_DDR_TVD,
    LV_MONITOR_BOX_DDR_VE,
    LV_MONITOR_BOX_DDR_DSP,
    LV_MONITOR_BOX_DDR_G2D,
    LV_MONITOR_BOX_DDR_GPU,
    LV_MONITOR_BOX_DDR_NDFC,
    LV_MONITOR_BOX_DDR_RDMA,
    LV_MONITOR_BOX_DDR_AIPU,
    LV_MONITOR_BOX_DDR_EINK,
    LV_MONITOR_BOX_DDR_OTHER,
    LV_MONITOR_BOX_DDR_END,
};

typedef struct {
    uint32_t bw_value;
    uint32_t bw_type;
}ddr_bw;

typedef struct {
    lv_obj_t obj;
    uint32_t type;
    char title[32];
    /* CPU TEST */
    lv_obj_t *label_cpu_usage;
    lv_obj_t *meter_cpu_usage;
    lv_meter_indicator_t *indic_cpu_usage[4];
    lv_obj_t *slider_label_cpu_usage;
    uint32_t set_cpu_usage;
    /* HARDWARE TEST */
    lv_obj_t *slider_label_gpu_usage;
    lv_obj_t *label_cpu_freq;
    lv_obj_t *label_cpu_online;
    lv_obj_t *label_gpu_usage;
    lv_obj_t *bar_gpu_temp;
    lv_obj_t *bar_ddr_temp;
    lv_obj_t *bar_cpu_temp;
    lv_obj_t *label_cpu_temp;
    lv_obj_t *label_ddr_temp;
    lv_obj_t *label_gpu_temp;
    /* DDR INFO */
    lv_obj_t *label_ddr_bw_title;
    lv_obj_t *obj_ddr_bw_info[LV_MONITOR_BOX_DDR_END];
    lv_obj_t *label_ddr_bw_info[LV_MONITOR_BOX_DDR_END];
    lv_obj_t *bar_ddr_bw_info[LV_MONITOR_BOX_DDR_END];
    lv_obj_t *label_ddr_bw_value[LV_MONITOR_BOX_DDR_END];
    lv_style_t style_ddr_indic[LV_MONITOR_BOX_DDR_END];

    uint32_t ddr_bw_y[LV_MONITOR_BOX_DDR_END];
    ddr_bw ddr_bw_info[LV_MONITOR_BOX_DDR_END];
    uint32_t ddr_bw_width;
    /* VIDEO INFO */
    lv_event_cb_t event_video_play_cb;
    /* CAMERA INFO */
    lv_obj_t *dropdown_camera_resolution;
    /* NETWORK INFO*/
    lv_obj_t *checkbox_download;
    lv_obj_t *checkbox_upload;
    lv_obj_t *checkbox_wifi_function;
}lv_monitor_box_t;

extern const lv_obj_class_t lv_monitor_box_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a monitor box object
 * @param parent    pointer to an object, it will be the parent of the monitor box
 * @return          pointer to the created monitor box
 */
lv_obj_t * lv_monitor_box_create(lv_obj_t *parent);

/**
 * Create a monitor box object
 * @param obj    pointer to an object
 */
void lv_monitor_box_set_type(lv_obj_t *obj, uint32_t type);

/**
 * Set cpu info
 * @param parent        pointer to an object
 * @param cpu_usage     all cpu usage, the range is 0~100
 * @param cpu0_usage    cpu0 usage, the range is 0~100
 * @param cpu1_usage    cpu1 usage, the range is 0~100
 * @param cpu2_usage    cpu2 usage, the range is 0~100
 * @param cpu3_usage    cpu3 usage, the range is 0~100
 */
void lv_monitor_box_set_cpu_info(lv_obj_t *obj, const char *cpu_online,
		uint32_t cpu_usage, uint32_t cpu0_usage, uint32_t cpu1_usage,
		uint32_t cpu2_usage, uint32_t cpu3_usage);

/**
 * Set hardware info
 * @param parent         pointer to an object
 * @param cpu_online     cpu online, ex: 0-3
 * @param cpu_freq       cpu freq(GHz), ex: 0.4 0.4 0.4 0.4
 * @param gpu_usage      gpu usage, the range is 0~100
 * @param cpu_temp       cpu temp, the range is 0~125
 * @param ddr_temp       ddr temp, the range is 0~125
 * @param gpu_temp       gpu temp, the range is 0~125
 */
void lv_monitor_box_set_hardware_info(lv_obj_t *obj, const char *cpu_freq,
		uint32_t gpu_usage, uint32_t cpu_temp,
        uint32_t ddr_temp, uint32_t gpu_temp);

/**
 * Set ddr bandwidth info
 * @param parent        pointer to an object
 * @param ddr_type      ddr type, ex: LV_MONITOR_BOX_DDR_TOTAL
 * @param ddr_value     ddr value, ex: 274780
 */
void lv_monitor_box_set_ddr_info(lv_obj_t *obj, const uint32_t ddr_type,
        const uint32_t ddr_value);

void lv_monitor_box_set_video_play_cb(lv_obj_t *obj, lv_event_cb_t event_cb);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_MONITOR_BOX_H*/
