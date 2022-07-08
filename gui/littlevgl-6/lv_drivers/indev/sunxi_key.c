/**
 * @file: sunxi_key.c
 * @autor: huangyixiu
 * @url: huangyixiu@allwinnertech.com
 */
/*********************
 *      INCLUDES
 *********************/
#include "sunxi_key.h"
//#include "input-event-codes.h"
#if USE_SUNXI_KEY != 0

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>

p_keydev_callback keydev_callback = NULL;
/*********************
 *      DEFINES
 *********************/
#ifdef USE_SUNXI_ADC_KEY
int adckey_dev_fd = -1;
#endif
#ifdef USE_SUNXI_IR_KEY
int irkey_dev_fd = -1;
#endif
pthread_mutex_t keydev_mutex;
pthread_t keydev_pthread_id;

uint32_t last_key;
lv_indev_state_t last_key_state = LV_INDEV_STATE_REL;

uint32_t key_switch(uint32_t in);

void *keydev_thread(void * data)
{
	(void)data;
	int ret;
	fd_set readfd;
	int max_fd = -1;
	struct timeval timeout;
	struct input_event in;
	static int press_cnt;

	while(1)
	{
		timeout.tv_sec=5;
	    timeout.tv_usec=0;

	    FD_ZERO(&readfd);
#ifdef USE_SUNXI_ADC_KEY
		max_fd = adckey_dev_fd;
	    FD_SET(adckey_dev_fd, &readfd);
#endif
#ifdef USE_SUNXI_IR_KEY
		FD_SET(irkey_dev_fd, &readfd);
		max_fd = irkey_dev_fd;
#endif

#if defined(USE_SUNXI_IR_KEY) && defined(USE_SUNXI_ADC_KEY)
		max_fd = (irkey_dev_fd > adckey_dev_fd)?irkey_dev_fd:adckey_dev_fd;
#endif
	    ret = select(max_fd+1, &readfd, NULL, NULL, &timeout);
		if (ret > 0)
	    {
#ifdef USE_SUNXI_ADC_KEY
			if(FD_ISSET(adckey_dev_fd,&readfd)) {
				FD_CLR(adckey_dev_fd, &readfd);
					read(adckey_dev_fd, &in, sizeof(in));
					//printf("input info:(%d, %d, %d)\n", in.code, in.type, in.value);
					pthread_mutex_lock(&keydev_mutex);
					if(in.type == 1) {
						last_key = in.code;
						if (in.value == 0) {
							last_key_state = LV_INDEV_STATE_REL;
						}
						else {
							last_key_state = LV_INDEV_STATE_PR;
						}
						if (keydev_callback) {
							if(last_key_state){
							press_cnt++;
							if(press_cnt++ > 40){
							last_key_state = LV_INDEV_STATE_LONG_PR;
							}
							}else{
							press_cnt = 0;
							}
							keydev_callback(key_switch(last_key), last_key_state);
						}
					}
					pthread_mutex_unlock(&keydev_mutex);
				}
#endif
#ifdef	USE_SUNXI_IR_KEY
			if(FD_ISSET(irkey_dev_fd,&readfd)) {
				FD_CLR(irkey_dev_fd, &readfd);
					read(irkey_dev_fd, &in, sizeof(in));
					// printf("input info:(%d, %d, %d)\n", in.code, in.type, in.value);

					pthread_mutex_lock(&keydev_mutex);
					if(in.type == 1) {
						last_key = in.code;
						if (in.value == 0) {
							last_key_state = LV_INDEV_STATE_REL;
						}
						else {
							last_key_state = LV_INDEV_STATE_PR;
						}
						if (keydev_callback) {
							if(last_key_state){
							press_cnt++;
							if(press_cnt++ > 40){
							last_key_state = LV_INDEV_STATE_LONG_PR;
							}
							}else{
							press_cnt = 0;
							}
							keydev_callback(key_switch(last_key), last_key_state);
						}
					}
					pthread_mutex_unlock(&keydev_mutex);
				}
#endif
		}

	}
}

void keydev_init(void)
{
	int ret;
	pthread_attr_t attr;

#ifdef USE_SUNXI_ADC_KEY
    adckey_dev_fd = open(ADCKEY_DEV_NAME,  O_RDONLY | O_NONBLOCK);
    if(adckey_dev_fd == -1) {
        perror("unable open evdev interface:");
        return;
    }
#endif

#ifdef USE_SUNXI_IR_KEY
	irkey_dev_fd = open(IRKEY_DEV_NAME,  O_RDONLY | O_NONBLOCK);
	if(irkey_dev_fd == -1) {
		perror("unable open evdev interface:");
		return;
	}
#endif
	last_key = 0;
	last_key_state = LV_INDEV_STATE_REL;
	pthread_mutex_init (&keydev_mutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x4000);

	ret = pthread_create(&keydev_pthread_id, &attr, keydev_thread, NULL);
	pthread_attr_destroy(&attr);
	if (ret == -1) {
		printf("create thread fail\n");
		return ;
	}
}

void keydev_uninit(void)
{
	pthread_join(keydev_pthread_id, NULL);
	pthread_mutex_destroy(&keydev_mutex);

	last_key = 0;
	last_key_state = LV_INDEV_STATE_REL;
#ifdef USE_SUNXI_ADC_KEY
	if (adckey_dev_fd != -1){
		close(adckey_dev_fd);
	}
#endif
#ifdef USE_SUNXI_IR_KEY
	if (irkey_dev_fd != -1){
		close(irkey_dev_fd);
	}
#endif
}

uint32_t key_switch(uint32_t in)
{
	uint32_t out = 0;

	switch(in)
	{
		case KEY_ENTER:	/* enter */
			out = LV_KEY_ENTER;
			break;
		case KEY_BACK: /* BACK */
			//out = LV_KEY_BACKSPACE;
			out = LV_KEY_RETURN;
			break;

		case KEY_NEXT:	/* NEXT */
			out = LV_KEY_NEXT;
			break;
		case KEY_PREVIOUS:	/* PREV */
			out = LV_KEY_PREV;
			break;

		case KEY_VOLUMEDOWN:	/* VOL- */
			out = LV_KEY_VOLUME_DOWN;
			break;
		case KEY_VOLUMEUP:	/* VOL+ */
			out = LV_KEY_VOLUME_UP;
			break;

		case KEY_LEFT:
			out = LV_KEY_LEFT;
			break;
		case KEY_RIGHT:
			out = LV_KEY_RIGHT;
			break;

		case KEY_UP:
			out = LV_KEY_UP;
			break;
		case KEY_DOWN:
			out = LV_KEY_DOWN;
			break;

		default:
			break;
	}

	return out;
}


bool keydev_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/

	pthread_mutex_lock(&keydev_mutex);
    data->state = last_key_state;
    data->key = key_switch(last_key);
	// printf("key=%d, state=%d\n", data->key, data->state);
	pthread_mutex_unlock(&keydev_mutex);
    return false;
}

void keydev_register_hook(p_keydev_callback func)
{
	keydev_callback = func;
}

#endif
