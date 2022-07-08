#include "pts_app.h"
#include "generic_onoff_server.h"

static pts_app_t *g_app_db;

static mesh_model_info_t m_sensor_server;
static int32_t sensor_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                     void *pargs)
{
    app_sensor_t   *sensor =&g_app_db->db.sensor;
    switch(type)
    {
        case SENSOR_SERVER_GET:
        {
            sensor_server_get_t *pdata = pargs;
            pdata->property_id = sensor->property_id;
            pdata->raw_data = sensor->raw_data;
            break;
        }
        case SENSOR_SERVER_GET_COLUMN:
        {
            sensor_server_get_column_t *pdata = pargs;
            pdata->column = sensor->column;
            pdata->column_len = sensor->column_len;
            pdata->property_id = sensor->property_id;
            pdata->raw_value_x = sensor->raw_value_x;
            pdata->raw_value_x_len = sensor->raw_value_x_len;
            break;
        }
        case SENSOR_SERVER_GET_SERIES:
        {
            sensor_series_get_t *pdata = pargs;
            l_info("%s,SENSOR_SERVER_GET_SERIES\n",__FUNCTION__);
            break;
        }
        case SENSOR_SERVER_SET_CADENCE:
        {
            sensor_server_set_cadence_t *pdata = pargs;
            l_info("%s,SENSOR_SERVER_GET_SERIES\n",__FUNCTION__);

            break;
        }
        case SENSOR_SERVER_SET_SETTING:
        {
            sensor_server_set_setting_t *pdata = pargs;
            l_info("%s,SENSOR_SERVER_GET_SERIES\n",__FUNCTION__);

            break;
        }
        case SENSOR_SERVER_COMPARE_CADENCE:
        {
            sensor_server_get_t *pdata = pargs;
            l_info("%s,SENSOR_SERVER_GET_SERIES\n",__FUNCTION__);
            break;
        }
        default:
            break;
    }
    return 0;
}

uint32_t pts_sensor_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&m_sensor_server,0,sizeof(m_sensor_server));
    m_sensor_server.model_data_cb = &sensor_server_data;
    ret = sensor_server_reg(element_index,&m_sensor_server);
    if(ret = false)
    {
        l_info("%s,server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    ret = sensor_setup_server_reg(element_index,&m_sensor_server);
    if(ret = false)
    {
        l_info("%s,setup server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    return status;
}
