#ifndef PTS_APP_H__
#define PTS_APP_H__


#include "app_timer_mesh.h"
#include <sys/time.h>
//rtk model
#include "generic_default_transition_time.h"
#include "generic_battery.h"
#include "scene.h"
#include "sensor.h"
#include "time.h"
#include "time_model.h"
#include "scheduler.h"
#include "light_lightness.h"
#include "light_ctl.h"
#include "light_hsl.h"
//#include "util.h"

#define PTS_DEFAULT_DELAY_MS    25
#define PTS_APP_REG_ELEMENT_0   0
#define WORK_AROUND_FOR_GPL_VB_14_C 1

typedef struct
{
    uint8_t goo;
    uint8_t glv;
    uint8_t gdtt;
    uint8_t gpoo;
    uint8_t gplv;
    uint8_t gbat;
    uint8_t lln;
    uint8_t lctl;
    uint8_t lhsl;
}app_seed_t;

typedef struct
{
    uint16_t lightness;
    uint16_t l_lightness;
    uint16_t d_lightness;
    uint16_t hue;
    uint16_t l_hue;
    uint16_t d_hue;
    uint16_t saturation;
    uint16_t l_saturation;
    uint16_t d_saturation;
    uint16_t hue_range_min;
    uint16_t hue_range_max;
    uint16_t saturation_range_min;
    uint16_t saturation_range_max;
    generic_transition_time_t trans_time;
    generic_transition_time_t total_time;
    generic_transition_time_t remaining_time;
}app_light_hsl_t;

typedef struct
{
    uint16_t lightness;
    uint16_t temperature;
    int16_t delta_uv;

    uint16_t l_lightness;
    uint16_t l_temperature;
    int16_t l_delta_uv;

    uint16_t d_lightness;
    uint16_t d_temperature;
    int16_t d_delta_uv;

    uint16_t temperature_range_min;
    uint16_t temperature_range_max;
    generic_transition_time_t trans_time;
    generic_transition_time_t total_time;
    generic_transition_time_t remaining_time;
}app_light_ctl_t;

typedef struct
{
    uint16_t lightness;
    uint16_t linear;
    uint16_t l_lightness;
    uint16_t d_lightness;
    uint16_t range_min;
    uint16_t range_max;
    generic_transition_time_t trans_time;
    generic_transition_time_t total_time;
    generic_transition_time_t remaining_time;
}app_light_lightness_t;

typedef struct
{
    uint32_t scheduler;
}app_scheduler_t;

typedef struct
{
    uint32_t time;
}app_time_t;

typedef struct
{
    uint16_t property_id;
    /** app need to modify pointer blew, default is NULL */
    void *raw_data;
    void *column;
    uint16_t column_len;
    const void *raw_value_x;
    uint8_t raw_value_x_len;
}app_sensor_t;

/** app need to modify pointer blew, default is NULL */
/** contains raw value x, column width and raw value y */


typedef struct
{
    uint16_t current_scene;
    scene_status_code_t status;
    uint16_t scene_number;
    void *pmemory;
    generic_transition_time_t total_time;
    generic_transition_time_t remaining_time;
    generic_transition_time_t trans_time;
}app_scene_t;

typedef struct
{
    bool present_onoff;
    bool target_onoff;
    generic_transition_time_t total_time;
    generic_transition_time_t remaining_time;
    generic_transition_time_t trans_time;
}app_goo_t;

typedef struct
{
    uint8_t battery_level;
    uint32_t time_to_discharge;
    uint32_t time_to_charge;
    generic_battery_flags_t flags;
}app_battery_t;

typedef struct
{
    uint16_t power;
    uint16_t onpowerup_bc;
    uint16_t default_power;
    uint16_t last_power;
    uint16_t range_min;
    uint16_t range_max;
    generic_transition_time_t trans_time;
    generic_transition_time_t total_time;
    generic_transition_time_t remaining_time;
}generic_power_level_t;

typedef struct
{
    bool present_onoff;
    uint32_t present_level;
    uint8_t poonoff_status;
    generic_transition_time_t glv_trans_time;
    generic_transition_time_t trans_time;
    generic_power_level_t   power_level;
    app_goo_t goo;
    app_battery_t   battery;
    app_scene_t scene;
    app_sensor_t sensor;
    app_time_t time;
    app_scheduler_t scheduler;
    app_light_lightness_t light_lightness;
    app_light_ctl_t light_ctl;
    app_light_hsl_t light_hsl;
}pts_database_t;

typedef struct
{
    uint16_t element_index;
    app_seed_t seed;
    pts_database_t db;
}pts_app_t;

#define TRACE_FULL(G, T, TAG, FMT, ...)
#define __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, FMT, ...) l_info(FMT, ##__VA_ARGS__)
typedef void(*timeout_handler_t)(struct l_timeout *timeout,void * p_context);
struct l_timeout* start_timeout(timeout_handler_t timeout,void *p_context,uint32_t ms_delay);


//pts app reg func
uint32_t pts_goo_reg(pts_app_t *p_db, uint8_t element_index);
uint32_t pts_ponoff_reg(pts_app_t *p_db, uint8_t element_index);
uint32_t pts_dtt_reg(pts_app_t *p_db, uint8_t element_index);
uint32_t pts_power_level_reg(pts_app_t *p_db, uint8_t element_index);
uint32_t pts_battery_reg(pts_app_t *p_db, uint8_t element_index);
uint32_t pts_light_lightness_reg(pts_app_t *p_db, uint8_t element_index);
uint32_t pts_light_ctl_reg(pts_app_t *p_db, uint8_t element_index);
uint32_t pts_light_hsl_reg(pts_app_t *p_db, uint8_t element_index);

//pts app reset func
void power_rst_goo(uint8_t onpower_state);
void power_rst_lctl(uint8_t onpower_state);
void power_rst_lln(uint8_t onpower_state);
void power_rst_lctl(uint8_t onpower_state);
void power_rst_lhsl(uint8_t onpower_state);
//pts app api
uint16_t app_light_ctl_temperature_to_generic_level(uint16_t temperature);

uint32_t pts_transition_time_to_ms(generic_transition_time_t trans_time);

// app update function for binding feature
void pts_app_seed_reset();
//bool pts_seed_access(pts_app_t *p_app, uint8_t *p_seed, uint8_t lower_seed);
#define PTS_SEED_CHECK(app,seed,lower_seed) {   \
                                                lower_seed++;   \
                                                l_info("%s,seed = %d,lower_seed = %d\n",__FUNCTION__,seed,lower_seed); \
                                                if(app == NULL) \
                                                    return ;    \
                                                if(seed <= lower_seed) \
                                                    return ;    \
                                                seed = lower_seed;  }

#define PTS_SEED_START(seed)    {   \
                                    l_info("%s,start seed = %d\n",__FUNCTION__,seed); \
                                    if(seed == 0xFF)   \
                                        seed = 0;  }


#define PTS_SEED_END(seed)     { \
                                    l_info("%s,end seed = %d\n",__FUNCTION__,seed); \
                                    if(seed == 0)   \
                                        pts_app_seed_reset();  }

#define PTS_SEED_INIT(seed)     seed = 0xFF

//goo update
void gdtt_update_goo(uint8_t seed,generic_transition_time_t trans_time);
void lln_update_goo(uint8_t seed,uint16_t lightness);
void gpl_update_goo(uint8_t seed,uint16_t power);

//level update
void gdtt_update_glv(uint8_t seed,generic_transition_time_t trans_time);
void hsl_hue_value_update_glv(uint8_t seed,uint16_t hue);
void hsl_saturation_value_update_glv(uint8_t seed,uint16_t saturation);
void ctl_temperature_value_update_glv(uint8_t seed,uint16_t temperature);
void lln_update_glv(uint8_t seed,uint16_t lightness);
void gpl_update_glv(uint8_t seed,uint16_t power);

// power level update
void gdtt_update_plv(uint8_t seed,generic_transition_time_t trans_time);
void glv_update_plv(uint8_t seed,int16_t present_level);
void goo_update_plv(uint8_t seed,bool onoff);
void gpoo_update_plv(uint8_t seed,uint8_t status);

// onpowerup update
void gplv_update_gpoo(uint8_t seed,uint16_t power, uint16_t power_default, uint16_t power_last);
void lln_update_gpoo(uint8_t seed,uint16_t lightness);

//  light ctl update
void gdtt_update_lctl(uint8_t seed,generic_transition_time_t trans_time);
void glv_update_lctl(uint8_t seed,int16_t present_level);
void lln_update_lctl(uint8_t seed,uint16_t lightness);

// light hsl update
void lln_update_hsl(uint8_t seed,uint16_t lightness);
void glv_update_lhsl(uint8_t seed,int16_t present_level);
void gdtt_update_lhsl(uint8_t seed,generic_transition_time_t trans_time);

//light lightness update
void goo_update_lln(uint8_t seed,bool onoff);
void gdtt_update_lln(uint8_t seed,generic_transition_time_t trans_time);
void glv_update_lln(uint8_t seed,int16_t present_level);
void light_lightness_update(uint8_t seed,uint16_t lightness);

//entry of the pts quality test
void pts_app_run();
void pts_app_reset();
void pts_app_onpowerup_reset();
void pts_app_onpowerup_hsl_reset();

extern void virtual_rx_message();

/** @} end of PTS_APP */
#endif /* PTS_APP_H__ */
