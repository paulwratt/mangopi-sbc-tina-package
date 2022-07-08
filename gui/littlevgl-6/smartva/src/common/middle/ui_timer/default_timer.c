#include "ui_timer.h"
#include "default_timer.h"
#include "common.h"
#include "smt_config.h"
#include "app_config_interface.h"

#define AUTO_POWER_OFF_TIMER_ID	(0xff00)
#define AUTO_CLOSE_SCREEN_TIMER_ID	(0xff01)
#define AUTO_SLEEP_TIMER_ID	(0xff02)

enum {
	DEFAULT_TIMER_AUTO_POWEROFF = 0,
	DEFAULT_TIMER_AUTO_CLOSE_SCREEN,
	DEFAULT_TIMER_AUTO_SLEEP,
	DEFAULT_TIMER_NUM
};
static timer_register_info_t default_timer[DEFAULT_TIMER_NUM];

static void auto_power_off_timer_callback(timerinfo_t *arg) {
	system("poweroff");
}

static void auto_sleep_callback(timerinfo_t *arg) {
	system("echo mem > /sys/power/state");
}

static void auto_close_screen_callback(timerinfo_t *arg) {
	if (va_display_lcd_backlight_status() == 1) {
		va_display_lcd_backlight_onoff(0);
	}
}

void auto_power_off_timer_modify(int timeout) {
	timer_modify_time(&default_timer[DEFAULT_TIMER_AUTO_SLEEP], timeout);
}

void auto_close_screen_timer_modify(int timeout) {
	timer_modify_time(&default_timer[DEFAULT_TIMER_AUTO_CLOSE_SCREEN], timeout);
}

void auto_sleep_timer_modify(int timeout) {
	timer_modify_time(&default_timer[DEFAULT_TIMER_AUTO_SLEEP], timeout);
}

void reset_default_timer(void) {
	int index = 0;
	for (index = 0; index < DEFAULT_TIMER_NUM; index++) {
		if (timer_is_exist(&default_timer[index])) {
//			printf("%s %d time_name:%s time:%d\n", __FUNCTION__, __LINE__, default_timer[index].timerinfo.time_name,  default_timer[index].timerinfo.time);
			timer_modify_time(&default_timer[index], default_timer[index].timerinfo.time);
		}
	}
}

void auto_close_screen_timer_disable(void) {
	timer_unregister(&default_timer[DEFAULT_TIMER_AUTO_CLOSE_SCREEN]);
}

void auto_close_screen_timer_enable(void) {
	timer_modify_time(&default_timer[DEFAULT_TIMER_AUTO_CLOSE_SCREEN], default_timer[DEFAULT_TIMER_AUTO_CLOSE_SCREEN].timerinfo.time);
}

void default_timer_settting(void) {
	int index = 0;
	default_timer[DEFAULT_TIMER_AUTO_POWEROFF].CallBackFunction = auto_power_off_timer_callback;
	default_timer[DEFAULT_TIMER_AUTO_POWEROFF].timerinfo.looptype = TIMER_LOOP_ALWAYS;
	default_timer[DEFAULT_TIMER_AUTO_POWEROFF].timerinfo.timerid = AUTO_POWER_OFF_TIMER_ID;
	default_timer[DEFAULT_TIMER_AUTO_POWEROFF].timerinfo.time_name = AUTO_POWEROFF;


	default_timer[DEFAULT_TIMER_AUTO_CLOSE_SCREEN].CallBackFunction = auto_close_screen_callback;
	default_timer[DEFAULT_TIMER_AUTO_CLOSE_SCREEN].timerinfo.looptype = TIMER_LOOP_ALWAYS;
	default_timer[DEFAULT_TIMER_AUTO_CLOSE_SCREEN].timerinfo.timerid = AUTO_CLOSE_SCREEN_TIMER_ID;
	default_timer[DEFAULT_TIMER_AUTO_CLOSE_SCREEN].timerinfo.time_name = AUTO_CLOSE_SCREEN;

	default_timer[DEFAULT_TIMER_AUTO_SLEEP].CallBackFunction = auto_sleep_callback;
	default_timer[DEFAULT_TIMER_AUTO_SLEEP].timerinfo.looptype = TIMER_LOOP_ALWAYS;
	default_timer[DEFAULT_TIMER_AUTO_SLEEP].timerinfo.timerid = AUTO_SLEEP_TIMER_ID;
	default_timer[DEFAULT_TIMER_AUTO_SLEEP].timerinfo.time_name = AUTO_SLEEP;

	for (index = 0; index < DEFAULT_TIMER_NUM; index++) {
		int ret = -1, time = 0;
		switch (index) {
			case DEFAULT_TIMER_AUTO_POWEROFF: {
				ret = read_int_type_param(PUBLIC_SCENE, AUTO_POWEROFF, &time);
				if(ret == -1) {
					write_int_type_param(PUBLIC_SCENE, AUTO_POWEROFF, time);
				}
			} break;

			case DEFAULT_TIMER_AUTO_CLOSE_SCREEN: {
				ret = read_int_type_param(PUBLIC_SCENE, AUTO_CLOSE_SCREEN, &time);
				if(ret == -1) {
					write_int_type_param(PUBLIC_SCENE, AUTO_CLOSE_SCREEN, time);
				}
			} break;

			case DEFAULT_TIMER_AUTO_SLEEP:
			default: {
				ret = read_int_type_param(PUBLIC_SCENE, AUTO_SLEEP, &time);
				if(ret == -1) {
					write_int_type_param(PUBLIC_SCENE, AUTO_SLEEP, time);
				}
			} break;
		}
		time = sleep_listid_to_time(time);
//		printf("%s %d %s time:%d\n", __FILE__, __LINE__, __func__, time);
		default_timer[index].timerinfo.time = time;
		timer_modify_time(&default_timer[index], default_timer[index].timerinfo.time);
	}

	return ;
}
