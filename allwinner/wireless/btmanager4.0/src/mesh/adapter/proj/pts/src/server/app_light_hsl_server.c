#include "pts_app.h"
#include "generic_onoff_server.h"

static pts_app_t *g_app_db;

static mesh_model_info_t light_hsl_server;
static mesh_model_info_t light_hsl_setup_server;
static mesh_model_info_t light_hsl_saturation_server;
static mesh_model_info_t light_hsl_hue_server;
//static uint8_t light_hsl_pooff_state;
static void app_light_hsl_value_set(const mesh_model_info_p pmodel_info, uint16_t lightness,
                                        uint16_t hue, uint16_t saturation)
{
    bool lightness_update = false;
    bool saturation_update = false;
    bool hue_update = false;
    app_light_hsl_t  *light_hsl =&g_app_db->db.light_hsl;
    l_info("%s,lightness = %d,hue = %d,saturation = %d",__FUNCTION__,lightness,hue,saturation);

    if(light_hsl->lightness != lightness)
    {
        light_hsl->lightness = lightness;
        lightness_update = true;
    }

    if(light_hsl->hue != hue)
    {
        light_hsl->hue = hue;
        hue_update = true;
    }

    if(light_hsl->saturation != saturation)
    {
        light_hsl->saturation = saturation;
        saturation_update = true;
    }
    PTS_SEED_START(g_app_db->seed.lhsl);

    if(lightness_update == true)
        light_lightness_update(g_app_db->seed.lhsl,lightness);

    if(hue_update == true)
       hsl_hue_value_update_glv(g_app_db->seed.lhsl,hue);

    if(saturation_update == true)
        hsl_saturation_value_update_glv(g_app_db->seed.lhsl,saturation);
    PTS_SEED_END(g_app_db->seed.lhsl);

    light_hsl_publish(&light_hsl_server,lightness,hue,saturation);
    light_hsl_saturation_publish(&light_hsl_saturation_server,saturation);
    light_hsl_hue_publish(&light_hsl_hue_server,hue);

}

void power_rst_lhsl(uint8_t onpowerup_state)
{
     mesh_model_info_p pmodel_info = &light_hsl_server;
    uint16_t lightness = g_app_db->db.light_hsl.lightness;
    uint16_t default_lightness = g_app_db->db.light_hsl.d_lightness;
    uint16_t last_lightness = g_app_db->db.light_hsl.l_lightness;

    uint16_t hue = g_app_db->db.light_hsl.hue;
    uint16_t default_hue = g_app_db->db.light_hsl.d_hue;
    uint16_t last_hue = g_app_db->db.light_hsl.l_hue;

    uint16_t saturation = g_app_db->db.light_hsl.saturation;
    uint16_t default_saturation = g_app_db->db.light_hsl.d_saturation;
    uint16_t last_saturation = g_app_db->db.light_hsl.l_saturation;
    switch(onpowerup_state)
    {
        case 0:
            lightness = 0;
            saturation = default_saturation;
            hue = default_hue;
            break;
        case 1:
            saturation = default_saturation;
            hue = default_hue;
            lightness = default_lightness;
            break;
        case 2:
            saturation = last_saturation;
            hue = last_hue;
            lightness = last_lightness;
            break;
        default:
            break;
    }
    app_light_hsl_value_set(pmodel_info,lightness,hue,saturation);
}

void lln_update_hsl(uint8_t seed,uint16_t lightness)
{
    mesh_model_info_p pmodel_info = &light_hsl_server;
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.lhsl,seed);
    app_light_hsl_value_set(pmodel_info,lightness,g_app_db->db.light_hsl.hue,g_app_db->db.light_hsl.saturation);
    l_info("%s,lightness = %d",__FUNCTION__,g_app_db->db.light_hsl.lightness);
}

void glv_update_lhsl(uint8_t seed,int16_t present_level)
{
    uint16_t hue = present_level + 32768;
    uint16_t saturation = present_level + 32768;
    mesh_model_info_p pmodel_info = &light_hsl_server;
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.lhsl,seed);

    app_light_hsl_value_set(pmodel_info,g_app_db->db.light_hsl.lightness,hue,saturation);

    l_info("%s,%d,%d\n",__FUNCTION__,present_level,hue);

    l_info("%s,lightness = %d",__FUNCTION__,g_app_db->db.light_hsl.lightness);
}

void gdtt_update_lhsl(uint8_t seed,generic_transition_time_t trans_time)
{
    //mesh_model_info_p pmodel_info = &light_hsl_server;
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.lhsl,seed);
    g_app_db->db.light_hsl.trans_time = trans_time;
    l_info("%s,%d,%d\n",__FUNCTION__,g_app_db->db.light_hsl.trans_time.num_steps,g_app_db->db.light_hsl.trans_time.step_resolution);
}

static int32_t light_hsl_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                           void *pargs)
{
    app_light_hsl_t  *light_hsl =&g_app_db->db.light_hsl;
    int32_t ret = MODEL_SUCCESS;
    l_info("%s,type = %d,%d,%d,%d\n",__FUNCTION__,type,light_hsl->lightness,light_hsl->hue,light_hsl->saturation);

    switch (type)
    {
    case LIGHT_HSL_SERVER_GET:
        {
            light_hsl_server_get_t *pdata = pargs;
            pdata->lightness = light_hsl->lightness;
            pdata->hue = light_hsl->hue;
            pdata->saturation = light_hsl->saturation;
        }
        break;
    case LIGHT_HSL_SERVER_GET_HUE:
        {
            light_hsl_server_get_hue_t *pdata = pargs;
            pdata->hue = light_hsl->hue;
            //ret = MODEL_STOP_TRANSITION;
        }
        break;

    case LIGHT_HSL_SERVER_GET_DEFAULT:
        {
             light_hsl_server_get_default_t *pdata = pargs;
             pdata->lightness = light_hsl->d_lightness ;
             pdata->hue = light_hsl->d_hue ;
             pdata->saturation = light_hsl->d_saturation ;
        }
        break;

    case LIGHT_HSL_SERVER_GET_RANGE:
        {
            light_hsl_server_get_range_t *pdata = pargs;
            pdata->hue_range_min = light_hsl->hue_range_min;
            pdata->hue_range_max = light_hsl->hue_range_max;
            pdata->saturation_range_min = light_hsl->saturation_range_min;
            pdata->saturation_range_max = light_hsl->saturation_range_max;
            //ret = MODEL_STOP_TRANSITION;
        }
        break;

    case LIGHT_HSL_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
           generic_transition_time_t *pdata = pargs;
           *pdata = light_hsl->trans_time;
        }
        break;

    case LIGHT_HSL_SERVER_SET:
        {
            light_hsl_server_set_t *pdata = pargs;
            light_hsl->total_time = pdata->total_time;
            light_hsl->remaining_time = pdata->remaining_time;
            if(pdata->remaining_time.num_steps == 0)
            {
                light_hsl->l_lightness = pdata->lightness;
                light_hsl->l_hue = pdata->hue;
                light_hsl->l_saturation = pdata->saturation;
                app_light_hsl_value_set(pmodel_info,pdata->lightness,pdata->hue,pdata->saturation);
            }
            else
            {
                l_info("pdata->remaining_time.num_steps = %d,step_resolution = %d\n",pdata->remaining_time.num_steps,pdata->remaining_time.step_resolution);
            }
        }
        break;

    case LIGHT_HSL_SERVER_SET_HUE:
        {
            light_hsl_server_set_hue_t *pdata = pargs;
            light_hsl->total_time = pdata->total_time;
            light_hsl->remaining_time = pdata->remaining_time;
            if(pdata->remaining_time.num_steps == 0)
            {
                light_hsl->l_hue = pdata->hue;
                app_light_hsl_value_set(pmodel_info,light_hsl->lightness,pdata->hue,light_hsl->saturation);
            }
        }
        break;

    case LIGHT_HSL_SERVER_GET_SATURATION:
        {
            light_hsl_server_get_saturation_t *pdata = pargs;
            pdata->saturation = light_hsl->saturation;
        }
        break;

    case LIGHT_HSL_SERVER_SET_SATURATION:
         {
            light_hsl_server_set_saturation_t *pdata = pargs;
            light_hsl->total_time = pdata->total_time;
            light_hsl->remaining_time = pdata->remaining_time;
            if(pdata->remaining_time.num_steps == 0)
            {
                light_hsl->l_saturation = pdata->saturation;
                app_light_hsl_value_set(pmodel_info,light_hsl->lightness,light_hsl->hue,pdata->saturation);
            }
        }
        break;

    case LIGHT_HSL_SERVER_SET_DEFAULT:
        {
            light_hsl_server_set_default_t *pdata = pargs;
            light_hsl->d_lightness = pdata->lightness;
            light_hsl->d_hue = pdata->hue;
            light_hsl->d_saturation = pdata->saturation;
        }
        break;
     case LIGHT_HSL_SERVER_SET_RANGE:
        {
            light_hsl_server_set_range_t *pdata = pargs;
            light_hsl->hue_range_min = pdata->hue_range_min;
            light_hsl->hue_range_max = pdata->hue_range_max;
            light_hsl->saturation_range_min = pdata->saturation_range_min;
            light_hsl->saturation_range_max = pdata->saturation_range_max;
        }
    default:
        break;
    }

    return ret;
}

uint32_t pts_light_hsl_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&light_hsl_server,0,sizeof(light_hsl_server));
    light_hsl_server.model_data_cb = &light_hsl_server_data;
    ret = light_hsl_server_reg(element_index,&light_hsl_server);
    if(ret == false)
    {
        l_info("%s,server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    memset(&light_hsl_setup_server,0,sizeof(light_hsl_setup_server));
    light_hsl_setup_server.model_data_cb = &light_hsl_server_data;
    ret = light_hsl_setup_server_reg(element_index,&light_hsl_setup_server);
    if(ret == false)
    {
        l_info("%s,setup server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    memset(&light_hsl_saturation_server,0,sizeof(light_hsl_saturation_server));
    light_hsl_saturation_server.model_data_cb = &light_hsl_server_data;
    ret = light_hsl_saturation_server_reg(element_index,&light_hsl_saturation_server);
    if(ret == false)
    {
        l_info("%s,saturation server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    memset(&light_hsl_hue_server,0,sizeof(light_hsl_hue_server));
    light_hsl_hue_server.model_data_cb = &light_hsl_server_data;
    ret = light_hsl_hue_server_reg(element_index,&light_hsl_hue_server);
    if(ret == false)
    {
        l_info("%s,hue server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    return status;
}
