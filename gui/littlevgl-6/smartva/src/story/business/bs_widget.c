/**********************
 *      includes
 **********************/
#include "bs_widget.h"
#include "default_timer.h"
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
	/* ÓÃ»§°´ÕÕÐèÒªÔÚÕâÀïÌí¼Ó×ÖÌå*/
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
	while ((rel_len = __getline(&line, &len, fp)) != -1) {
		total_len += rel_len;
	}

	fseek(fp, 0, SEEK_SET);
	lang_info.text_buff = (char *)malloc(total_len*sizeof(char));
	if (lang_info.text_buff == NULL) {
		com_err("malloc fail\n");
		goto out;
	}
	index = 0;
	while ((rel_len = __getline(&line, &len, fp)) != -1) {
		lang_info.text[index] = lang_info.text_buff + offset;
		memcpy((void *)lang_info.text[index], (void *)line, rel_len);
		*(lang_info.text[index] + (rel_len - 1)) = '\0';
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

	unsigned int i = 0;
	const lv_font_t *font = NULL;
	const lv_style_t *style_get = NULL;//fix warnning
	lv_obj_t *list_btn = NULL, *list_label = NULL;

	list_btn = lv_list_get_next_btn(list, NULL);
	font = style->text.font;
	while (list_btn) {
		list_label = lv_list_get_btn_label(list_btn);

		if (i == 0) {
			style_get = lv_label_get_style(list_label, LV_LABEL_STYLE_MAIN);
			memcpy(style, style_get, sizeof(lv_style_t));
			style->text.font = font;
		}
		lv_label_set_style(list_label, LV_LABEL_STYLE_MAIN, style);
		lv_label_set_long_mode(list_label, LV_LABEL_LONG_EXPAND);
		lv_label_set_text(list_label, get_text_by_id(text[i]));
		com_info("list_label == %s", get_text_by_id(text[i]));
		i++;
		list_btn = lv_list_get_next_btn(list, list_btn);
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
\res\lang\zh-CN.binÏÂÄÚÈÝÎª
10 ===== Ö÷½çÃæ=====
11 µçÓ°
12 ÒôÀÖ
13 ÏàÆ¬
5 µçÓ°
getLabel(11) = "µçÓ°"
getLabel(12) = "ÒôÀÖ"
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


/*
************************************************************************************
*                                     volume bar ctl
************************************************************************************
*/
#define VOLUME_BAR_SHOW_TIME 50
static int volume_ctl_hide_time = 0;
static lv_style_t style0_slider_volume;
static lv_obj_t *slider_volume;
static lv_style_t style0_label_volume_num;
static lv_obj_t *label_volume_num;

int volume_ctl_create(void)
{
#ifdef LV_USE_SLIDER
	lv_style_copy(&style0_slider_volume, &lv_style_pretty);
	slider_volume = lv_slider_create(lv_layer_sys(), NULL);
	lv_obj_set_pos(slider_volume, 300, 30);
	lv_obj_set_size(slider_volume, 200, 26);
	lv_slider_set_range(slider_volume, 0, 100);
	lv_slider_set_knob_in(slider_volume, false);
	lv_slider_set_value(slider_volume, 0, LV_ANIM_OFF);
	lv_slider_set_style(slider_volume, LV_SLIDER_STYLE_BG, &style0_slider_volume);
	lv_obj_set_hidden(slider_volume, true);
#endif // LV_USE_SLIDER

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_volume_num, &lv_style_transp);
	label_volume_num = lv_label_create(lv_layer_sys(), NULL);
	lv_label_set_text(label_volume_num, "00");
	lv_label_set_long_mode(label_volume_num, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(label_volume_num, 260, 30);
	lv_obj_set_size(label_volume_num, 31, 19);
	lv_label_set_style(label_volume_num, LV_LABEL_STYLE_MAIN, &style0_label_volume_num);
	lv_obj_set_hidden(label_volume_num, true);
#endif // LV_USE_LABEL
	lv_obj_set_click(slider_volume, false);
	return 0;
}

int volume_ctl_destory(void)
{
	volume_ctl_hide_time = 0;
	lv_obj_clean(slider_volume);
	lv_obj_clean(label_volume_num);
	return 0;
}

int volume_ctl(char cmd)
{
	int volume;
	char str[10];

	read_int_type_param(PUBLIC_SCENE, VOLUME, &volume);
	lv_obj_set_hidden(label_volume_num, false);
	lv_obj_set_hidden(slider_volume, false);
	volume_ctl_hide_time = VOLUME_BAR_SHOW_TIME;
	com_info("volume = %d",volume);
	if(cmd == '+')
	{
		if(volume >= 100)
			return -1;
		volume++;
	}
	else if(cmd == '-')
	{
		if(volume <= 0)
			return -1;
		volume--;
	}
	else
	{
		com_err("volume cmd err!");
	}
	sprintf(str, "%d%%", volume);
	lv_label_set_text(label_volume_num, str);
	lv_slider_set_value(slider_volume, volume, LV_ANIM_ON);
	va_audio_play_set_volume_value((unsigned int)volume, (unsigned int)volume);
	write_int_type_param(PUBLIC_SCENE, VOLUME, (int)volume);
	return 0;
}

int volume_ctl_hide(void)
{
	if(volume_ctl_hide_time > 0)
	{
		volume_ctl_hide_time--;
	}
	else
	{
		lv_obj_set_hidden(label_volume_num, true);
		lv_obj_set_hidden(slider_volume, true);
	}
	return 0;
}

/*
************************************************************************************
*                                     param save
************************************************************************************
*/
int sleep_listid_to_time(int id)
{
	int sleep_timeout = -1;

	switch(id)
	{
		case 0:
		sleep_timeout = 10;
		break;

		case 1:
		sleep_timeout = 20;
		break;

		case 2:
		sleep_timeout = 30;
		break;

		case 3:
		sleep_timeout = 60;
		break;

		case 4:
		sleep_timeout = 120;
		break;

		case 5:
		sleep_timeout = -1;
		break;

		default:
		sleep_timeout = -1;
		break;
	}
	return sleep_timeout;
}
/*
************************************************************************************
*                                     net
************************************************************************************
*/
wifi_data_t wifi_data = {0};
static pthread_mutex_t wifi_data_mutex;

void wifi_data_init(void)
{
	memset(&wifi_data, 0, sizeof(wifi_data_t));
	pthread_mutex_init(&wifi_data_mutex, NULL);
}

void wifi_data_uninit(void)
{
	memset(&wifi_data, 0, sizeof(wifi_data_t));
	wifi_pic_res_uninit();
	pthread_mutex_destroy(&wifi_data_mutex);
}

void set_wifi_data(wifi_data_t *wifi)
{
	pthread_mutex_lock(&wifi_data_mutex);
	memcpy((void *)&wifi_data, (void *)wifi, sizeof(wifi_data_t));
	pthread_mutex_unlock(&wifi_data_mutex);
}

void get_wifi_data(wifi_data_t *wifi)
{
	pthread_mutex_lock(&wifi_data_mutex);
	memcpy((void *)wifi, (void *)&wifi_data, sizeof(wifi_data_t));
	pthread_mutex_unlock(&wifi_data_mutex);
}

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
	/* audio */
	read_int_type_param(PUBLIC_SCENE, VOLUME, &param);
	va_audio_play_set_volume_value((unsigned int)param, (unsigned int)param);
	/* timer */
	default_timer_settting();
}
