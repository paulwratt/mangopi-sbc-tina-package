#include "pts_client_app.h"

#if (ENABLE_GENERIC_TRANSITION_TIME_CLIENT_APP == 1)
static bool g_init = false;
static mesh_model_info_t gdtt_client;
static struct
{
    pts_client_tx_info tx_info;
    uint8_t pts_testcase;
    bool ack;
    generic_transition_time_t trans_time;
}gdtt_test_param;

static const generic_transition_time_t gdtt_trans_time_c_02 = {15,3};
static const generic_transition_time_t gdtt_trans_time_c_03 = {15,2};

static int32_t generic_default_transition_time_client_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
    UNUSED(pmodel_info);
    switch (type)
    {
    case GENERIC_DEFAULT_TRANSITION_TIME_CLIENT_STATUS:
        {
            generic_default_transition_time_client_status_t *pdata = pargs;
            ts_rx(pdata,sizeof(generic_default_transition_time_client_status_t));
            gdtt_log("%s,trans_time.num_steps = %d, trans_time.step_resolution = %d\r\n",__FUNCTION__,pdata->trans_time.num_steps,pdata->trans_time.step_resolution);
        }
        break;
    default:
        break;
    }

    return 0;
}

uint32_t pts_generic_default_transition_time_reg(uint8_t element_index)
{
    bool ret = false;

    if(g_init == true)
    {
        return PTS_SUCCESS;
    }

    memset(&gdtt_client,0,sizeof(gdtt_client));
    gdtt_client.model_data_cb = generic_default_transition_time_client_data;

    ret = generic_default_transition_time_client_reg(element_index, &gdtt_client);
    if(ret == false)
    {
        return PTS_ERROR;
    }
    g_init = true;
    return PTS_SUCCESS;
}


//model app intf
static void dtt_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_default_transition_time_get(pmodel_info,gdtt_test_param.tx_info.dst,gdtt_test_param.tx_info.app_key_idx);
}

static void dtt_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_default_transition_time_set(pmodel_info,gdtt_test_param.tx_info.dst,gdtt_test_param.tx_info.app_key_idx,gdtt_test_param.trans_time,gdtt_test_param.ack);
}

static app_client_cmd_cb pts_cmd_tab[] =
{
    dtt_get_cmd,
    dtt_set_cmd
};

#define GET_CMD 0
#define SET_CMD 1

static void bv_rx_cb(ts_mgs_p p_msg,void *data, uint32_t len)
{

    //generic_default_transition_time_client_status_t *trans_time = data;
    if(len != sizeof(generic_default_transition_time_client_status_t))
    {
        gdtt_log("%s %s",STR_CL_ACK_RX(GDTT),STR_PTS_RESULT_RX_INVAL);
        return ;
    }
    //no need to implement
    p_msg->ts_state = TS_STATE_STOP;
}

static void bv_tx_cb(ts_mgs_p p_msg)
{
    //mesh_model_info_p pmodel_info = p_msg->pargs;

    switch(p_msg->ts_state)
    {
        case TS_STATE_0:
            {
                pts_cmd_tab[p_msg->cmd_id](p_msg);
            }
            break;
        default:
            p_msg->ts_state = TS_STATE_STOP;
            break;
    }
}

static bool gdtt_bv_run(ts_mgs_p p_msg)
{
    uint32_t err_code;
    err_code = pts_generic_default_transition_time_reg(PTS_APP_REG_ELEMENT_0);
    if((p_msg == NULL)||(err_code != PTS_SUCCESS))
    {
        return false;
    }
    memset(p_msg,0,sizeof(ts_msg));
    p_msg->pargs = &gdtt_client;
    p_msg->msg_tx = true;
    p_msg->msg_rx = true;
    gdtt_test_param.tx_info.app_key_idx = 0;
    gdtt_test_param.tx_info.dst = 1;
    gdtt_test_param.tx_info.tid = 0;
    return true;
}

static void gdtt_bv_01_c()
{
    ts_msg msg;
    if(gdtt_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_CMD;
    ts_run(&msg);
}

static void gdtt_bv_02_c()
{
    ts_msg msg;
    if(gdtt_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    gdtt_test_param.pts_testcase = 2;
    gdtt_test_param.ack = true;
    gdtt_test_param.trans_time = gdtt_trans_time_c_02;
    ts_run(&msg);
}

static void gdtt_bv_03_c()
{
    ts_msg msg;
    if(gdtt_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    gdtt_test_param.pts_testcase = 3;
    gdtt_test_param.ack = false;
    gdtt_test_param.trans_time = gdtt_trans_time_c_03;
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
        {STR_TESTCASE_BV_NB(01), func_testcase(gdtt,01), STR_TESTCASE_NAME(GDTT,01)},    //pts_client glv bv-01-c
        {STR_TESTCASE_BV_NB(02), func_testcase(gdtt,02), STR_TESTCASE_NAME(GDTT,02)},    //pts_client glv bv-02-c
        {STR_TESTCASE_BV_NB(03), func_testcase(gdtt,03), STR_TESTCASE_NAME(GDTT,03)},    //pts_client glv bv-03-c
};

void run_gdtt_client_ts(int argc, char *args[])
{
    uint8_t i;
#if 0
    goo_bv_01_c();
#else
	for (i = 0; ts_command[i].cmd; i++) {
		if (strcmp(args[1], ts_command[i].cmd))
        {
			continue;
        }
		if (ts_command[i].func) {
			ts_command[i].func(argc, args);
			break;
		}
	}
    if((ts_command[i].doc != NULL)&&(ts_command[i].cmd == NULL))
    {
        gdtt_log("%s %s %s\n",args[0],args[1],ts_command[i].doc);
    }
#endif
}

#endif
