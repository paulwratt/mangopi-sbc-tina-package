#include "pts_app.h"
#include "generic_onoff_server.h"

static pts_app_t *g_app_db;

static mesh_model_info_t light_lightness_server;
static mesh_model_info_t light_lightness_setup_server;
//static uint8_t light_lightness_pooff_state;
typedef void (*app_lightness_update_cb)(void *bound_context,uint16_t lightness);

static void app_light_linear_value_set(mesh_model_info_p pmodel_info,uint16_t linear)
{
    g_app_db->db.light_lightness.linear = linear;
}

static void app_light_lightness_value_set(mesh_model_info_p pmodel_info,uint16_t lightness)
{
    /* Resolve the server instance here if required, this example uses only 1 instance. */
    l_info("%s,lightness = %d\n",__FUNCTION__,lightness);
    if(g_app_db->db.light_lightness.lightness != lightness)
    {
        g_app_db->db.light_lightness.lightness = lightness;
        PTS_SEED_START(g_app_db->seed.lln);
        lln_update_goo(g_app_db->seed.lln,lightness);
        lln_update_glv(g_app_db->seed.lln,lightness);
        lln_update_hsl(g_app_db->seed.lln,lightness);
        lln_update_lctl(g_app_db->seed.lln,lightness);
        PTS_SEED_END(g_app_db->seed.lln);
    }
    light_lightness_publish(pmodel_info,lightness);
}

void light_lightness_update(uint8_t seed,uint16_t lightness)
{
    mesh_model_info_p pmodel_info = &light_lightness_server;
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.lln,seed);
    app_light_lightness_value_set(pmodel_info,lightness);
    app_light_linear_value_set(pmodel_info,light_lightness_actual_to_linear(lightness));
}

void goo_update_lln(uint8_t seed,bool onoff)
{
    //PTS_SEED_CHECK(g_app_db,g_app_db->seed.lln,seed);
    app_light_lightness_t  *p_lightness =&g_app_db->db.light_lightness;
    uint16_t lightness;

    if (onoff == true)
    {
        if(p_lightness->d_lightness == 0)
        {
            lightness = p_lightness->l_lightness;
        }
        else
        {
            lightness = p_lightness->d_lightness;
        }
    }
    else
    {
        lightness = 0;
    }
    l_info("%s,%d,%d,%d,%d\n",__FUNCTION__,onoff,lightness,p_lightness->d_lightness,p_lightness->l_lightness);
    light_lightness_update(seed,lightness);
}

void gdtt_update_lln(uint8_t seed,generic_transition_time_t trans_time)
{
//    mesh_model_info_p pmodel_info = &light_lightness_server;
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.lln,seed);
    g_app_db->db.light_lightness.trans_time = trans_time;
    l_info("%s,%d,%d\n",__FUNCTION__,g_app_db->db.light_lightness.trans_time.num_steps,g_app_db->db.light_lightness.trans_time.step_resolution);
}

void glv_update_lln(uint8_t seed,int16_t present_level)
{
    //PTS_SEED_CHECK(g_app_db,g_app_db->seed.lln,seed);
    uint16_t lightness = 0;
    //mesh_model_info_p pmodel_info = &light_lightness_server;
    l_info("%s,start:%d,%d\n",__FUNCTION__,present_level,lightness);
    lightness = present_level + 32768;
    light_lightness_update(seed,lightness);
    //app_light_lightness_value_set(pmodel_info,lightness);
    //app_light_linear_value_set(pmodel_info,light_lightness_actual_to_linear(lightness));
    l_info("%s,end:%d,%d\n",__FUNCTION__,present_level,lightness);
}

void power_rst_lln(uint8_t onpowerup_state)
{
     mesh_model_info_p pmodel_info = &light_lightness_server;
    uint16_t lightness = g_app_db->db.light_lightness.lightness;
    uint16_t default_lightness = g_app_db->db.light_lightness.d_lightness;
    uint16_t last_lightness = g_app_db->db.light_lightness.l_lightness;
    switch(onpowerup_state)
    {
        case 0:
            lightness = 0;
            break;
        case 1:
            if(default_lightness == 0)
            {
                lightness = last_lightness;
            }
            else
            {
                lightness = default_lightness;
            }
            break;
        case 2:
            lightness = last_lightness;
            break;
        default:
            break;
    }
    app_light_lightness_value_set(pmodel_info,lightness);
    app_light_linear_value_set(pmodel_info,light_lightness_actual_to_linear(lightness));
    l_info("%s,%d(%d)\n",__FUNCTION__,g_app_db->db.light_lightness.lightness,onpowerup_state);
}

static int32_t light_lightness_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                           void *pargs)
{
    app_light_lightness_t  *p_lightness =&g_app_db->db.light_lightness;
    int32_t ret = MODEL_SUCCESS;
    switch (type)
    {
    case LIGHT_LIGHTNESS_SERVER_GET:
        {
            light_lightness_server_get_t *pdata = pargs;
            pdata->lightness = p_lightness->lightness;
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_SET:
        {
            light_lightness_server_set_t *pdata = pargs;
            if(pdata->remaining_time.num_steps == 0)
            {
                //PTS_SEED_INIT(g_app_db->seed.lln);
                //light_lightness_update(g_app_db->seed.lln,pdata->lightness);
                //PTS_SEED_END(g_app_db->seed.lln);
                app_light_lightness_value_set(pmodel_info,pdata->lightness);
                p_lightness->linear = light_lightness_actual_to_linear(pdata->lightness);
            }
            p_lightness->remaining_time = pdata->remaining_time;
            p_lightness->total_time = pdata->total_time;
            //ret = MODEL_STOP_TRANSITION;
        }
        break;

    case LIGHT_LIGHTNESS_SERVER_GET_LINEAR:
        {
            light_lightness_server_get_t *pdata = pargs;
            pdata->lightness = p_lightness->linear;
            //pdata->lightness = light_lightness_actual_to_linear(p_lightness->lightness);
        }
        break;

    case LIGHT_LIGHTNESS_SERVER_SET_LINEAR:
        {
            light_lightness_server_set_t *pdata = pargs;
            if(pdata->remaining_time.num_steps == 0)
            {
                //PTS_SEED_INIT(g_app_db->seed.lln);
               //light_lightness_update(g_app_db->seed.lln,light_lightness_linear_to_actual(pdata->lightness));
                //PTS_SEED_END(g_app_db->seed.lln);
                app_light_lightness_value_set(pmodel_info,light_lightness_linear_to_actual(pdata->lightness));
                p_lightness->linear = pdata->lightness;
            }
            //ret = MODEL_STOP_TRANSITION;
        }
        break;

    case LIGHT_LIGHTNESS_SERVER_GET_LAST:
        {
           light_lightness_server_get_last_t *pdata = pargs;
           pdata->lightness = p_lightness->l_lightness;
        }
        break;

    case LIGHT_LIGHTNESS_SERVER_SET_LAST:
        {
            light_lightness_server_set_last_t *pdata = pargs;
            p_lightness->l_lightness = pdata->lightness;
        }
        break;

    case LIGHT_LIGHTNESS_SERVER_GET_DEFAULT:
        {
            light_lightness_server_get_default_t *pdata = pargs;
            pdata->lightness = p_lightness->d_lightness;
        }
        break;

    case LIGHT_LIGHTNESS_SERVER_SET_DEFAULT:
        {
            light_lightness_server_set_default_t *pdata = pargs;
            p_lightness->d_lightness = pdata->lightness;
        }
        break;

    case LIGHT_LIGHTNESS_SERVER_GET_RANGE:
        {
            light_lightness_server_get_range_t *pdata = pargs;
            pdata->range_min = p_lightness->range_min;
            pdata->range_max = p_lightness->range_max;
        }
        break;

    case LIGHT_LIGHTNESS_SERVER_SET_RANGE:
        {
            light_lightness_server_set_range_t *pdata = pargs;
            p_lightness->range_min = pdata->range_min;
            p_lightness->range_max = pdata->range_max;
        }
        break;

    case LIGHT_LIGHTNESS_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
            generic_transition_time_t *pdata = pargs;
            pdata->num_steps = p_lightness->trans_time.num_steps;
            pdata->step_resolution = p_lightness->trans_time.step_resolution;
            break;
        }
        break;

    default:
        break;
    }

    return ret;
}

uint32_t pts_light_lightness_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&light_lightness_server,0,sizeof(light_lightness_server));
    light_lightness_server.model_data_cb = &light_lightness_server_data;
    ret = light_lightness_server_reg(element_index,&light_lightness_server);
    if(ret == false)
    {
        l_info("%s,server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    memset(&light_lightness_setup_server,0,sizeof(light_lightness_setup_server));
    light_lightness_setup_server.model_data_cb = &light_lightness_server_data;
    ret = light_lightness_setup_server_reg(element_index,&light_lightness_setup_server);
    if(ret == false)
    {
        l_info("%s,setup server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    return status;
}
