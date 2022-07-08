#include "pts_client_app.h"

#define STR_TESTCASE_RX "MMDL/CL/GOO/ACK"

#define STR_TESTCASE_01_NAME "MMDL/CL/GOO/BV-01-C" //cas 5, 6,7
#define STR_TESTCASE_02_NAME "MMDL/CL/GOO/BV-02-C"
#define STR_TESTCASE_03_NAME "MMDL/CL/GOO/BV-03-C"
#define STR_TESTCASE_04_NAME "MMDL/CL/GOO/BV-04-C"
#define STR_TESTCASE_05_NAME "MMDL/CL/GOO/BV-05-C"
#define STR_TESTCASE_06_NAME "MMDL/CL/GOO/BV-06-C"
#define STR_TESTCASE_07_NAME "MMDL/CL/GOO/BV-07-C"

#define TEST_CASE_GOO_BV_01_C 1
#define TEST_CASE_GOO_BV_02_C 2
#define TEST_CASE_GOO_BV_03_C 3
#define TEST_CASE_GOO_BV_04_C 4
#define TEST_CASE_GOO_BV_05_C 5
#define TEST_CASE_GOO_BV_06_C 6
#define TEST_CASE_GOO_BV_07_C 7


#if (ENABLE_GENERIC_ONOFF_CLIENT_APP == 1)
//static pts_client_goo_db g_onoff_db;
static bool g_init = false;
static mesh_model_info_t goo_client;
static struct
{
    bool ack;
    bool trans_time_optional;
    uint8_t pts_testcase;
    uint8_t tid;
    uint8_t delay;
    pts_client_tx_info tx_info;
    generic_on_off_t onoff;
    generic_transition_time_t trans_time;
}goo_test_param;

static int32_t generic_on_off_client_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
    UNUSED(pmodel_info);
    switch (type)
    {
    case GENERIC_ON_OFF_CLIENT_STATUS:
        {
            generic_on_off_client_status_t *pdata = pargs;
            ts_rx(pdata,sizeof(generic_on_off_client_status_t));
            if (pdata->optional)
            {
                goo_log("goo client receive: present = %d, target = %d, remain time = step(%d), resolution(%d)\r\n",    \
                    pdata->present_on_off, pdata->target_on_off,pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);
            }
            else
            {
                goo_log("goo client receive: present = %d\r\n", pdata->present_on_off);
            }
        }
        break;
    default:
        break;
    }

    return 0;
}

uint32_t pts_generic_onoff_reg(uint8_t element_index)
{
    bool ret = false;

    if(g_init == true)
    {
        return PTS_SUCCESS;
    }

    memset(&goo_client,0,sizeof(goo_client));
    goo_client.model_data_cb = generic_on_off_client_data;

    ret = generic_on_off_client_reg(element_index, &goo_client);
    if(ret == false)
    {
        return PTS_ERROR;
    }
    g_init = true;
    return PTS_SUCCESS;
}

static void bv_01_tx_cb(ts_mgs_p p_msg)
{
    //static uint8_t testcase_step = 0;
    mesh_model_info_p pmodel_info = p_msg->pargs;

    switch(p_msg->ts_state)
    {
        case TS_STATE_0:
            {
                //step 1 , send generic onoff state get
                generic_on_off_get(pmodel_info,goo_test_param.tx_info.dst,goo_test_param.tx_info.app_key_idx);
            }
            break;
        default:
            p_msg->ts_state = TS_STATE_STOP;
            break;
    }
}

static void bv_tx_cb(ts_mgs_p p_msg)
{
    //static uint8_t testcase_step = 0;
    mesh_model_info_p pmodel_info = p_msg->pargs;

    switch(p_msg->ts_state)
    {
        case TS_STATE_0:
            {
                generic_on_off_set(pmodel_info,goo_test_param.tx_info.dst,goo_test_param.tx_info.app_key_idx,   \
                    goo_test_param.onoff,goo_test_param.tid,goo_test_param.trans_time_optional, \
                        goo_test_param.trans_time,goo_test_param.delay,goo_test_param.ack);
            }
            break;
        default:
            p_msg->ts_state = TS_STATE_STOP;
            break;

    }
}

static void bv_rx_cb(ts_mgs_p p_msg,void *data, uint32_t len)
{
    generic_on_off_client_status_t *p_onoff = data;
    if(len < sizeof(generic_on_off_client_status_t))
    {
        goo_log("%s %s",STR_TESTCASE_RX,STR_PTS_RESULT_RX_INVAL);
        return ;
    }
    if(p_onoff->present_on_off)
    {
        mesh_test_log("goo end on\n");
    }
    else
    {
        mesh_test_log("goo end off\n");
    }
    if((goo_test_param.ack == true)&&(goo_test_param.onoff == p_onoff->present_on_off))
    {
        goo_log("%s %s:ack:%d,except onoff:%d, present onoff:%d",STR_TESTCASE_RX,STR_PTS_RESULT_SUCCESS,goo_test_param.ack,goo_test_param.onoff,p_onoff->present_on_off);
    }
    else
    {
        goo_log("%s %s:%d,except onoff:%d, present onoff:%d",STR_TESTCASE_RX,STR_PTS_RESULT_FAIL,goo_test_param.ack,goo_test_param.onoff,p_onoff->present_on_off);
    }

    p_msg->ts_state = TS_STATE_STOP;
}

static bool goo_bv_run(ts_mgs_p p_msg)
{
    uint32_t err_code;
    err_code = pts_generic_onoff_reg(PTS_APP_REG_ELEMENT_0);
    if((p_msg == NULL)||(err_code != PTS_SUCCESS))
    {
        return false;
    }
    memset(p_msg,0,sizeof(ts_msg));
    p_msg->pargs = &goo_client;
    p_msg->msg_tx = true;
    p_msg->msg_rx = true;
    goo_test_param.tx_info.app_key_idx = 0;
    goo_test_param.tx_info.dst = 1;
    goo_test_param.tid = 0;
    return true;
}

static void goo_bv_07_c()
{
    ts_msg msg;
    if(goo_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;

    goo_test_param.pts_testcase = 5;
    goo_test_param.onoff = GENERIC_OFF;
    goo_test_param.ack = false;
    goo_test_param.trans_time_optional = true;
    goo_test_param.delay = 0x05;
    goo_test_param.trans_time.num_steps = (0x54&0x3F);
    goo_test_param.trans_time.step_resolution = (0x54&0xC0)>>6;
    ts_run(&msg);
}

static void goo_bv_06_c()
{
    ts_msg msg;
    if(goo_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;

    goo_test_param.pts_testcase = 5;
    goo_test_param.onoff = GENERIC_OFF;
    goo_test_param.ack = false;

    goo_test_param.trans_time_optional = true;
    goo_test_param.delay = 0;
    goo_test_param.trans_time.num_steps = 0;
    goo_test_param.trans_time.step_resolution = 0;
    ts_run(&msg);
}

static void goo_bv_05_c()
{
    ts_msg msg;
    if(goo_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;

    goo_test_param.pts_testcase = 5;
    goo_test_param.onoff = GENERIC_OFF;
    goo_test_param.ack = false;

    goo_test_param.trans_time_optional = false;
    //goo_test_param.delay = 5;
    //goo_test_param.trans_time.num_steps = 20;
    //goo_test_param.trans_time.step_resolution = 1;
    ts_run(&msg);
}

static void goo_bv_04_c()
{
    ts_msg msg;
    if(goo_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;

    goo_test_param.pts_testcase = 4;
    goo_test_param.onoff = GENERIC_ON;
    goo_test_param.ack = true;

    goo_test_param.trans_time_optional = true;
    goo_test_param.delay = 5;
    goo_test_param.trans_time.num_steps = 20;
    goo_test_param.trans_time.step_resolution = 1;
    ts_run(&msg);
}


static void goo_bv_03_c()
{
    ts_msg msg;
    if(goo_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    goo_test_param.pts_testcase = 3;

    goo_test_param.onoff = GENERIC_ON;
    goo_test_param.ack = true;

    goo_test_param.trans_time_optional = true;
    goo_test_param.delay = 0;
    goo_test_param.trans_time.num_steps = 0;
    goo_test_param.trans_time.step_resolution = 0;
    ts_run(&msg);
}

static void goo_bv_02_c()
{
    ts_msg msg;
    if(goo_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    goo_test_param.pts_testcase = 2;
    goo_test_param.onoff = GENERIC_ON;
    goo_test_param.trans_time_optional = false;
    //goo_test_param.delay = 0;
    //goo_test_param.trans_time.num_steps = 0;
    //goo_test_param.trans_time.step_resolution = 0;
    goo_test_param.ack = true;
    ts_run(&msg);
}

static void goo_bv_01_c()
{
    ts_msg msg;
    if(goo_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_01_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    goo_test_param.pts_testcase = 1;
    goo_test_param.onoff = GENERIC_OFF;
    goo_test_param.ack = true;
    ts_run(&msg);
}

static struct
{
    char *cmd;
    void (*func)();
    char *doc;
}ts_command[] =
{
        {STR_TESTCASE_01, goo_bv_01_c, STR_TESTCASE_01_NAME},    //pts_client goo bv-01-c
        {STR_TESTCASE_02, goo_bv_02_c, STR_TESTCASE_02_NAME},    //pts_client goo bv-02-c
        {STR_TESTCASE_03, goo_bv_03_c, STR_TESTCASE_03_NAME},    //pts_client goo bv-03-c
        {STR_TESTCASE_04, goo_bv_04_c, STR_TESTCASE_04_NAME},    //pts_client goo bv-04-c
        {STR_TESTCASE_05, goo_bv_05_c, STR_TESTCASE_05_NAME},    //pts_client goo bv-05-c
        {STR_TESTCASE_06, goo_bv_06_c, STR_TESTCASE_06_NAME},    //pts_client goo bv-06-c
        {STR_TESTCASE_07, goo_bv_07_c, STR_TESTCASE_07_NAME},    //pts_client goo bv-07-c
        { NULL, NULL, "not found"}
};

void run_goo_client_ts(int argc, char *args[])
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
        goo_log("%s %s %s\n",args[0],args[1],ts_command[i].doc);
    }
#endif
}
//reuse goo client pts code to do goo test
static bool goo_bv_run_test(ts_mgs_p p_msg)
{
    uint32_t err_code;
    static uint8_t tid = 0;
    err_code = pts_generic_onoff_reg(pts_get_element_idx());
    if((p_msg == NULL)||(err_code != PTS_SUCCESS))
    {
        return false;
    }
    memset(p_msg,0,sizeof(ts_msg));
    p_msg->pargs = &goo_client;
    p_msg->msg_tx = true;
    p_msg->msg_rx = true;

    goo_test_param.tid = tid;
    tid++;
    return true;
}

void goo_onoff(uint16_t dst,uint16_t appkey_idx,uint16_t on_off)
{
    ts_msg msg;
    if(goo_bv_run_test(&msg) == false)
        return ;
    goo_test_param.tx_info.app_key_idx = appkey_idx;
    goo_test_param.tx_info.dst = dst;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    if(on_off == 0)
    {
        goo_test_param.onoff = GENERIC_OFF;
        mesh_test_log("goo off\tstart\n");
    }
    else
    {
        goo_test_param.onoff = GENERIC_ON;
        mesh_test_log("goo on\tstart\n");
    }
    goo_test_param.ack = true;
    ts_run(&msg);
}

#endif
