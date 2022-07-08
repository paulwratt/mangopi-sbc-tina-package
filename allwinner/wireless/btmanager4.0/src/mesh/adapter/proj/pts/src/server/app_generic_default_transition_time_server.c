
#include "pts_app.h"
static pts_app_t *g_app_db;

static mesh_model_info_t generic_default_transition_time_server;
static int32_t generic_dtt_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                     void *pargs)
{
    switch(type)
    {
        case GENERIC_DEFAULT_TRANSITION_TIME_SERVER_GET:
        {
            generic_default_transition_time_server_get_t *pdata = pargs;
            pdata->trans_time.num_steps = g_app_db->db.trans_time.num_steps;
            pdata->trans_time.step_resolution = g_app_db->db.trans_time.step_resolution;
            break;
        }
        case GENERIC_DEFAULT_TRANSITION_TIME_SERVER_SET:
       {
            generic_default_transition_time_server_set_t *pdata = pargs;
            g_app_db->db.trans_time.num_steps = pdata->trans_time.num_steps;
            g_app_db->db.trans_time.step_resolution = pdata->trans_time.step_resolution;
            PTS_SEED_START(g_app_db->seed.gbat);
            gdtt_update_goo(g_app_db->seed.gbat, g_app_db->db.trans_time);
            gdtt_update_glv(g_app_db->seed.gbat, g_app_db->db.trans_time);
            gdtt_update_plv(g_app_db->seed.gbat,g_app_db->db.trans_time);
            gdtt_update_lln(g_app_db->seed.gbat,g_app_db->db.trans_time);
            gdtt_update_lctl(g_app_db->seed.gbat,g_app_db->db.trans_time);
            gdtt_update_lhsl(g_app_db->seed.gbat,g_app_db->db.trans_time);
            PTS_SEED_END(g_app_db->seed.gbat);
            break;
       }
        default:
            break;
    }
    return 0;
}

uint32_t pts_dtt_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&generic_default_transition_time_server,0,sizeof(generic_default_transition_time_server));
    generic_default_transition_time_server.model_data_cb = &generic_dtt_server_data;
    ret = generic_default_transition_time_server_reg(element_index,&generic_default_transition_time_server);
    if(ret == false)
    {
        l_info("%s,fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    return status;
}
