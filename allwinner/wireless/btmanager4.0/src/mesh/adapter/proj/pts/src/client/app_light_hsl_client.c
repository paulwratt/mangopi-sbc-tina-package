#include "pts_client_app.h"
#if (ENABLE_LIGHT_HSL_CLIENT_APP == 1)
static bool g_init = false;
static mesh_model_info_t lhsl_client;
static struct
{
    uint8_t pts_testcase;
    uint16_t lightness;
    uint16_t hue;
    uint16_t saturation;
    uint16_t hue_range_min;
    uint16_t hue_range_max;
    uint16_t saturation_range_min;
    uint16_t saturation_range_max;
    pts_client_tx_info tx_info;
    bool optional;
    generic_transition_time_t trans_time;
    uint8_t delay;
    bool ack;
}lhsl_test_param;


#define APP_TAG lhsl

static int32_t generic_light_hsl_client_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
    UNUSED(pmodel_info);
    switch (type)
    {
    case LIGHT_HSL_CLIENT_STATUS:
    case LIGHT_HSL_CLIENT_STATUS_TARGET:
        {
            light_hsl_client_status_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_hsl_client_status_t));
            if (pdata->optional)
            {
                app_ts_log(APP_TAG,"client receive: lightness = %d, hue = %d, saturation = %d, remain time = step(%d), resolution(%d)\r\n",    \
                    pdata->lightness, pdata->hue,pdata->saturation,pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);
            }
            else
            {
                app_ts_log(APP_TAG,"client receive: lightness = %d, hue = %d, saturation = %d\r\n",pdata->lightness, pdata->hue,pdata->saturation);
            }
        }
        break;

    case LIGHT_HSL_CLIENT_STATUS_HUE:
        {
            light_hsl_client_status_hue_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_hsl_client_status_hue_t));
            if (pdata->optional)
            {
                app_ts_log(APP_TAG,"client receive: present_hue = %d, target_hue = %d, remain time = step(%d), resolution(%d)\r\n",    \
                    pdata->present_hue, pdata->target_hue,pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);
            }
            else
            {
                app_ts_log(APP_TAG,"client receive: present_hue = %d, target_hue = %d\r\n",pdata->present_hue, pdata->target_hue);
            }
        }
        break;
    case LIGHT_HSL_CLIENT_STATUS_SATURATION:
        {
            light_hsl_client_status_saturation_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_hsl_client_status_saturation_t));
            if (pdata->optional)
            {
                app_ts_log(APP_TAG,"client receive: present_saturation = %d, target_saturation = %d, remain time = step(%d), resolution(%d)\r\n",    \
                    pdata->present_saturation, pdata->target_saturation,pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);
            }
            else
            {
                app_ts_log(APP_TAG,"client receive: present_saturation = %d, target_saturation = %d\r\n",pdata->present_saturation, pdata->target_saturation);
            }
        }
        break;
     case LIGHT_HSL_CLIENT_STATUS_DEFAULT:
        {
            light_hsl_client_status_default_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_hsl_client_status_default_t));
            app_ts_log(APP_TAG,"client receive: lightness = %d, hue = %d, saturation =%d\r\n",pdata->lightness, pdata->hue,pdata->saturation);
        }
        break;
     case LIGHT_HSL_CLIENT_STATUS_RANGE:
        {
            light_hsl_client_status_range_t *pdata = pargs;
            ts_rx(pdata,sizeof(light_hsl_client_status_range_t));
            app_ts_log(APP_TAG,"client receive: status = %d, hue_range_min = %d, hue_range_max =%d, saturation_range_min = %d, saturation_range_max = %d\r\n",pdata->status, pdata->hue_range_min,pdata->hue_range_max \
                ,pdata->saturation_range_min,pdata->saturation_range_max);
        }
        break;
   default:
        break;
    }
    return 0;
}

uint32_t pts_light_hsl_client_reg(uint8_t element_index)
{
    bool ret = false;

    if(g_init == true)
    {
        return PTS_SUCCESS;
    }

    memset(&lhsl_client,0,sizeof(lhsl_client));
    lhsl_client.model_data_cb = generic_light_hsl_client_data;

    ret = light_hsl_client_reg(element_index, &lhsl_client);
    if(ret == false)
    {
        return PTS_ERROR;
    }
    g_init = true;
    return PTS_SUCCESS;
}

//model app intf
static void lhsl_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_get(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx);
}

static void lhsl_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_set(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx,lhsl_test_param.lightness, \
        lhsl_test_param.hue,lhsl_test_param.saturation,lhsl_test_param.tx_info.tid,lhsl_test_param.optional,lhsl_test_param.trans_time,lhsl_test_param.delay,lhsl_test_param.ack);
}

static void lhsl_target_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_target_get(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx);
}

static void lhsl_hue_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_hue_get(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx);
}

static void lhsl_hue_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_hue_set(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx,lhsl_test_param.hue, \
        lhsl_test_param.tx_info.tid,lhsl_test_param.optional,lhsl_test_param.trans_time,lhsl_test_param.delay,lhsl_test_param.ack);
}

static void lhsl_saturation_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_saturation_get(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx);
}

static void lhsl_saturation_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_saturation_set(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx,lhsl_test_param.saturation, \
        lhsl_test_param.tx_info.tid,lhsl_test_param.optional,lhsl_test_param.trans_time,lhsl_test_param.delay,lhsl_test_param.ack);
}

static void lhsl_default_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_default_get(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx);
}

static void lhsl_default_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_default_set(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx,lhsl_test_param.lightness, \
        lhsl_test_param.hue,lhsl_test_param.saturation,lhsl_test_param.ack);
}

static void lhsl_range_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_range_get(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx);
}

static void lhsl_range_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    light_hsl_range_set(pmodel_info,lhsl_test_param.tx_info.dst,lhsl_test_param.tx_info.app_key_idx,lhsl_test_param.hue_range_min, \
        lhsl_test_param.hue_range_max,lhsl_test_param.saturation_range_min,lhsl_test_param.saturation_range_max,lhsl_test_param.ack);
}

#define GET_CMD 0
#define SET_CMD 1
#define GET_TARGET_CMD 2
#define GET_HUE_CMD 3
#define SET_HUE_CMD 4
#define GET_SATURATION_CMD 5
#define SET_SATURATION_CMD 6
#define GET_DEFAULT_CMD 7
#define SET_DEFAULT_CMD 8
#define GET_RANGE_CMD 9
#define SET_RANGE_CMD 10

static app_client_cmd_cb pts_cmd_tab[] =
{
    [GET_CMD] = lhsl_get_cmd,
    [SET_CMD] = lhsl_set_cmd,
    [GET_TARGET_CMD] = lhsl_target_get_cmd,
    [GET_HUE_CMD] = lhsl_hue_get_cmd,
    [SET_HUE_CMD] = lhsl_hue_set_cmd,
    [GET_SATURATION_CMD] = lhsl_saturation_get_cmd,
    [SET_SATURATION_CMD] = lhsl_saturation_set_cmd,
    [GET_DEFAULT_CMD] = lhsl_default_get_cmd,
    [SET_DEFAULT_CMD] = lhsl_default_set_cmd,
    [GET_RANGE_CMD] = lhsl_range_get_cmd,
    [SET_RANGE_CMD] = lhsl_range_set_cmd,
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

                if(lhsl_test_param.ack == false)
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

static bool lhsl_bv_run(ts_mgs_p p_msg)
{
    uint32_t err_code;
    err_code = pts_light_hsl_client_reg(PTS_APP_REG_ELEMENT_0);
    if((p_msg == NULL)||(err_code != PTS_SUCCESS))
    {
        return false;
    }
    memset(p_msg,0,sizeof(ts_msg));
    p_msg->pargs = &lhsl_client;
    p_msg->msg_tx = true;
    p_msg->msg_rx = true;
    lhsl_test_param.tx_info.app_key_idx = 0;
    lhsl_test_param.tx_info.dst = 1;
    lhsl_test_param.tx_info.tid = 0;
    return true;
}

static void lhsl_bv_01_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_CMD;
    lhsl_test_param.pts_testcase = 1;
    lhsl_test_param.ack = true;
    ts_run(&msg);
}

static void lhsl_bv_02_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lhsl_test_param.pts_testcase = 2;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lhsl_test_param.lightness   = cmd_line_value[0];//0xD80
            lhsl_test_param.hue         = cmd_line_value[1];//0x1ED2
            lhsl_test_param.saturation  = cmd_line_value[2];//0x3039
        }
    }
    lhsl_test_param.ack = true;
    lhsl_test_param.optional = false;
    ts_run(&msg);
}

//pts_client lhsl bv-02-c -v 0x4D2 0x162E 0x2334
static void lhsl_bv_03_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lhsl_test_param.pts_testcase = 3;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lhsl_test_param.lightness   = cmd_line_value[0];
            lhsl_test_param.hue         = cmd_line_value[1];
            lhsl_test_param.saturation  = cmd_line_value[2];
        }
    }
    lhsl_test_param.ack = true;
    lhsl_test_param.optional = true;
    lhsl_test_param.trans_time = pts_trans_time_ei;
    lhsl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}

//pts_client lhsl bv-04-c -v 0x4D2 0x162E 0x2334  -t 0x54 0x05
static void lhsl_bv_04_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lhsl_test_param.pts_testcase = 4;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lhsl_test_param.lightness   = cmd_line_value[0];
            lhsl_test_param.hue         = cmd_line_value[1];
            lhsl_test_param.saturation  = cmd_line_value[2];
        }
    }
    lhsl_test_param.ack = true;
    lhsl_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            app_ts_log(APP_TAG,"get param true\n");
            lhsl_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lhsl_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lhsl_test_param.delay = cmd_line_value[1];

        }
        else
        {
            app_ts_log(APP_TAG,"get param false\n");
            lhsl_test_param.trans_time = pts_trans_time_c;
            lhsl_test_param.delay = pts_delay_c;
        }
    }
    ts_run(&msg);
}
//pts_client lhsl bv-05-c -v 0xD80 0x1ED2  0x3039
static void lhsl_bv_05_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lhsl_test_param.pts_testcase = 5;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lhsl_test_param.lightness   = cmd_line_value[0];
            lhsl_test_param.hue         = cmd_line_value[1];
            lhsl_test_param.saturation  = cmd_line_value[2];
        }

    }
    lhsl_test_param.ack = false;
    lhsl_test_param.optional = false;
    ts_run(&msg);
}
//pts_client lhsl bv-06-c -v 0x4D2 0x162E 0x2334
static void lhsl_bv_06_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lhsl_test_param.pts_testcase = 6;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lhsl_test_param.lightness   = cmd_line_value[0];
            lhsl_test_param.hue         = cmd_line_value[1];
            lhsl_test_param.saturation  = cmd_line_value[2];
        }

    }
    lhsl_test_param.ack = false;
    lhsl_test_param.optional = true;
    lhsl_test_param.trans_time = pts_trans_time_ei;
    lhsl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}
//pts_client lhsl bv-07-c -v 0x4D2 0x162E 0x2334 -t 0x54 0x05
static void lhsl_bv_07_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    lhsl_test_param.pts_testcase = 7;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lhsl_test_param.lightness   = cmd_line_value[0];
            lhsl_test_param.hue         = cmd_line_value[1];
            lhsl_test_param.saturation  = cmd_line_value[2];
        }

    }
    lhsl_test_param.ack = false;
    lhsl_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lhsl_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lhsl_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lhsl_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lhsl_test_param.trans_time = pts_trans_time_c;
            lhsl_test_param.delay = pts_delay_c;
        }
    }
    ts_run(&msg);
}
//pts_client lhsl bv-08-c
static void lhsl_bv_08_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_TARGET_CMD;
    lhsl_test_param.pts_testcase = 8;
    lhsl_test_param.ack = true;
    ts_run(&msg);
}
//pts_client lhsl bv-09-c

static void lhsl_bv_09_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_DEFAULT_CMD;
    lhsl_test_param.pts_testcase = 9;
    lhsl_test_param.ack = true;
    ts_run(&msg);
}
//pts_client lhsl bv-10-c  -v 0x64 0x7B 0x3039
static void lhsl_bv_10_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_DEFAULT_CMD;
    lhsl_test_param.pts_testcase = 10;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lhsl_test_param.lightness   = cmd_line_value[0];
            lhsl_test_param.hue         = cmd_line_value[1];
            lhsl_test_param.saturation  = cmd_line_value[2];
        }
    }
    lhsl_test_param.ack = true;
    lhsl_test_param.optional = false;
    ts_run(&msg);
}
//pts_client lhsl bv-11-c  -v 0x2710 0xD05 0x4D2
static void lhsl_bv_11_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_DEFAULT_CMD;
    lhsl_test_param.pts_testcase = 11;
    {
        uint16_t cmd_line_value[3];
        if(ts_get_param(cmd_line_value,3,"-v") == true)
        {
            lhsl_test_param.lightness   = cmd_line_value[0];
            lhsl_test_param.hue         = cmd_line_value[1];
            lhsl_test_param.saturation  = cmd_line_value[2];
        }
    }
    lhsl_test_param.ack = false;
    lhsl_test_param.optional = false;
    ts_run(&msg);
}


static void lhsl_bv_12_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_RANGE_CMD;
    lhsl_test_param.pts_testcase = 12;
    lhsl_test_param.ack = true;
    ts_run(&msg);
}

static void lhsl_bv_13_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_RANGE_CMD;
    lhsl_test_param.pts_testcase = 13;
    {
        uint16_t cmd_line_value[4];
        if(ts_get_param(cmd_line_value,4,"-v") == true)
        {
            lhsl_test_param.hue_range_min           = cmd_line_value[0];
            lhsl_test_param.hue_range_max           = cmd_line_value[1];
            lhsl_test_param.saturation_range_min    = cmd_line_value[2];
            lhsl_test_param.saturation_range_max    = cmd_line_value[3];
        }
    }
    lhsl_test_param.ack = true;
    lhsl_test_param.optional = false;
    ts_run(&msg);
}

static void lhsl_bv_14_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_RANGE_CMD;
    lhsl_test_param.pts_testcase = 14;
    {
        uint16_t cmd_line_value[4];
        if(ts_get_param(cmd_line_value,4,"-v") == true)
        {
            lhsl_test_param.hue_range_min           = cmd_line_value[0];
            lhsl_test_param.hue_range_max           = cmd_line_value[1];
            lhsl_test_param.saturation_range_min    = cmd_line_value[2];
            lhsl_test_param.saturation_range_max    = cmd_line_value[3];
        }
    }
    lhsl_test_param.ack = false;
    lhsl_test_param.optional = false;
    ts_run(&msg);
}

static void lhsl_bv_15_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_HUE_CMD;
    lhsl_test_param.pts_testcase = 15;
    lhsl_test_param.ack = true;
    ts_run(&msg);
}

static void lhsl_bv_16_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_HUE_CMD;
    lhsl_test_param.pts_testcase = 16;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.hue = cmd_line_value[0];
        }
    }
    lhsl_test_param.ack = true;
    lhsl_test_param.optional = false;
    ts_run(&msg);
}

static void lhsl_bv_17_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_HUE_CMD;
    lhsl_test_param.pts_testcase = 17;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.hue = cmd_line_value[0];
        }
    }

    lhsl_test_param.ack = true;
    lhsl_test_param.optional = true;
    lhsl_test_param.trans_time = pts_trans_time_ei;
    lhsl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}

static void lhsl_bv_18_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_HUE_CMD;
    lhsl_test_param.pts_testcase = 18;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.hue = cmd_line_value[0];
        }
    }

    lhsl_test_param.ack = true;
    lhsl_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lhsl_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lhsl_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lhsl_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lhsl_test_param.trans_time = pts_trans_time_c;
            lhsl_test_param.delay = pts_delay_c;
        }
    }
    ts_run(&msg);
}

static void lhsl_bv_19_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_HUE_CMD;
    lhsl_test_param.pts_testcase = 19;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.hue = cmd_line_value[0];
        }
    }

    lhsl_test_param.ack = false;
    lhsl_test_param.optional = false;
    ts_run(&msg);
}

static void lhsl_bv_20_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_HUE_CMD;
    lhsl_test_param.pts_testcase = 20;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.hue = cmd_line_value[0];
        }
    }

    lhsl_test_param.ack = false;
    lhsl_test_param.optional = true;
    lhsl_test_param.trans_time = pts_trans_time_ei;
    lhsl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}

static void lhsl_bv_21_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_HUE_CMD;
    lhsl_test_param.pts_testcase = 21;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.hue = cmd_line_value[0];
        }
    }
    lhsl_test_param.ack = false;
    lhsl_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lhsl_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lhsl_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lhsl_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lhsl_test_param.trans_time = pts_trans_time_c;
            lhsl_test_param.delay = pts_delay_c;
        }
    }
    ts_run(&msg);
}

static void lhsl_bv_22_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_SATURATION_CMD;
    lhsl_test_param.pts_testcase = 22;
    lhsl_test_param.ack = true;
    ts_run(&msg);
}

static void lhsl_bv_23_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_SATURATION_CMD;
    lhsl_test_param.pts_testcase = 23;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.saturation = cmd_line_value[0];
        }
    }
    lhsl_test_param.ack = true;
    lhsl_test_param.optional = false;
    ts_run(&msg);
}

static void lhsl_bv_24_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_SATURATION_CMD;
    lhsl_test_param.pts_testcase = 24;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.saturation = cmd_line_value[0];
        }
    }

    lhsl_test_param.ack = true;
    lhsl_test_param.optional = true;
    lhsl_test_param.trans_time = pts_trans_time_ei;
    lhsl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}

static void lhsl_bv_25_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_SATURATION_CMD;
    lhsl_test_param.pts_testcase = 25;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.saturation = cmd_line_value[0];
        }
    }

    lhsl_test_param.ack = true;
    lhsl_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lhsl_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lhsl_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lhsl_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lhsl_test_param.trans_time = pts_trans_time_c;
            lhsl_test_param.delay = pts_delay_c;
        }
    }
    ts_run(&msg);
}

static void lhsl_bv_26_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_SATURATION_CMD;
    lhsl_test_param.pts_testcase = 26;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.saturation = cmd_line_value[0];
        }
    }

    lhsl_test_param.ack = false;
    lhsl_test_param.optional = false;
    ts_run(&msg);
}

static void lhsl_bv_27_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_SATURATION_CMD;
    lhsl_test_param.pts_testcase = 27;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.saturation = cmd_line_value[0];
        }
    }

    lhsl_test_param.ack = false;
    lhsl_test_param.optional = true;
    lhsl_test_param.trans_time = pts_trans_time_ei;
    lhsl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}

static void lhsl_bv_28_c()
{
    ts_msg msg;
    if(lhsl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_SATURATION_CMD;
    lhsl_test_param.pts_testcase = 28;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            lhsl_test_param.saturation = cmd_line_value[0];
        }
    }
    lhsl_test_param.ack = false;
    lhsl_test_param.optional = true;
    {
        uint16_t cmd_line_value[2];
        if(ts_get_param(cmd_line_value,2,"-t") == true)
        {
            lhsl_test_param.trans_time.num_steps = (uint8_t)(cmd_line_value[0]&0x3F);
            lhsl_test_param.trans_time.step_resolution = ((uint8_t)(cmd_line_value[0]&0xC0))>>6;
            lhsl_test_param.delay = cmd_line_value[1];

        }
        else
        {
            lhsl_test_param.trans_time = pts_trans_time_c;
            lhsl_test_param.delay = pts_delay_c;
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
        {STR_TESTCASE_BV_NB(01), func_testcase(lhsl,01), STR_TESTCASE_NAME(LCTL,01)},    //pts_client lctl bv-01-c
        {STR_TESTCASE_BV_NB(02), func_testcase(lhsl,02), STR_TESTCASE_NAME(LCTL,02)},    //pts_client lctl bv-02-c
        {STR_TESTCASE_BV_NB(03), func_testcase(lhsl,03), STR_TESTCASE_NAME(LCTL,03)},    //pts_client lctl bv-03-c
        {STR_TESTCASE_BV_NB(04), func_testcase(lhsl,04), STR_TESTCASE_NAME(LCTL,04)},
        {STR_TESTCASE_BV_NB(05), func_testcase(lhsl,05), STR_TESTCASE_NAME(LCTL,05)},
        {STR_TESTCASE_BV_NB(06), func_testcase(lhsl,06), STR_TESTCASE_NAME(LCTL,06)},
        {STR_TESTCASE_BV_NB(07), func_testcase(lhsl,07), STR_TESTCASE_NAME(LCTL,07)},
        {STR_TESTCASE_BV_NB(08), func_testcase(lhsl,08), STR_TESTCASE_NAME(LCTL,08)},
        {STR_TESTCASE_BV_NB(09), func_testcase(lhsl,09), STR_TESTCASE_NAME(LCTL,09)},
        {STR_TESTCASE_BV_NB(10), func_testcase(lhsl,10), STR_TESTCASE_NAME(LCTL,10)},
        {STR_TESTCASE_BV_NB(11), func_testcase(lhsl,11), STR_TESTCASE_NAME(LCTL,11)},
        {STR_TESTCASE_BV_NB(12), func_testcase(lhsl,12), STR_TESTCASE_NAME(LCTL,12)},
        {STR_TESTCASE_BV_NB(13), func_testcase(lhsl,13), STR_TESTCASE_NAME(LCTL,13)},
        {STR_TESTCASE_BV_NB(14), func_testcase(lhsl,14), STR_TESTCASE_NAME(LCTL,14)},
        {STR_TESTCASE_BV_NB(15), func_testcase(lhsl,15), STR_TESTCASE_NAME(LCTL,15)},
        {STR_TESTCASE_BV_NB(16), func_testcase(lhsl,16), STR_TESTCASE_NAME(LCTL,16)},
        {STR_TESTCASE_BV_NB(17), func_testcase(lhsl,17), STR_TESTCASE_NAME(LCTL,17)},
        {STR_TESTCASE_BV_NB(18), func_testcase(lhsl,18), STR_TESTCASE_NAME(LCTL,18)},
        {STR_TESTCASE_BV_NB(19), func_testcase(lhsl,19), STR_TESTCASE_NAME(LCTL,19)},
        {STR_TESTCASE_BV_NB(20), func_testcase(lhsl,20), STR_TESTCASE_NAME(LCTL,20)},
        {STR_TESTCASE_BV_NB(21), func_testcase(lhsl,21), STR_TESTCASE_NAME(LCTL,21)},
        {STR_TESTCASE_BV_NB(22), func_testcase(lhsl,22), STR_TESTCASE_NAME(LCTL,22)},
        {STR_TESTCASE_BV_NB(23), func_testcase(lhsl,23), STR_TESTCASE_NAME(LCTL,23)},
        {STR_TESTCASE_BV_NB(24), func_testcase(lhsl,24), STR_TESTCASE_NAME(LCTL,24)},
        {STR_TESTCASE_BV_NB(25), func_testcase(lhsl,25), STR_TESTCASE_NAME(LCTL,25)},
        {STR_TESTCASE_BV_NB(26), func_testcase(lhsl,26), STR_TESTCASE_NAME(LCTL,26)},
        {STR_TESTCASE_BV_NB(27), func_testcase(lhsl,27), STR_TESTCASE_NAME(LCTL,27)},
        {STR_TESTCASE_BV_NB(28), func_testcase(lhsl,28), STR_TESTCASE_NAME(LCTL,28)}
};

void run_lhsl_client_ts(int argc, char *args[])
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
