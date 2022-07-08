/**
 * @file: msg.c
 * @autor: liujiaming
 * @url: liujiaming@allwinnertech.com
 *  smartva message system
 */
#include "msg.h"
#include <pthread.h>
#include <unistd.h>

#define MGS_BUF_LEN 16

static pthread_t msg_task_id;
//static pthread_mutex_t this_mutex;
static void *message_sys_loop_task(void *arg);
static int msg_lock(void);
static int msg_unlock(void);
static int (*msg2app_callback)(MsgDataInfo *);

typedef struct MsgList_t
{
    unsigned int nod_id;
    MsgDataInfo msg;
    struct MsgList_t *next;
}MsgList_t;

MsgList_t *msglist = NULL;

int sent_msg(MsgDataInfo msg)
{
    int cnt = 0;
    msg_lock();
    while(msglist->msg.used != 0)
    {
        msglist = msglist->next;
        cnt++;
        if(cnt == MGS_BUF_LEN)
        {
            perr("msglist is full!");
            usleep(50000);
            cnt = 0;
        }
    }
    //pmsg("save msg to list i = %d",msglist->nod_id);
    memcpy(&msglist->msg, &msg, sizeof(MsgDataInfo));
    //pmsg("type:%d  value:%d  status:%d",msglist->msg.type,msglist->msg.value,msglist->msg.status);
    msglist = msglist->next;
    msg_unlock();
    return 0;
}

MsgDataInfo *get_msg(void)
{
    int cnt = 0;
    msg_lock();
    while(msglist->msg.used == 0)
    {
        msglist = msglist->next;
        cnt++;
        if(cnt == MGS_BUF_LEN)
        {
            cnt = 0;
            return NULL;
        }
    }
    //perr("get msg to list i = %d",msglist->nod_id);
    msglist->msg.used = 0;
    msg_unlock();
    return &msglist->msg;
}


static int msg_lock(void)
{
    return 0;
}

static int msg_unlock(void)
{
    return 0;
}

static int msg_list_init(int MsgLen)
{
    int i;
    MsgList_t *head = NULL;
    MsgList_t *node = NULL;

    msglist = (MsgList_t *)malloc(sizeof(MsgList_t));
    if(msglist==NULL)
    {
        perr("node malloc fail!");
        return -1;
    }
    msg_lock();
    memset(msglist, 0x0, sizeof(MsgList_t));
    head = msglist;
    msglist->nod_id = 0;
    msg_unlock();
    for(i=1;i<MsgLen;i++)
    {
        node = (MsgList_t *)malloc(sizeof(MsgList_t));
        if(node==NULL)
        {
            perr("node malloc fail!");
            return -1;
        }
        memset(node, 0x0, sizeof(MsgList_t));
        node->nod_id = i;
        msg_lock();
        msglist->next = node;
        msglist = msglist->next;
        msg_unlock();
        //pmsg("create list i = %d",node->nod_id);
    }
    msglist->next = head;
    return 0;
}

static int msg_list_unit(MsgList_t *msglist)
{
    MsgList_t * node  = NULL;
    if(msglist == NULL)
    {
        perr("msglist is null");
        return -1;
    }

    node = msglist->next;
    msg_lock();
    while(node != msglist)
    {
        msglist->next = node->next;
        pmsg("free list node i = %d",node->nod_id);
        free(node);
        node = msglist->next;
    }
    msg_unlock();
    pmsg("free list node i = %d",node->nod_id);
    free(node);
    pmsg("free list finish!");
    return 0;
}

int message_sys_init(void)
{
    int ret = -1;
    pmsg("msg sys init..............");
//    pthread_mutex_init(&this_mutex, NULL);
    msg_list_init(MGS_BUF_LEN);
    ret = pthread_create(&msg_task_id, NULL, &message_sys_loop_task, NULL);
	if (ret == -1) {
		pmsg("create thread fail\n");
		return -1;
	}
    return 0;
}


int message_sys_unit(void)
{
    perr("msg sys exit..............");
    msg_list_unit(msglist);
    //pthread_detach(msg_task_id);
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
