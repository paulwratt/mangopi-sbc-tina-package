#include "pts_app.h"
#include "generic_ponoff_setup_server.h"
#include "generic_ponoff_messages.h"

static pts_app_t *g_app_db;
#if 0
static bool alloc_idle_binding(uint8_t *state,uint8_t *hdl)
{
    uint8_t i = 0;
    bool ret = false;
    for(i = 0; i< MAX_BINGDING_MODLES_CNT; i++)
    {
        if(state[i] == BINDING_STATE_IDLE)
        {
            *hdl = i;
            state[i] = BINDING_STATE_ALLOC;
            ret = true;
            break;
        }
    }
    return ret;
};
#endif
//static bool mmdl_sr_gpl_bv_14_c_used = true;
/******** Model Callbacks ********/
#if 0
static uint8_t app_get_onpowerup_state()
{
    return g_app_db->db.poonoff_status;
}
#endif

static void app_ponoff_value_set(generic_ponoff_setup_server_t *p_server,uint8_t status)
{

    uint8_t *p_on_powerup = &g_app_db->db.poonoff_status;
    generic_ponoff_status_params_t param;

    //publish first
    param.on_powerup = status;
    generic_ponoff_setup_server_status_publish(p_server,&param);

    if(*p_on_powerup != status)
    {
        *p_on_powerup = status;
    }
    PTS_SEED_START(g_app_db->seed.gpoo);
    gpoo_update_plv(g_app_db->seed.gpoo,status);
    PTS_SEED_END(g_app_db->seed.gpoo);
}

static void ponoff_server_set_cb(const generic_ponoff_setup_server_t * p_self,
                                 const access_message_rx_meta_t * p_meta,
                                 const generic_ponoff_set_params_t * p_in,
                                 generic_ponoff_status_params_t * p_out)
{
    app_ponoff_value_set((generic_ponoff_setup_server_t * )p_self,p_in->on_powerup);

    if(p_out != NULL)
    {
        p_out->on_powerup = g_app_db->db.poonoff_status;
    }
}

static void ponoff_server_get_cb(const generic_ponoff_setup_server_t * p_self,
                         const access_message_rx_meta_t * p_meta,
                         generic_ponoff_status_params_t * p_out)
{
    p_out->on_powerup = g_app_db->db.poonoff_status;
}

//poonoff_status
static const generic_ponoff_setup_server_callbacks_t m_server_cbs =
{
    .ponoff_cbs.get_cb = ponoff_server_get_cb,
    .ponoff_cbs.set_cb = ponoff_server_set_cb,
};

static generic_ponoff_setup_server_t  m_ponoff_server;

uint32_t pts_ponoff_reg(pts_app_t *pts_db,uint8_t element_index)
{
    uint32_t status = NRF_ERROR_INTERNAL;
    g_app_db = pts_db;
    memset(&m_ponoff_server, 0, sizeof(generic_ponoff_setup_server_t));
    m_ponoff_server.settings.p_callbacks = &m_server_cbs;
    status = generic_ponoff_setup_server_init(&m_ponoff_server, element_index);
    if(status != NRF_SUCCESS)
    {
        l_info("%s,fail\n",__FUNCTION__);
    }

    return status;
}

void lln_update_gpoo(uint8_t seed,uint16_t lightness)
{
    generic_ponoff_setup_server_t * p_server = &m_ponoff_server;
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.gpoo,seed);
    uint8_t poonoff_status = g_app_db->db.poonoff_status;
    //generic_ponoff_status_params_t param;
    if(g_app_db == NULL)
        return ;

    if(lightness == 0)
    {
        poonoff_status = 0;
    }
    else
    {
        poonoff_status = 1;
    }
    app_ponoff_value_set(p_server, poonoff_status);
    l_info("%s,%d,%d\n",__FUNCTION__,g_app_db->db.poonoff_status,g_app_db->db.poonoff_status);

}

void gplv_update_gpoo(uint8_t seed,uint16_t power, uint16_t power_default, uint16_t power_last)
{
    generic_ponoff_setup_server_t * p_server = &m_ponoff_server;
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.gpoo,seed);
    uint8_t poonoff_status = g_app_db->db.poonoff_status;
    //generic_ponoff_status_params_t param;
    if(g_app_db == NULL)
        return ;

    if(power == 0)
    {
        poonoff_status = 0;
    }
    else
    {
        poonoff_status = 1;
    }
    app_ponoff_value_set(p_server, poonoff_status);
}
