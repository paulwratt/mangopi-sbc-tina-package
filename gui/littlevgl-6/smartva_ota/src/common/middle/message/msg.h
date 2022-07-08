
#ifndef __MSG_H_
#define __MSG_H_

#include<stdint.h>
#include<stdlib.h>
#include <string.h>
#include<stdio.h>
#include<lv_hal_indev.h>

#define pmsg(fmt,...) printf("msg:<%s:%d>"fmt"\n",__func__, __LINE__, ##__VA_ARGS__)
#define perr(fmt,...) printf("\033[1;40;31merr:<%s:%d>"fmt"\n\033[0m",__func__, __LINE__, ##__VA_ARGS__)


typedef enum __MSG_TYPE
{
    MSG_KEY,
    MSG_POWER,
    MSG_USB,
    MSG_UI,
    MSG_TIMER,
    MSG_SLEEP,
    MSG_BACKLIGHT_OFF,
    MSG_BACKLIGHT_ON,
    MSG_END,
} MsgType;

typedef struct __MSGINFO
{
    MsgType type;  //msg type
    int from;     //from where
    int to;       //go to where
    int value;       //msg value
    int status;     //msg status
    int used;
} MsgDataInfo;


int message_sys_init(void);
int message_sys_unit(void);
int sent_msg(MsgDataInfo msg);
MsgDataInfo *get_msg(void);
int init_root_callback(void *cb);

#endif //__MSG_H_
