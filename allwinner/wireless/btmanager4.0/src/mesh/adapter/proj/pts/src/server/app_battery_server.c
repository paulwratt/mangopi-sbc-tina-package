#include "pts_app.h"
#include "generic_onoff_server.h"

static pts_app_t *g_app_db;

static mesh_model_info_t m_battery_server;
static int32_t battery_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                     void *pargs)
{
    app_battery_t   *p_battery =&g_app_db->db.battery;
    switch(type)
    {
        case GENERIC_BATTERY_SERVER_GET:
        {
            generic_battery_server_get_t *pdata = pargs;
            pdata->battery_level = p_battery->battery_level;
            pdata->time_to_discharge = p_battery->time_to_discharge;
            pdata->time_to_charge = p_battery->time_to_charge;
            pdata->flags = p_battery->flags;
            break;
        }
        default:
            break;
    }
    return 0;
}

uint32_t pts_battery_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&m_battery_server,0,sizeof(m_battery_server));
    m_battery_server.model_data_cb = &battery_server_data;
    ret = generic_battery_server_reg(element_index,&m_battery_server);
    if(ret == false)
    {
        l_info("%s,fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }

    return status;
}
