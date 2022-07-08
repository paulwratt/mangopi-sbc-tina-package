/**
 * @file: sunxi_rtpdev.c
 * @autor: liuajiming
 * @url: liujiaming@allwinnertech.com
 */
/*********************
 *      INCLUDES
 *********************/
#ifdef __RTP_USE__

#include "sunxi_rtpdev.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include "tslib.h"

/*********************
 *      DEFINES
 *********************/
#define RTP_DEV_POLL	0		/* simulation */
#define RTP_DEV_SYNC	1		/* sync , else async*/
//
int rtpdev_root_x;
int rtpdev_root_y;
int rtpdev_button;
pthread_t rtpdev_pthread_id;
pthread_mutex_t rtpdev_mutex;

static struct tsdev *ts = NULL;
static struct ts_sample samp;

int ts_init(void)
{
	ts = ts_setup(NULL, 0);
	if(!ts)
    {
        printf("ts_setup error!\n");
        return -1;
    }
//	printf("tsinfo ts = 0x%x\n",ts);
	return 0;
}

void *rtpdev_thread(void * data)
{
	int ret;
	int press_cnt = 0;
	while(1)
	{
		ret = ts_read(ts, &samp, 1);
		//printf("samp: x: %d  y: %d  press:%d   tv_sec:%d   tv_usec:%d\n",samp.x,samp.y,samp.pressure,samp.tv.tv_sec,samp.tv.tv_usec);
		press_cnt++;
		if(press_cnt < 3)
		{
			continue;
		}
		pthread_mutex_lock(&rtpdev_mutex);
		rtpdev_root_x = samp.x;
		rtpdev_root_y = samp.y;
		if(samp.pressure == 0xff)//按下
		{

			rtpdev_button = LV_INDEV_STATE_PR;
		}
		else//抬起
		{
			press_cnt = 0;
			rtpdev_button = LV_INDEV_STATE_REL;
		}
		pthread_mutex_unlock(&rtpdev_mutex);
	}
}

/**
 * Initialize the rtpdev interface
 */
void rtpdev_init(void)
{
	pthread_attr_t attr;

	ts_init();
	rtpdev_root_x = 0;
    rtpdev_root_y = 0;
    rtpdev_button = LV_INDEV_STATE_REL;
	pthread_mutex_init (&rtpdev_mutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x4000);
	int ret = pthread_create(&rtpdev_pthread_id, &attr, rtpdev_thread, NULL);
	pthread_attr_destroy(&attr);
	if (ret == -1)
	{
		printf("create thread fail\n");
		return ;
	}
}

/**
 * uninitialize the rtpdev interface
 */
void rtpdev_uninit(void)
{
	pthread_join(rtpdev_pthread_id, NULL);
	pthread_mutex_destroy(&rtpdev_mutex);

	rtpdev_root_x = 0;
    rtpdev_root_y = 0;
    rtpdev_button = LV_INDEV_STATE_REL;
	ts_close(ts);
	ts = NULL;
}

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool rtpdev_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
	//printf("#####L:%d, F=%s, t=%d\n", __LINE__, __FUNCTION__, lv_tick_get());

	pthread_mutex_lock(&rtpdev_mutex);
    data->point.x = rtpdev_root_x;
    data->point.y = rtpdev_root_y;
    data->state = rtpdev_button;
	pthread_mutex_unlock(&rtpdev_mutex);
	return false;
}

#endif
