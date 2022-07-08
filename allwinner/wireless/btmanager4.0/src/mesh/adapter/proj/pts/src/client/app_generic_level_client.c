#include "pts_client_app.h"

#define STR_TESTCASE_01_NAME "MMDL/CL/GLV/BV-01-C" //cas 5, 6,7
#define STR_TESTCASE_02_NAME "MMDL/CL/GLV/BV-02-C"
#define STR_TESTCASE_03_NAME "MMDL/CL/GLV/BV-03-C"
#define STR_TESTCASE_04_NAME "MMDL/CL/GLV/BV-04-C"
#define STR_TESTCASE_05_NAME "MMDL/CL/GLV/BV-05-C"
#define STR_TESTCASE_06_NAME "MMDL/CL/GLV/BV-06-C"
#define STR_TESTCASE_07_NAME "MMDL/CL/GLV/BV-07-C"
#define STR_TESTCASE_08_NAME "MMDL/CL/GLV/BV-08-C"
#define STR_TESTCASE_09_NAME "MMDL/CL/GLV/BV-09-C"
#define STR_TESTCASE_10_NAME "MMDL/CL/GLV/BV-10-C"
#define STR_TESTCASE_11_NAME "MMDL/CL/GLV/BV-11-C"
#define STR_TESTCASE_12_NAME "MMDL/CL/GLV/BV-12-C"
#define STR_TESTCASE_13_NAME "MMDL/CL/GLV/BV-13-C"
#define STR_TESTCASE_14_NAME "MMDL/CL/GLV/BV-14-C"
#define STR_TESTCASE_15_NAME "MMDL/CL/GLV/BV-15-C"

#define GLV_DEFAULT_LEVEL   0x1FF //delta_level
#define GLV_DELTA_LEVEL     0x1FF
#define GLV_MOVE_LEVEL      0x1FF

static const generic_transition_time_t glv_trans_time_i = {0x00,0x03};

#if (ENABLE_GENERIC_LEVEL_CLIENT_APP == 1)
static bool g_init = false;
static mesh_model_info_t glv_client;
static struct
{
    pts_client_tx_info tx_info;
    uint8_t pts_testcase;
    int16_t level;
    int16_t delta_level;
    bool ack;
    bool trans_time_optional;
    uint8_t tid;
    uint8_t delay;
    generic_transition_time_t trans_time;
}glv_test_param;

static int32_t generic_level_client_data(const mesh_model_info_p pmodel_info,
                                          uint32_t type, void *pargs)
{
    UNUSED(pmodel_info);
    switch (type)
    {
    case GENERIC_LEVEL_CLIENT_STATUS:
        {
            generic_level_client_status_t *pdata = pargs;
            ts_rx(pdata,sizeof(generic_level_client_status_t));
            if (pdata->optional)
            {
                glv_log("genric level client receive: present = %d, target = %d, remain time = step(%d), resolution(%d)\r\n",    \
                    pdata->present_level, pdata->target_level,pdata->remaining_time.num_steps, pdata->remaining_time.step_resolution);
            }
            else
            {
                glv_log("goo client receive: present = %d\r\n", pdata->present_level);
            }
        }
        break;
    default:
        break;
    }

    return 0;
}

uint32_t pts_generic_level_reg(uint8_t element_index)
{
    bool ret = false;

    if(g_init == true)
    {
        return PTS_SUCCESS;
    }

    memset(&glv_client,0,sizeof(glv_client));
    glv_client.model_data_cb = generic_level_client_data;

    ret = generic_level_client_reg(element_index, &glv_client);
    if(ret == false)
    {
        return PTS_ERROR;
    }
    g_init = true;
    return PTS_SUCCESS;
}

//model app intf
static void move_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_move_set(pmodel_info,glv_test_param.tx_info.dst,glv_test_param.tx_info.app_key_idx,    \
        glv_test_param.delta_level,glv_test_param.tid,glv_test_param.trans_time_optional, \
            glv_test_param.trans_time,glv_test_param.delay,glv_test_param.ack);
}

static void delta_set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_delta_set(pmodel_info,glv_test_param.tx_info.dst,glv_test_param.tx_info.app_key_idx,    \
        glv_test_param.delta_level,glv_test_param.tid,glv_test_param.trans_time_optional, \
            glv_test_param.trans_time,glv_test_param.delay,glv_test_param.ack);
}

static void set_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_level_set(pmodel_info,glv_test_param.tx_info.dst,glv_test_param.tx_info.app_key_idx,    \
        glv_test_param.level,glv_test_param.tid,glv_test_param.trans_time_optional, \
            glv_test_param.trans_time,glv_test_param.delay,glv_test_param.ack);
}

static void get_cmd(ts_mgs_p p_msg)
{
    mesh_model_info_p pmodel_info = p_msg->pargs;
    generic_level_get(pmodel_info,glv_test_param.tx_info.dst,glv_test_param.tx_info.app_key_idx);
}

typedef void (*glv_cmd_cb)(ts_mgs_p p_msg);

static glv_cmd_cb pts_cmd_tab[] =
{
    get_cmd,
    set_cmd,
    delta_set_cmd,
    move_set_cmd
};

#define GET_CMD 0
#define SET_CMD 1
#define DELTA_SET_CMD 2
#define MOVE_SET_CMD 3

static void bv_rx_cb(ts_mgs_p p_msg,void *data, uint32_t len)
{

    //generic_level_client_status_t *p_level = data;
    if(len != sizeof(generic_level_client_status_t))
    {
        glv_log("%s %s",STR_CL_ACK_RX(GLV),STR_PTS_RESULT_RX_INVAL);
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
                if(glv_test_param.ack == false)
                    p_msg->ts_state = TS_STATE_STOP;
            }
            break;
        default:
            p_msg->ts_state = TS_STATE_STOP;
            break;
    }
}

static bool glv_bv_run(ts_mgs_p p_msg)
{
    uint32_t err_code;
    err_code = pts_generic_level_reg(PTS_APP_REG_ELEMENT_0);
    if((p_msg == NULL)||(err_code != PTS_SUCCESS))
    {
        return false;
    }
    memset(p_msg,0,sizeof(ts_msg));
    p_msg->pargs = &glv_client;
    p_msg->msg_tx = true;
    p_msg->msg_rx = true;
    glv_test_param.tx_info.app_key_idx = 0;
    glv_test_param.tx_info.dst = 1;
    glv_test_param.tid = 0;
    return true;
}

static void glv_bv_01_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = GET_CMD;
    ts_run(&msg);
}

static void glv_bv_02_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    glv_test_param.pts_testcase = 2;
    glv_test_param.level = GLV_DEFAULT_LEVEL;
    glv_test_param.ack = true;
    glv_test_param.trans_time_optional = false;
    ts_run(&msg);
}

static void glv_bv_03_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    glv_test_param.pts_testcase = 3;
    glv_test_param.level = GLV_DEFAULT_LEVEL;
    glv_test_param.ack = true;
    glv_test_param.trans_time_optional = true;
    glv_test_param.trans_time = glv_trans_time_i;
    glv_test_param.delay = pts_delay_i;
    ts_run(&msg);
}

static void glv_bv_04_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    glv_test_param.pts_testcase = 4;
    glv_test_param.level = GLV_DEFAULT_LEVEL;
    glv_test_param.ack = true;
    glv_test_param.trans_time_optional = true;
    glv_test_param.trans_time = pts_trans_time_c;
    glv_test_param.delay = pts_delay_c;
    ts_run(&msg);
}

static void glv_bv_05_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    glv_test_param.pts_testcase = 5;
    glv_test_param.level = GLV_DEFAULT_LEVEL;
    glv_test_param.ack = false;
    glv_test_param.trans_time_optional = false;
    ts_run(&msg);
}

static void glv_bv_06_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    glv_test_param.pts_testcase = 6;
    glv_test_param.level = GLV_DEFAULT_LEVEL;
    glv_test_param.ack = false;
    glv_test_param.trans_time_optional = true;
    glv_test_param.trans_time = glv_trans_time_i;
    glv_test_param.delay = pts_delay_i;

    ts_run(&msg);
}

static void glv_bv_07_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = SET_CMD;
    glv_test_param.pts_testcase = 7;
    glv_test_param.level = GLV_DEFAULT_LEVEL;
    glv_test_param.ack = false;
    glv_test_param.trans_time_optional = true;
    glv_test_param.trans_time = pts_trans_time_c;
    glv_test_param.delay = pts_delay_c;
    ts_run(&msg);
}

static void glv_bv_08_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = DELTA_SET_CMD;
    glv_test_param.pts_testcase = 8;
    glv_test_param.delta_level = GLV_DELTA_LEVEL;
    glv_test_param.ack = true;
    glv_test_param.trans_time_optional = false;
    ts_run(&msg);
}

static void glv_bv_09_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = DELTA_SET_CMD;
    glv_test_param.pts_testcase = 9;
    glv_test_param.delta_level = GLV_DELTA_LEVEL;
    glv_test_param.ack = true;
    glv_test_param.trans_time_optional = true;
    glv_test_param.trans_time = glv_trans_time_i;
    glv_test_param.delay = pts_delay_i;

    ts_run(&msg);
}

static void glv_bv_10_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = DELTA_SET_CMD;
    glv_test_param.pts_testcase = 10;
    glv_test_param.delta_level = GLV_DELTA_LEVEL;
    glv_test_param.ack = true;
    glv_test_param.trans_time_optional = true;
    glv_test_param.trans_time = pts_trans_time_c;
    glv_test_param.delay = pts_delay_c;

    ts_run(&msg);
}

static void glv_bv_11_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = DELTA_SET_CMD;
    glv_test_param.pts_testcase = 11;
    glv_test_param.delta_level = GLV_DELTA_LEVEL;
    glv_test_param.ack = false;
    glv_test_param.trans_time_optional = false;
    ts_run(&msg);
}

static void glv_bv_12_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = DELTA_SET_CMD;
    glv_test_param.pts_testcase = 12;
    glv_test_param.delta_level = GLV_DELTA_LEVEL;
    glv_test_param.ack = false;
    glv_test_param.trans_time_optional = true;
    glv_test_param.trans_time = glv_trans_time_i;
    glv_test_param.delay = pts_delay_i;

    ts_run(&msg);
}

static void glv_bv_13_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = DELTA_SET_CMD;
    glv_test_param.pts_testcase = 13;
    glv_test_param.delta_level = GLV_DELTA_LEVEL;
    glv_test_param.ack = false;
    glv_test_param.trans_time_optional = true;
    glv_test_param.trans_time = pts_trans_time_c;
    glv_test_param.delay = pts_delay_c;

    ts_run(&msg);
}

static void glv_bv_14_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = MOVE_SET_CMD;
    glv_test_param.pts_testcase = 14;
    glv_test_param.delta_level = GLV_MOVE_LEVEL;
    glv_test_param.ack = true;
    glv_test_param.trans_time_optional = false;
    ts_run(&msg);
}

static void glv_bv_15_c()
{
    ts_msg msg;
    if(glv_bv_run(&msg) == false)
        return ;
    msg.tx_cb = &bv_tx_cb;
    msg.rx_cb = &bv_rx_cb;
    msg.cmd_id = MOVE_SET_CMD;
    glv_test_param.pts_testcase = 15;
    glv_test_param.delta_level = GLV_MOVE_LEVEL;
    glv_test_param.ack = false;
    glv_test_param.trans_time_optional = false;
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
        {STR_TESTCASE_01, glv_bv_01_c, STR_TESTCASE_01_NAME},    //pts_client glv bv-01-c
        {STR_TESTCASE_02, glv_bv_02_c, STR_TESTCASE_02_NAME},    //pts_client glv bv-02-c
        {STR_TESTCASE_03, glv_bv_03_c, STR_TESTCASE_03_NAME},    //pts_client glv bv-03-c
        {STR_TESTCASE_04, glv_bv_04_c, STR_TESTCASE_04_NAME},    //pts_client glv bv-04-c
        {STR_TESTCASE_05, glv_bv_05_c, STR_TESTCASE_05_NAME},    //pts_client glv bv-05-c
        {STR_TESTCASE_06, glv_bv_06_c, STR_TESTCASE_06_NAME},    //pts_client glv bv-06-c
        {STR_TESTCASE_07, glv_bv_07_c, STR_TESTCASE_07_NAME},    //pts_client glv bv-07-c
        {STR_TESTCASE_08, glv_bv_08_c, STR_TESTCASE_08_NAME},    //pts_client glv bv-08-c
        {STR_TESTCASE_09, glv_bv_09_c, STR_TESTCASE_09_NAME},    //pts_client glv bv-09-c
        {STR_TESTCASE_10, glv_bv_10_c, STR_TESTCASE_10_NAME},    //pts_client glv bv-10-c
        {STR_TESTCASE_11, glv_bv_11_c, STR_TESTCASE_11_NAME},    //pts_client glv bv-11-c
        {STR_TESTCASE_12, glv_bv_12_c, STR_TESTCASE_12_NAME},    //pts_client glv bv-12-c
        {STR_TESTCASE_13, glv_bv_13_c, STR_TESTCASE_13_NAME},    //pts_client glv bv-13-c
        {STR_TESTCASE_14, glv_bv_14_c, STR_TESTCASE_14_NAME},    //pts_client glv bv-14-c
        {STR_TESTCASE_15, glv_bv_15_c, STR_TESTCASE_15_NAME},    //pts_client glv bv-15-c
        { NULL, NULL, "not found"}
};

void run_glv_client_ts(int argc, char *args[])
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

#endif
