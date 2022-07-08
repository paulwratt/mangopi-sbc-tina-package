#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ell/ell.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

//model app
#include "app_generic_on_off_server.h"
#include "app_generic_level_server.h"

pts_app_t pts_app = {
    .element_index = PTS_APP_REG_ELEMENT_0,
    .db = {
        .power_level = {
            .range_min = 0x0000,
            .range_max = 0xFFFF
        },
        .light_ctl = {
            .temperature_range_min = 801,
            .temperature_range_max = 19999
        }
    }
};

static struct
{
    char *name;
    uint32_t (*func)(pts_app_t *pts_db, uint8_t element_index);
}pts_reg_tab[] =
{
    {"MMDL/SR/GOO",pts_goo_reg},// pts_onoff_reg
    {"MMDL/SR/GLV",pts_level_reg},
    {"MMDL/SR/GPOO",pts_ponoff_reg},
    {"MMDL/SR/GDTT",pts_dtt_reg},
    {"MMDL/SR/GPL",pts_power_level_reg},
    {"MMDL/SR/GBAT",pts_battery_reg},
    {"MMDL/SR/LLN",pts_light_lightness_reg},
    {"MMDL/SR/LCTL",pts_light_ctl_reg},
    {"MMDL/SR/LHSL",pts_light_hsl_reg},
    {"",NULL}
};

struct l_timeout* start_timeout(timeout_handler_t timeout,void *p_context,uint32_t ms_delay)
{
    return l_timeout_create_ms(ms_delay,timeout,p_context, NULL);
}

void pts_app_reg_foundation_models()
{
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_HEALTH_SERVER,NULL);
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_HEALTH_CLIENT,NULL);
    aw_mesh_add_model(pts_app.element_index,0x0000,NULL);
    aw_mesh_add_model(pts_app.element_index,0x0001,NULL);
}

void pts_app_reg_client_models()
{
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_GENERIC_ONOFF_CLIENT,NULL);
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_GENERIC_LEVEL_CLIENT,NULL);
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_CLIENT,NULL);
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_CLIENT,NULL);
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_GENERIC_POWER_LEVEL_CLIENT,NULL);
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_GENERIC_BATTERY_CLIENT,NULL);
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_CLIENT,NULL);
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_LIGHT_CTL_CLIENT,NULL);
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_LIGHT_HSL_CLIENT,NULL);
    aw_mesh_add_model(pts_app.element_index,AW_MESH_SIG_MODEL_ID_LIGHT_XYL_CLIENT,NULL);
}
#if 0
bool pts_seed_access(pts_app_t *p_app, uint8_t *p_seed, uint8_t upper_seed)
{

    if(p_app == NULL)
        return false;

    if(*p_seed <= upper_seed)
        return false;

    return true;
}
#endif
uint32_t pts_transition_time_to_ms(generic_transition_time_t trans_time)
{
    uint32_t ret = 0;
    switch(trans_time.step_resolution)
    {
        case 0:
            ret = 100*trans_time.num_steps;
            break;
        case 1:
            ret = 1000*trans_time.num_steps;
            break;
        case 2:
            ret = 10000*trans_time.num_steps;
            break;
        case 3:
            ret = 600000*trans_time.num_steps;
            break;
        default:
            ret = 0;
            break;
    }
    return ret;

}
void pts_app_seed_reset()
{
    memset(&pts_app.seed, 0xFF, sizeof(app_seed_t));
}

void pts_app_init()
{
    access_init();
    aw_mesh_add_element(0,&pts_app.element_index);
    pts_app_seed_reset();
}

void pts_app_run()
{
    uint8_t i;
    uint32_t status = 0;
    //void *p_func = NULL;
    pts_app_init();
	for (i = 0; pts_reg_tab[i].func; i++) {
        status = pts_reg_tab[i].func(&pts_app, pts_app.element_index);
        if(status != 0)
        {
            l_info("%s\t%s ret fail errcode %d",__FUNCTION__,pts_reg_tab[i].name,status);
        }
	}
    pts_app_reg_foundation_models();
    pts_app_reg_client_models();
    //virtual_rx_message();
}

void pts_app_reset()
{
    pts_app_run();
}

void pts_app_onpowerup_reset()
{
    uint8_t onpowerup_state = pts_app.db.poonoff_status;
    l_info("%s,onpowerup_state = %d\n",__FUNCTION__,pts_app.db.poonoff_status);
    power_rst_goo(onpowerup_state);
    power_rst_lctl(onpowerup_state);
    power_rst_lln(onpowerup_state);
    power_rst_lctl(onpowerup_state);
}

void pts_app_onpowerup_hsl_reset()
{
    uint8_t onpowerup_state = pts_app.db.poonoff_status;
    power_rst_lhsl(onpowerup_state);
}

uint16_t pts_get_element_idx()
{
    return pts_app.element_index;
}
