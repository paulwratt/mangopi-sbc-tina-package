#include "pts_client_app.h"

#if(ENABLE_GENERIC_BATTERY_CLIENT_APP == 1)
static bool g_init = false;
static mesh_model_info_t gbat_client;
static struct
{
    pts_client_tx_info tx_info;
    uint8_t pts_testcase;
}gbat_test_param;

#define APP_TAG gbat

static int32_t generic_battery_client_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
    UNUSED(pmodel_info);
    switch (type)
    {
    case GENERIC_BATTERY_CLIENT_STATUS:
        {
            generic_battery_client_status_t *pdata = pargs;
            ts_rx(pdata,sizeof(generic_battery_client_status_t));
            app_ts_log(APP_TAG,"client receive: battery_level = %d\ttime_to_discharge=%d\ttime_to_charge=%d\tflags=0x%d/%d/%d/%d\r\n", pdata->battery_level,   \
                pdata->time_to_discharge,pdata->time_to_charge,pdata->flags.charging,pdata->flags.indicator,pdata->flags.presence,pdata->flags.serviceability);
        }
        break;
    default:
        break;
   }
    return 0;
}

uint32_t pts_generic_battery_reg(uint8_t element_index)
{
    bool ret = false;

    if(g_init == true)
    {
        return PTS_SUCCESS;
    }

    memset(&gbat_client,0,sizeof(gbat_client));
    gbat_client.model_data_cb = generic_battery_client_data;

    ret = generic_battery_client_reg(element_index, &gbat_client);
    if(ret == false)
    {
        return PTS_ERROR;
    }
    g_init = true;
    return PTS_SUCCESS;
}

static void bv_rx_cb(ts_mgs_p p_msg,void *data, uint32_t len)
{
    if(len != sizeof(generic_battery_client_status_t))
    {
        app_ts_log(APP_TAG,"%s\t%s\n",STR_CL_ACK_RX(GBAT),STR_PTS_RESULT_RX_INVAL);
        return ;
    }
    //no need to implement
    p_msg->ts_state = TS_STATE_STOP;
}

static void bv_tx_cb(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;

    switch(p_msg->ts_state)
    {
        case TS_STATE_0:
            {
                generic_battery_get(pmodel_info,gbat_test_param.tx_info.dst,gbat_test_param.tx_info.app_key_idx);
            }
            break;
        default:
            p_msg->ts_state = TS_STATE_STOP;
            break;
    }
}

static bool gbat_bv_run(ts_mgs_p p_msg)
{
    uint32_t err_code;
    err_code = pts_generic_battery_reg(PTS_APP_REG_ELEMENT_0);
    if((p_msg == NULL)||(err_code != PTS_SUCCESS))
    {
        return false;
    }
    memset(p_msg,0,sizeof(ts_msg));
    p_msg->pargs = &gbat_client;
    p_msg->msg_tx = true;
    p_msg->msg_rx = true;
    gbat_test_param.tx_info.app_key_idx = 0;
    gbat_test_param.tx_info.dst = 1;
    gbat_test_param.tx_info.tid = 0;
    return true;
}

static void gbat_bv_01_c()
{
    ts_msg msg;
    if(gbat_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    gbat_test_param.pts_testcase = 1;
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
        {STR_TESTCASE_BV_NB(01), func_testcase(gbat,01), STR_TESTCASE_NAME(GBAT,01)}    //pts_client gbat bv-01-c
};

void run_gbat_client_ts(int argc, char *args[])
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