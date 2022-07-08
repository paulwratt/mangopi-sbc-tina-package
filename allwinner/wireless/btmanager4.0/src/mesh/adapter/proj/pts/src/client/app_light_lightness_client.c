#include "pts_client_app.h"
#if (ENABLE_LIGHT_LIGHTNESS_CLIENT_APP == 1)
static bool g_init = false;
static mesh_model_info_t lln_client;

#define LLN_DEFAULT_LIGHTNESS 0
#define LLN_RANGE_MAX 0
#define LLN_RANGE_MIN 0

//static generic_transition_time_t loc_trans_time_c = {20,1};
//static uint8_t loc_delay_c = 5;
//static generic_transition_time_t loc_trans_time_i = {0,3};
//static uint8_t loc_delay_i = 0;
//static uint16_t loc_default_value = LLN_DEFAULT_LIGHTNESS;
//static uint16_t loc_default_range_min = LLN_RANGE_MIN;
//static uint16_t loc_default_range_max = LLN_RANGE_MAX;

static struct
{
    uint8_t pts_testcase;
    uint16_t lightness;
    uint16_t linear;
    uint16_t d_lightness;
    uint16_t range_min;
    uint16_t range_max;
    pts_client_tx_info tx_info;
    bool optional;
    generic_transition_time_t trans_time;
    uint8_t delay;
    bool ack;
}lln_test_param;

#define APP_TAG lln

static int32_t generic_light_lightness_client_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
    UNUSED(pmodel_info);
    switch (type)
    {
    case LIGHT_LIGHTNESS_CLIENT_STATUS:
        {
            light_lightness_client_status_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_lightness_client_status_t));
            if (pdata->optional)
            {
                app_ts_log(APP_TAG,"client receive: present_lightness = %d, target_lightness = %d, remain time = step(%d), resolution(%d)\r\n",    \
                    pdata->present_lightness, pdata->target_lightness,pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);
            }
            else
            {
                app_ts_log(APP_TAG,"client receive: present_lightness = %d,target_lightness = %d\r\n",pdata->present_lightness, pdata->target_lightness);
            }
        }
        break;
    case LIGHT_LIGHTNESS_CLIENT_STATUS_LINEAR:
        {
            light_lightness_client_status_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_lightness_client_status_t));
            app_ts_log(APP_TAG,"client receive: present_lightness = %d,target_lightness = %d,remain time = step(%d), resolution(%d)\r\n", pdata->present_lightness,pdata->target_lightness,    \
                pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);
        }
        break;

    case LIGHT_LIGHTNESS_CLIENT_STATUS_LAST:
        {
            light_lightness_client_status_last_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_lightness_client_status_last_t));
            app_ts_log(APP_TAG,"client receive: last lightness = %d\r\n", pdata->lightness);
        }
        break;
    case LIGHT_LIGHTNESS_CLIENT_STATUS_DEFAULT:
        {
            light_lightness_client_status_default_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_lightness_client_status_default_t));
            app_ts_log(APP_TAG,"client receive: default lightness = %d\r\n", pdata->lightness);
        }
        break;
    case LIGHT_LIGHTNESS_CLIENT_STATUS_RANGE:
        {
            light_lightness_client_status_range_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_lightness_client_status_range_t));
            app_ts_log(APP_TAG,"client receive: range_max = %d, range_min = %d, status = %d\r\n",    \
                    pdata->range_max, pdata->range_min,pdata->status);

        }
        break;
   default:
        break;
    }

    return 0;
}

uint32_t pts_light_lightness_client_reg(uint8_t element_index)
{
    bool ret = false;

    if(g_init == true)
    {
        return PTS_SUCCESS;
    }

    memset(&lln_client,0,sizeof(lln_client));
    lln_client.model_data_cb = generic_light_lightness_client_data;

    ret = light_lightness_client_reg(element_index, &lln_client);
    if(ret == false)
    {
        return PTS_ERROR;
    }
    g_init = true;
    return PTS_SUCCESS;
}

//model app intf
static void lln_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_lightness_get(pmodel_info,lln_test_param.tx_info.dst,lln_test_param.tx_info.app_key_idx);
}

static void lln_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_lightness_set(pmodel_info,lln_test_param.tx_info.dst,lln_test_param.tx_info.app_key_idx,lln_test_param.lightness, \
        lln_test_param.tx_info.tid,lln_test_param.optional,lln_test_param.trans_time,lln_test_param.delay,lln_test_param.ack);
}

static void lln_linear_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_lightness_linear_get(pmodel_info,lln_test_param.tx_info.dst,lln_test_param.tx_info.app_key_idx);
}

static void lln_linear_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_lightness_linear_set(pmodel_info,lln_test_param.tx_info.dst,lln_test_param.tx_info.app_key_idx,lln_test_param.linear, \
        lln_test_param.tx_info.tid,lln_test_param.optional,lln_test_param.trans_time,lln_test_param.delay,lln_test_param.ack);
}

static void gpl_last_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_lightness_last_get(pmodel_info,lln_test_param.tx_info.dst,lln_test_param.tx_info.app_key_idx);
}

static void gpl_default_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_lightness_default_get(pmodel_info,lln_test_param.tx_info.dst,lln_test_param.tx_info.app_key_idx);
}

static void gpl_default_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_lightness_default_set(pmodel_info,lln_test_param.tx_info.dst,lln_test_param.tx_info.app_key_idx,lln_test_param.d_lightness, \
        lln_test_param.ack);
}

static void gpl_range_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_lightness_range_get(pmodel_info,lln_test_param.tx_info.dst,lln_test_param.tx_info.app_key_idx);
}

static void gpl_range_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_lightness_range_set(pmodel_info,lln_test_param.tx_info.dst,lln_test_param.tx_info.app_key_idx,lln_test_param.range_min,   \
        lln_test_param.range_max,lln_test_param.ack);
}

#define GET_CMD 0
#define SET_CMD 1
#define GET_LINEAR_CMD 2
#define SET_LINEAR_CMD 3
#define GET_LAST_CMD 4
#define GET_DEFAULT_CMD 5
#define SET_DEFAULT_CMD 6
#define SET_RANGE_GET_CMD 7
#define SET_RANGE_SET_CMD 8




static app_client_cmd_cb pts_cmd_tab[] =
{
    [GET_CMD] = lln_get_cmd,
    [SET_CMD] = lln_set_cmd,
    [GET_LINEAR_CMD] = lln_linear_get_cmd,
    [SET_LINEAR_CMD] = lln_linear_set_cmd,
    [GET_LAST_CMD] = gpl_last_get_cmd,
    [GET_DEFAULT_CMD] = gpl_default_get_cmd,
    [SET_DEFAULT_CMD] = gpl_default_set_cmd,
    [SET_RANGE_GET_CMD] = gpl_range_get_cmd,
    [SET_RANGE_SET_CMD] = gpl_range_set_cmd,
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

                if(lln_test_param.ack == false)
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

static bool lln_bv_run(ts_mgs_p p_msg)
{
    uint32_t err_code;
    err_code = pts_light_lightness_client_reg(PTS_APP_REG_ELEMENT_0);
    if((p_msg == NULL)||(err_code != PTS_SUCCESS))
    {
        return false;
    }
    memset(p_msg,0,sizeof(ts_msg));
    p_msg->pargs = &lln_client;
    p_msg->msg_tx = true;
    p_msg->msg_rx = true;
    lln_test_param.tx_info.app_key_idx = 0;
    lln_test_param.tx_info.dst = 1;
    lln_test_param.tx_info.tid = 0;
    return true;
}

static void lln_bv_01_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_CMD;
    lln_test_param.pts_testcase = 1;
    lln_test_param.ack = true;
    ts_run(&msg);
}

static void lln_bv_02_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lln_test_param.pts_testcase = 2;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.lightness   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = true;
    lln_test_param.optional = false;
    ts_run(&msg);
}

static void lln_bv_03_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lln_test_param.pts_testcase = 3;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.lightness   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = true;
    lln_test_param.optional = true;
    lln_test_param.trans_time = pts_trans_time_ei;
    lln_test_param.delay = pts_delay_i;
    ts_run(&msg);
}
//pts_client lln bv-04-c -v 3e8 -t 0x54 0x05
static void lln_bv_04_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lln_test_param.pts_testcase = 4;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.lightness   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = true;
    lln_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lln_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lln_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lln_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lln_test_param.trans_time = pts_trans_time_c;
            lln_test_param.delay = pts_delay_c;
        }
    }

    ts_run(&msg);
}

static void lln_bv_05_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lln_test_param.pts_testcase = 5;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.lightness   = cmd_line_value[0];
        }
    }

    lln_test_param.ack = false;
    lln_test_param.optional = false;
    ts_run(&msg);
}
//pts_client lln bv-06-c -v 3e8 -t c0 0
//pts_client RAW 0x01 0x00 0x82 0x4D 0xe8 0x03  0x00 0xc0 0x00

static void lln_bv_06_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lln_test_param.pts_testcase = 6;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.lightness   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = false;
    lln_test_param.optional = true;
    lln_test_param.trans_time = pts_trans_time_ei;
    lln_test_param.delay = pts_delay_i;

    ts_run(&msg);
}
//pts_client RAW 0x01 0x00 0x82 0x4D 0xe8 0x03  0x00 0x54 0x05
static void lln_bv_07_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lln_test_param.pts_testcase = 7;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.lightness   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = false;
    lln_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lln_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lln_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lln_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lln_test_param.trans_time = pts_trans_time_c;
            lln_test_param.delay = pts_delay_c;
        }
    }
    ts_run(&msg);
}
//pts_client lln bv-08-c

static void lln_bv_08_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_LINEAR_CMD;
    lln_test_param.pts_testcase = 8;
    lln_test_param.ack = true;
    ts_run(&msg);
}
//pts_client lln bv-09-c -v 0x2328

static void lln_bv_09_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_LINEAR_CMD;
    lln_test_param.pts_testcase = 9;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.linear   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = true;
    lln_test_param.optional = false;
    ts_run(&msg);
}
//pts_client lln bv-10-c -v bb8 -t c0 00
static void lln_bv_10_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_LINEAR_CMD;
    lln_test_param.pts_testcase = 10;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.linear   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = true;
    lln_test_param.optional = true;
    lln_test_param.trans_time = pts_trans_time_ei;
    lln_test_param.delay = pts_delay_i;

    ts_run(&msg);
}
//pts_client lln bv-11-c -v 3e8 -t 0x54 0x05
static void lln_bv_11_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_LINEAR_CMD;
    lln_test_param.pts_testcase = 11;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.linear   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = true;
    lln_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lln_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lln_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lln_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lln_test_param.trans_time = pts_trans_time_c;
            lln_test_param.delay = pts_delay_c;
        }
    }

    ts_run(&msg);
}
//pts_client lln bv-12-c -v 2710
static void lln_bv_12_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_LINEAR_CMD;
    lln_test_param.pts_testcase = 12;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.linear   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = false;
    lln_test_param.optional = false;
    ts_run(&msg);
}
//pts_client lln bv-13-c -v 2710
static void lln_bv_13_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_LINEAR_CMD;
    lln_test_param.pts_testcase = 13;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.linear   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = false;
    lln_test_param.optional = true;
    lln_test_param.trans_time = pts_trans_time_ei;
    lln_test_param.delay = pts_delay_i;

    ts_run(&msg);
}
//pts_client lln bv-14-c -v 7530 -t 0x54 0x05
static void lln_bv_14_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_LINEAR_CMD;
    lln_test_param.pts_testcase = 14;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.linear   = cmd_line_value[0];
        }
    }
    lln_test_param.ack = false;
    lln_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lln_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lln_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lln_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lln_test_param.trans_time = pts_trans_time_c;
            lln_test_param.delay = pts_delay_c;
        }
    }

    ts_run(&msg);
}
//pts_client lln bv-15-c
static void lln_bv_15_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_LAST_CMD;
    lln_test_param.pts_testcase = 15;
    lln_test_param.ack = true;
    ts_run(&msg);
}
//pts_client lln bv-16-c
static void lln_bv_16_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_DEFAULT_CMD;
    lln_test_param.pts_testcase = 16;
    lln_test_param.ack = true;
    ts_run(&msg);
}
//pts_client lln bv-17-c -v 1388
static void lln_bv_17_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_DEFAULT_CMD;
    lln_test_param.pts_testcase = 17;
    lln_test_param.ack = true;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.d_lightness  = cmd_line_value[0];
        }
    }

    ts_run(&msg);
}
//pts_client lln bv-18-c -v 2710
static void lln_bv_18_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_DEFAULT_CMD;
    lln_test_param.pts_testcase = 18;
    lln_test_param.ack = false;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lln_test_param.d_lightness  = cmd_line_value[0];
        }
    }
    ts_run(&msg);
}
//pts_client lln bv-19-c
static void lln_bv_19_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_RANGE_GET_CMD;
    lln_test_param.pts_testcase = 19;
    lln_test_param.ack = true;
    ts_run(&msg);
}
 //pts_client lln bv-20-c -v F000 64
static void lln_bv_20_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_RANGE_SET_CMD;
    lln_test_param.pts_testcase = 20;
    lln_test_param.ack = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-v") == true)
        {
            lln_test_param.range_max  = cmd_line_value[1];
            lln_test_param.range_min  = cmd_line_value[0];
        }
    }
    ts_run(&msg);
}
  //pts_client lln bv-21-c -v  F000 0x64
static void lln_bv_21_c()
{
    ts_msg msg;
    if(lln_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_RANGE_SET_CMD;
    lln_test_param.pts_testcase = 21;
    lln_test_param.ack = false;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-v") == true)
        {
            lln_test_param.range_max  = cmd_line_value[1];
            lln_test_param.range_min  = cmd_line_value[0];
        }
    }

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
        {STR_TESTCASE_BV_NB(01), func_testcase(lln,01), STR_TESTCASE_NAME(LLN,01)},    //pts_client lln bv-01-c
        {STR_TESTCASE_BV_NB(02), func_testcase(lln,02), STR_TESTCASE_NAME(LLN,02)},    //pts_client lln bv-02-c
        {STR_TESTCASE_BV_NB(03), func_testcase(lln,03), STR_TESTCASE_NAME(LLN,03)},    //pts_client lln bv-03-c
        {STR_TESTCASE_BV_NB(04), func_testcase(lln,04), STR_TESTCASE_NAME(LLN,04)},
        {STR_TESTCASE_BV_NB(05), func_testcase(lln,05), STR_TESTCASE_NAME(LLN,05)},
        {STR_TESTCASE_BV_NB(06), func_testcase(lln,06), STR_TESTCASE_NAME(LLN,06)},
        {STR_TESTCASE_BV_NB(07), func_testcase(lln,07), STR_TESTCASE_NAME(LLN,07)},
        {STR_TESTCASE_BV_NB(08), func_testcase(lln,08), STR_TESTCASE_NAME(LLN,08)},
        {STR_TESTCASE_BV_NB(09), func_testcase(lln,09), STR_TESTCASE_NAME(LLN,09)},
        {STR_TESTCASE_BV_NB(10), func_testcase(lln,10), STR_TESTCASE_NAME(LLN,10)},
        {STR_TESTCASE_BV_NB(11), func_testcase(lln,11), STR_TESTCASE_NAME(LLN,11)},
        {STR_TESTCASE_BV_NB(12), func_testcase(lln,12), STR_TESTCASE_NAME(LLN,12)},
        {STR_TESTCASE_BV_NB(13), func_testcase(lln,13), STR_TESTCASE_NAME(LLN,13)},
        {STR_TESTCASE_BV_NB(14), func_testcase(lln,14), STR_TESTCASE_NAME(LLN,14)},
        {STR_TESTCASE_BV_NB(15), func_testcase(lln,15), STR_TESTCASE_NAME(LLN,15)},
        {STR_TESTCASE_BV_NB(16), func_testcase(lln,16), STR_TESTCASE_NAME(LLN,16)},
        {STR_TESTCASE_BV_NB(17), func_testcase(lln,17), STR_TESTCASE_NAME(LLN,17)},
        {STR_TESTCASE_BV_NB(18), func_testcase(lln,18), STR_TESTCASE_NAME(LLN,18)},
        {STR_TESTCASE_BV_NB(19), func_testcase(lln,19), STR_TESTCASE_NAME(LLN,19)},
        {STR_TESTCASE_BV_NB(20), func_testcase(lln,20), STR_TESTCASE_NAME(LLN,20)},
        {STR_TESTCASE_BV_NB(21), func_testcase(lln,21), STR_TESTCASE_NAME(LLN,21)}
};

//testcase 2 0x190
//testcase 3 pts_client lln bv-03-c 0x12C 0x00 0xC0 0x00
//testcase 4 pts_client lln bv-04-c 0x3E8 0x00 0x54 0x05

void run_lln_client_ts(int argc, char *args[])
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
