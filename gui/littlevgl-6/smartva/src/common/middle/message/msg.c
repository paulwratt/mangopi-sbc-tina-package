/**
 * @file: msg.c
 * @autor: liujiaming
 * @url: liujiaming@allwinnertech.com
 *  smartva message system
 */
#include "msg.h"
#include <pthread.h>
#include <unistd.h>
#include "dbList.h"

static pthread_t msg_task_id;
static pthread_mutex_t msg_mutex;
static void *message_sys_loop_task(void *arg);
static int (*msg2app_callback)(MsgDataInfo *);

static db_list_t *msg_info_list = NULL;

int sent_msg(MsgDataInfo msg)
{
    if(msg_info_list)
    __db_list_put_tail(msg_info_list, (void *)&msg);
    return 0;
}

static MsgDataInfo *get_msg(void)
{
    MsgDataInfo *msg = NULL;
    if(msg_info_list)
    msg = (MsgDataInfo *)__db_list_pop(msg_info_list);
    return msg;
}

int message_sys_init(void)
{
    int ret = -1;
    pthread_attr_t attr;

    pmsg("msg sys init..............");
//    pthread_mutex_init(&this_mutex, NULL);
	msg_info_list = db_list_create("msg_info_list", 0);
	if (NULL == msg_info_list) {
		perr("db_list_create Error!\n");
		return -1;
	}
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 0x2000);
    ret = pthread_create(&msg_task_id, &attr, &message_sys_loop_task, NULL);
	if (ret < 0) {
		pmsg("create thread fail\n");
	}
    pthread_attr_destroy(&attr);
    return ret;
}


int message_sys_unit(void)
{
    perr("msg sys exit..............");
    __db_list_destory(msg_info_list);
    return 0;
}


static void *message_sys_loop_task(void *arg)
{
    MsgDataInfo *msg = NULL;

    while(1)
    {
        msg = get_msg();
        if(msg == NULL)
        {
            usleep(50000);
            continue;
        }
        if(msg2app_callback)
            msg2app_callback(msg);
    }
    return NULL;
}

//msg pass to app level
int init_root_callback(void *cb)
{
    if(msg2app_callback == NULL)
    {
        msg2app_callback = cb;
        return 0;
    }
    else
    {
        perr("cb had been set");
    }
    return -1;
}
