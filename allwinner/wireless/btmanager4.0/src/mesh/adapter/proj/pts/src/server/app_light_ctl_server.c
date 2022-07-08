#include "pts_app.h"
#include "generic_onoff_server.h"

static pts_app_t *g_app_db;

static mesh_model_info_t light_ctl_server;
static mesh_model_info_t light_ctl_setup_server;
static mesh_model_info_t light_ctl_temperature_server;
static struct l_timeout *g_light_ctl_publish_timer = NULL;
#define LIGHT_CTL_PUBLISH_TIME_MS       500

uint16_t app_light_ctl_temperature_to_generic_level(uint16_t temperature)
{
    return light_ctl_temperature_to_generic_level(temperature,g_app_db->db.light_ctl.temperature_range_min,g_app_db->db.light_ctl.temperature_range_max);
}

void light_ctl_publish_timeout_handle(struct l_timeout *timeout,void *context)
{
//    mesh_model_info_p pmodel_info = context;
    uint16_t lightness = g_app_db->db.light_ctl.lightness;
    uint16_t temperature = g_app_db->db.light_ctl.temperature;
    int16_t delta_uv = g_app_db->db.light_ctl.delta_uv;

    if(g_app_db->db.light_ctl.remaining_time.num_steps == 0)
    {
        g_light_ctl_publish_timer = NULL;
        l_timeout_remove(timeout);
        light_ctl_publish(&light_ctl_server,lightness,temperature);
        light_ctl_temperature_publish(&light_ctl_temperature_server,temperature,delta_uv);
    }
    else
    {
        l_timeout_modify_ms(g_light_ctl_publish_timer, LIGHT_CTL_PUBLISH_TIME_MS);
    }
    l_info("%s,%d,%d\n",__FUNCTION__,lightness,temperature);
}

static void app_light_ctl_value_set(mesh_model_info_p pmodel_info,uint16_t lightness,uint16_t temperature,uint16_t delta_uv)
{
    /* Resolve the server instance here if required, this example uses only 1 instance. */
    l_info("%s,1@lightness = %d,temperature = %d,delta_uv = %d",__FUNCTION__,lightness,temperature,delta_uv);
    PTS_SEED_START(g_app_db->seed.lctl);
    if(g_app_db->db.light_ctl.lightness != lightness)
    {
        g_app_db->db.light_ctl.lightness = lightness;
        light_lightness_update(g_app_db->seed.lctl,lightness);
    }
    l_info("%s,2@lightness = %d,temperature = %d,delta_uv = %d",__FUNCTION__,lightness,temperature,delta_uv);
    if(g_app_db->db.light_ctl.temperature != temperature)
    {
        g_app_db->db.light_ctl.temperature = temperature;
        if(g_app_db->db.light_ctl.remaining_time.num_steps == 0)
        {
            ctl_temperature_value_update_glv(g_app_db->seed.lctl,temperature);
        }
    }
    PTS_SEED_END(g_app_db->seed.lctl);

    g_app_db->db.light_ctl.delta_uv = delta_uv;
    l_info("%s,3@lightness = %d,temperature = %d,delta_uv=%d",__FUNCTION__,lightness,temperature,delta_uv);
    if(g_light_ctl_publish_timer == NULL)
    {
        g_light_ctl_publish_timer = start_timeout(light_ctl_publish_timeout_handle,pmodel_info,LIGHT_CTL_PUBLISH_TIME_MS);
    }
    else
    {
        l_timeout_modify_ms(g_light_ctl_publish_timer, LIGHT_CTL_PUBLISH_TIME_MS);
    }
}

void power_rst_lctl(uint8_t onpowerup_state)
{
     mesh_model_info_p pmodel_info = &light_ctl_server;
    uint16_t lightness = g_app_db->db.light_ctl.lightness;
    uint16_t temperature = g_app_db->db.light_ctl.temperature;
    uint16_t default_temperature = g_app_db->db.light_ctl.d_temperature;
    uint16_t last_temperature = g_app_db->db.light_ctl.l_temperature;

    uint16_t delta_uv = g_app_db->db.light_ctl.delta_uv;
    uint16_t d_delta_uv = g_app_db->db.light_ctl.d_delta_uv;
    uint16_t l_delta_uv = g_app_db->db.light_ctl.l_delta_uv;

    switch(onpowerup_state)
    {
        case 0:
        case 1:
            temperature = default_temperature;
            delta_uv = d_delta_uv;
            break;
        case 2:
            temperature = last_temperature;
            delta_uv = l_delta_uv;
            break;
        default:
            break;
    }
    g_app_db->db.light_ctl.delta_uv = delta_uv;
    app_light_ctl_value_set(pmodel_info,lightness,temperature,delta_uv);
    l_info("%s,%d(%d)%d\n",__FUNCTION__,g_app_db->db.light_ctl.temperature,onpowerup_state,delta_uv);
}

void gdtt_update_lctl(uint8_t seed,generic_transition_time_t trans_time)
{
    //mesh_model_info_p pmodel_info = &light_ctl_server;
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.lctl,seed);

    g_app_db->db.light_ctl.trans_time = trans_time;

    l_info("%s,%d,%d\n",__FUNCTION__,g_app_db->db.light_ctl.trans_time.num_steps,g_app_db->db.light_ctl.trans_time.step_resolution);
}

void glv_update_lctl(uint8_t seed,int16_t present_level)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.lctl,seed);
    uint16_t temperature = generic_level_to_light_ctl_temperature(present_level,g_app_db->db.light_ctl.temperature_range_min, \
        g_app_db->db.light_ctl.temperature_range_max);

    int16_t lightness = g_app_db->db.light_ctl.lightness;
    int16_t delta_uv = g_app_db->db.light_ctl.delta_uv;

    mesh_model_info_p pmodel_info = &light_ctl_server;

    l_info("%s,%d,%d\n",__FUNCTION__,present_level,lightness);
    g_app_db->db.light_ctl.temperature = temperature;
    app_light_ctl_value_set(pmodel_info,lightness,temperature,delta_uv);

}

void lln_update_lctl(uint8_t seed,uint16_t lightness)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.lctl,seed);
    mesh_model_info_p pmodel_info = &light_ctl_server;
    l_info("%s,%d\n",__FUNCTION__,lightness);
    app_light_ctl_value_set(pmodel_info,lightness,g_app_db->db.light_ctl.temperature,g_app_db->db.light_ctl.delta_uv);
}

static int32_t light_ctl_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                           void *pargs)
{
    app_light_ctl_t  *light_ctl =&g_app_db->db.light_ctl;
    int32_t ret = MODEL_SUCCESS;
    //bool init = false;

    switch (type)
    {
    case LIGHT_CTL_SERVER_GET:
        {
            light_ctl_server_get_t *pdata = pargs;
            pdata->lightness = light_ctl->lightness;
            pdata->temperature = light_ctl->temperature;
        }
        break;
    case LIGHT_CTL_SERVER_GET_DEFAULT:
        {
            light_ctl_server_get_default_t *pdata = pargs;
            pdata->lightness = light_ctl->d_lightness;
            pdata->temperature = light_ctl->d_temperature;
            pdata->delta_uv = light_ctl->d_delta_uv;
            //ret = MODEL_STOP_TRANSITION;
        }
        break;

    case LIGHT_CTL_SERVER_GET_TEMPERATURE:
        {
            light_ctl_server_get_temperature_t *pdata = pargs;
            pdata->temperature = light_ctl->temperature;
            pdata->delta_uv = light_ctl->delta_uv;
            //pdata->lightness = light_lightness_actual_to_linear(p_lightness->lightness);
        }
        break;

    case LIGHT_CTL_SERVER_GET_TEMPERATURE_RANGE:
        {
            light_ctl_server_get_temperature_range_t *pdata = pargs;
            pdata->range_min = light_ctl->temperature_range_min;
            pdata->range_max = light_ctl->temperature_range_max;
            //ret = MODEL_STOP_TRANSITION;
        }
        break;

    case LIGHT_CTL_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
           generic_transition_time_t *pdata = pargs;
           *pdata = light_ctl->trans_time;
        }
        break;

    case LIGHT_CTL_SERVER_SET:
        {
            light_ctl_server_set_t *pdata = pargs;
            light_ctl->total_time = pdata->total_time;
            light_ctl->remaining_time = pdata->remaining_time;
            if(pdata->remaining_time.num_steps == 0)
            {
                //store last value
                light_ctl->l_lightness = pdata->lightness;
                light_ctl->l_temperature = pdata->temperature;
                light_ctl->l_delta_uv= pdata->delta_uv;
                // set new value
                l_info("%s,LIGHT_CTL_SERVER_SET(%d,%d,%d)\n",__FUNCTION__,pdata->lightness,pdata->temperature,pdata->delta_uv);
                app_light_ctl_value_set(pmodel_info,pdata->lightness,pdata->temperature,pdata->delta_uv);
            }
        }
        break;

    case LIGHT_CTL_SERVER_SET_TEMPERATURE:
        {
            light_ctl_server_set_temperature_t *pdata = pargs;
            light_ctl->total_time = pdata->total_time;
            light_ctl->remaining_time = pdata->remaining_time;
            if(pdata->remaining_time.num_steps == 0)
            {
                //light_ctl->temperature = pdata->temperature;
                //store last value
                light_ctl->l_temperature= pdata->temperature;
                light_ctl->l_delta_uv = pdata->delta_uv;
                // set new value
                l_info("%s,LIGHT_CTL_SERVER_SET_TEMPERATURE(%d,%d,%d)\n",__FUNCTION__,light_ctl->lightness,pdata->temperature,pdata->delta_uv);
                app_light_ctl_value_set(pmodel_info,light_ctl->lightness,pdata->temperature,pdata->delta_uv);
            }
        }
        break;

    case LIGHT_CTL_SERVER_SET_DEFAULT:
        {
            light_ctl_server_set_default_t *pdata = pargs;
            light_ctl->d_lightness =  pdata->lightness;
            light_ctl->d_temperature =  pdata->temperature;
            light_ctl->d_delta_uv =  pdata->delta_uv;
        }
        break;

    case LIGHT_CTL_SERVER_SET_TEMPERATURE_RANGE:
        {
            light_ctl_server_set_temperature_range_t *pdata = pargs;
            light_ctl->temperature_range_min = pdata->range_min;
            light_ctl->temperature_range_max = pdata->range_max;
        }
        break;

    default:
        break;
    }

    return ret;
}

uint32_t pts_light_ctl_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&light_ctl_server,0,sizeof(light_ctl_server));
    light_ctl_server.model_data_cb = &light_ctl_server_data;
    ret = light_ctl_server_reg(element_index,&light_ctl_server);
    if(ret == false)
    {
        l_info("%s,server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    memset(&light_ctl_setup_server,0,sizeof(light_ctl_setup_server));
    light_ctl_setup_server.model_data_cb = &light_ctl_server_data;
    ret = light_ctl_setup_server_reg(element_index,&light_ctl_setup_server);
    if(ret == false)
    {
        l_info("%s,server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    memset(&light_ctl_temperature_server,0,sizeof(light_ctl_temperature_server));
    light_ctl_temperature_server.model_data_cb = &light_ctl_server_data;
    ret = light_ctl_temperature_server_reg(element_index,&light_ctl_temperature_server);
    if(ret == false)
    {
        l_info("%s,server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }

    return status;
}
