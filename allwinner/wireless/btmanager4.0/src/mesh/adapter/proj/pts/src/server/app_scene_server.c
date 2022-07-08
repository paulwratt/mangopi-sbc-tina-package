#include "pts_app.h"
#include "generic_onoff_server.h"

static pts_app_t *g_app_db;

static mesh_model_info_t m_scene_server;
static int32_t scene_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                     void *pargs)
{
    app_scene_t   *p_scene =&g_app_db->db.scene;
    switch(type)
    {
        case SCENE_SERVER_GET:
        {
            scene_server_get_t *pdata = pargs;
            pdata->current_scene = p_scene->current_scene;
            break;
        }
        case SCENE_SERVER_GET_REGISTER_STATUS:
        {
            scene_server_get_register_status_t *pdata = pargs;
            pdata->status = p_scene->status;
            break;
        }
        case SCENE_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
            generic_transition_time_t *pdata = pargs;
            *pdata = p_scene->trans_time;
            //pdata->num_steps = p_scene->trans_time.num_steps;
            //pdata->step_resolution = p_scene->trans_time.step_resolution;
            break;
        }
        case SCENE_SERVER_STORE:
        {
            scene_server_store_t *pdata = pargs;
            p_scene->status = pdata->status;
            p_scene->scene_number = pdata->scene_number;
            p_scene->pmemory = pdata->pmemory;
            break;
        }
        case SCENE_SERVER_RECALL:
        {
            scene_server_recall_t *pdata = pargs;
            p_scene->pmemory = pdata->pmemory;
            p_scene->total_time = pdata->total_time;
            p_scene->remaining_time = pdata->remaining_time;
            break;
        }
        case SCENE_SERVER_DELETE:
        {
            scene_server_delete_t *pdata = pargs;
            if(p_scene->scene_number == pdata->scene_number)
            {
                l_info("%s,scene[%d] delete found\n",__FUNCTION__,p_scene->scene_number );
            }
            else
            {
                l_info("%s,scene[%d] delete not found\n",__FUNCTION__,p_scene->scene_number );
            }
            break;
        }
        default:
            break;
    }
    return 0;
}

uint32_t pts_scene_reg(pts_app_t *pts_db,uint8_t element_index)
{
    bool ret = false;
    uint32_t status = NRF_SUCCESS;
    g_app_db = pts_db;
    memset(&m_scene_server,0,sizeof(m_scene_server));
    m_scene_server.model_data_cb = &scene_server_data;
    ret = scene_server_reg(element_index,&m_scene_server);
    if(ret = false)
    {
        l_info("%s,server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    ret = scene_setup_server_reg(element_index,&m_scene_server);
    if(ret = false)
    {
        l_info("%s,setup server reg fail\n",__FUNCTION__);
        status = NRF_ERROR_NULL;
    }
    return status;
}
