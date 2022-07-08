#ifndef __APP_CONFIG_INTERFACE_H__
#define __APP_CONFIG_INTERFACE_H__
#include "app_config_param.h"
void va_save_param_init(void);
void va_save_param_uninit(void);
int read_int_type_param(char *major, char *minor, int *param);
int write_int_type_param(char *major, char *minor, int param);
int read_string_type_param(char *major, char *minor, char *str, int len);
int write_string_type_param(char *major, char *minor, char *str, int len);
void app_config_param_init(int reset);
#endif
