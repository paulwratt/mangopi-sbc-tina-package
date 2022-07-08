#include "pts_app.h"
#include "generic_power_level.h"
static pts_app_t *g_app_db;

static mesh_model_info_t generic_power_level_server;
static mesh_model_info_t generic_power_setup_level_server;
static void app_power_level_power_value_set(mesh_model_info_p pmodel_info,uint16_t power);

void gdtt_update_plv(uint8_t seed, generic_transition_time_t trans_time)
{
//    mesh_model_info_p pmodel_info = &generic_power_level_server;
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.gplv,seed);
    g_app_db->db.power_level.trans_time = trans_time;
    l_info("%s,%d,%d\n",__FUNCTION__,g_app_db->db.power_level.trans_time.num_steps,g_app_db->db.power_level.trans_time.step_resolution);
}

void glv_update_plv(uint8_t seed, int16_t present_level)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.gplv,seed);
    uint16_t power = g_app_db->db.power_level.power;
    mesh_model_info_p pmodel_info = &generic_power_level_server;
    power = present_level + 32768;
    app_power_level_power_value_set(pmodel_info,power);
    l_info("%s,%d\n",__FUNCTION__,g_app_db->db.power_level.power);
}

void goo_update_plv(uint8_t seed, bool onoff)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.gplv,seed);
    uint16_t power = g_app_db->db.power_level.power;
    mesh_model_info_p pmodel_info = &generic_power_level_server;

    if(onoff == false)
    {
         power = 0;
    }
    else
    {
        if(g_app_db->db.power_level.default_power == 0)
        {
            power = g_app_db->db.power_level.last_power;
        }
        else
        {
            power = g_app_db->db.power_level.default_power;
        }
    }

    app_power_level_power_value_set(pmodel_info,power);
    l_info("%s,%d,%d,%d\n",__FUNCTION__,g_app_db->db.power_level.power,g_app_db->db.power_level.default_power,g_app_db->db.power_level.last_power);

}
#if (WORK_AROUND_FOR_GPL_VB_14_C == 1)
void generic_onpowerup_update_timeout_handle(struct l_timeout *timeout,void *context)
{
    mesh_model_info_p pmodel_info = context;
    uint16_t power = g_app_db->db.power_level.power;
    uint16_t default_power = g_app_db->db.power_level.default_power;
    uint16_t last_power = g_app_db->db.power_level.last_power;

    switch(g_app_db->db.power_level.onpowerup_bc)
    {
        case 0:
            power = 0;
            break;
        case 1:
            if(default_power == 0)
            {
                power = last_power;
            }
            else
            {
                power = default_power;
            }
            break;
        case 2:
            power = last_power;
            break;
        default:
            break;
    }

    app_power_level_power_value_set(pmodel_info,power);
    l_timeout_remove(timeout);
    l_info("%s,%d\n",__FUNCTION__,g_app_db->db.power_level.power);
}

void gpoo_update_plv(uint8_t seed, uint8_t status)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.gplv,seed);
    mesh_model_info_p pmodel_info = &generic_power_level_server;
    g_app_db->db.power_level.onpowerup_bc = status;
    start_timeout(generic_onpowerup_update_timeout_handle,pmodel_info,15000);
    l_info("%s,%d,%d\n",__FUNCTION__,status,g_app_db->db.power_level.onpowerup_bc);
}
#else
void gpoo_update_plv(uint8_t seed, uint8_t status)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.gplv,seed);
    mesh_model_info_p pmodel_info = &generic_power_level_server;
    uint16_t power = g_app_db->db.power_level.power;
    uint16_t default_power = g_app_db->db.power_level.default_power;
    uint16_t last_power = g_app_db->db.power_level.last_power;
    switch(status)
    {
        case 0:
            power = 0;
            break;
        case 1:
            if(default_power == 0)
            {
                power = last_power;
            }
            else
            {
                power = default_power;
            }
            break;
        case 2:
            power = last_power;
            break;
        default:
            break;
    }
    app_power_level_power_value_set(pmodel_info,power);
    l_info("%s,%d,%d\n",__FUNCTION__,status,g_app_db->db.power_level.power);
}
#endif

static void app_power_level_power_value_set(mesh_model_info_p pmodel_info,uint16_t power)
{
    /* Resolve the server instance here if required, this example uses only 1 instance. */
    if(g_app_db->db.power_level.power != power)
    {
        g_app_db->db.power_level.power = power;
        PTS_SEED_START(g_app_db->seed.gplv);
        gpl_update_glv(g_app_db->seed.gplv,g_app_db->db.power_level.power);
        gpl_update_goo(g_app_db->seed.gplv,g_app_db->db.power_level.power);
        PTS_SEED_END(g_app_db->seed.gplv);
    }

    //app_ponoff_power_value_update(&m_ponoff_server,g_app_db->db.power_level.power,g_app_db->db.power_level.default_power,g_app_db->db.power_level.last_power);
    generic_power_level_publish(pmodel_info,g_app_db->db.power_level.power);

}

static int32_t generic_power_level_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                     void *pargs)
{
    switch(type)
    {
        case GENERIC_POWER_LEVEL_SERVER_GET:
        {
            generic_power_level_server_get_t *pdata = pargs;
            pdata->power = g_app_db->db.power_level.power;
            break;
        }
        case GENERIC_POWER_LEVEL_SERVER_GET_DEFAULT:
       {
            generic_power_level_server_get_t *pdata = pargs;
            pdata->power = g_app_db->db.power_level.default_power;
            break;
       }
        case GENERIC_POWER_LEVEL_SERVER_GET_LAST:
        {
            generic_power_level_server_get_t *pdata = pargs;
            pdata->power = g_app_db->db.power_level.last_power;
            break;
        }
        case GENERIC_POWER_LEVEL_SERVER_GET_RANGE:
        {
            generic_power_level_server_get_range_t *pdata = pargs;
            l_info("%s,%x,%x",__FUNCTION__,g_app_db->db.power_level.range_max,g_app_db->db.power_level.range_min);
            pdata->range_max = g_app_db->db.power_level.range_max;
            pdata->range_min = g_app_db->db.power_level.range_min;
            break;
        }
        case GENERIC_POWER_LEVEL_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
            generic_transition_time_t *pdata = pargs;
            pdata->num_steps = g_app_db->db.power_level.trans_time.num_steps;
            pdata->step_resolution = g_app_db->db.power_level.trans_time.step_resolution;
            break;
        }
        case GENERIC_POWER_LEVEL_SERVER_SET:
        {
            generic_power_level_server_set_t *pdata = pargs;
            l_info("GENERIC_POWER_LEVEL_SERVER_SET(%d,%d)\n",pdata->remaining_time.num_steps,pdata->remaining_time.step_resolution);
            if(pdata->remaining_time.num_steps == 0)
                app_power_level_power_value_set(pmodel_info,pdata->power);

            g_app_db->db.power_level.total_time = pdata->total_time;
            g_app_db->db.power_level.remaining_time = pdata->remaining_time;
            break;
        }
        case GENERIC_POWER_LEVEL_SERVER_SET_LAST:
        {
            generic_power_level_server_set_last_t *pdata = pargs;
            g_app_db->db.power_level.last_power = pdata->power;
            break;
        }
        case GENERIC_POWER_LEVEL_SERVER_SET_DEFAULT:
        {
            generic_power_level_server_set_default_t *pdata = pargs;
            g_app_db->db.power_level.default_power = pdata->power;
            break;
        }
        case GENERIC_POWER_LEVEL_SERVER_SET_RANGE:
        {
            generic_power_level_server_set_range_t *pdata = pargs;
            g_app_db->db.power_level.range_min = pdata->range_min;
            g_app_db->db.power_level.range_max = pdata->range_max;
            break;
        }
        default:
            break;
    }
    return 0;
}

uint32_t pts_power_level_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&generic_power_level_server,0,sizeof(generic_power_level_server));
    generic_power_level_server.model_data_cb = &generic_power_level_server_data;
    ret = generic_power_level_server_reg(element_index,&generic_power_level_server);
    if(ret == false)
    {
        l_info("%s,fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    memset(&generic_power_setup_level_server,0,sizeof(generic_power_setup_level_server));
    generic_power_setup_level_server.model_data_cb = &generic_power_level_server_data;
    ret = generic_power_level_setup_server_reg(element_index,&generic_power_setup_level_server);
    if(ret == false)
    {
        l_info("%s,fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }

    return status;
}
