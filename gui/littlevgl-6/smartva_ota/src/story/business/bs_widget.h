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
#define CONFIG_FONT_ENABLE 0
#if CONFIG_FONT_ENABLE
/* ×?ì?1üàí*/
typedef struct {
	lv_font_t *msyh_16;		/* ?￠èí??oú 16o?×?ì?*/
	lv_font_t *msyh_20;		/* ?￠èí??oú 20o?×?ì?*/
							/* ó??§°′Dèìí?ó*/
}font_lib_t;

void font_init(void);
void font_uninit(void);
/* ó??§??μ?font_lib éè??×??oDèòaμ?×?ì?*/
font_lib_t *get_font_lib(void);

#define TEXT_MAX 500	/* ó??§°′Dèìí?ó*/

/* ?àó???1üàí*/
#define LANG_FILE_CN_S	"/usr/res/font/ChineseS.bin"
#define LANG_FILE_EN	"/usr/res/font/English.bin"
#define LANG_FILE_CN_T	"/usr/res/font/zh-CN_T.bin"
#define LANG_FILE_JPN	"/usr/res/font/jpn.bin"

#if 0
typedef enum {
	HOME_START = 10,		/* home3??°?eê???±?id*/
	SETTING_START = 60,
	MOVIE_START = 113,      /* movie3??°?eê???±?id*/
	MUSIC_START = 144,		/* music3??°?eê???±?id*/
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
#endif
void list_label_set_text(lv_obj_t * list, lv_style_t *style, unsigned int text[]);
void app_param_effect(int reset);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_TEST_HBAR_H__*/
