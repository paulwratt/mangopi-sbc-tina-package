/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ell/ell.h>

#include "include/util.h"
//
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/signalfd.h>

#define goo_log(FMT, ...) l_info(FMT, ##__VA_ARGS__)

static int log_fd = -1;
static char *log_path = "/data/aw/bluetooth/mesh/";

static char * get_time(uint8_t type)
{
    static char str_time[128];
    struct timeval tv_start;
    struct tm *ptm;
    gettimeofday(&tv_start,NULL);
    ptm = localtime(&tv_start.tv_sec);
    if(type == 0)
    {
        strftime(str_time,sizeof(str_time),"%Y-%m-%d-%H-%M-%S",ptm);
    }
    else
    {
        //sprintf(str_time,"[%016ld]\t",(long long)tv_start.tv_sec*1000 + tv_start.tv_usec/1000);
        sprintf(str_time,"[%013lld",(long long)tv_start.tv_sec);
        sprintf(&str_time[strlen(str_time)],"%03ld]\t",tv_start.tv_usec/1000);
        strftime(&str_time[strlen(str_time)],sizeof(str_time),"%Y%m%d.%H:%M:%S.",ptm);
        sprintf(&str_time[strlen(str_time)],"%03ld",tv_start.tv_usec/1000);
        //sprintf(str_time,"%ld.%ld\t",tv_start.tv_sec,tv_start.tv_usec);
    }
    return str_time;
}

static int create_logfile()
{
    char fn[1024];
    strcpy(fn,log_path);
    //strcat(fn,get_time(0));
    strcat(fn,"mesh_log");
    log_fd = open(fn,O_WRONLY|O_CREAT);
    l_info("create logfile %s,success %d\n",fn,log_fd);
    return log_fd;
}

static void save_log(char *log,uint32_t size)
{
    int b_w = 0;
    //log file not exist ,try to create file

    if(log_fd <= 0)
    {
        log_fd = create_logfile();
    }
    if(log_fd > 0)
    {
        while((b_w = write(log_fd,log,size))!=0)
        {
            if(((b_w == -1)&&(errno != EINTR))  \
                ||(b_w >= size))
            {
                break;
            }
            else
            {
                log += b_w;
                size -= b_w;
            }
        }
    }
}

void mesh_test_log(const char *fmt,...)
{
#if 0
    char str_log[1024];
    //char *time;
    uint32_t size = 0;
    int saved_errno = errno;
    va_list ap;
    va_start(ap,fmt);
    sprintf(str_log,"%s\t",get_time(1));
    size = strlen(str_log);
    size += vsprintf(&str_log[size],fmt,ap);
    str_log[size] = '\n';
    size++;
    save_log(str_log,size);
    va_end(ap);
    errno = saved_errno;
#endif
    va_list args;
    char buf[1024] = {'\0'};
    va_start(args,fmt);
    sprintf(buf,"[%s]%s\n","FILE LOG",fmt);
    vprintf(buf,args);
    va_end(args);
}
