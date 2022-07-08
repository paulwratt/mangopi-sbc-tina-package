#include "app_config_interface.h"
#include "cjson_config.h"

typedef enum {
	CONFIG_PARAM_TYPE_INIT = 0,
	CONFIG_PARAM_TYPE_STRING,
} config_param_type_t;

typedef struct __config_param_tag {
	char				*major;
	char				*minor;
	config_param_type_t type;
	int				val;
	char				*str_config;
} config_param_t;

config_param_t config_param[] = {
	/*PUBLIC_SCENE*/
	{PUBLIC_SCENE, VOLUME, CONFIG_PARAM_TYPE_INIT, DEFAULT_VOLUME, NULL},
	{PUBLIC_SCENE, AUTO_SLEEP, CONFIG_PARAM_TYPE_INIT, DEFAULT_AUTO_SLEEP, NULL},
	{PUBLIC_SCENE, AUTO_CLOSE_SCREEN, CONFIG_PARAM_TYPE_INIT, DEFAULT_CLOSE_SCREEN, NULL},
	{PUBLIC_SCENE, AUTO_POWEROFF, CONFIG_PARAM_TYPE_INIT, DEFAULT_AUTO_POWEROFF, NULL},
	{PUBLIC_SCENE, BACKLIGHT, CONFIG_PARAM_TYPE_INIT, DEFAULT_BACKLIGHT, NULL},
	{PUBLIC_SCENE, ENHANCE_MODE, CONFIG_PARAM_TYPE_INIT, DEFAULT_ENHANCE_MODE, NULL},
	{PUBLIC_SCENE, ENHANCE_BRIGHT, CONFIG_PARAM_TYPE_INIT, DEFAULT_ENHANCE_BRIGHT, NULL},
	{PUBLIC_SCENE, ENHANCE_STATUTION, CONFIG_PARAM_TYPE_INIT, DEFAULT_ENHANCE_STATUTION, NULL},
	{PUBLIC_SCENE, ENHANCE_CONTRAST, CONFIG_PARAM_TYPE_INIT, DEFAULT_ENHANCE_CONTRAST, NULL},
	{PUBLIC_SCENE, LANGUAGE, CONFIG_PARAM_TYPE_INIT, DEFAULT_LANGUAGE, NULL},

	/*MUSIC_SCENE*/
	{MUSIC_SCENE, LOOP, CONFIG_PARAM_TYPE_INIT, DEFAULT_LOOP, NULL},
	{MUSIC_SCENE, PLAYMODE, CONFIG_PARAM_TYPE_INIT, DEFAULT_PLAYMODE, NULL},
	{MUSIC_SCENE, PATH, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_PATH},
	{MUSIC_SCENE, FILENAME, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_FILENAME},
	{MUSIC_SCENE, FILETYPE, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_FILETYPE},
	{MUSIC_SCENE, OFFSET, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_OFFSET},
	{MUSIC_SCENE, INDEX, CONFIG_PARAM_TYPE_INIT, DEFAULT_INDEX, NULL},
	{MUSIC_SCENE, DURATION, CONFIG_PARAM_TYPE_INIT, DEFAULT_DURATION, NULL},

	/*MOVIE_SCENE*/
	{MOVIE_SCENE, LOOP, CONFIG_PARAM_TYPE_INIT, DEFAULT_LOOP, NULL},
	{MOVIE_SCENE, PLAYMODE, CONFIG_PARAM_TYPE_INIT, DEFAULT_PLAYMODE, NULL},
	{MOVIE_SCENE, PATH, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_PATH},
	{MOVIE_SCENE, FILENAME, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_FILENAME},
	{MOVIE_SCENE, FILETYPE, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_FILETYPE},
	{MOVIE_SCENE, OFFSET, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_OFFSET},
	{MOVIE_SCENE, INDEX, CONFIG_PARAM_TYPE_INIT, DEFAULT_INDEX, NULL},
	{MOVIE_SCENE, DURATION, CONFIG_PARAM_TYPE_INIT, DEFAULT_DURATION, NULL},

	/*EXT_MUSIC_SCENE*/
	{EXT_MUSIC_SCENE, LOOP, CONFIG_PARAM_TYPE_INIT, DEFAULT_LOOP, NULL},
	{EXT_MUSIC_SCENE, PLAYMODE, CONFIG_PARAM_TYPE_INIT, DEFAULT_PLAYMODE, NULL},
	{EXT_MUSIC_SCENE, PATH, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_PATH},
	{EXT_MUSIC_SCENE, FILENAME, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_FILENAME},
	{EXT_MUSIC_SCENE, FILETYPE, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_FILETYPE},
	{EXT_MUSIC_SCENE, OFFSET, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_OFFSET},
	{EXT_MUSIC_SCENE, INDEX, CONFIG_PARAM_TYPE_INIT, DEFAULT_INDEX, NULL},
	{EXT_MUSIC_SCENE, DURATION, CONFIG_PARAM_TYPE_INIT, DEFAULT_DURATION, NULL},

	/*WLAN_SCENE*/
	{WLAN_SCENE, WLAN_MANU_ON, CONFIG_PARAM_TYPE_INIT, DEFAULT_WLAN_MANU_ON, NULL},
	{WLAN_SCENE, WLAN_MANU_CONNECTED, CONFIG_PARAM_TYPE_INIT, DEFAULT_MANU_CONNECTED, NULL},
	{WLAN_SCENE, WLAN_MANU_SSID, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_MANU_SSID},
	{WLAN_SCENE, WLAN_MANU_PASSWORD, CONFIG_PARAM_TYPE_STRING, 0, DEFAULT_MANU_PASSWORD},
};

static cjson_config_t va_save_param;

void va_save_param_init(void)
{
	memset(&va_save_param, 0x00, sizeof(va_save_param));
	va_save_param.config_path = VA_SAVE_PATH;
	init_cjson_config(&va_save_param);
}

void va_save_param_uninit(void)
{
	uninit_cjson_config(&va_save_param);
}

int read_int_type_param(char *major, char *minor, int *param)
{
	int ret = 0;
	if (strlen(major) == 0 || strlen(minor)==0) {
		com_warn("\n");
		ret = -1;
		goto END;
	}
	*param = 0;

	ret = read_int_type_cjson_config(&va_save_param, major, minor, param);
END:
	return ret;
}

int write_int_type_param(char *major, char *minor, int param)
{
	int ret;
	if (strlen(major) == 0 || strlen(minor)==0) {
		com_warn("\n");
		ret = -1;
		goto END;
	}
	ret = write_int_type_cjson_config(&va_save_param, major, minor, param);
END:
	return ret;
}

int read_string_type_param(char *major, char *minor, char *str, int len)
{
	int ret;
	if(strlen(major)==0 || strlen(minor)==0 || len<=0) {
		com_warn("\n");
		ret = -1;
		goto END;
	}

	ret = read_string_type_cjson_config(&va_save_param,  major, minor, str, len);
END:
	return ret;
}

int write_string_type_param(char *major, char *minor, char *str, int len)
{
	int ret;
	if(strlen(major)==0 || strlen(minor)==0 || len<=0) {
		com_warn("major:%s, minor:%s\n", major, minor);
		ret = -1;
		goto END;
	}
	ret = write_string_type_cjson_config(&va_save_param, major, minor, str, len);
END:
	return ret;
}

void app_config_param_init(int reset)
{
	int index = 0;

	if (!reset) {
		va_save_param_init();
	}

	if (reset || access(VA_SAVE_PATH, F_OK) != 0) {
		for (index = 0; index < sizeof(config_param)/sizeof(config_param[0]); index++) {
			if (config_param[index].type == CONFIG_PARAM_TYPE_INIT) {
				write_int_type_param(config_param[index].major, config_param[index].minor, config_param[index].val);
			} else if (config_param[index].type == CONFIG_PARAM_TYPE_STRING) {
				int len = 0;
				if (config_param[index].str_config != NULL) {
					len = strlen(config_param[index].str_config);
				}
				write_string_type_param(config_param[index].major, config_param[index].minor, config_param[index].str_config, len);
			}
		}
	}
}
