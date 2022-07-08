#include "pts_client_app.h"
#if (ENABLE_LIGHT_CTL_CLIENT_APP == 1)
static bool g_init = false;
static mesh_model_info_t lctl_client;
static struct
{
    uint8_t pts_testcase;
    uint16_t lightness;
    uint16_t temperature;
    int16_t delta_uv;
    uint16_t range_min;
    uint16_t range_max;
    pts_client_tx_info tx_info;
    bool optional;
    generic_transition_time_t trans_time;
    uint8_t delay;
    bool ack;
}lctl_test_param;


#define APP_TAG lctl

static int32_t generic_light_ctl_client_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
    UNUSED(pmodel_info);
    switch (type)
    {
    case LIGHT_CTL_CLIENT_STATUS:
        {
            light_ctl_client_status_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_ctl_client_status_t));
            if (pdata->optional)
            {
                app_ts_log(APP_TAG,"client receive: present_lightness = %d, present_temperature = %d, remain time = step(%d), resolution(%d)\r\n",    \
                    pdata->present_lightness, pdata->present_temperature,pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);
            }
            else
            {
                app_ts_log(APP_TAG,"client receive: present_lightness = %d,present_temperature = %d\r\n",pdata->present_lightness, pdata->present_temperature);
            }
        }
        break;
    case LIGHT_CTL_CLIENT_STATUS_TEMPERATURE:
        {
            light_ctl_client_status_temperature_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_ctl_client_status_temperature_t));
                        if (pdata->optional)
            {
                app_ts_log(APP_TAG,"client receive: present_temperature = %d,present_delta_uv = %d,remain time = step(%d), resolution(%d)\r\n", pdata->present_temperature,pdata->present_delta_uv,    \
                    pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);
            }
            else
            {
                app_ts_log(APP_TAG,"client receive: present_temperature = %d,present_delta_uv = %d\r\n",pdata->present_temperature, pdata->present_delta_uv);
            }
        }
        break;

    case LIGHT_CTL_CLIENT_STATUS_TEMPERATURE_RANGE:
        {
            light_ctl_client_status_temperature_range_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_ctl_client_status_temperature_range_t));
            app_ts_log(APP_TAG,"client receive: status = %d\trange_min = %d\trange_max=%d\r\n", pdata->status,pdata->range_min,pdata->range_max);
        }
        break;
    case LIGHT_CTL_CLIENT_STATUS_DEFAULT:
        {
            light_ctl_client_status_default_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_ctl_client_status_default_t));
            app_ts_log(APP_TAG,"client receive: default lightness = %d\ttemperature=%d\tdelta_uv=%d\r\n", pdata->lightness,pdata->temperature,pdata->delta_uv);
        }
        break;
   default:
        break;
    }

    return 0;
}

uint32_t pts_light_ctl_client_reg(uint8_t element_index)
{
    bool ret = false;

    if(g_init == true)
    {
        return PTS_SUCCESS;
    }

    memset(&lctl_client,0,sizeof(lctl_client));
    lctl_client.model_data_cb = generic_light_ctl_client_data;

    ret = light_ctl_client_reg(element_index, &lctl_client);
    if(ret == false)
    {
        return PTS_ERROR;
    }
    g_init = true;
    return PTS_SUCCESS;
}

//model app intf
static void lctl_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_ctl_get(pmodel_info,lctl_test_param.tx_info.dst,lctl_test_param.tx_info.app_key_idx);
}

static void lctl_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_ctl_set(pmodel_info,lctl_test_param.tx_info.dst,lctl_test_param.tx_info.app_key_idx,lctl_test_param.lightness, \
        lctl_test_param.temperature,lctl_test_param.delta_uv,lctl_test_param.tx_info.tid,lctl_test_param.optional,lctl_test_param.trans_time,lctl_test_param.delay,lctl_test_param.ack);
}

static void lctl_temperature_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_ctl_temperature_get(pmodel_info,lctl_test_param.tx_info.dst,lctl_test_param.tx_info.app_key_idx);
}

static void lctl_temperature_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_ctl_temperature_set(pmodel_info,lctl_test_param.tx_info.dst,lctl_test_param.tx_info.app_key_idx, \
        lctl_test_param.temperature,lctl_test_param.delta_uv,lctl_test_param.tx_info.tid,lctl_test_param.optional,lctl_test_param.trans_time,lctl_test_param.delay,lctl_test_param.ack);
}

static void lctl_temperature_range_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_ctl_temperature_range_get(pmodel_info,lctl_test_param.tx_info.dst,lctl_test_param.tx_info.app_key_idx);
}

static void lctl_temperature_range_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_ctl_temperature_range_set(pmodel_info,lctl_test_param.tx_info.dst,lctl_test_param.tx_info.app_key_idx, \
        lctl_test_param.range_min,lctl_test_param.range_max,lctl_test_param.ack);
}

static void lctl_default_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_ctl_default_get(pmodel_info,lctl_test_param.tx_info.dst,lctl_test_param.tx_info.app_key_idx);
}

static void lctl_default_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_ctl_default_set(pmodel_info,lctl_test_param.tx_info.dst,lctl_test_param.tx_info.app_key_idx,lctl_test_param.lightness, \
        lctl_test_param.temperature,lctl_test_param.delta_uv,lctl_test_param.ack);
}

#define GET_CMD 0
#define SET_CMD 1
#define GET_TEMPERATURE_CMD 2
#define SET_TEMPERATURE_CMD 3
#define GET_TEMPERATURE_RANGE_CMD 4
#define SET_TEMPERATURE_RANGE_CMD 5
#define GET_DEFAULT_CMD 6
#define SET_DEFAULT_CMD 7

static app_client_cmd_cb pts_cmd_tab[] =
{
    [GET_CMD] = lctl_get_cmd,
    [SET_CMD] = lctl_set_cmd,
    [GET_TEMPERATURE_CMD] = lctl_temperature_get_cmd,
    [SET_TEMPERATURE_CMD] = lctl_temperature_set_cmd,
    [GET_TEMPERATURE_RANGE_CMD] = lctl_temperature_range_get_cmd,
    [SET_TEMPERATURE_RANGE_CMD] = lctl_temperature_range_set_cmd,
    [GET_DEFAULT_CMD] = lctl_default_get_cmd,
    [SET_DEFAULT_CMD] = lctl_default_set_cmd,
};

static void bv_rx_cb(ts_mgs_p p_msg,void *data, uint32_t len)
{
    //no need to implement
    p_msg->ts_state = TS_STATE_STOP;
}

static void bv_tx_cb(ts_mgs_p p_msg)
{
//    mesh_model_info_p pmodel_info = p_msg->pargs;

    switch(p_msg->ts_state)
    {
        case TS_STATE_0:
            {
                pts_cmd_tab[p_msg->cmd_id](p_msg);

                if(lctl_test_param.ack == false)
                {
                    p_msg->msg_rx = false;
                    p_msg->ts_state = TS_STATE_STOP;
                }
                else
                {
                    p_msg->msg_rx = true;
                }
            }
            break;
        default:
            p_msg->ts_state = TS_STATE_STOP;
            break;
    }
}

static bool lctl_bv_run(ts_mgs_p p_msg)
{
    uint32_t err_code;
    err_code = pts_light_ctl_client_reg(PTS_APP_REG_ELEMENT_0);
    if((p_msg == NULL)||(err_code != PTS_SUCCESS))
    {
        return false;
    }
    memset(p_msg,0,sizeof(ts_msg));
    p_msg->pargs = &lctl_client;
    p_msg->msg_tx = true;
    p_msg->msg_rx = true;
    lctl_test_param.tx_info.app_key_idx = 0;
    lctl_test_param.tx_info.dst = 1;
    lctl_test_param.tx_info.tid = 0;
    return true;
}
//pts_client lctl bv-01-c
static void lctl_bv_01_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_CMD;
    lctl_test_param.pts_testcase = 1;
    lctl_test_param.ack = true;
    ts_run(&msg);
}
//pts_client lctl bv-02-c
static void lctl_bv_02_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lctl_test_param.pts_testcase = 2;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lctl_test_param.lightness   = cmd_line_value[0];
            lctl_test_param.temperature = cmd_line_value[1];
            lctl_test_param.delta_uv    = cmd_line_value[2];
        }
    }
    lctl_test_param.ack = true;
    lctl_test_param.optional = false;
    ts_run(&msg);
}
 //pts_client lctl bv-03-c
static void lctl_bv_03_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lctl_test_param.pts_testcase = 3;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lctl_test_param.lightness   = cmd_line_value[0];
            lctl_test_param.temperature = cmd_line_value[1];
            lctl_test_param.delta_uv    = cmd_line_value[2];
        }
    }
    lctl_test_param.ack = true;
    lctl_test_param.optional = true;
    lctl_test_param.trans_time = pts_trans_time_ei;
    lctl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}
 //pts_client lctl bv-04-c
static void lctl_bv_04_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lctl_test_param.pts_testcase = 4;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lctl_test_param.lightness   = cmd_line_value[0];
            lctl_test_param.temperature = cmd_line_value[1];
            lctl_test_param.delta_uv    = cmd_line_value[2];
        }
    }
    lctl_test_param.ack = true;
    lctl_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lctl_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lctl_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lctl_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lctl_test_param.trans_time = pts_trans_time_c;
            lctl_test_param.delay = pts_delay_c;
        }
    }
    ts_run(&msg);
}
 //pts_client lctl bv-05-c
static void lctl_bv_05_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lctl_test_param.pts_testcase = 5;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lctl_test_param.lightness   = cmd_line_value[0];
            lctl_test_param.temperature = cmd_line_value[1];
            lctl_test_param.delta_uv    = cmd_line_value[2];
        }
    }
    lctl_test_param.ack = false;
    lctl_test_param.optional = false;
    ts_run(&msg);
}
 //pts_client lctl bv-06-c
static void lctl_bv_06_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lctl_test_param.pts_testcase = 6;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lctl_test_param.lightness   = cmd_line_value[0];
            lctl_test_param.temperature = cmd_line_value[1];
            lctl_test_param.delta_uv    = cmd_line_value[2];
        }
    }
    lctl_test_param.ack = false;
    lctl_test_param.optional = true;
    lctl_test_param.trans_time = pts_trans_time_ei;
    lctl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}
 //pts_client lctl bv-07-c
static void lctl_bv_07_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lctl_test_param.pts_testcase = 7;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lctl_test_param.lightness   = cmd_line_value[0];
            lctl_test_param.temperature = cmd_line_value[1];
            lctl_test_param.delta_uv    = cmd_line_value[2];
        }
    }
    lctl_test_param.ack = false;
    lctl_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lctl_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lctl_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lctl_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lctl_test_param.trans_time = pts_trans_time_c;
            lctl_test_param.delay = pts_delay_c;
        }
    }
    ts_run(&msg);
}
 //pts_client lctl bv-08-c
static void lctl_bv_08_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_TEMPERATURE_CMD;
    lctl_test_param.pts_testcase = 8;
    lctl_test_param.ack = true;
    ts_run(&msg);
}
 //pts_client lctl bv-09-c
static void lctl_bv_09_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_TEMPERATURE_CMD;
    lctl_test_param.pts_testcase = 9;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-v") == true)
        {
            lctl_test_param.temperature = cmd_line_value[0];
            lctl_test_param.delta_uv = cmd_line_value[1];
        }
    }
    lctl_test_param.ack = true;
    lctl_test_param.optional = false;
    ts_run(&msg);
}
 //pts_client lctl bv-10-c

static void lctl_bv_10_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_TEMPERATURE_CMD;
    lctl_test_param.pts_testcase = 10;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-v") == true)
        {
            lctl_test_param.temperature = cmd_line_value[0];
            lctl_test_param.delta_uv = cmd_line_value[1];
        }
    }
    lctl_test_param.ack = true;
    lctl_test_param.optional = true;
    lctl_test_param.trans_time = pts_trans_time_ei;
    lctl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}
 //pts_client lctl bv-11-c

static void lctl_bv_11_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_TEMPERATURE_CMD;
    lctl_test_param.pts_testcase = 11;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-v") == true)
        {
            lctl_test_param.temperature = cmd_line_value[0];
            lctl_test_param.delta_uv = cmd_line_value[1];
        }
    }

    lctl_test_param.ack = true;
    lctl_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lctl_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lctl_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lctl_test_param.delay = cmd_line_value[1];
        }
        else
        {
            lctl_test_param.trans_time = pts_trans_time_c;
            lctl_test_param.delay = pts_delay_c;
        }
    }
    ts_run(&msg);
}

static void lctl_bv_12_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_TEMPERATURE_CMD;
    lctl_test_param.pts_testcase = 12;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-v") == true)
        {
            lctl_test_param.temperature = cmd_line_value[0];
            lctl_test_param.delta_uv = cmd_line_value[1];
        }
    }
    lctl_test_param.ack = false;
    lctl_test_param.optional = false;
    ts_run(&msg);
}

static void lctl_bv_13_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_TEMPERATURE_CMD;
    lctl_test_param.pts_testcase = 13;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-v") == true)
        {
            lctl_test_param.temperature = cmd_line_value[0];
            lctl_test_param.delta_uv = cmd_line_value[1];
        }
    }
    lctl_test_param.ack = false;
    lctl_test_param.optional = true;
    lctl_test_param.trans_time = pts_trans_time_ei;
    lctl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}

static void lctl_bv_14_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_TEMPERATURE_CMD;
    lctl_test_param.pts_testcase = 14;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-v") == true)
        {
            lctl_test_param.temperature = cmd_line_value[0];
            lctl_test_param.delta_uv = cmd_line_value[1];
        }
    }

    lctl_test_param.ack = false;
    lctl_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lctl_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lctl_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lctl_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lctl_test_param.trans_time = pts_trans_time_c;
            lctl_test_param.delay = pts_delay_c;
        }
    }
    ts_run(&msg);
}

static void lctl_bv_15_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_DEFAULT_CMD;
    lctl_test_param.pts_testcase = 15;
    lctl_test_param.ack = true;
    ts_run(&msg);
}

static void lctl_bv_16_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_DEFAULT_CMD;
    lctl_test_param.pts_testcase = 16;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lctl_test_param.lightness   = cmd_line_value[0];
            lctl_test_param.temperature = cmd_line_value[1];
            lctl_test_param.delta_uv    = cmd_line_value[2];
        }
    }
    lctl_test_param.ack = true;
    lctl_test_param.optional = false;
    ts_run(&msg);
}

static void lctl_bv_17_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_DEFAULT_CMD;
    lctl_test_param.pts_testcase = 17;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lctl_test_param.lightness   = cmd_line_value[0];
            lctl_test_param.temperature = cmd_line_value[1];
            lctl_test_param.delta_uv    = cmd_line_value[2];
        }
    }
    lctl_test_param.ack = false;
    lctl_test_param.optional = false;
    ts_run(&msg);
}

static void lctl_bv_18_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_TEMPERATURE_RANGE_CMD;
    lctl_test_param.pts_testcase = 18;
    lctl_test_param.ack = true;
    ts_run(&msg);
}

static void lctl_bv_19_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_TEMPERATURE_RANGE_CMD;
    lctl_test_param.pts_testcase = 19;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-v") == true)
        {
            lctl_test_param.range_min   = cmd_line_value[0];
            lctl_test_param.range_max   = cmd_line_value[1];
        }
    }
    lctl_test_param.ack = true;
    lctl_test_param.optional = false;
    ts_run(&msg);
}

static void lctl_bv_20_c()
{
    ts_msg msg;
    if(lctl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_TEMPERATURE_RANGE_CMD;
    lctl_test_param.pts_testcase = 20;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-v") == true)
        {
            lctl_test_param.range_min   = cmd_line_value[0];
            lctl_test_param.range_max   = cmd_line_value[1];
        }
    }
    lctl_test_param.ack = false;
    lctl_test_param.optional = false;
    ts_run(&msg);
}

//cmd line
static struct
{
    char *cmd;
    void (*func)();
    char *doc;
}ts_command[] =
{
        {STR_TESTCASE_BV_NB(01), func_testcase(lctl,01), STR_TESTCASE_NAME(LCTL,01)},    //pts_client lctl bv-01-c
        {STR_TESTCASE_BV_NB(02), func_testcase(lctl,02), STR_TESTCASE_NAME(LCTL,02)},    //pts_client lctl bv-02-c
        {STR_TESTCASE_BV_NB(03), func_testcase(lctl,03), STR_TESTCASE_NAME(LCTL,03)},    //pts_client lctl bv-03-c
        {STR_TESTCASE_BV_NB(04), func_testcase(lctl,04), STR_TESTCASE_NAME(LCTL,04)},
        {STR_TESTCASE_BV_NB(05), func_testcase(lctl,05), STR_TESTCASE_NAME(LCTL,05)},
        {STR_TESTCASE_BV_NB(06), func_testcase(lctl,06), STR_TESTCASE_NAME(LCTL,06)},
        {STR_TESTCASE_BV_NB(07), func_testcase(lctl,07), STR_TESTCASE_NAME(LCTL,07)},
        {STR_TESTCASE_BV_NB(08), func_testcase(lctl,08), STR_TESTCASE_NAME(LCTL,08)},
        {STR_TESTCASE_BV_NB(09), func_testcase(lctl,09), STR_TESTCASE_NAME(LCTL,09)},
        {STR_TESTCASE_BV_NB(10), func_testcase(lctl,10), STR_TESTCASE_NAME(LCTL,10)},
        {STR_TESTCASE_BV_NB(11), func_testcase(lctl,11), STR_TESTCASE_NAME(LCTL,11)},
        {STR_TESTCASE_BV_NB(12), func_testcase(lctl,12), STR_TESTCASE_NAME(LCTL,12)},
        {STR_TESTCASE_BV_NB(13), func_testcase(lctl,13), STR_TESTCASE_NAME(LCTL,13)},
        {STR_TESTCASE_BV_NB(14), func_testcase(lctl,14), STR_TESTCASE_NAME(LCTL,14)},
        {STR_TESTCASE_BV_NB(15), func_testcase(lctl,15), STR_TESTCASE_NAME(LCTL,15)},
        {STR_TESTCASE_BV_NB(16), func_testcase(lctl,16), STR_TESTCASE_NAME(LCTL,16)},
        {STR_TESTCASE_BV_NB(17), func_testcase(lctl,17), STR_TESTCASE_NAME(LCTL,17)},
        {STR_TESTCASE_BV_NB(18), func_testcase(lctl,18), STR_TESTCASE_NAME(LCTL,18)},
        {STR_TESTCASE_BV_NB(19), func_testcase(lctl,19), STR_TESTCASE_NAME(LCTL,19)},
        {STR_TESTCASE_BV_NB(20), func_testcase(lctl,20), STR_TESTCASE_NAME(LCTL,20)}
};

void run_lctl_client_ts(int argc, char *args[])
{
    uint8_t i;

    for (i = 0; ts_command[i].cmd; i++) {
        if (strcmp(args[1], ts_command[i].cmd))
        {
            continue;
        }
        if (ts_command[i].func) {
            ts_command[i].func();
            break;
        }
    }
    if((ts_command[i].doc != NULL)&&(ts_command[i].cmd == NULL))
    {
        app_ts_log(APP_TAG,"%s %s %s\n",args[0],args[1],ts_command[i].doc);
    }
}

#endif
