#include "errno.h"
#include "pts_client_app.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

static struct ts_obj_t{
    bool running;
    sem_t m_sem;
    ts_msg *pmsg;
}ts_obj;

static uint16_t g_argc;
static char **g_pargs;

#if 1
generic_transition_time_t pts_trans_time_i = {0,0};
generic_transition_time_t pts_trans_time_c = {20,1};
generic_transition_time_t pts_trans_time_ei = {0,3};
uint8_t pts_delay_i = 0;
uint8_t pts_delay_c = 5;
#else
const generic_transition_time_t pts_trans_time_i = {0,0};
const generic_transition_time_t pts_trans_time_c = {20,1};
const generic_transition_time_t pts_trans_time_ei = {0,3};

const uint8_t pts_delay_i = 0;
const uint8_t pts_delay_c = 5;
#endif

static void ts_stop( )
{
    if((ts_obj.running == false)||(ts_obj.pmsg == NULL))
    return ;
    sem_init(&ts_obj.m_sem, 0, 0);
#if 0
    if(ts_obj.pmsg->release_cb != NULL)
    {
        ts_obj.pmsg->release_cb(ts_obj.pmsg);
    }
#endif
    memset(&ts_obj,0,sizeof(ts_obj));
    ts_log("%s",__FUNCTION__);
}

bool ts_get_param(uint16_t *p_value, uint8_t cnt, char *c_pattern)
{
    uint8_t i = 0,j=0;
    if((g_argc == 0)||(g_pargs == NULL)||(c_pattern == NULL))
    {
        return false;
    }

    for(i = 0; i < g_argc; i++)
    {
        if(strcmp(g_pargs[i],c_pattern) == 0)
        {
            break;
        }
    }

    for(i++,j = 0; (i < g_argc)&&(cnt > 0); i++,cnt--,j++)
    {
        p_value[j] = strtol(g_pargs[i], NULL, 16);
    }

    if(cnt == 0)
        return true;

    return false;
}


bool ts_get_value(uint16_t *p_value, uint8_t cnt)
{
    uint8_t i = 0,j=0;
    if((g_argc == 0)||(g_pargs == NULL))
    {
        return false;
    }

    for(i = 0; i < g_argc; i++)
    {
        if(strcmp(g_pargs[i],"-v") == 0)
        {
            break;
        }
    }

    for(i++,j = 0; ((i < g_argc)&&(cnt)); i++,cnt--,j++)
    {
        p_value[j] = strtol(g_pargs[i], NULL, 16);
    }

    if(cnt == 0)
        return true;

    return false;
}

void ts_rx(void *p_data, uint32_t len)
{
    if((ts_obj.running == false)||(ts_obj.pmsg == NULL))
        return ;

    if(ts_obj.pmsg->rx_cb)
    {
        ts_obj.pmsg->rx_cb(ts_obj.pmsg,p_data,len);
    }
    sem_post(&ts_obj.m_sem);
}

void ts_run(ts_mgs_p p_msg)
{
    uint32_t run_cnt = 0;
    uint32_t retry_cnt = 0;
    int sem_ret;
    //sem_t    *p_sem;
    //struct timespec ts;

    if((ts_obj.running == true)||(p_msg == NULL))
    {
        ts_log("%s,p_msg null or ts running\n",__FUNCTION__);
        return ;
    }
    if((p_msg->tx_cb == NULL)||(p_msg->rx_cb == NULL))
    {
        ts_log("%s,p_msg cb null\n",__FUNCTION__);
        return ;
    }

    ts_obj.running = true;
    ts_obj.pmsg = p_msg;
    if(sem_init(&ts_obj.m_sem, 0, 0) == -1)
    {
        ts_log("%s,sem_init %s",__FUNCTION__,strerror(errno));
        goto ts_end;
    }
    //set 10 s rx timeout
    if(clock_gettime(CLOCK_REALTIME,&p_msg->ts) < 0)
    {
        ts_log("%s,clock_gettime %s",__FUNCTION__,strerror(errno));
        goto ts_end;
    }

    p_msg->p_sem = &ts_obj.m_sem;

    do
    {
        p_msg->tx_cb(p_msg);
        if(p_msg->ts_state == TS_STATE_STOP)
            break;

        p_msg->ts.tv_sec += 10;

        //while((sem_ret = sem_wait(&ts_obj.m_sem)) == -1 && errno == EINTR)
        if(p_msg->msg_rx == true)
        {
            while((sem_ret = sem_timedwait(&ts_obj.m_sem,&p_msg->ts)) == -1 && errno == EINTR)
            {
                ts_log("%s,sem_rx %s\n",__FUNCTION__,strerror(errno));
                continue;
            }
            if(sem_ret == -1)
            {
                retry_cnt++;
                ts_log("%s,sem_rx %s,%ld,%ld\n",__FUNCTION__,strerror(errno),p_msg->ts.tv_sec,p_msg->ts.tv_nsec);
            }
        }

        if(p_msg->run_delay_ms)
        {
            usleep(MS_TO_US(p_msg->run_delay_ms));
        }
        run_cnt++;
        ts_log("%s,ts_run nb %d\n",__FUNCTION__,run_cnt);

        if(retry_cnt > TS_RETRY_MAX_CNT)
            break;

    }while(p_msg->ts_state != TS_STATE_STOP);

ts_end:
    ts_stop(p_msg);
}

static void run_test_client_ts(int argc, char *args[])
{
    bool ret = false;
    uint16_t value[2];

    ret = ts_get_param(value,2,"-t");
    ts_log("%s,%d:%d,%d\n",__FUNCTION__,ret,value[0],value[1]);
}

static void run_raw_client_ts(int argc, char *args[])
{
    uint8_t raw_data[50];
    uint8_t i = 0;
    int32_t dst = 0;
    int32_t app_key_index = 0;
    if(argc < 4)
    {
        return ;
    }
    dst = strtol(args[1], NULL, 16);
    app_key_index = strtol(args[2], NULL, 16);
    printf("%s,dst = %d,key = %d,raw[%d]=",__FUNCTION__,dst,app_key_index,argc-3);
    argc -=3;
    args += 3;
    for(i = 0; i < argc; i++)
    {
        raw_data[i] = strtol(args[i], NULL, 16);
        printf("%x\t",raw_data[i]);
    }
    printf("\n");
    ts_stop();
    aw_mesh_send_packet(dst, 0, 0, 127, 0, app_key_index , raw_data, argc);
    //generic onoff client model raw data make pts pass
    //goo/bv-01-c :pts_client RAW 0x01 0x 00 0x82 0x01
    //goo/bv-02-c :pts_client RAW 0x01 0x00 0x82 0x02 0x01 0x00
    //goo/bv-03-c :pts_client RAW 0x01 0x00 0x82 0x02 0x01 0x00 0x00 0x00
    //goo/bv-04-c :pts_client RAW 0x01 0x00 0x82 0x02 0x00 0x00 0x54 0x05
    //goo/bv-05-c :pts_client RAW 0x01 0x00 0x82 0x03 0x00 0x00
    //goo/bv-06-c :pts_client RAW 0x01 0x00 0x82 0x03 0x00 0x00 0x00 0x00
    //goo/bv-07-c :pts_client RAW 0x01 0x00 0x82 0x03 0x00 0x00 0x54 0x05
    //generic level client model raw data make pts pass

    //glv/bv-02-c :pts_client RAW 0x01 0x00 0x82 0x06 0xFF 0x01 0x00
    //glv/bv-03-c :pts_client RAW 0x01 0x00 0x82 0x06 0xFF 0x01 0x00 0xC0 0x00
    //glv/bv-04-c :pts_client RAW 0x01 0x00 0x82 0x06 0xFF 0x01 0x00 0x54 0x05
    //glv/bv-05-c :pts_client RAW 0x01 0x00 0x82 0x07 0xFF 0x01 0x00
    //glv/bv-06-c :pts_client RAW 0x01 0x00 0x82 0x07 0xFF 0x01 0x00 0xC0 0x00
    //glv/bv-07-c :pts_client RAW 0x01 0x00 0x82 0x07 0xFF 0x01 0x00 0x54 0x05
    //glv/bv-08-c :pts_client RAW 0x01 0x00 0x82 0x09 0xFF 0x01 0x00 0x00 0x00
    //glv/bv-09-c :pts_client RAW 0x01 0x00 0x82 0x09 0xFF 0x01 0x00 0x00 0x00 0xC0 0x00

    //glv/bv-10-c :pts_client RAW 0x01 0x00 0x82 0x09 0xFF 0x01 0x00 0x00  0x54 0x05

    //glv/bv-14-c :pts_client RAW 0x01 0x00 0x82 0x0B 0xFF 0x01 0x00
    //glv/bv-14-c :pts_client RAW 0x01 0x00 0x82 0x0C 0xFF 0x01 0x00
    //generic default transition time  client model raw data make pts pass
    //gdtt/bv-02-c :pts_client RAW 0x01 0x00 0x82 0x0E 0xCF
    //gdtt/bv-03-c :pts_client RAW 0x01 0x00 0x82 0x0F 0x8F
    //generic power level  client model raw data make pts pass
    //gpl/bv-02-c :pts_client RAW 0x01 0x00 0x82 0x16 0xFF 0x01 0x00
    //gpl/bv-03-c :pts_client RAW 0x01 0x00 0x82 0x16 0xFF 0x01 0x00 0xC0 0x00
    //gpl/bv-04-c :pts_client RAW 0x01 0x00 0x82 0x16 0xFF 0x01 0x00 0x54 0x00 0x05
    //gpl/bv-05-c :pts_client RAW 0x01 0x00 0x82 0x17 0xFF 0x01 0x00
    //gpl/bv-06-c :pts_client RAW 0x01 0x00 0x82 0x16 0xFF 0x01 0x00 0x54 0x00 0x05
    //gpl/bv-10-c :pts_client RAW 0x01 0x00 0x82 0x1F 0x67 0x45
    //gpl/bv-11-c :pts_client RAW 0x01 0x00 0x82 0x20 0x67 0x34
    //gpl/bv-13-c :pts_client RAW 0x01 0x00 0x82 0x21 0x10 0x00 0x00 0x10
    //gpl/bv-14-c :pts_client RAW 0x01 0x00 0x82 0x22 0xF0 0x00 0x00 0xF0


     //generic onoff client model raw data make pts pass
}

static struct
{
    char *cmd;
    void (*func)();
    char *doc;
}ts_command[] =
{
        {"RAW",  run_raw_client_ts, "run raw data ts"},         //pts_client glv bv-01-c
        {"TEST", run_test_client_ts, "run ts cmd line test"},   //pts_client glv bv-01-c
        {"goo",  run_goo_client_ts, "run mmdl/cl/goo"},         //pts_client goo bv-01-c
        {"glv",  run_glv_client_ts, "run mmdl/cl/glv"},         //pts_client glv bv-01-c
        {"gdtt", run_gdtt_client_ts, "run mmdl/cl/gdtt"},       //pts_client gdtt bv-01-c
        {"gpoo", run_gpoo_client_ts, "run mmdl/cl/gpoo"},       //pts_client gpoo bv-01-c
        {"gpl",  run_gpl_client_ts, "run mmdl/cl/gpl"},         //pts_client gpl bv-01-c
        {"gbat", run_gbat_client_ts,"run mmdl/cl/gbat"},        //pts_client gbat bv-01-c
        {"lln",  run_lln_client_ts, "run mmdl/cl/lln"},         //pts_client lln bv-01-c
        {"lctl",  run_lctl_client_ts, "run mmdl/cl/lctl"},      //pts_client lctl bv-01-c
        {"lhsl",  run_lhsl_client_ts, "run mmdl/cl/lhsl"},      //pts_client lhsl bv-01-c
        {"hm",    run_hm_client_ts, "run mmdl/cl/hm"},      //pts_client lhsl bv-01-c
        { NULL, NULL, "not found"}
};

void run_client_ts(int argc, char *args[])
{
    uint8_t i;

    if(argc != 2)
    {
        ts_log("%s,warn param len:%d != 2",__FUNCTION__,argc);
        g_argc = argc;
        g_pargs = args;
    }
    else
    {
        g_argc = 0;
        g_pargs = NULL;
    }

	for (i = 0; ts_command[i].cmd; i++) {
		if (strcmp(args[0], ts_command[i].cmd))
			continue;

		if (ts_command[i].func) {
			ts_command[i].func(argc, args);
			break;
		}
	}

    if((ts_command[i].doc != NULL)&&(ts_command[i].cmd == NULL))
    {
        ts_log("%s %s %s\n",args[0],args[1],ts_command[i].doc);
    }

}
