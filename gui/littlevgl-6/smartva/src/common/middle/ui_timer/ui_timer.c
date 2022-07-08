#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include "ui_timer.h"
#include "common.h"
#include "dbList.h"

#define TIMER_PTHREAD_PER_SECONED (1000 / 100)
static lv_task_t *timer_task_id;
static pthread_t time_message_pthread;

static db_list_t *timer_message_list = NULL;
static db_list_t *timer_manage_list = NULL;

static void *timer_message_process_loop(void *arg)
{
	timer_register_info_t *registerinfo = NULL;
	while (1) {
		registerinfo = __db_list_pop(timer_message_list);
		registerinfo->CallBackFunction(&registerinfo->timerinfo);
	}
	return NULL;
}

static int timer_compare_register_info(void *data, void *compare_param)
{
	timer_register_info_t *timer_register = (timer_register_info_t *)data;
	timer_register_info_t *compare_timer_register = (timer_register_info_t *)compare_param;

	if (strlen(timer_register->timerinfo.time_name) == strlen(compare_timer_register->timerinfo.time_name) &&
		strncmp(timer_register->timerinfo.time_name, compare_timer_register->timerinfo.time_name,
									strlen(compare_timer_register->timerinfo.time_name)) == 0 &&
		timer_register->timerinfo.timerid == compare_timer_register->timerinfo.timerid) {
		return 0;
	}
	return 1;

}
int timer_message_is_exist(timer_register_info_t *register_info)
{
	timer_register_info_t *tmp = NULL;

	tmp = __db_list_search_node(timer_message_list, register_info, timer_compare_register_info);
	if (NULL != tmp) {
		return 1;
	}
	return 0;
}

static void timer_send_message(timer_register_info_t *register_info)
{
	if (!timer_message_is_exist(register_info)) {
		__db_list_put_tail(timer_message_list, register_info);
	}
}
static int timer_process(void *data, void *param)
{
	timer_register_info_t *tmp = (timer_register_info_t *)data;

//	com_err("cur_tick:%d!\n", tmp->timerinfo.cur_tick);
//	com_err("time:%d!\n", tmp->timerinfo.time);
//	com_err("time_name:%s!\n", tmp->timerinfo.time_name);
	tmp->timerinfo.cur_tick++;
	if (tmp->timerinfo.cur_tick / TIMER_PTHREAD_PER_SECONED == tmp->timerinfo.time) {
		timer_send_message(tmp);
		if (tmp->timerinfo.looptype == TIMER_LOOP_ALWAYS) {
			tmp->timerinfo.cur_tick = 0;
		} else {
			return 1; //return 1. will pop this
		}
	}
	return 0;
}

static void timer_process_loop(lv_task_t *arg)
{
	__db_list_for_each_entry_and_pop(timer_manage_list, NULL, timer_process);
	return NULL;
}

int timer_is_exist(timer_register_info_t *register_info)
{
	timer_register_info_t *tmp = NULL;

	tmp = __db_list_search_node(timer_manage_list, register_info, timer_compare_register_info);
	if (NULL != tmp) {
		return 1;
	}
	return 0;
}

int timer_register(timer_register_info_t *register_info)
{
	int ret = 0;
	if (register_info == NULL) {
		com_err("Paramter Error!\n");
		return -1;
	}
	if (NULL == register_info->CallBackFunction) {
		com_err("Paramter Error!\n");
		return -1;
	}

//	com_err("time:%d!\n", register_info->timerinfo.time);
//	com_err("time_name:%s!\n", register_info->timerinfo.time_name);
	register_info->timerinfo.cur_tick = 0;
	if (0 == timer_is_exist(register_info)) {
		ret = __db_list_put_tail(timer_manage_list, register_info);
	} else {
		ret = -1;
	}
	return ret;
}

int timer_unregister(timer_register_info_t *register_info)
{
	if (timer_is_exist(register_info)) {
		while (timer_message_is_exist(register_info)) {
			usleep(5000);
		}
		__db_list_search_and_pop(timer_manage_list, register_info, timer_compare_register_info);
	}
	return 0;
}

int timer_modify_time(timer_register_info_t *register_info, int timeout)
{
	while (0 != timer_message_is_exist(register_info)) {
		usleep(5000);
	}
	if (timeout <= 0) {
		if (timer_is_exist(register_info)) {
			timer_unregister(register_info);
		}
		return 0;
	} else {
		register_info->timerinfo.time = timeout;
		register_info->timerinfo.cur_tick = 0;
		if (!timer_is_exist(register_info)) {
			timer_register(register_info);
		}
	}
	return 0;
}


int timer_init(void)
{
	int ret = 0;

	timer_message_list = db_list_create("time_message_list", 1);
	if (NULL == timer_message_list) {
		com_err("db_list_create time_message_list fail\n");
		goto done;
	}
	timer_manage_list = db_list_create("timer_manage_list", 0);
	if (NULL == timer_manage_list) {
		com_err("db_list_create fail\n");
		goto error1;
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x4000);

	timer_task_id = lv_task_create(timer_process_loop, 100, LV_TASK_PRIO_MID, NULL);
	ret = pthread_create(&time_message_pthread, &attr,
			     &timer_message_process_loop, NULL);
	pthread_attr_destroy(&attr);
	if (ret < 0) {
		com_err("pthread_create error\n");
		goto error2;
	}
	return 0;
error2:
	__db_list_destory(timer_manage_list);
	timer_manage_list = NULL;
error1:
	__db_list_destory(timer_message_list);
	timer_message_list = NULL;
done:
	return 0;
}
