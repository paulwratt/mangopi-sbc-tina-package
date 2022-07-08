/**********************
 *      includes
 **********************/
#include "bs_widget.h"
#include "app_config_interface.h"
/*
************************************************************************************
*                                     background
************************************************************************************
*/
void *cur_bgd_photo = NULL;
int cur_id = 0;

int background_photo_init(void)
{
	int param;

	param = cur_id;

	if (param == 0) {
		cur_bgd_photo = (void *)parse_image_from_file(LV_IMAGE_PATH"background0.jpg");
	} else if(param == 1) {
		cur_bgd_photo = (void *)parse_image_from_file(LV_IMAGE_PATH"background1.jpg");
	} else {
		cur_bgd_photo = (void *)parse_image_from_file(LV_IMAGE_PATH"background2.jpg");
	}

	return 0;
}

int background_photo_uninit(void)
{
	if(cur_bgd_photo) {
		free_image(cur_bgd_photo);
		cur_bgd_photo = NULL;
	}
	return 0;
}

int update_background_photo(int id)
{
	if(id == cur_id) {
		return -1;
	}

	if(cur_bgd_photo) {
		free_image(cur_bgd_photo);
		cur_bgd_photo = NULL;
	}

	if(id == 0) {
		cur_bgd_photo = (void *)parse_image_from_file(LV_IMAGE_PATH"background0.jpg");
	}
	else if(id == 1) {
		cur_bgd_photo = (void *)parse_image_from_file(LV_IMAGE_PATH"background1.jpg");
	}
	else {
		cur_bgd_photo = (void *)parse_image_from_file(LV_IMAGE_PATH"background2.jpg");
	}

	cur_id = id;
	return 0;
}

void* get_background_photo(void)
{
	return cur_bgd_photo;
}


/*
************************************************************************************
*                                     key group
************************************************************************************
*/
lv_group_t *key_group = NULL;

lv_group_t* key_group_create(void)
{
	key_group = lv_group_create();
	return key_group;
}

void key_group_del()
{
	if(key_group) {
		lv_group_del(key_group);
		key_group = NULL;
	}
}

lv_group_t* get_key_group(void)
{
	return key_group;
}


/*
************************************************************************************
*                                     font & multi-language
************************************************************************************
*/
#if CONFIG_FONT_ENABLE
font_lib_t font_lib;
lang_info_t lang_info;
void font_init(void)
{
	ttf_font_init();

	memset(&font_lib, 0, sizeof(font_lib_t));
	font_lib.msyh_16 = ttf_font_create("/usr/res/font/msyh.ttf", 16, 4);
	if(font_lib.msyh_16 == NULL) {
		com_err("font_lib.msyh_16 == NULL\n");
	}

	font_lib.msyh_20 = ttf_font_create("/usr/res/font/msyh.ttf", 20, 4);
	if(font_lib.msyh_20 == NULL) {
		com_err("font_lib.msyh_20 == NULL\n");
	}
	/* 用户按照需要在这里添加字体*/
}

void font_uninit(void)
{
	if(font_lib.msyh_20)
{
		ttf_font_destory(font_lib.msyh_20);
		font_lib.msyh_20 = NULL;
	}

	if(font_lib.msyh_16)
{
		ttf_font_destory(font_lib.msyh_16);
		font_lib.msyh_16= NULL;
	}

	ttf_font_uninit();
}

font_lib_t *get_font_lib(void)
{
	return &font_lib;
}

static ssize_t  __getline(char **lineptr, ssize_t *n, FILE *stream)
{
	int count=0;
	int buf;

	if(*lineptr == NULL) {
		*n=256;
		*lineptr = (char*)malloc(*n);
	}

	if(( buf = fgetc(stream) ) == EOF ) {
		return -1;
	}

	do
	{
		if(buf=='\n') {
			count += 1;
			break;
		}

		count++;

		if(buf==';') {
			buf = '\n';
		}

		*(*lineptr+count-1) = buf;
		*(*lineptr+count) = '\0';

		if(*n <= count)
		{
			com_err("count=%d over *n=%d\n", count, (int)*n);
			return -1;
		}

		buf = fgetc(stream);
	} while( buf != EOF);

	return count;
}

int load_text_from_file(char *langFile)
{
	int index = 0;
	FILE *fp = NULL;
	char *line = NULL;
	ssize_t len = 0, total_len = 0, rel_len = 0, offset = 0;

	fp = fopen(langFile, "r");
	if (fp == NULL) {
		com_err("open file %s failed\n", langFile);
		return -1;
	}

	/*
	lang_info.text[i] = (char *)malloc(strlen(langFile)+1);
	if(lang_info.text[i] == NULL) {
		com_err("malloc fail\n");
	}
	memcpy((void *)lang_info.text[i], (void *)langFile, strlen(langFile)+1);
	com_err("lang_info.text[%d] = %s",i,lang_info.text[i]);
	i++;
	*/
	while ((rel_len = __getline(&line, &len, fp)) != -1)
	{
		total_len += rel_len;
	}
	printf("%s %d %s total_len:%d\n", __FILE__, __LINE__, __func__, total_len);
	lang_info.text_buff = (char *)malloc(total_len*sizeof(char));
	if (lang_info.text_buff == NULL) {
		com_err("malloc fail\n");
		goto out;
	}
	index = 0;
	while ((rel_len = __getline(&line, &len, fp)) != -1)
	{
		lang_info.text[index] = lang_info.text_buff + offset;
		memcpy((void *)lang_info.text[index], (void *)line, rel_len);
		lang_info.text[index][rel_len-1] = '\0';
		index++;
		offset += rel_len;
	}
out:
	free(line);
	fclose(fp);
	return 0;
}

void list_label_set_text(lv_obj_t * list, lv_style_t *style, unsigned int text[])
{

	lv_obj_t *list_btn[16];
	lv_obj_t *list_label[16];
	lv_font_t *font;
	unsigned int i = 0;
	const lv_style_t *style_get = NULL;//fix warnning
	list_btn[0] = lv_list_get_next_btn(list, NULL);
	font = style->text.font;
	while(list_btn[i])
	{
		list_label[i] = lv_list_get_btn_label(list_btn[i]);
		if(i == 0){
			style_get = lv_label_get_style(list_label[i],LV_LABEL_STYLE_MAIN);
			memcpy(style, style_get, sizeof(lv_style_t));
			style->text.font = font;
		}
		lv_label_set_style(list_label[i], LV_LABEL_STYLE_MAIN, style);
		lv_label_set_long_mode(list_label[i],LV_LABEL_LONG_EXPAND);
		lv_label_set_text(list_label[i], get_text_by_id(text[i]));
		com_info("list_label[%d] == %s",i,get_text_by_id(text[i]));
		i++;
		list_btn[i] = lv_list_get_next_btn(list, list_btn[i-1]);
	}

}

static int init_text(void)
{
	char *dataFile = NULL;
	if(lang_info.lang == LANG_ERR) {
		com_err("lang is no initialized\n");
		return -1;
	}

	if(lang_info.lang == LANG_CN_S) {
		dataFile = LANG_FILE_CN_S;
	} else if(lang_info.lang == LANG_EN) {
		dataFile = LANG_FILE_EN;
	} else if(lang_info.lang == LANG_CN_S) {
		dataFile = LANG_FILE_CN_T;
	} else if(lang_info.lang == LANG_JPN) {
		dataFile = LANG_FILE_JPN;
	} else {
		com_err("invalid lang %d\n", lang_info.lang);
		return -1;
	}
	com_info("lang=%d, dataFile=%s\n",lang_info.lang, dataFile);
	if(load_text_from_file(dataFile) < 0) {
		com_err("load label from %s failed\n", dataFile);
		return -1;
	}

	return 0;
}

static void uninit_text(void)
{
	int i;

	for (i=0; i < TEXT_MAX; i++)
	{
		if(lang_info.text[i] != NULL)
		{
			lang_info.text[i] = NULL;
		}
	}
	free(lang_info.text_buff);
	lang_info.text_buff = NULL;
}

void lang_and_text_init(language_t lang)
{
	lang_info.lang = lang;
	init_text();
}

void lang_and_text_uninit(void)
{
	uninit_text();
	lang_info.lang = LANG_ERR;
}

void lang_and_text_update(language_t lang)
{
    language_t newLang = LANG_ERR;

        newLang = lang;
        if(newLang == LANG_ERR){
            com_err("invalide lang=%d\n", lang);
        return;
    }

    if(newLang != lang_info.lang) {
        com_info("%d > %d\n", lang_info.lang, newLang);
        lang_info.lang = newLang;
    }

    uninit_text();
    init_text();
}


language_t get_language(void)
{
	return lang_info.lang;
}

/*
function: get language text

example:
\res\lang\zh-CN.bin下内容为
10 ===== 主界面=====
11 电影
12 音乐
13 相片
5 电影
getLabel(11) = "电影"
getLabel(12) = "音乐"
*/
const char* get_text_by_id(int id)
{
	if(id < TEXT_MAX) {
		return (const char*)lang_info.text[id];
	}
	else {
		com_err("invalide id: %d, size is %d\n", (int)id, TEXT_MAX);
		return NULL;
	}
}
#endif
/*
************************************************************************************
*                                     app set
************************************************************************************
*/
void app_param_effect(int reset)
{
	int param;
	/* lcd setting */
	read_int_type_param(PUBLIC_SCENE, BACKLIGHT, &param);
	va_display_lcd_set_backlight(param);
	va_display_lcd_backlight_onoff(1);
	read_int_type_param(PUBLIC_SCENE, ENHANCE_MODE, &param);
	va_display_set_enhance_mode(param);
	if(param){
		read_int_type_param(PUBLIC_SCENE, ENHANCE_CONTRAST, &param);
		va_display_enhance_set_contrast(param);
		read_int_type_param(PUBLIC_SCENE, ENHANCE_STATUTION, &param);
		va_display_enhance_set_saturation(param);
		read_int_type_param(PUBLIC_SCENE, ENHANCE_BRIGHT, &param);
		va_display_enhance_set_bright(param);
	}else{
		va_display_enhance_set_contrast(DEFAULT_ENHANCE_CONTRAST);
		va_display_enhance_set_saturation(DEFAULT_ENHANCE_STATUTION);
		va_display_enhance_set_bright(DEFAULT_ENHANCE_BRIGHT);
	}

	/* language */
#if CONFIG_FONT_ENABLE
	read_int_type_param(PUBLIC_SCENE, LANGUAGE, &param);
	if(!reset){
		lang_and_text_init((language_t)param);
	}else{
		lang_and_text_update((language_t)param);
	}
#endif
}
