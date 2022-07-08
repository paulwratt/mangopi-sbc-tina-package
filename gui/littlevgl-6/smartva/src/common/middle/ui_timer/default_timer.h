#ifndef __DEFAULT_TIMER_H__
#define __DEFAULT_TIMER_H__
void auto_power_off_timer_modify(int timeout);
void auto_close_screen_timer_modify(int timeout);
void auto_sleep_timer_modify(int timeout);
void reset_default_timer(void);
void default_timer_settting(void);
void auto_close_screen_timer_disable(void);
void auto_close_screen_timer_enable(void);
#endif
