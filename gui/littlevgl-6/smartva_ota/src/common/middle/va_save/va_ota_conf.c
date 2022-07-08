#include "va_ota_conf.h"
#include "smt_config.h"
#include "cjson_config.h"

static cjson_config_t va_ota_param;

void va_ota_param_init(char *ota_config_path)
{
	memset(&va_ota_param, 0x00, sizeof(va_ota_param));
	va_ota_param.config_path = ota_config_path;
	init_cjson_config(&va_ota_param);
}

void va_ota_param_uninit(void)
{
	uninit_cjson_config(&va_ota_param);
}

int va_ota_read_int_type_param(char *major, char *minor, int *param)
{
	int ret = 0;
	if (strlen(major) == 0 || strlen(minor)==0) {
		com_warn("\n");
		ret = -1;
		goto END;
	}
	*param = 0;

	ret = read_int_type_cjson_config(&va_ota_param, major, minor, param);
END:
	return ret;
}
#if 0
int va_ota_write_int_type_param(char *major, char *minor, int param)
{
	int ret;
	if (strlen(major) == 0 || strlen(minor)==0) {
		com_warn("\n");
		ret = -1;
		goto END;
	}
	ret = write_int_type_cjson_config(&va_ota_param, major, minor, param);
END:
	return ret;
}
#endif
int va_ota_read_string_type_param(char *major, char *minor, char *str, int len)
{
	int ret;
	if(strlen(major)==0 || strlen(minor)==0 || len<=0) {
		com_warn("\n");
		ret = -1;
		goto END;
	}

	ret = read_string_type_cjson_config(&va_ota_param,  major, minor, str, len);
END:
	return ret;
}
#if 0
int va_ota_write_string_type_param(char *major, char *minor, char *str, int len)
{
	int ret;
	if(strlen(major)==0 || strlen(minor)==0 || len<=0) {
		com_warn("\n");
		ret = -1;
		goto END;
	}
	ret = write_string_type_cjson_config(&va_ota_param, major, minor, str, len);
END:
	return ret;
}
#endif
