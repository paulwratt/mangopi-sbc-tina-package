#include "pts_app.h"
#include "generic_on_off.h"

static pts_app_t *g_app_db = NULL;

static mesh_model_info_t goo_server;

#define ENABLE_MODEL_BINDINGS 0

static void on_off_value_set(mesh_model_info_p pmodel_info,bool on_off)
{
    if(g_app_db->db.goo.present_onoff != on_off)
    {
        g_app_db->db.goo.present_onoff = on_off;
#if(ENABLE_MODEL_BINDINGS == 1)
        PTS_SEED_START(g_app_db->seed.goo);
        goo_update_plv(g_app_db->seed.goo,on_off);
        goo_update_lln(g_app_db->seed.goo,on_off);
        PTS_SEED_END(g_app_db->seed.goo);
#endif
    }
    generic_on_off_publish(pmodel_info,on_off);
}

void gdtt_update_goo(uint8_t seed, generic_transition_time_t trans_time)
{
#if(ENABLE_MODEL_BINDINGS == 1)
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.goo,seed);
    //mesh_model_info_p pmodel_info = &goo_server;
    g_app_db->db.goo.trans_time = trans_time;
#endif
}

void power_rst_goo(uint8_t onpower_state)
{
#if(ENABLE_MODEL_BINDINGS == 1)

    mesh_model_info_p pmodel_info = &goo_server;

    switch(onpower_state)
    {
        case 0:
            g_app_db->db.goo.present_onoff = 0;
            break;

        case 1:

            g_app_db->db.goo.present_onoff = 1;
            break;

        case 2:
            g_app_db->db.goo.present_onoff = g_app_db->db.goo.target_onoff;
            break;

        default:
            break;
    }
    l_info("%s,onoff state = %d\n",__FUNCTION__,g_app_db->db.goo.present_onoff);
    generic_on_off_publish(pmodel_info,g_app_db->db.present_onoff);
#endif
}

void lln_update_goo(uint8_t seed,uint16_t lightness)
{
#if(ENABLE_MODEL_BINDINGS == 1)
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.goo,seed);
    mesh_model_info_p pmodel_info = &goo_server;
    bool onoff ;
    if(lightness == 0)
    {
        onoff = false;
    }
    else
    {
        onoff = true;
    }
    g_app_db->db.present_onoff = onoff;
    on_off_value_set(pmodel_info,onoff);
#endif
}

void gpl_update_goo(uint8_t seed,uint16_t power)
{
#if(ENABLE_MODEL_BINDINGS == 1)

    PTS_SEED_CHECK(g_app_db,g_app_db->seed.goo,seed);
    mesh_model_info_p pmodel_info = &goo_server;
    bool onoff ;
    if(power == 0)
    {
        onoff = false;
    }
    else
    {
        onoff = true;
    }
    g_app_db->db.present_onoff = onoff;
    on_off_value_set(pmodel_info,onoff);
#endif
}

static void goo_delay_to_handle(struct l_timeout *timeout, void * p_context)
{
    app_goo_t  *goo =&g_app_db->db.goo;
    mesh_model_info_p pmodel_info = p_context;
    on_off_value_set(pmodel_info,goo->target_onoff);
    l_timeout_remove(timeout);
}

static int32_t goo_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                           void *pargs)
{
    app_goo_t  *goo =&g_app_db->db.goo;
    int32_t ret = MODEL_SUCCESS;
    //bool init = false;
    switch (type)
    {
    case GENERIC_ON_OFF_SERVER_GET:
        {
            generic_on_off_server_get_t *pdata = pargs;
            pdata->on_off = goo->present_onoff;
            l_info("%s get on_off %d",__func__,goo->present_onoff);
        }
        break;

    case GENERIC_ON_OFF_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
           generic_transition_time_t *pdata = pargs;
           *pdata = goo->trans_time;
           l_info("%s get gdtt trans_time step 0x%x resolution 0x%x",__func__,goo->trans_time.num_steps,goo->trans_time.step_resolution);
        }
        break;

    case GENERIC_ON_OFF_SERVER_SET:
        {
            generic_on_off_server_set_t *pdata = pargs;
            goo->total_time = pdata->total_time;
            goo->remaining_time = pdata->remaining_time;
            goo->target_onoff = pdata->on_off;
            if(pdata->remaining_time.num_steps == 0)
            {
                on_off_value_set(pmodel_info,pdata->on_off);
            }
            else
            {
                if(goo->target_onoff != goo->present_onoff)
                    start_timeout(goo_delay_to_handle,pmodel_info,PTS_DEFAULT_DELAY_MS);
            }
            l_info("%s set target on_off %d present on_off %d",__func__,pdata->on_off,goo->present_onoff);
        }
        break;

    default:
        break;
    }

    return ret;
}

uint32_t pts_goo_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&goo_server,0,sizeof(goo_server));
    goo_server.model_data_cb = &goo_server_data;
    ret = generic_on_off_server_reg(element_index,&goo_server);
    if(ret == false)
    {
        l_info("%s,server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    return status;
}
