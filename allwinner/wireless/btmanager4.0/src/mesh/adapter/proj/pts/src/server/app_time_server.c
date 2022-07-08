#include "pts_app.h"
#include "generic_onoff_server.h"

static pts_app_t *g_app_db;

static mesh_model_info_t m_time_server;
static int32_t time_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                     void *pargs)
{
    app_time_t   *time =&g_app_db->db.time;
    switch(type)
    {
        case TIME_SERVER_GET:
        {
            l_info("%s,TIME_SERVER_GET\n",__FUNCTION__);
            time_server_get_t *pdata = pargs;
            break;
        }
        case TIME_SERVER_GET_ROLE:
        {
            l_info("%s,TIME_SERVER_GET_ROLE\n",__FUNCTION__);
            time_server_get_role_t *pdata = pargs;
            break;
        }
        case TIME_SERVER_GET_ZONE:
        {
            l_info("%s,TIME_SERVER_GET_ZONE\n",__FUNCTION__);
            time_server_get_zone_t *pdata = pargs;
            break;
        }
        case TIME_SERVER_GET_TAI_UTC_DELTA:
        {
            l_info("%s,TIME_SERVER_GET_TAI_UTC_DELTA\n",__FUNCTION__);
            time_server_get_tai_utc_delta_t *pdata = pargs;
            break;
        }
        case TIME_SERVER_SET:
        {
            l_info("%s,TIME_SERVER_SET\n",__FUNCTION__);
            time_role_set_t *pdata = pargs;
            break;
        }
        case TIME_SERVER_SET_ROLE:
        {
            l_info("%s,TIME_SERVER_SET_ROLE\n",__FUNCTION__);
            time_role_set_t *pdata = pargs;
            break;
        }
        case TIME_SERVER_STATUS_SET:
        {
            l_info("%s,SENSOR_SERVER_GET_SERIES\n",__FUNCTION__);
            time_server_status_set_t *pdata = pargs;
            break;
        }
        case TIME_SERVER_SET_ZONE:
        {
            l_info("%s,TIME_SERVER_SET_ZONE\n",__FUNCTION__);
            time_server_set_zone_t *pdata = pargs;
            break;
        }
        case TIME_SERVER_SET_TAI_UTC_DELTA:
        {
            l_info("%s,TIME_SERVER_SET_TAI_UTC_DELTA\n",__FUNCTION__);
            time_server_set_tai_utc_delta_t *pdata = pargs;
            break;
        }
        default:
            break;
    }
    return 0;
}

uint32_t pts_time_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&m_time_server,0,sizeof(m_time_server));
    m_time_server.model_data_cb = &time_server_data;
    ret = time_server_reg(element_index,&m_time_server);
    if(ret = false)
    {
        l_info("%s,server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    ret = time_setup_server_reg(element_index,&m_time_server);
    if(ret = false)
    {
        l_info("%s,setup server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    return status;
}
