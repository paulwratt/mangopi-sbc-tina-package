#ifndef __UI_TIMER_H__
#define __UI_TIMER_H__
#include "default_timer.h"
typedef enum { TIMER_LOOP_ONE_SHOOT = 0, TIMER_LOOP_ALWAYS } timer_loop_type_t;
typedef struct timerinfo_tag {
	unsigned int cur_tick;
	unsigned int time;
	unsigned int timerid;
	timer_loop_type_t looptype;
	char *time_name;
} timerinfo_t;
typedef void (*timer_callback)(timerinfo_t *arg);
typedef struct _timer_register_info {
	timerinfo_t timerinfo;
	timer_callback CallBackFunction;
} timer_register_info_t;

int timer_init(void);
int timer_is_exist(timer_register_info_t *register_info);
int timer_modify_time(timer_register_info_t *register_info, int timeout);
int timer_register(timer_register_info_t *register_info);
int timer_unregister(timer_register_info_t *register_info);
#endif
