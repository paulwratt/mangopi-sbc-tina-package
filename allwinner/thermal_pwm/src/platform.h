#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdint.h>

int thermal_init(const char* path);
uint32_t thermal_temperature_get();

int pwm_init(int id, int period /* nanoseconds */);
int pwm_config_duty(int duty /* nanoseconds */);

#endif
