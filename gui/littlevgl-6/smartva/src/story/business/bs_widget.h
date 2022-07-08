#ifndef __BS_WIDGET_H__
#define __BS_WIDGET_H__

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *      includes
 **********************/
#include "lvgl.h"
#include "common.h"
#include "moc_wlan_set1.h"
/*
************************************************************************************
*                                     background
************************************************************************************
*/
int background_photo_init(void);
int background_photo_uninit(void);
int update_background_photo(int id);
void* get_background_photo(void);


/*
************************************************************************************
*                                     key group
************************************************************************************
*/
lv_group_t* key_group_create(void);
void key_group_del();
lv_group_t* get_key_group(void);


/*
************************************************************************************
*                                     font & multi-language
************************************************************************************
*/
#define CONFIG_FONT_ENABLE 1

/* 字体管理*/
typedef struct {
	lv_font_t *msyh_16;		/* 微软雅黑 16号字体*/
	lv_font_t *msyh_20;		/* 微软雅黑 20号字体*/
							/* 用户按需添加*/
}font_lib_t;

void font_init(void);
void font_uninit(void);
/* 用户拿到font_lib 设置自己需要的字体*/
font_lib_t *get_font_lib(void);

#define TEXT_MAX 500	/* 用户按需添加*/

/* 多语言管理*/
#define LANG_FILE_CN_S	"/usr/res/font/ChineseS.bin"
#define LANG_FILE_EN	"/usr/res/font/English.bin"
#define LANG_FILE_CN_T	"/usr/res/font/zh-CN_T.bin"
#define LANG_FILE_JPN	"/usr/res/font/jpn.bin"

#if 0
typedef enum {
	HOME_START = 10,		/* home场景起始文本id*/
	SETTING_START = 60,
	MOVIE_START = 113,      /* movie场景起始文本id*/
	MUSIC_START = 144,		/* music场景起始文本id*/
	TEXT_MAX = 500,
}lang_text_id;
#endif
typedef enum {
	LANG_CN_S = 0,		/* 中文简体*/
	LANG_EN = 1,		/* 英文*/
	LANG_CN_T = 2,		/* 中文繁体*/
	LANG_JPN = 3,		/* 日语*/
	LANG_ERR			/* 用户按需添加*/
}language_t;

typedef struct {
	language_t lang;
	char *text_buff;
	char *text[TEXT_MAX];
}lang_info_t;

void lang_and_text_init(language_t lang);
void lang_and_text_uninit(void);
void lang_and_text_update(language_t lang);
language_t get_language(void);
const char* get_text_by_id(int id);
void list_label_set_text(lv_obj_t * list, lv_style_t *style, unsigned int text[]);

/*
************************************************************************************
*                                      volume bar ctl
************************************************************************************
*/
int volume_ctl_hide(void);
int volume_ctl(char cmd);
int volume_ctl_destory(void);
int volume_ctl_create(void);

/*
************************************************************************************
*                                     param save
************************************************************************************
*/
int sleep_listid_to_time(int id);
/*
************************************************************************************
*                                     net
************************************************************************************
*/
typedef struct {
	aw_wifi_interface_t *p_wifi_hd;
	int event_label;
	int update_flag;

	bool manu_on;	/* 1:on; 0:off */
	bool manu_connected;	/* 1:yes; 0:no */
	char manu_ssid[WIFI_MAX_SSID_SIZE];		/* ssid */
	char manu_password[WIFI_MAX_PASSWORD_SIZE];		/* password */

	bool is_on;	/* 1:on; 0:off */
	bool is_connected;	/* 1:yes; 0:no */
	char ssid[WIFI_MAX_SSID_SIZE];		/* ssid */
	char password[WIFI_MAX_PASSWORD_SIZE];		/* password */
	char mac_address[WIFI_MAX_BSSID_SIZE];		/* MAC address */
	int rssi;	/* level 0-100 */
} wifi_data_t;

void wifi_data_init(void);
void wifi_data_uninit(void);
void set_wifi_data(wifi_data_t *wifi);
void get_wifi_data(wifi_data_t *wifi);
/*
************************************************************************************
*                                     app set
************************************************************************************
*/
void app_param_effect(int reset);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_TEST_HBAR_H__*/
