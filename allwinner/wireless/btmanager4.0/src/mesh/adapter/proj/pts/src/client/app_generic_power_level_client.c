#include "pts_client_app.h"

#if (ENABLE_GENERIC_POWER_LEVEL_CLIENT_APP == 1)
static bool g_init = false;
static mesh_model_info_t gpl_client;
#define GPL_POWER_LEVEL 0x01FF
#define GPL_D_POWER_LEVEL 0x4567
#define GPL_D_POWER_LEVEL_1 0x3467
#define GPL_RANGE_MIN_13 0x0010
#define GPL_RANGE_MAX_13 0x1000
#define GPL_RANGE_MIN_14 0x00F0
#define GPL_RANGE_MAX_14 0xF000

static const generic_transition_time_t gpl_trans_time_i = {0x00,0x03};


static struct
{
    pts_client_tx_info tx_info;
    uint8_t pts_testcase;
    uint16_t power;
    uint16_t d_power;
    uint16_t range_min;
    uint16_t range_max;
    bool optional;
    generic_transition_time_t trans_time;
    uint8_t delay;
    bool ack;
}gpl_test_param;

#define APP_TAG gpl

static int32_t generic_power_level_client_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
    UNUSED(pmodel_info);
    switch (type)
    {
    case GENERIC_POWER_LEVEL_CLIENT_STATUS:
        {
            generic_power_level_client_status_t *pdata = pargs;
            ts_rx(pdata,sizeof(generic_power_level_client_status_t));
            if (pdata->optional)
            {
                app_ts_log(APP_TAG,"client receive: present = %d, target = %d, remain time = step(%d), resolution(%d)\r\n",    \
                    pdata->present_power, pdata->target_power,pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);
            }
            else
            {
                app_ts_log(APP_TAG,"client receive: present = %d\r\n", pdata->present_power);
            }
        }
        break;
    case GENERIC_POWER_LEVEL_CLIENT_STATUS_LAST:
        {
            generic_power_level_client_status_simple_t *pdata = pargs;
            ts_rx(pdata,sizeof(generic_power_level_client_status_simple_t));
            app_ts_log(APP_TAG,"client receive: last = %d\r\n", pdata->power);
        }
        break;

    case GENERIC_POWER_LEVEL_CLIENT_STATUS_DEFAULT:
        {
            generic_power_level_client_status_simple_t *pdata = pargs;
            ts_rx(pdata,sizeof(generic_power_level_client_status_simple_t));
            app_ts_log(APP_TAG,"client receive: default = %d\r\n", pdata->power);
        }
        break;
    case GENERIC_POWER_LEVEL_CLIENT_STATUS_RANGE:
        {
            generic_power_level_client_status_range_t *pdata = pargs;
            ts_rx(pdata,sizeof(generic_power_level_client_status_range_t));
            app_ts_log(APP_TAG,"client receive: range_max = %d, range_min = %d, stat = %d\r\n",    \
                    pdata->range_max, pdata->range_min,pdata->stat);

        }
        break;
   default:
        break;
    }

    return 0;
}

uint32_t pts_generic_power_level_reg(uint8_t element_index)
{
    bool ret = false;

    if(g_init == true)
    {
        return PTS_SUCCESS;
    }

    memset(&gpl_client,0,sizeof(gpl_client));
    gpl_client.model_data_cb = generic_power_level_client_data;

    ret = generic_power_level_client_reg(element_index, &gpl_client);
    if(ret == false)
    {
        return PTS_ERROR;
    }
    g_init = true;
    return PTS_SUCCESS;
}


//model app intf
static void gpl_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_power_level_get(pmodel_info,gpl_test_param.tx_info.dst,gpl_test_param.tx_info.app_key_idx);
}

static void gpl_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_power_level_set(pmodel_info,gpl_test_param.tx_info.dst,gpl_test_param.tx_info.app_key_idx,gpl_test_param.power,gpl_test_param.tx_info.tid,  \
        gpl_test_param.optional,gpl_test_param.trans_time,gpl_test_param.delay,gpl_test_param.ack);
}

static void gpl_last_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_power_last_get(pmodel_info,gpl_test_param.tx_info.dst,gpl_test_param.tx_info.app_key_idx);
}


static void gpl_default_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_power_default_get(pmodel_info,gpl_test_param.tx_info.dst,gpl_test_param.tx_info.app_key_idx);
}

static void gpl_default_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_power_default_set(pmodel_info,gpl_test_param.tx_info.dst,gpl_test_param.tx_info.app_key_idx,gpl_test_param.d_power,gpl_test_param.ack);
}

static void gpl_range_get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_power_range_get(pmodel_info,gpl_test_param.tx_info.dst,gpl_test_param.tx_info.app_key_idx);
}

static void gpl_range_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_power_range_set(pmodel_info,gpl_test_param.tx_info.dst,gpl_test_param.tx_info.app_key_idx,gpl_test_param.range_min,gpl_test_param.range_max,gpl_test_param.ack);
}
#define GET_CMD 0
#define SET_CMD 1
#define GET_LAST_CMD 2
#define GET_DEFAULT_CMD 3
#define SET_DEFAULT_CMD 4
#define GET_RANGE_CMD 5
#define SET_RANGE_CMD 6

static app_client_cmd_cb pts_cmd_tab[] =
{
    [GET_CMD] = gpl_get_cmd,
    [SET_CMD] = gpl_set_cmd,
    [GET_LAST_CMD] = gpl_last_get_cmd,
    [GET_DEFAULT_CMD] = gpl_default_get_cmd,
    [SET_DEFAULT_CMD] = gpl_default_set_cmd,
    [GET_RANGE_CMD] = gpl_range_get_cmd,
    [SET_RANGE_CMD] = gpl_range_set_cmd,
};

static void bv_rx_cb(ts_mgs_p p_msg,void *data, uint32_t len)
{
    switch(gpl_test_param.pts_testcase)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        {
            //generic_power_level_client_status_t *power_level = data;
            if(len != sizeof(generic_power_on_off_client_status_t))
            {
                app_ts_log(APP_TAG,"%s\t%s\n",STR_CL_ACK_RX(GPL),STR_PTS_RESULT_RX_INVAL);
                return ;
            }
        }
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        {
            //generic_power_level_client_status_simple_t *power = data;
            if(len != sizeof(generic_power_level_client_status_simple_t))
            {
                app_ts_log(APP_TAG,"%s\t%s\n",STR_CL_ACK_RX(GPL),STR_PTS_RESULT_RX_INVAL);
                return ;
            }
        }
            break;
        case 12:
        case 13:
        case 14:
        {
            //generic_power_level_client_status_range_t *power_range = data;
            if(len != sizeof(generic_power_level_client_status_range_t))
            {
                app_ts_log(APP_TAG,"%s\t%s\n",STR_CL_ACK_RX(GPL),STR_PTS_RESULT_RX_INVAL);
                return ;
            }

        }
            break;
       default:
            break;
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

static bool gpl_bv_run(ts_mgs_p p_msg)
{
    uint32_t err_code;
    err_code = pts_generic_power_level_reg(PTS_APP_REG_ELEMENT_0);
    if((p_msg == NULL)||(err_code != PTS_SUCCESS))
    {
        return false;
    }
    memset(p_msg,0,sizeof(ts_msg));
    p_msg->pargs = &gpl_client;
    p_msg->msg_tx = true;
    p_msg->msg_rx = true;
    gpl_test_param.tx_info.app_key_idx = 0;
    gpl_test_param.tx_info.dst = 1;
    gpl_test_param.tx_info.tid = 0;
    return true;
}

static void gpl_bv_01_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_CMD;
    gpl_test_param.pts_testcase = 2;
    ts_run(&msg);
}

static void gpl_bv_02_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    gpl_test_param.pts_testcase = 2;
    gpl_test_param.ack = true;
    gpl_test_param.power = GPL_POWER_LEVEL;
    gpl_test_param.optional = false;
    ts_run(&msg);
}

static void gpl_bv_03_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    gpl_test_param.pts_testcase = 3;
    gpl_test_param.ack = true;
    gpl_test_param.power = GPL_POWER_LEVEL;
    gpl_test_param.optional = true;
    gpl_test_param.trans_time = gpl_trans_time_i;
    gpl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}

static void gpl_bv_04_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    gpl_test_param.pts_testcase = 4;
    gpl_test_param.ack = true;
    gpl_test_param.power = GPL_POWER_LEVEL;
    gpl_test_param.optional = true;
    gpl_test_param.trans_time = pts_trans_time_c;
    gpl_test_param.delay = pts_delay_c;

    ts_run(&msg);
}

static void gpl_bv_05_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    gpl_test_param.pts_testcase = 5;
    gpl_test_param.ack = false;
    gpl_test_param.power = GPL_POWER_LEVEL;
    gpl_test_param.optional = false;
    ts_run(&msg);
}

static void gpl_bv_06_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    gpl_test_param.pts_testcase = 6;
    gpl_test_param.ack = false;
    gpl_test_param.power = GPL_POWER_LEVEL;
    gpl_test_param.optional = true;
    gpl_test_param.trans_time = gpl_trans_time_i;
    gpl_test_param.delay = pts_delay_i;
    ts_run(&msg);
}

static void gpl_bv_07_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    gpl_test_param.pts_testcase = 7;
    gpl_test_param.ack = false;
    gpl_test_param.power = GPL_POWER_LEVEL;
    gpl_test_param.optional = true;
    gpl_test_param.trans_time = pts_trans_time_c;
    gpl_test_param.delay = pts_delay_c;

    ts_run(&msg);
}

static void gpl_bv_08_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_LAST_CMD;
    gpl_test_param.pts_testcase = 8;
    ts_run(&msg);
}

static void gpl_bv_09_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_DEFAULT_CMD;
    gpl_test_param.pts_testcase = 9;
    ts_run(&msg);
}

static void gpl_bv_10_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_DEFAULT_CMD;
    gpl_test_param.pts_testcase = 10;
    gpl_test_param.ack = true;
    gpl_test_param.d_power = GPL_D_POWER_LEVEL;
    ts_run(&msg);
}

static void gpl_bv_11_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_DEFAULT_CMD;
    gpl_test_param.pts_testcase = 11;
    gpl_test_param.ack = false;
    gpl_test_param.d_power = GPL_D_POWER_LEVEL_1;

    ts_run(&msg);
}

static void gpl_bv_12_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_RANGE_CMD;
    gpl_test_param.pts_testcase = 12;
    ts_run(&msg);
}
//pts_client RAW 0x01 0x00 0x82 0x21 0x10 0x00 0x00 0x10
static void gpl_bv_13_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_RANGE_CMD;
    gpl_test_param.pts_testcase = 13;
    gpl_test_param.ack = true;
    gpl_test_param.range_max = GPL_RANGE_MAX_13;
    gpl_test_param.range_min = GPL_RANGE_MIN_13;

    ts_run(&msg);
}

static void gpl_bv_14_c()
{
    ts_msg msg;
    if(gpl_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_RANGE_CMD;
    gpl_test_param.pts_testcase = 14;
    gpl_test_param.ack = false;
    gpl_test_param.range_max = GPL_RANGE_MAX_14;
    gpl_test_param.range_min = GPL_RANGE_MIN_14;
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
        {STR_TESTCASE_BV_NB(01), func_testcase(gpl,01), STR_TESTCASE_NAME(GPL,01)},    //pts_client glv bv-01-c
        {STR_TESTCASE_BV_NB(02), func_testcase(gpl,02), STR_TESTCASE_NAME(GPL,02)},    //pts_client glv bv-02-c
        {STR_TESTCASE_BV_NB(03), func_testcase(gpl,03), STR_TESTCASE_NAME(GPL,03)},    //pts_client glv bv-03-c
        {STR_TESTCASE_BV_NB(04), func_testcase(gpl,04), STR_TESTCASE_NAME(GPL,04)},
        {STR_TESTCASE_BV_NB(05), func_testcase(gpl,05), STR_TESTCASE_NAME(GPL,05)},
        {STR_TESTCASE_BV_NB(06), func_testcase(gpl,06), STR_TESTCASE_NAME(GPL,06)},
        {STR_TESTCASE_BV_NB(07), func_testcase(gpl,07), STR_TESTCASE_NAME(GPL,07)},
        {STR_TESTCASE_BV_NB(08), func_testcase(gpl,08), STR_TESTCASE_NAME(GPL,08)},
        {STR_TESTCASE_BV_NB(09), func_testcase(gpl,09), STR_TESTCASE_NAME(GPL,09)},
        {STR_TESTCASE_BV_NB(10), func_testcase(gpl,10), STR_TESTCASE_NAME(GPL,10)},
        {STR_TESTCASE_BV_NB(11), func_testcase(gpl,11), STR_TESTCASE_NAME(GPL,11)},
        {STR_TESTCASE_BV_NB(12), func_testcase(gpl,12), STR_TESTCASE_NAME(GPL,12)},
        {STR_TESTCASE_BV_NB(13), func_testcase(gpl,13), STR_TESTCASE_NAME(GPL,13)},
        {STR_TESTCASE_BV_NB(14), func_testcase(gpl,14), STR_TESTCASE_NAME(GPL,14)}
};

void run_gpl_client_ts(int argc, char *args[])
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
