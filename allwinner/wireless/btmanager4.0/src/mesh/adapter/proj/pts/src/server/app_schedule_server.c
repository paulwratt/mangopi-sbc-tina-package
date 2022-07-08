#include "pts_app.h"
#include "generic_onoff_server.h"

static pts_app_t *g_app_db;

static mesh_model_info_t m_scheduler_server;
static int32_t scheduler_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                     void *pargs)
{
    app_scheduler_t   *scheduler =&pts_app.db.scheduler;
    switch(type)
    {
        case SCHEDULER_SERVER_GET:
        {
            l_info("%s,SCHEDULER_SERVER_GET\n",__FUNCTION__);
            scheduler_server_get_t *pdata = pargs;
            break;
        }
        case SCHEDULER_SERVER_GET_ACTION:
        {
            l_info("%s,SCHEDULER_SERVER_GET_ACTION\n",__FUNCTION__);
            scheduler_server_get_action_t *pdata = pargs;
            break;
        }
        case SCHEDULER_SERVER_SET_ACTION:
        {
            l_info("%s,SCHEDULER_SERVER_SET_ACTION\n",__FUNCTION__);
            scheduler_action_set_t *pdata = pargs;
            break;
        }
        default:
            break;
    }
    return 0;
}

uint32_t pts_scheduler_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&m_scheduler_server,0,sizeof(m_scheduler_server));
    m_scheduler_server.model_data_cb = &scheduler_server_data;
    ret = scheduler_server_reg(element_index,&m_scheduler_server);
    if(ret = false)
    {
        l_info("%s,server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    ret = scheduler_setup_server_reg(element_index,&m_scheduler_server);
    if(ret = false)
    {
        l_info("%s,setup server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    return status;
}
