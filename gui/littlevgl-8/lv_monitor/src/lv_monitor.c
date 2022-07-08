/**
 * @file lv_monitor.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"
#include "lv_monitor.h"
#include "lv_monitor_box.h"
#include <sys/time.h>
#include "middle_ware/player_int.h"
#include "middle_ware/media_file_play.h"
#include "middle_ware/cpu_test.h"

/*********************
 *      DEFINES
 *********************/
#define MONITOR_BOX_NUM 6

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t *monitor_box[MONITOR_BOX_NUM];
static lv_obj_t *label_test_time;
static lv_timer_t *timer;
static lv_style_t style_flex;
static lv_obj_t *cont;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static void monitor_timer(lv_timer_t * timer)
{
    static struct timeval time_start, time_end;
    if (time_start.tv_usec == 0)
        gettimeofday(&time_start, NULL);
    gettimeofday(&time_end, NULL);

    time_t test_msec, test_sec, test_hour, test_data;
    test_msec = (time_end.tv_sec + time_end.tv_usec / 1000 / 1000)
            - (time_start.tv_sec + time_start.tv_usec / 1000 / 1000);
    test_sec = test_msec / 60;
    test_hour = test_sec / 60;
    test_data = test_hour / 24;

    test_msec = test_msec - test_sec * 60;
    test_sec = test_sec - test_hour * 60;
    test_hour = test_hour - test_data * 24;

    lv_label_set_text_fmt(label_test_time, "TEST TIME: %02ld:%02ld:%02ld:%02ld",
            test_data, test_hour, test_sec, test_msec);

    u32 cpu_online_num = 0;
    u32 cpu_usages_all = 0;
    u32 cpu_usages[CPU_NRS_MAX];
    char cpu_online[MAX_LINE_LEN] = {'\0'};
    char cpu_freq[MAX_LINE_LEN] = { '\0' };
    lv_monitor_box_t *monitor_box_cpu_test =
            (lv_monitor_box_t*) monitor_box[LV_MONITOR_BOX_CPU_TEST];

    monitor_get_cpu_info(cpu_online, cpu_freq, cpu_usages, &cpu_online_num,
            &cpu_usages_all);
    monitor_add_cpu_load(monitor_box_cpu_test->set_cpu_usage, cpu_online_num,
            cpu_usages_all);

    lv_monitor_box_set_cpu_info(monitor_box[LV_MONITOR_BOX_CPU_TEST],
            cpu_online, cpu_usages_all, cpu_usages[0], cpu_usages[1],
            cpu_usages[2], cpu_usages[3]);

    lv_monitor_box_set_hardware_info(monitor_box[LV_MONITOR_BOX_HARDWARE_TEST],
            cpu_freq, 55, sysfs_thermal_get_one_value(CPU_THERMAL),
            sysfs_thermal_get_one_value(GPU_THERMAL),
            sysfs_thermal_get_one_value(DDR_THERMAL));

    uint32_t i;
    for (i = 0; i < DDR_INFO_NUM - 1; i++) {
        if (ddr_info[i].use == 1) {
            lv_monitor_box_set_ddr_info(monitor_box[LV_MONITOR_BOX_DDR_TEST],
                    ddr_info[i].type, ddr_info[i].value);
        }
    }
}

static void video_button_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_RELEASED) {
        if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
            lv_style_set_flex_flow(&style_flex, LV_FLEX_FLOW_ROW);
            lv_style_set_flex_main_place(&style_flex, LV_FLEX_ALIGN_START);
            lv_obj_report_style_change(&style_flex);

            char *path;
            path = malloc(FILE_PATH_MAXT_LEN * sizeof(char));
            if (media_get_current_file_path(path) == 0)
                tplayer_play_url(path);
            tplayer_setlooping(1);
            free(path);
            tplayer_play();
        } else {
            tplayer_stop();

            lv_style_set_flex_flow(&style_flex, LV_FLEX_FLOW_ROW_WRAP);
            lv_style_set_flex_main_place(&style_flex,
                    LV_FLEX_ALIGN_SPACE_EVENLY);
            lv_obj_report_style_change(&style_flex);
            lv_obj_scroll_to(cont, 0, 0, LV_ANIM_ON);
        }
    }
}

void lv_monitor(void) {
    lv_style_init(&style_flex);
    lv_style_set_flex_flow(&style_flex, LV_FLEX_FLOW_ROW_WRAP);
    lv_style_set_flex_main_place(&style_flex, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_style_set_layout(&style_flex, LV_LAYOUT_FLEX);
    lv_style_set_radius(&style_flex, 0);
    lv_style_set_bg_opa(&style_flex, LV_OPA_TRANSP);

    static lv_style_t style_box;
    lv_style_init(&style_box);
    lv_style_set_radius(&style_box, 0);
    lv_style_set_pad_top(&style_box, 0);
    lv_style_set_pad_left(&style_box, 0);
    lv_style_set_bg_opa(&style_box, LV_OPA_COVER);
    if (LV_HOR_RES >= 1280 || LV_VER_RES >= 1280)
        lv_style_set_text_font(&style_box, &lv_font_montserrat_22);
    else if (LV_HOR_RES >= 800 || LV_VER_RES >= 800)
        lv_style_set_text_font(&style_box, &lv_font_montserrat_14);
    else
        lv_style_set_text_font(&style_box, &lv_font_montserrat_12);

    /*Add a shadow*/
    lv_style_set_shadow_width(&style_box, 20);
    lv_style_set_shadow_color(&style_box, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_shadow_ofs_x(&style_box, 2);
    lv_style_set_shadow_ofs_y(&style_box, 4);

    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_center(cont);
    lv_obj_add_style(cont, &style_flex, 0);

    uint32_t i, w, h;
    if (LV_HOR_RES > LV_VER_RES) {
        w = LV_HOR_RES * 0.3;
        h = LV_VER_RES * 0.4;
    } else {
        w = LV_HOR_RES * 0.4;
        h = LV_VER_RES * 0.3;
    }

    if (LV_HOR_RES <= 320 || LV_VER_RES <= 320) {
        if (LV_HOR_RES > LV_VER_RES) {
            w = LV_VER_RES * 0.8;
            h = LV_VER_RES * 0.8;
        } else {
            w = LV_HOR_RES * 0.8;
            h = LV_HOR_RES * 0.8;
        }
    }

    for (i = 0; i < MONITOR_BOX_NUM; i++) {
        lv_obj_t *obj = lv_obj_create(cont);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_style(obj, &style_box, 0);
        lv_obj_set_size(obj, w, h);

        monitor_box[i] = lv_monitor_box_create(obj);
        lv_obj_set_size(monitor_box[i], w, h);

        lv_monitor_box_set_type(monitor_box[i], i);

        if (i == LV_MONITOR_BOX_VIDEO_TEST)
            lv_monitor_box_set_video_play_cb(monitor_box[i],
                    video_button_event_handler);
    }

    label_test_time = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(label_test_time, "%s", "TEST TIME: 00:00:00:00");
    lv_obj_add_style(label_test_time, &style_box, 0);
    lv_obj_align(label_test_time, LV_ALIGN_BOTTOM_MID, 0, 0);

    monitor_add_ddr_load();

    timer = lv_timer_create(monitor_timer, 500, NULL);
}

void lv_monitor_destroy(void) {
    uint32_t i;
    for (i = 0; i < MONITOR_BOX_NUM; i++) {
        lv_obj_clean(monitor_box[i]);
        lv_obj_del(monitor_box[i]);
    }
    lv_timer_del(timer);
    monitor_add_ddr_cancel();
}
/**********************
 *   STATIC FUNCTIONS
 **********************/
