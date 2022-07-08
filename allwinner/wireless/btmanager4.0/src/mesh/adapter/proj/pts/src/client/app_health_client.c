#include "pts_client_app.h"

#if (ENABLE_LIGHT_HM_CLIENT_APP == 1)
static bool g_init = false;
static mesh_model_info_t hm_client;
static struct
{
    uint8_t pts_testcase;
    pts_client_tx_info tx_info;
    uint16_t company_id;
    uint8_t test_id;
    uint8_t fast_period_divisor;
    uint8_t attn;
    bool ack;
}hm_test_param;

#define APP_TAG hm

static int32_t health_client_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
    UNUSED(pmodel_info);
    switch (type)
    {
    case HEALTH_CLIENT_STATUS_CURRENT:
    case HEALTH_CLIENT_STATUS_REGISTERED:
        {
            health_client_status_t *pdata = pargs;
            ts_rx(pdata,sizeof(health_client_status_t));
            if(pdata->fault_array == NULL)
            {
                app_ts_log(APP_TAG,"client receive: test_id = %d, company_id = %d, len = %d\r\n",pdata->test_id, pdata->company_id,pdata->fault_array_len);
            }
            else
            {
                app_ts_log(APP_TAG,"client receive: test_id = %d, company_id = %d, len = %d,data[0]=%d\r\n",pdata->test_id, pdata->company_id,pdata->fault_array_len,  \
                    pdata->fault_array[0]);
            }
        }
        break;

    case HEALTH_CLIENT_STATUS_PERIOD:
        {
            health_client_status_period_t *pdata = pargs;
            ts_rx(pdata,sizeof(health_client_status_period_t));
            app_ts_log(APP_TAG,"client receive: fast_period_divisor = %d\r\n",pdata->fast_period_divisor);
        }
        break;

    case HEALTH_CLIENT_STATUS_ATTENTION:
        {
            health_client_status_attention_t *pdata = pargs;
            ts_rx(pdata,sizeof(health_client_status_attention_t));
            app_ts_log(APP_TAG,"client receive: attention = %d\r\n",pdata->attention);
        }
        break;
   default:
        break;
    }
    return 0;
}

uint32_t pts_health_client_reg(uint8_t element_index)
{
    bool ret = false;

    if(g_init == true)
    {
        return PTS_SUCCESS;
    }

    memset(&hm_client,0,sizeof(hm_client));
    hm_client.model_data_cb = health_client_data;

    ret = health_client_reg(element_index, &hm_client);
    if(ret == false)
    {
        return PTS_ERROR;
    }
    g_init = true;
    return PTS_SUCCESS;
}

//model app intf
static void hm_fault_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    health_fault_get(pmodel_info,hm_test_param.tx_info.dst,hm_test_param.tx_info.app_key_idx,hm_test_param.company_id);
}

static void hm_fault_clear_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    health_fault_clear(pmodel_info,hm_test_param.tx_info.dst,hm_test_param.tx_info.app_key_idx,hm_test_param.company_id,hm_test_param.ack);
}

static void hm_fault_test_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;

    health_fault_test(pmodel_info,hm_test_param.tx_info.dst,hm_test_param.tx_info.app_key_idx,hm_test_param.test_id,hm_test_param.company_id,hm_test_param.ack);
}

static void hm_period_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    health_period_get(pmodel_info,hm_test_param.tx_info.dst,hm_test_param.tx_info.app_key_idx);
}

static void hm_period_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    health_period_set(pmodel_info,hm_test_param.tx_info.dst,hm_test_param.tx_info.app_key_idx,hm_test_param.fast_period_divisor,hm_test_param.ack);
}

static void hm_attn_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    health_attn_get(pmodel_info,hm_test_param.tx_info.dst,hm_test_param.tx_info.app_key_idx);
}

static void hm_attn_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    health_attn_set(pmodel_info,hm_test_param.tx_info.dst,hm_test_param.tx_info.app_key_idx,hm_test_param.attn,hm_test_param.ack);
}

#define FAULT_GET_CMD 0
#define FAULT_CLEAR_CMD 1
#define FAULT_TEST_CMD 2
#define PERIOD_GET_CMD 3
#define PERIOD_SET_CMD 4
#define ATTN_GET_CMD 5
#define ATTN_SET_CMD 6

static app_client_cmd_cb pts_cmd_tab[] =
{
    [FAULT_GET_CMD] = hm_fault_get_cmd,
    [FAULT_CLEAR_CMD] = hm_fault_clear_cmd,
    [FAULT_TEST_CMD] = hm_fault_test_cmd,
    [PERIOD_GET_CMD] = hm_period_get_cmd,
    [PERIOD_SET_CMD] = hm_period_set_cmd,
    [ATTN_GET_CMD] = hm_attn_get_cmd,
    [ATTN_SET_CMD] = hm_attn_set_cmd,
};

#define TS_RFS 1
#define TS_HPF 2
#define TS_ATS 3

static void bv_rx_cb(ts_mgs_p p_msg,void *data, uint32_t len)
{
    app_ts_log(APP_TAG,"cur_state %d",p_msg->ts_state);
    switch(p_msg->ts_state)
    {
        case TS_STATE_0:
            if(hm_test_param.pts_testcase == TS_ATS)
            {
                app_ts_log(APP_TAG,"please input attn_#");
                p_msg->cmd_id = ATTN_SET_CMD;
                p_msg->ts_state = TS_STATE_1;
                hm_test_param.ack = true;
                scanf("%c",&hm_test_param.attn);
                app_ts_log(APP_TAG,"get user input attn_ %d\n",hm_test_param.attn);
            }
            else if(hm_test_param.pts_testcase == TS_HPF)
            {
                p_msg->cmd_id = PERIOD_SET_CMD;
                p_msg->ts_state = TS_STATE_1;
                hm_test_param.ack = true;
                app_ts_log(APP_TAG,"please input fast_period_divisor#");
                scanf("%c",&hm_test_param.fast_period_divisor);
                app_ts_log(APP_TAG,"get user input fast_period_divisor %d\n",hm_test_param.fast_period_divisor);
            }
            else if(TS_RFS == hm_test_param.pts_testcase)
            {
                p_msg->cmd_id = FAULT_CLEAR_CMD;
                 p_msg->ts_state = TS_STATE_1;
                 hm_test_param.ack = true;
            }
            break;

        case TS_STATE_1:
            if(hm_test_param.pts_testcase == TS_ATS)
            {
                app_ts_log(APP_TAG,"please input attn_#");
                p_msg->cmd_id = ATTN_SET_CMD;
                p_msg->ts_state = TS_STATE_2;
                hm_test_param.ack = false;
                scanf("%c",&hm_test_param.attn);
                app_ts_log(APP_TAG,"get user input attn_ %d\n",hm_test_param.attn);
            }
            else if(hm_test_param.pts_testcase == TS_HPF)
            {
                p_msg->cmd_id = PERIOD_SET_CMD;
                p_msg->ts_state = TS_STATE_2;
                hm_test_param.ack = false;
                app_ts_log(APP_TAG,"please input fast_period_divisor#");
                scanf("%c",&hm_test_param.fast_period_divisor);
                app_ts_log(APP_TAG,"get user input fast_period_divisor %d\n",hm_test_param.fast_period_divisor);
            }
            else if(TS_RFS == hm_test_param.pts_testcase)
            {
                p_msg->cmd_id = FAULT_CLEAR_CMD;
                 p_msg->ts_state = TS_STATE_2;
                 hm_test_param.ack = false;
            }

            break;
        case TS_STATE_2:
            if(TS_RFS == hm_test_param.pts_testcase)
            {
                p_msg->cmd_id = FAULT_TEST_CMD;
                p_msg->ts_state = TS_STATE_3;
                hm_test_param.ack = true;
                hm_test_param.test_id = 0x00;
                break;
            }
       case TS_STATE_3:
            if(TS_RFS == hm_test_param.pts_testcase)
            {
                p_msg->cmd_id = FAULT_TEST_CMD;
                p_msg->ts_state = TS_STATE_4;
                hm_test_param.ack = false;
                hm_test_param.test_id = 0x00;
                break;
            }
        default:
            p_msg->ts_state = TS_STATE_STOP;
            break;
    }
}

static void bv_tx_cb(ts_mgs_p p_msg)
{
//    mesh_model_info_p pmodel_info = p_msg->pargs;

    switch(p_msg->ts_state)
    {
        case TS_STATE_0:
        case TS_STATE_1:
        case TS_STATE_2:
            {
                pts_cmd_tab[p_msg->cmd_id](p_msg);

                if(hm_test_param.ack == false)
                {
                    p_msg->msg_rx = false;
                    if(TS_RFS != hm_test_param.pts_testcase)
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

static bool hm_bv_run(ts_mgs_p p_msg)
{
    uint32_t err_code;
    err_code = pts_health_client_reg(PTS_APP_REG_ELEMENT_0);
    if((p_msg == NULL)||(err_code != PTS_SUCCESS))
    {
        return false;
    }
    memset(p_msg,0,sizeof(ts_msg));
    p_msg->pargs = &hm_client;
    p_msg->msg_tx = true;
    p_msg->msg_rx = true;
    hm_test_param.tx_info.app_key_idx = 0;
    hm_test_param.tx_info.dst = 1;
    hm_test_param.tx_info.tid = 0;
    return true;
}
// pts_client RAW 0x01 0x00 0x80 0x04
//pts_client RAW 0x01 0x00 0x80 0x05 00
//pts_client RAW 0x01 0x00 0x80 0x06 00
static void hm_ats_bv_01_c()
{
    ts_msg msg;
    if(hm_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = ATTN_GET_CMD;
    hm_test_param.pts_testcase = TS_ATS;
    hm_test_param.ack = true;
    ts_run(&msg);
}
// pts_client RAW 0x01 0x00 0x80 0x34 0x00
// pts_client RAW 0x01 0x00 0x80 0x35 0x00
// pts_client RAW 0x01 0x00 0x80 0x36 0x00
static void hm_hpf_bv_01_c()
{
    ts_msg msg;
    if(hm_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = PERIOD_GET_CMD;
    hm_test_param.pts_testcase = TS_HPF;
    hm_test_param.ack = true;
    ts_run(&msg);
}
//pts_client RAW 0x01 0x00 0x80 0x31 0x3f 0x00
//pts_client RAW 0x01 0x00 0x80 0x32 0x00 0x3f 0x00
//pts_client RAW 0x01 0x00 0x80 0x33 0x00 0x3f 0x00
static void hm_rfs_bv_01_c()
{
    printf("%s",__FUNCTION__);
    ts_msg msg;
    if(hm_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = FAULT_GET_CMD;
    hm_test_param.pts_testcase = TS_RFS;
    hm_test_param.ack = true;
    {
        uint16_t cmd_line_value[1];
        if(ts_get_param(cmd_line_value,1,"-v") == true)
        {
            hm_test_param.company_id   = cmd_line_value[0];
        }
    }
    printf("%s,ts_run START",__FUNCTION__);
    ts_run(&msg);
    printf("%s,ts_run END",__FUNCTION__);
}

//cmd line
static struct
{
    char *cmd;
    void (*func)();
    char *doc;
}ts_command[] =
{
        {"rfs-bv-01-c", hm_rfs_bv_01_c, "MMDL/CL/HM/RFS/BV-01-C"},
        {"hps-bv-01-c", hm_hpf_bv_01_c, "MMDL/CL/HM/HPS/BV-01-C"},
        {"ats-bv-01-c", hm_ats_bv_01_c, "MMDL/CL/HM/RFS/BV-01-C"},//pts_client hm bv-01-c -v
        {NULL,          NULL,           "not found"}
};

void run_hm_client_ts(int argc, char *args[])
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
