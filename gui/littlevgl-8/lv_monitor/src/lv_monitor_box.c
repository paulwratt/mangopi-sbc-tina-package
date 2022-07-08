/**
 * @file lv_monitor_box.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_monitor_box.h"
#if LV_USE_BTN != 0

#include <stdio.h>
#include "lvgl/src/widgets/lv_slider.h"
#include "lvgl/src/extra/layouts/flex/lv_flex.h"
#include "middle_ware/media_file_play.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_monitor_box_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_monitor_box_constructor(const lv_obj_class_t *class_p,
        lv_obj_t *obj);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_monitor_box_class = {
    .constructor_cb = lv_monitor_box_constructor,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_monitor_box_t),
    .base_class = &lv_obj_class
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t* lv_monitor_box_create(lv_obj_t *parent) {
    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void slider_cpu_usage_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *slider = lv_event_get_target(e);
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) lv_obj_get_parent(
            slider);

    if (code == LV_EVENT_VALUE_CHANGED) {
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d%%", lv_slider_get_value(slider));
        lv_label_set_text(monitor_box->slider_label_cpu_usage, buf);
    } else if (code == LV_EVENT_RELEASED) {
        printf("slider_cpu_usage_event_cb Value: %d\n",
                lv_slider_get_value(slider));
        monitor_box->set_cpu_usage = lv_slider_get_value(slider);
	}
}

static void slider_gpu_usage_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *slider = lv_event_get_target(e);
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) lv_obj_get_parent(
            slider);

    if (code == LV_EVENT_VALUE_CHANGED) {
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d%%", lv_slider_get_value(slider));
        lv_label_set_text(monitor_box->slider_label_gpu_usage, buf);
    } else if (code == LV_EVENT_RELEASED)
        printf("slider_gpu_usage_event_cb Value: %d\n",
                lv_slider_get_value(slider));
}

static void temp_switch_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_RELEASED)
        printf("temp_switch_event_handler State: %s\n",
                lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
}

static void video_button_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) lv_obj_get_parent(btn);
    /* Transparent background style */
    static lv_style_t style_scr_act;
    if (style_scr_act.prop_cnt == 0) {
        lv_style_init(&style_scr_act);
        lv_style_set_bg_opa(&style_scr_act, LV_OPA_COVER);
        lv_obj_add_style(lv_scr_act(), &style_scr_act, 0);
    }

    if (code == LV_EVENT_RELEASED) {
        if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
            lv_label_set_text(label, "Stop");
            lv_disp_get_default()->driver->screen_transp = 1;
            lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
            /* Empty the buffer, not emptying will cause the UI to be opaque */
            lv_memset_00(lv_disp_get_default()->driver->draw_buf->buf_act,
                    lv_disp_get_default()->driver->draw_buf->size
                            * sizeof(lv_color32_t));
            lv_style_set_bg_opa(&style_scr_act, LV_OPA_TRANSP);
            lv_obj_report_style_change(&style_scr_act);

            if (monitor_box->event_video_play_cb)
                monitor_box->event_video_play_cb(e);
        } else {
            lv_label_set_text(label, "Play");
            lv_disp_get_default()->driver->screen_transp = 0;
            lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
            lv_style_set_bg_opa(&style_scr_act, LV_OPA_COVER);
            lv_obj_report_style_change(&style_scr_act);

            if (monitor_box->event_video_play_cb)
                monitor_box->event_video_play_cb(e);
        }
    }
}

static void camera_button_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) lv_obj_get_parent(btn);

    if (code == LV_EVENT_RELEASED) {
        if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
            lv_label_set_text(label, "Stop");
            int index = lv_dropdown_get_selected(
                    monitor_box->dropdown_camera_resolution);
            printf("camera_button_event_handler Index: %d\n", index);
        } else {
            lv_label_set_text(label, "Preview");
        }
    }
}

static void network_button_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) lv_obj_get_parent(btn);

    if (code == LV_EVENT_RELEASED) {
        if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
            lv_label_set_text(label, "Stop");
            char *state1 =
                    lv_obj_get_state(monitor_box->checkbox_download)
                            & LV_STATE_CHECKED ? "Checked" : "Unchecked";
            char *state2 =
                    lv_obj_get_state(monitor_box->checkbox_upload)
                            & LV_STATE_CHECKED ? "Checked" : "Unchecked";
            char *state3 =
                    lv_obj_get_state(monitor_box->checkbox_wifi_function)
                            & LV_STATE_CHECKED ? "Checked" : "Unchecked";
            printf("network_button_event_handler State: %s %s %s\n", state1,
                    state2, state3);
        } else {
            lv_label_set_text(label, "Start");
        }
    }
}

static void lv_monitor_box_cpu_test(lv_obj_t *obj) {
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);

    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_bg_color(&style_title,
            lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_bg_opa(&style_title, LV_OPA_COVER);

    lv_obj_t *label_title = lv_label_create(obj);
    lv_label_set_recolor(label_title, true);
    lv_label_set_text_fmt(label_title, "#ffffff %s #", monitor_box->title);
    lv_obj_set_size(label_title, w, LV_SIZE_CONTENT);
    lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(label_title, &style_title, 0);

    monitor_box->label_cpu_usage = lv_label_create(obj);
    lv_obj_set_size(monitor_box->label_cpu_usage, w, LV_SIZE_CONTENT);
    lv_label_set_long_mode(monitor_box->label_cpu_usage,
            LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text_fmt(monitor_box->label_cpu_usage, "%s", "CPU-Usage:\n0%");
    lv_obj_align_to(monitor_box->label_cpu_usage, label_title,
            LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);

    /* Create an meter */
    monitor_box->meter_cpu_usage = lv_meter_create(obj);

    /*Remove the circle from the middle*/
    lv_obj_remove_style(monitor_box->meter_cpu_usage, NULL, LV_PART_MAIN);
    lv_obj_remove_style(monitor_box->meter_cpu_usage, NULL, LV_PART_INDICATOR);

    if (w > h)
        lv_obj_set_size(monitor_box->meter_cpu_usage, w * 0.5, w * 0.5);
    else
        lv_obj_set_size(monitor_box->meter_cpu_usage, h * 0.5, h * 0.5);

    lv_obj_center(monitor_box->meter_cpu_usage);

    /*Add a scale first*/
    lv_meter_scale_t *scale = lv_meter_add_scale(monitor_box->meter_cpu_usage);
    lv_meter_set_scale_ticks(monitor_box->meter_cpu_usage, scale, 6, 2, 10,
            lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(monitor_box->meter_cpu_usage, scale, 1, 2,
            30, lv_color_hex3(0xeee), 10);
    lv_meter_set_scale_range(monitor_box->meter_cpu_usage, scale, 0, 100, 270,
            90);

    /*Add a three arc indicator*/
    monitor_box->indic_cpu_usage[0] = lv_meter_add_arc(
            monitor_box->meter_cpu_usage, scale, 4,
            lv_palette_main(LV_PALETTE_RED), 0);
    monitor_box->indic_cpu_usage[1] = lv_meter_add_arc(
            monitor_box->meter_cpu_usage, scale, 4,
            lv_palette_main(LV_PALETTE_GREEN), -10);
    monitor_box->indic_cpu_usage[2] = lv_meter_add_arc(
            monitor_box->meter_cpu_usage, scale, 4,
            lv_palette_main(LV_PALETTE_BLUE), -20);
    monitor_box->indic_cpu_usage[3] = lv_meter_add_arc(
            monitor_box->meter_cpu_usage, scale, 4,
            lv_palette_main(LV_PALETTE_YELLOW), -30);

    lv_meter_set_indicator_end_value(monitor_box->meter_cpu_usage,
            monitor_box->indic_cpu_usage[0], 0);
    lv_meter_set_indicator_end_value(monitor_box->meter_cpu_usage,
            monitor_box->indic_cpu_usage[1], 0);
    lv_meter_set_indicator_end_value(monitor_box->meter_cpu_usage,
            monitor_box->indic_cpu_usage[2], 0);
    lv_meter_set_indicator_end_value(monitor_box->meter_cpu_usage,
            monitor_box->indic_cpu_usage[3], 0);

    /*Create a label below the slider*/
    monitor_box->slider_label_cpu_usage = lv_label_create(obj);
    lv_label_set_text(monitor_box->slider_label_cpu_usage, "0%");
    lv_obj_align(monitor_box->slider_label_cpu_usage, LV_ALIGN_BOTTOM_MID, 0,
            -4);

    /*Create a slider in the center of the display*/
    lv_obj_t *slider_cpu_usage = lv_slider_create(obj);
    lv_obj_set_size(slider_cpu_usage, w * 0.8, h * 0.04);
    lv_obj_align_to(slider_cpu_usage, monitor_box->slider_label_cpu_usage,
            LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_obj_add_event_cb(slider_cpu_usage, slider_cpu_usage_event_cb,
            LV_EVENT_ALL, NULL);

	/*CPU Online*/
    monitor_box->label_cpu_online = lv_label_create(obj);
    lv_label_set_text_fmt(monitor_box->label_cpu_online, "%s",
            "CPU-Online:\nCPU 0 1 2 3 ");
    lv_obj_align_to(monitor_box->label_cpu_online, slider_cpu_usage,
            LV_ALIGN_OUT_TOP_RIGHT, w * 0.1 - 4, -(h * 0.04));
}

static void lv_monitor_box_hardware_test(lv_obj_t *obj) {
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);

    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_bg_color(&style_title,
            lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_bg_opa(&style_title, LV_OPA_COVER);

    lv_obj_t *label_title = lv_label_create(obj);
    lv_label_set_recolor(label_title, true);
    lv_label_set_text_fmt(label_title, "#ffffff %s #", monitor_box->title);
    lv_obj_set_size(label_title, w, LV_SIZE_CONTENT);
    lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(label_title, &style_title, 0);

    lv_obj_t *label_sw = lv_label_create(obj);
    lv_label_set_text_fmt(label_sw, "%s", "Temp control");
    lv_obj_align_to(label_sw, label_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    lv_obj_t *sw_temp = lv_switch_create(obj);
    lv_obj_set_size(sw_temp, w * 0.2, h * 0.1);
    lv_obj_add_event_cb(sw_temp, temp_switch_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align_to(sw_temp, label_sw, LV_ALIGN_OUT_RIGHT_MID, 4, 2);

    monitor_box->label_cpu_freq = lv_label_create(obj);
    lv_label_set_text_fmt(monitor_box->label_cpu_freq, "%s",
            "CPU-Freq(GHz):\n0.0 0.0 0.0 0.0");
    lv_obj_align(monitor_box->label_cpu_freq, LV_ALIGN_LEFT_MID, 0, 0);

    monitor_box->label_gpu_usage = lv_label_create(obj);
    lv_label_set_text_fmt(monitor_box->label_gpu_usage, "%s", "GPU-Usage:\n0%");
    lv_obj_align_to(monitor_box->label_gpu_usage, monitor_box->label_cpu_freq,
            LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    static lv_style_t style_indic;

    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);

    monitor_box->bar_gpu_temp = lv_bar_create(obj);
    lv_obj_add_style(monitor_box->bar_gpu_temp, &style_indic,
            LV_PART_INDICATOR);
    lv_obj_set_size(monitor_box->bar_gpu_temp, w * 0.05, h * 0.5);
    lv_obj_align(monitor_box->bar_gpu_temp, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_bar_set_range(monitor_box->bar_gpu_temp, 0, 125);
    lv_bar_set_value(monitor_box->bar_gpu_temp, 0, LV_ANIM_ON);

    lv_obj_t *label_gpu_bar = lv_label_create(obj);
    lv_label_set_text_fmt(label_gpu_bar, "%s", "G\nP\nU");
    lv_obj_align_to(label_gpu_bar, monitor_box->bar_gpu_temp,
            LV_ALIGN_OUT_LEFT_MID, 0, 0);

    monitor_box->bar_ddr_temp = lv_bar_create(obj);
    lv_obj_add_style(monitor_box->bar_ddr_temp, &style_indic,
            LV_PART_INDICATOR);
    lv_obj_set_size(monitor_box->bar_ddr_temp, w * 0.05, h * 0.5);
    lv_obj_align_to(monitor_box->bar_ddr_temp, label_gpu_bar,
            LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_bar_set_range(monitor_box->bar_ddr_temp, 0, 125);
    lv_bar_set_value(monitor_box->bar_ddr_temp, 0, LV_ANIM_ON);

    lv_obj_t *label_ddr_bar = lv_label_create(obj);
    lv_label_set_text_fmt(label_ddr_bar, "%s", "D\nD\nR");
    lv_obj_align_to(label_ddr_bar, monitor_box->bar_ddr_temp,
            LV_ALIGN_OUT_LEFT_MID, 0, 0);

    monitor_box->bar_cpu_temp = lv_bar_create(obj);
    lv_obj_add_style(monitor_box->bar_cpu_temp, &style_indic,
            LV_PART_INDICATOR);
    lv_obj_set_size(monitor_box->bar_cpu_temp, w * 0.05, h * 0.5);
    lv_obj_align_to(monitor_box->bar_cpu_temp, label_ddr_bar,
            LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_bar_set_range(monitor_box->bar_cpu_temp, 0, 125);
    lv_bar_set_value(monitor_box->bar_cpu_temp, 0, LV_ANIM_ON);

    lv_obj_t *label_cpu_bar = lv_label_create(obj);
    lv_label_set_text_fmt(label_cpu_bar, "%s", "C\nP\nU");
    lv_obj_align_to(label_cpu_bar, monitor_box->bar_cpu_temp,
            LV_ALIGN_OUT_LEFT_MID, 0, 0);

    monitor_box->label_cpu_temp = lv_label_create(obj);
    lv_label_set_text_fmt(monitor_box->label_cpu_temp, "%d", 0);
    lv_obj_align_to(monitor_box->label_cpu_temp, monitor_box->bar_cpu_temp,
            LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    monitor_box->label_ddr_temp = lv_label_create(obj);
    lv_label_set_text_fmt(monitor_box->label_ddr_temp, "%d", 0);
    lv_obj_align_to(monitor_box->label_ddr_temp, monitor_box->bar_ddr_temp,
            LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    monitor_box->label_gpu_temp = lv_label_create(obj);
    lv_label_set_text_fmt(monitor_box->label_gpu_temp, "%d", 0);
    lv_obj_align_to(monitor_box->label_gpu_temp, monitor_box->bar_gpu_temp,
            LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    /*Create a label below the slider*/
    monitor_box->slider_label_gpu_usage = lv_label_create(obj);
    lv_label_set_text(monitor_box->slider_label_gpu_usage, "0%");
    lv_obj_align(monitor_box->slider_label_gpu_usage, LV_ALIGN_BOTTOM_MID, 0,
            -4);

    /*Create a slider in the center of the display*/
    lv_obj_t *slider_gpu_usage = lv_slider_create(obj);
    lv_obj_set_size(slider_gpu_usage, w * 0.8, h * 0.04);
    lv_obj_align_to(slider_gpu_usage, monitor_box->slider_label_gpu_usage,
            LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_obj_add_event_cb(slider_gpu_usage, slider_gpu_usage_event_cb,
            LV_EVENT_ALL, NULL);
}

static void lv_monitor_box_ddr_test_bar(lv_obj_t *obj, lv_obj_t *align_obj,
        uint32_t type) {
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);
    uint32_t defaut_ddr = 274780;
    char *name[LV_MONITOR_BOX_DDR_END] = { "TOTAL  ", "RISCV", "CPU", "CE",
            "CSI", "ISP", "DE0", "DE1", "DI", "DMA", "IOMMU", "MAHB", "TVD",
            "VE", "DSP", "G2D", "GPU", "NDFC", "RDMA", "AIPU", "EINK", "OTHER" };

    static lv_style_t style_box;
    if (!style_box.prop_cnt) {
        lv_style_init(&style_box);
        lv_style_set_radius(&style_box, 0);
        lv_style_set_pad_top(&style_box, 0);
        lv_style_set_pad_left(&style_box, 0);
        lv_style_set_border_width(&style_box, 0);
        lv_style_set_bg_opa(&style_box, LV_OPA_COVER);
    }

    monitor_box->obj_ddr_bw_info[type] = lv_obj_create(obj);
    lv_obj_clear_flag(monitor_box->obj_ddr_bw_info[type],
            LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(monitor_box->obj_ddr_bw_info[type], &style_box, 0);
    lv_obj_align_to(monitor_box->obj_ddr_bw_info[type], align_obj,
            LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    monitor_box->label_ddr_bw_info[type] = lv_label_create(
            monitor_box->obj_ddr_bw_info[type]);
    lv_label_set_text_fmt(monitor_box->label_ddr_bw_info[type], "%s",
            name[type]);
    lv_obj_align(monitor_box->label_ddr_bw_info[type], LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_refr_size(monitor_box->label_ddr_bw_info[type]);

    lv_obj_set_size(monitor_box->obj_ddr_bw_info[type], w,
            lv_obj_get_height(monitor_box->label_ddr_bw_info[type]));

    lv_obj_refr_size(monitor_box->obj_ddr_bw_info[type]);
    lv_obj_refr_pos(monitor_box->obj_ddr_bw_info[type]);

    monitor_box->ddr_bw_y[type] = lv_obj_get_y(
            monitor_box->obj_ddr_bw_info[type]);
    monitor_box->ddr_bw_width = w
            - lv_obj_get_width(monitor_box->label_ddr_bw_info[0]);

    lv_style_init(&monitor_box->style_ddr_indic[type]);
    lv_style_set_bg_opa(&monitor_box->style_ddr_indic[type], LV_OPA_COVER);

    if (type < _LV_PALETTE_LAST)
        lv_style_set_bg_color(&monitor_box->style_ddr_indic[type],
                lv_palette_main(type));
    else
        lv_style_set_bg_color(&monitor_box->style_ddr_indic[type],
                lv_palette_lighten(type - _LV_PALETTE_LAST, 2));
    lv_style_set_radius(&monitor_box->style_ddr_indic[type], 0);

    monitor_box->bar_ddr_bw_info[type] = lv_bar_create(
            monitor_box->obj_ddr_bw_info[type]);
    lv_obj_remove_style_all(monitor_box->bar_ddr_bw_info[type]);
    lv_obj_add_style(monitor_box->bar_ddr_bw_info[type],
            &monitor_box->style_ddr_indic[type], LV_PART_INDICATOR);
    lv_obj_set_size(monitor_box->bar_ddr_bw_info[type],
            monitor_box->ddr_bw_width - 6,
            lv_obj_get_height(monitor_box->label_ddr_bw_info[type]));

    lv_obj_align_to(monitor_box->bar_ddr_bw_info[type],
            monitor_box->label_ddr_bw_info[type], LV_ALIGN_OUT_RIGHT_MID, 2, 0);
    lv_obj_set_x(monitor_box->bar_ddr_bw_info[type],
            lv_obj_get_x2(monitor_box->label_ddr_bw_info[0]));
    lv_bar_set_value(monitor_box->bar_ddr_bw_info[type], 100, LV_ANIM_OFF);

    /* Calculate the width of the label */
    lv_obj_t *label = lv_label_create(monitor_box->obj_ddr_bw_info[type]);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text_fmt(label, "%d", defaut_ddr);
    lv_obj_refr_size(label);

    monitor_box->label_ddr_bw_value[type] = lv_label_create(
            monitor_box->obj_ddr_bw_info[type]);
    lv_label_set_recolor(monitor_box->label_ddr_bw_value[type], true);

    monitor_box->ddr_bw_info[type].bw_value = defaut_ddr;
    monitor_box->ddr_bw_info[type].bw_type = type;
    if (lv_obj_get_width(label)
            < lv_obj_get_width(monitor_box->bar_ddr_bw_info[type])) {
        lv_label_set_text_fmt(monitor_box->label_ddr_bw_value[type],
                "#ffffff %d #", defaut_ddr);
        lv_obj_align_to(monitor_box->label_ddr_bw_value[type],
                monitor_box->bar_ddr_bw_info[type], LV_ALIGN_RIGHT_MID, 0, 0);
    } else {
        lv_label_set_text_fmt(monitor_box->label_ddr_bw_value[type],
                "#888888 %d #", defaut_ddr);
        lv_obj_align_to(monitor_box->label_ddr_bw_value[type],
                monitor_box->bar_ddr_bw_info[type], LV_ALIGN_OUT_RIGHT_MID, 2,
                0);
    }

    lv_obj_del(label);
}

static void anim_y_cb(void * var, int32_t v)
{
    lv_obj_set_y(var, v);
}

static void lv_monitor_box_ddr_test(lv_obj_t *obj) {
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);

    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_bg_color(&style_title,
            lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_bg_opa(&style_title, LV_OPA_COVER);

    monitor_box->label_ddr_bw_title = lv_label_create(obj);
    lv_label_set_recolor(monitor_box->label_ddr_bw_title, true);
    lv_label_set_text_fmt(monitor_box->label_ddr_bw_title, "#ffffff %s #",
            monitor_box->title);
    lv_obj_set_size(monitor_box->label_ddr_bw_title, w, LV_SIZE_CONTENT);
    lv_obj_align(monitor_box->label_ddr_bw_title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(monitor_box->label_ddr_bw_title, &style_title, 0);

    uint32_t i;
    for (i = 0; i < LV_MONITOR_BOX_DDR_END; i++) {
        if (i == 0)
            lv_monitor_box_ddr_test_bar(obj, monitor_box->label_ddr_bw_title,
                    i);
        else {
            lv_monitor_box_ddr_test_bar(obj,
                    monitor_box->obj_ddr_bw_info[i - 1], i);
        }
    }

    for (i = 1; i < LV_MONITOR_BOX_DDR_END; i++) {
        lv_obj_add_flag(monitor_box->obj_ddr_bw_info[i], LV_OBJ_FLAG_HIDDEN);
        lv_monitor_box_set_ddr_info(obj, i, 0);
    }
}

static void lv_monitor_box_video_test(lv_obj_t *obj) {
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);

    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_bg_color(&style_title,
            lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_bg_opa(&style_title, LV_OPA_COVER);

    lv_obj_t *label_title = lv_label_create(obj);
    lv_label_set_recolor(label_title, true);
    lv_label_set_text_fmt(label_title, "#ffffff %s #", monitor_box->title);
    lv_obj_set_size(label_title, w, LV_SIZE_CONTENT);
    lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(label_title, &style_title, 0);

    char *path;
    path = malloc(FILE_PATH_MAXT_LEN * sizeof(char));
    lv_obj_t *label_video_path = lv_label_create(obj);
    lv_obj_set_size(label_video_path, w, LV_SIZE_CONTENT);
    lv_label_set_long_mode(label_video_path, LV_LABEL_LONG_WRAP);
    if (media_get_current_file_path(path) == 0) {
        lv_label_set_text_fmt(label_video_path, "%s", path);
    } else {
        lv_label_set_text_fmt(label_video_path, "%s",
                "/mnt/UDISK not video file");
    }
    free(path);
    lv_obj_align_to(label_video_path, label_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0,
            2);

    lv_obj_t *btn_video = lv_btn_create(obj);
    lv_obj_add_event_cb(btn_video, video_button_event_handler, LV_EVENT_ALL,
            NULL);
    lv_obj_center(btn_video);
    lv_obj_add_flag(btn_video, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_t *label_btn = lv_label_create(btn_video);
    lv_label_set_text(label_btn, "Play");
    lv_obj_center(label_btn);
}

static void lv_monitor_box_camera_test(lv_obj_t *obj) {
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);

    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_bg_color(&style_title,
            lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_bg_opa(&style_title, LV_OPA_COVER);

    lv_obj_t *labe_title = lv_label_create(obj);
    lv_label_set_recolor(labe_title, true);
    lv_label_set_text_fmt(labe_title, "#ffffff %s #", monitor_box->title);
    lv_obj_set_size(labe_title, w, LV_SIZE_CONTENT);
    lv_obj_align(labe_title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(labe_title, &style_title, 0);

    /*Create a normal drop down list*/
    monitor_box->dropdown_camera_resolution = lv_dropdown_create(obj);
    lv_dropdown_set_options(monitor_box->dropdown_camera_resolution, "640*480\n"
            "1280x720\n"
            "1920x1080");

    lv_obj_set_size(monitor_box->dropdown_camera_resolution, w / 2,
            LV_SIZE_CONTENT);
    lv_obj_align_to(monitor_box->dropdown_camera_resolution, labe_title,
            LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    lv_obj_t *btn_camera = lv_btn_create(obj);
    lv_obj_add_event_cb(btn_camera, camera_button_event_handler, LV_EVENT_ALL,
            NULL);
    lv_obj_center(btn_camera);
    lv_obj_add_flag(btn_camera, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_t *labe_btn = lv_label_create(btn_camera);
    lv_label_set_text(labe_btn, "Preview");
    lv_obj_center(labe_btn);
}

static void lv_monitor_box_network_test(lv_obj_t *obj) {
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);

    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_bg_color(&style_title,
            lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_bg_opa(&style_title, LV_OPA_COVER);

    lv_obj_t *label_title = lv_label_create(obj);
    lv_label_set_recolor(label_title, true);
    lv_label_set_text_fmt(label_title, "#ffffff %s #", monitor_box->title);
    lv_obj_set_size(label_title, w, LV_SIZE_CONTENT);
    lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(label_title, &style_title, 0);

    lv_obj_t *label_tips = lv_label_create(obj);
    lv_obj_set_size(label_tips, w, LV_SIZE_CONTENT);
    lv_label_set_long_mode(label_tips, LV_LABEL_LONG_WRAP);
    lv_label_set_text_static(label_tips,
            "Perform 4 concurrent network data downloads");
    lv_obj_align_to(label_tips, label_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);

    monitor_box->checkbox_download = lv_checkbox_create(obj);
    lv_checkbox_set_text(monitor_box->checkbox_download, "Download test");
    lv_obj_align_to(monitor_box->checkbox_download, label_tips,
            LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);

    monitor_box->checkbox_upload = lv_checkbox_create(obj);
    lv_checkbox_set_text(monitor_box->checkbox_upload, "Upload test");
    lv_obj_align_to(monitor_box->checkbox_upload,
            monitor_box->checkbox_download, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

    monitor_box->checkbox_wifi_function = lv_checkbox_create(obj);
    lv_checkbox_set_text(monitor_box->checkbox_wifi_function,
            "Wifi function test");
    lv_obj_align_to(monitor_box->checkbox_wifi_function,
            monitor_box->checkbox_upload, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

    lv_obj_t *btn_network = lv_btn_create(obj);
    lv_obj_add_event_cb(btn_network, network_button_event_handler, LV_EVENT_ALL,
            NULL);
    lv_obj_align_to(btn_network, monitor_box->checkbox_wifi_function,
            LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
    lv_obj_add_flag(btn_network, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_t *label_btn = lv_label_create(btn_network);
    lv_label_set_text(label_btn, "Start");
    lv_obj_center(label_btn);
}

static void lv_monitor_box_constructor(const lv_obj_class_t *class_p,
        lv_obj_t *obj) {
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;
    monitor_box->type = LV_MONITOR_BOX_CPU_TEST;
    strcpy(monitor_box->title, "CPU TEST");

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    LV_TRACE_OBJ_CREATE("finished");
}

void lv_monitor_box_set_type(lv_obj_t *obj, uint32_t type) {
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;
    monitor_box->type = type;

    lv_obj_refr_size(obj);

    switch (monitor_box->type) {
    case LV_MONITOR_BOX_CPU_TEST:
        strcpy(monitor_box->title, "CPU TEST");
        lv_monitor_box_cpu_test(obj);
        break;
    case LV_MONITOR_BOX_HARDWARE_TEST:
        strcpy(monitor_box->title, "HARDWARE TEST");
        lv_monitor_box_hardware_test(obj);
        break;
    case LV_MONITOR_BOX_DDR_TEST:
        strcpy(monitor_box->title, "DDR TEST");
        lv_monitor_box_ddr_test(obj);
        break;
    case LV_MONITOR_BOX_VIDEO_TEST:
        strcpy(monitor_box->title, "VIDEO TEST");
        lv_monitor_box_video_test(obj);
        break;
    case LV_MONITOR_BOX_CAMERA_TEST:
        strcpy(monitor_box->title, "CAMERA TEST");
        lv_monitor_box_camera_test(obj);
        break;
    case LV_MONITOR_BOX_NETWORK_TEST:
        strcpy(monitor_box->title, "NETWORK TEST");
        lv_monitor_box_network_test(obj);
        break;
    default:
        strcpy(monitor_box->title, "CPU TEST");
        lv_monitor_box_cpu_test(obj);
        break;
    }
}

void lv_monitor_box_set_cpu_info(lv_obj_t *obj, const char *cpu_online,
        uint32_t cpu_usage, uint32_t cpu0_usage, uint32_t cpu1_usage,
        uint32_t cpu2_usage, uint32_t cpu3_usage) {
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;

    if (monitor_box->type != LV_MONITOR_BOX_CPU_TEST)
        return;

    lv_label_set_text_fmt(monitor_box->label_cpu_usage, "CPU-Usage:\n%d%%",
            cpu_usage);
    lv_label_set_text_fmt(monitor_box->label_cpu_online, "CPU-Online:\n%s",
            cpu_online);

    lv_meter_set_indicator_end_value(monitor_box->meter_cpu_usage,
            monitor_box->indic_cpu_usage[0], cpu0_usage);
    lv_meter_set_indicator_end_value(monitor_box->meter_cpu_usage,
            monitor_box->indic_cpu_usage[1], cpu1_usage);
    lv_meter_set_indicator_end_value(monitor_box->meter_cpu_usage,
            monitor_box->indic_cpu_usage[2], cpu2_usage);
    lv_meter_set_indicator_end_value(monitor_box->meter_cpu_usage,
            monitor_box->indic_cpu_usage[3], cpu3_usage);
}

void lv_monitor_box_set_hardware_info(lv_obj_t *obj, const char *cpu_freq,
        uint32_t gpu_usage, uint32_t cpu_temp, uint32_t ddr_temp,
        uint32_t gpu_temp) {
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;

    if (monitor_box->type != LV_MONITOR_BOX_HARDWARE_TEST)
        return;

    lv_label_set_text_fmt(monitor_box->label_cpu_freq, "CPU-Freq(GHz):\n%s",
            cpu_freq);
    lv_label_set_text_fmt(monitor_box->label_gpu_usage, "GPU-Usage:\n%d%%",
            gpu_usage);

    lv_bar_set_value(monitor_box->bar_cpu_temp, cpu_temp, LV_ANIM_ON);
    lv_bar_set_value(monitor_box->bar_ddr_temp, ddr_temp, LV_ANIM_ON);
    lv_bar_set_value(monitor_box->bar_gpu_temp, gpu_temp, LV_ANIM_ON);

    lv_label_set_text_fmt(monitor_box->label_cpu_temp, "%d", cpu_temp);
    lv_obj_align_to(monitor_box->label_cpu_temp, monitor_box->bar_cpu_temp,
            LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_label_set_text_fmt(monitor_box->label_ddr_temp, "%d", ddr_temp);
    lv_obj_align_to(monitor_box->label_ddr_temp, monitor_box->bar_ddr_temp,
            LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_label_set_text_fmt(monitor_box->label_gpu_temp, "%d", gpu_temp);
    lv_obj_align_to(monitor_box->label_gpu_temp, monitor_box->bar_gpu_temp,
            LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
}

void lv_monitor_box_set_ddr_info(lv_obj_t *obj, const uint32_t ddr_type,
        const uint32_t ddr_value) {
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;

    if (monitor_box->type != LV_MONITOR_BOX_DDR_TEST)
        return;

    if (ddr_value == 0)
        lv_obj_add_flag(monitor_box->obj_ddr_bw_info[ddr_type],
                LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_clear_flag(monitor_box->obj_ddr_bw_info[ddr_type],
                LV_OBJ_FLAG_HIDDEN);

    monitor_box->ddr_bw_info[ddr_type].bw_value = ddr_value;

    uint32_t i, j;
    ddr_bw buf, value[LV_MONITOR_BOX_DDR_END];
    memcpy(&value, &monitor_box->ddr_bw_info,
            sizeof(ddr_bw) * LV_MONITOR_BOX_DDR_END);

    /* Bubble Sort */
    for (i = 0; i < LV_MONITOR_BOX_DDR_END - 1; i++)
        for (j = 0; j < LV_MONITOR_BOX_DDR_END - 1 - i; j++) {
            if (value[j].bw_value < value[j + 1].bw_value) {
                buf = value[j];
                value[j] = value[j + 1];
                value[j + 1] = buf;
            }
        }

    for (i = 0; i < LV_MONITOR_BOX_DDR_END; i++) {
        if (value[i].bw_value != 0) {
            lv_obj_set_width(monitor_box->bar_ddr_bw_info[ddr_type],
                    monitor_box->ddr_bw_width
                            * ((float) ddr_value
                                    / (float) monitor_box->ddr_bw_info[0].bw_value)
                            - 6);

            /* Calculate the width of the label */
            lv_obj_t *label = lv_label_create(
                    monitor_box->obj_ddr_bw_info[ddr_type]);
            lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(label, "%d", ddr_value);
            lv_obj_refr_size(label);

            if (lv_obj_get_width(label)
                    < lv_obj_get_width(monitor_box->bar_ddr_bw_info[ddr_type])
                            - 6) {
                lv_label_set_text_fmt(monitor_box->label_ddr_bw_value[ddr_type],
                        "#ffffff %d #", ddr_value);
                lv_obj_align_to(monitor_box->label_ddr_bw_value[ddr_type],
                        monitor_box->bar_ddr_bw_info[ddr_type],
                        LV_ALIGN_RIGHT_MID, 0, 0);
            } else {
                lv_label_set_text_fmt(monitor_box->label_ddr_bw_value[ddr_type],
                        "#888888 %d #", ddr_value);
                lv_obj_align_to(monitor_box->label_ddr_bw_value[ddr_type],
                        monitor_box->bar_ddr_bw_info[ddr_type],
                        LV_ALIGN_OUT_RIGHT_MID, 2, 0);
            }

            lv_obj_del(label);

#if 1
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, monitor_box->obj_ddr_bw_info[value[i].bw_type]);
            lv_anim_set_values(&a,
                    lv_obj_get_y(
                            monitor_box->obj_ddr_bw_info[value[i].bw_type]),
                    monitor_box->ddr_bw_y[i]);
            lv_anim_set_time(&a, 250);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);

            lv_anim_set_exec_cb(&a, anim_y_cb);
            lv_anim_start(&a);
#else
            lv_obj_set_y(monitor_box->obj_ddr_bw_info[value[i].bw_type],
                    monitor_box->ddr_bw_y[i]);
#endif
        }
    }
}

void lv_monitor_box_set_video_play_cb(lv_obj_t *obj, lv_event_cb_t event_cb) {
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_monitor_box_t *monitor_box = (lv_monitor_box_t*) obj;

    if (monitor_box->type != LV_MONITOR_BOX_VIDEO_TEST)
        return;

    monitor_box->event_video_play_cb = event_cb;
}

#endif
