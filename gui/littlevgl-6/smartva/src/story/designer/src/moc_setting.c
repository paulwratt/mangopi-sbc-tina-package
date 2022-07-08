/**********************
 *      includes
 **********************/
#include "moc_setting.h"
#include "ui_setting.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "bs_widget.h"
#include "app_config_interface.h"
#include "default_timer.h"

/**********************
 *       variables
 **********************/
enum {
	DROP_DOWN_AUTO_CLOSE_SCREEN = 0,
	DROP_DOWN_AUTO_SLEEP,
	DROP_DOWN_AUTO_POWEROFF,
	DROP_DOWN_LIST_NUM
};
typedef struct _drop_down_list_tag {
	lv_obj_t *label;
	lv_point_t label_pos;
	lv_coord_t label_w;
	lv_coord_t label_h;
	lv_obj_t *drop_down;
	lv_point_t drop_down_pos;
	lv_coord_t drop_down_w;
	lv_coord_t drop_down_h;
} drop_down_list_t;

typedef struct
{
	setting_ui_t ui;
	lv_obj_t *language_cont;
	lv_obj_t *sleep_cont;
	lv_obj_t *drop_down_list_language;
	drop_down_list_t drop_down_list[DROP_DOWN_LIST_NUM];
} setting_para_t;
static setting_para_t para;
static void *time_pic[10] = {0};
static struct tm set_time;

static lv_task_t *dialog_task = NULL;
static int dialog_show_time;
static void configure_display(void);
static int create_setting(void);
static int destory_setting(void);
static int list_keygroup_add(const lv_obj_t * list, lv_group_t *g);
static int menu_level;
static lv_obj_t * fc_list_btn;
static lv_obj_t * fc_list_cont;

#if CONFIG_FONT_ENABLE
static unsigned int list_txt[] = {
	LANG_SET_GENERAL,//general
	LANG_SET_DISPLAY,//display
	LANG_SET_LANG,//Language
	LANG_SET_INFORMATION,//information
	LANG_SET_WALLPAPER,//Wallpaper
	LANG_SET_PASSWORD,//Password
	LANG_SET_TIME,//Time
	LANG_SET_DEFAULT//Default setting
};
#endif
/**********************
 *  functions
 **********************/
static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		switch_page(PAGE_SETTING, PAGE_HOME);
	}
}

static void language_event(lv_obj_t * btn, lv_event_t event)
{
	uint16_t index;
	int old_index;

	com_info("event=%d\n", event);
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		index = lv_ddlist_get_selected(btn);
		read_int_type_param(PUBLIC_SCENE, LANGUAGE, &old_index);
		if(old_index == index)
		return;

		#if CONFIG_FONT_ENABLE
		lang_and_text_update(index);
		write_int_type_param(PUBLIC_SCENE, LANGUAGE, (int)index);
		switch_page(PAGE_SETTING, PAGE_SETTING);
		#endif
	}
}

static void reset_setting_event(struct _lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		com_info("reset setting ...");
		dialog_show_time = 2;//2s
		#if CONFIG_FONT_ENABLE
		lv_label_set_text(para.ui.label_dialog, get_text_by_id(LANG_SET_DEFAULT_ING));
		#endif
		lv_obj_set_hidden(para.ui.cont_dialog, false);
		app_config_param_init(1);
		app_param_effect(1);
		switch_page(PAGE_SETTING, PAGE_SETTING);
	}
}

static void hide_enhance_item(bool en)
{
	if(en == true){//disable enhance
		com_info("disable enhance ...");
		va_display_enhance_set_bright(DEFAULT_ENHANCE_BRIGHT);
		va_display_enhance_set_contrast(DEFAULT_ENHANCE_CONTRAST);
		va_display_enhance_set_saturation(DEFAULT_ENHANCE_STATUTION);
	}else{//enable enhance
		int param;
		com_info("enable enhance ...");
		read_int_type_param(PUBLIC_SCENE, ENHANCE_CONTRAST, &param);
		va_display_enhance_set_contrast(param);
		read_int_type_param(PUBLIC_SCENE, ENHANCE_STATUTION, &param);
		va_display_enhance_set_saturation(param);
		read_int_type_param(PUBLIC_SCENE, ENHANCE_BRIGHT, &param);
		va_display_enhance_set_bright(param);
	}
	lv_obj_set_hidden(para.ui.slider_saturation, en);
	lv_obj_set_hidden(para.ui.slider_contrast, en);
	lv_obj_set_hidden(para.ui.slider_bright, en);
	lv_obj_set_hidden(para.ui.label_num_contrast, en);
	lv_obj_set_hidden(para.ui.label_num_saturation, en);
	lv_obj_set_hidden(para.ui.label_num_bright, en);
	lv_obj_set_hidden(para.ui.label_saturation, en);
	lv_obj_set_hidden(para.ui.label_contrast, en);
	lv_obj_set_hidden(para.ui.label_bright, en);
}


static void button_backlight_cb(struct _lv_obj_t * obj, lv_event_t event)
{
	bool backlight_status;
	com_info("event = %d",event);
	if (event == LV_EVENT_CLICKED)
	{
		backlight_status = va_display_lcd_backlight_status();
		backlight_status ^= 1;
		while(lv_disp_get_inactive_time(NULL) <= 200)
			usleep(1000);
		va_display_lcd_backlight_onoff(backlight_status);
	}
}

static void backlight_cb(struct _lv_obj_t * obj, lv_event_t event)
{
	int16_t backlight;
	char str[8];
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		backlight = lv_slider_get_value(obj);
		va_display_lcd_set_backlight(backlight);
		sprintf(str, "%d%%", backlight);
		lv_label_set_text(para.ui.label_num_backlight, str);
		write_int_type_param(PUBLIC_SCENE, BACKLIGHT, backlight);
	}
}

static void contrast_cb(struct _lv_obj_t * obj, lv_event_t event)
{
	int16_t contrast;
	char str[8];
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		contrast = lv_slider_get_value(obj);
		if(lv_sw_get_state(para.ui.switch_enhance))
		va_display_enhance_set_contrast(contrast);
		sprintf(str, "%d%%", contrast);
		lv_label_set_text(para.ui.label_num_contrast, str);
		write_int_type_param(PUBLIC_SCENE, ENHANCE_CONTRAST, contrast);
	}
}

static void saturation_cb(struct _lv_obj_t * obj, lv_event_t event)
{
	int16_t saturation;
	char str[8];
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		saturation = lv_slider_get_value(obj);
		if(lv_sw_get_state(para.ui.switch_enhance))
		va_display_enhance_set_saturation(saturation);
		sprintf(str, "%d%%", saturation);
		lv_label_set_text(para.ui.label_num_saturation, str);
		write_int_type_param(PUBLIC_SCENE, ENHANCE_STATUTION, saturation);
	}
}

static void bright_cb(struct _lv_obj_t * obj, lv_event_t event)
{
	int16_t bright;
	char str[8];
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		bright = lv_slider_get_value(obj);
		if(lv_sw_get_state(para.ui.switch_enhance))
		va_display_enhance_set_bright(bright);
		sprintf(str, "%d%%", bright);
		lv_label_set_text(para.ui.label_num_bright, str);
		write_int_type_param(PUBLIC_SCENE, ENHANCE_BRIGHT, bright);
	}
}

static void enhance_mode_cb(struct _lv_obj_t * obj, lv_event_t event)
{
	int16_t mode;
	const lv_style_t *style_get = NULL;//fix warnning
	static lv_style_t style_switch_enhance;
	// com_info("event = %d",event);
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		style_get = lv_sw_get_style(obj, LV_SW_STYLE_BG);
		memcpy(&style_switch_enhance, style_get, sizeof(lv_style_t));
		mode = lv_sw_get_state(obj);
		write_int_type_param(PUBLIC_SCENE, ENHANCE_MODE, mode);
		va_display_set_enhance_mode(mode);
		usleep(10000);
		if(mode){
		style_switch_enhance.body.main_color = lv_color_hex(0x55aaff);
		style_switch_enhance.body.grad_color = lv_color_hex(0x55aaff);
		hide_enhance_item(false);
		}else{
		style_switch_enhance.body.main_color = lv_color_hex(0xa9a9a9);
		style_switch_enhance.body.grad_color = lv_color_hex(0xa9a9a9);
		hide_enhance_item(true);
		}
		lv_sw_set_style(obj, LV_SW_STYLE_BG, &style_switch_enhance);
	}
}

static void time_set_cb(struct _lv_obj_t * obj, lv_event_t event)
{
	bool change_flage = 0;
	// com_info("event = %d",event);
	if (event == LV_EVENT_CLICKED)
	{
		if(obj == para.ui.button_hour_up){
			change_flage = 0;
			set_time.tm_hour += 1;
			if(set_time.tm_hour == 24)
			set_time.tm_hour = 0;
		}else if(obj == para.ui.button_hour_down){
			change_flage = 0;
			set_time.tm_hour -= 1;
			if(set_time.tm_hour == -1)
			set_time.tm_hour = 23;
		}else if(obj == para.ui.button_min_up){
			change_flage = 1;
			set_time.tm_min += 1;
			if(set_time.tm_min == 60)
			set_time.tm_min = 0;
		}else if(obj == para.ui.button_min_down){
			change_flage = 1;
			set_time.tm_min -= 1;
			if(set_time.tm_min == -1)
			set_time.tm_min = 59;
		}
		if(change_flage == 0){
			lv_img_set_src(para.ui.image_hour1, time_pic[set_time.tm_hour/10]);
			lv_img_set_src(para.ui.image_hour2, time_pic[set_time.tm_hour%10]);
		}else{
			lv_img_set_src(para.ui.image_min1, time_pic[set_time.tm_min/10]);
			lv_img_set_src(para.ui.image_min2, time_pic[set_time.tm_min%10]);
		}
		// com_info("set time to sys");
		va_rtc_set_local_time(&set_time);
	}
}

static void time_pic_res_init(void)
{
	int cnt = 1;
	char pic_name[22];
	sprintf(pic_name, LV_IMAGE_PATH"%d.png", cnt);
	for(cnt = 0; cnt <= 9; cnt++)
	{
		sprintf(pic_name, LV_IMAGE_PATH"%d.png", cnt);
		// com_info("time pic name = %s",pic_name);
		time_pic[cnt] = (void *)parse_image_from_file(pic_name);
	}
}

static void time_pic_res_uninit(void)
{
	int cnt;
	for(cnt = 0; cnt <= 9; cnt++)
	{
		if(time_pic[cnt] != 0){
			free_image(time_pic[cnt]);
			time_pic[cnt] = NULL;
		}
	}
}

static void configure_time_set(void)
{
	//get now time
	time_pic_res_init();
	va_rtc_get_local_time(&set_time);
	// com_info("get time = %d:%d",set_time.tm_hour, set_time.tm_min);
	//set the time show in setting
	lv_img_set_src(para.ui.image_hour1, time_pic[set_time.tm_hour/10]);
	lv_img_set_src(para.ui.image_hour2, time_pic[set_time.tm_hour%10]);
	lv_img_set_src(para.ui.image_min1, time_pic[set_time.tm_min/10]);
	lv_img_set_src(para.ui.image_min2, time_pic[set_time.tm_min%10]);
	lv_obj_set_event_cb(para.ui.button_hour_up, time_set_cb);
	lv_obj_set_event_cb(para.ui.button_hour_down, time_set_cb);
	lv_obj_set_event_cb(para.ui.button_min_up, time_set_cb);
	lv_obj_set_event_cb(para.ui.button_min_down, time_set_cb);

}

static void configure_display(void)
{
	int val;
	char str[8];
	static lv_style_t style_switch_enhance;
	const lv_style_t *style_get;
	style_get = lv_sw_get_style(para.ui.switch_enhance, LV_SW_STYLE_BG);
	memcpy(&style_switch_enhance, style_get, sizeof(lv_style_t));
//backlight
	read_int_type_param(PUBLIC_SCENE, BACKLIGHT, &val);
	sprintf(str, "%d%%", val);
	lv_label_set_text(para.ui.label_num_backlight, str);
	lv_slider_set_value(para.ui.slider_backlight, val, LV_ANIM_ON);/*init the first show*/
	lv_obj_set_event_cb(para.ui.slider_backlight, backlight_cb);
//mode
	read_int_type_param(PUBLIC_SCENE, ENHANCE_MODE, &val);
	com_info("enhance mode get  =  %d",val);
	sprintf(str, "%d%%", val);
	if(val){
	lv_sw_on(para.ui.switch_enhance, LV_ANIM_ON);/*init the first show*/
	style_switch_enhance.body.main_color = lv_color_hex(0x55aaff);
	style_switch_enhance.body.grad_color = lv_color_hex(0x55aaff);
	hide_enhance_item(false);
	}
	else{
	lv_sw_off(para.ui.switch_enhance, LV_ANIM_ON);/*init the first show*/
	style_switch_enhance.body.main_color = lv_color_hex(0xa9a9a9);
	style_switch_enhance.body.grad_color = lv_color_hex(0xa9a9a9);
	hide_enhance_item(true);
	}
	lv_sw_set_style(para.ui.switch_enhance, LV_SW_STYLE_BG, &style_switch_enhance);
	lv_obj_set_event_cb(para.ui.switch_enhance, enhance_mode_cb);
//contrast
	read_int_type_param(PUBLIC_SCENE, ENHANCE_CONTRAST, &val);
	sprintf(str, "%d%%", val);
	lv_label_set_text(para.ui.label_num_contrast, str);
	lv_slider_set_value(para.ui.slider_contrast, val, LV_ANIM_ON);/*init the first show*/
	lv_obj_set_event_cb(para.ui.slider_contrast, contrast_cb);
//saturation
	read_int_type_param(PUBLIC_SCENE, ENHANCE_STATUTION, &val);
	sprintf(str, "%d%%", val);
	lv_label_set_text(para.ui.label_num_saturation, str);
	lv_slider_set_value(para.ui.slider_saturation, val, LV_ANIM_ON);/*init the first show*/
	lv_obj_set_event_cb(para.ui.slider_saturation, saturation_cb);
//bright
	read_int_type_param(PUBLIC_SCENE, ENHANCE_BRIGHT, &val);
	sprintf(str, "%d%%", val);
	lv_label_set_text(para.ui.label_num_bright, str);
	lv_slider_set_value(para.ui.slider_bright, val, LV_ANIM_ON);/*init the first show*/
	lv_obj_set_event_cb(para.ui.slider_bright, bright_cb);
}

static void volume_cb(struct _lv_obj_t * obj, lv_event_t event)
{
	int16_t volume = 0;
	char str[8];
	volume = lv_slider_get_value(obj);
	if (event == LV_EVENT_VALUE_CHANGED){
		sprintf(str, "%d%%", volume);
		lv_label_set_text(para.ui.label_volume_num, str);
		va_audio_play_set_volume_value((unsigned int)volume, (unsigned int)volume);
	}else if(event == LV_EVENT_RELEASED){
		write_int_type_param(PUBLIC_SCENE, VOLUME, (int)volume);
	}
}
//
static void hidden_all_contents(void)
{
	lv_obj_set_hidden(para.ui.general_cont, true);
	lv_obj_set_hidden(para.ui.display_cont, true);
	lv_obj_set_hidden(para.language_cont, true);
	lv_obj_set_hidden(para.sleep_cont, true);
	lv_obj_set_hidden(para.ui.infomation_cont, true);
	lv_obj_set_hidden(para.ui.button_def_setting, true);
	lv_obj_set_hidden(para.ui.cont_time_set, true);

}

static void setting_cb(struct _lv_obj_t * btn, lv_event_t event)
{
	const char *item_name;
//	com_info("event = %d",event);
	if (event == LV_EVENT_FOCUSED)
	{
		hidden_all_contents();
		item_name = lv_list_get_btn_text(btn);
		printf("%s %d: name = %s\n", __func__, __LINE__, item_name);
		#if CONFIG_FONT_ENABLE
		if(!strncmp(item_name, get_text_by_id(LANG_SET_DISPLAY), strlen(get_text_by_id(LANG_SET_DISPLAY)))) {//display
			lv_obj_set_hidden(para.ui.display_cont, false);
			fc_list_cont = para.ui.display_cont;
		} else if(!strncmp(item_name, get_text_by_id(LANG_SET_GENERAL), strlen(get_text_by_id(LANG_SET_GENERAL)))) {//general
			lv_obj_set_hidden(para.ui.general_cont, false);
			fc_list_cont = para.ui.general_cont;
		} else if(!strncmp(item_name, get_text_by_id(LANG_SET_LANG), strlen(get_text_by_id(LANG_SET_LANG)))) {//language
			lv_obj_set_hidden(para.language_cont, false);
			fc_list_cont = para.language_cont;
		} else if(!strncmp(item_name, get_text_by_id(LANG_SET_INFORMATION), strlen(get_text_by_id(LANG_SET_INFORMATION)))) {//infomation
			lv_obj_set_hidden(para.ui.infomation_cont, false);
			fc_list_cont = para.ui.infomation_cont;
		}else if(!strncmp(item_name, get_text_by_id(LANG_SET_DEFAULT), strlen(get_text_by_id(LANG_SET_DEFAULT)))) {//default setting
			lv_obj_set_hidden(para.ui.button_def_setting, false);
			fc_list_cont = para.ui.button_def_setting;
		}else if(!strncmp(item_name, get_text_by_id(LANG_SET_PASSWORD), strlen(get_text_by_id(LANG_SET_PASSWORD)))) {//password
			com_info("no this function\n");
			fc_list_cont = NULL;
		}else if(!strncmp(item_name, get_text_by_id(LANG_SET_TIME), strlen(get_text_by_id(LANG_SET_TIME)))) {//time
			com_info("no this function\n");
			fc_list_cont = para.ui.cont_time_set;
			lv_obj_set_hidden(para.ui.cont_time_set, false);
		}else if(!strncmp(item_name, get_text_by_id(LANG_SET_WALLPAPER), strlen(get_text_by_id(LANG_SET_WALLPAPER)))) {//wallpaper
			com_info("no this function\n");
			fc_list_cont = NULL;
		} else {
			com_info("no this function\n");
			fc_list_cont = NULL;
		}
		#else
		if(!strncmp(item_name, "Display", strlen("Display"))) {//display
			lv_obj_set_hidden(para.ui.display_cont, false);
			fc_list_cont = para.ui.display_cont;
		} else if(!strncmp(item_name, "General", strlen("General"))) {//general
			lv_obj_set_hidden(para.ui.general_cont, false);
			fc_list_cont = para.ui.general_cont;
		} else if(!strncmp(item_name, "Language", strlen("Language"))) {//language
			lv_obj_set_hidden(para.language_cont, false);
			fc_list_cont = para.language_cont;
		} else if(!strncmp(item_name, "Information", strlen("Information"))) {//infomation
			lv_obj_set_hidden(para.ui.infomation_cont, false);
			fc_list_cont = para.ui.infomation_cont;
		}else if(!strncmp(item_name, "Default setting", strlen("Default setting"))) {//default setting
			lv_obj_set_hidden(para.ui.button_def_setting, false);
			fc_list_cont = para.ui.button_def_setting;
		}else if(!strncmp(item_name, "Password", strlen("Password"))) {//password
			com_info("no this function\n");
			fc_list_cont = NULL;
		}else if(!strncmp(item_name, "Time", strlen("Time"))) {//time
			lv_obj_set_hidden(para.ui.cont_time_set, false);
			fc_list_cont = para.ui.cont_time_set;
		}else if(!strncmp(item_name, "Wallpaper", strlen("Wallpaper"))) {//wallpaper
			com_info("no this function\n");
			fc_list_cont = NULL;
		} else {
			com_info("no this function\n");
			fc_list_cont = NULL;
		}
		#endif
		fc_list_btn = btn;
		if(btn){
			lv_page_focus(para.ui.list_1, btn, LV_ANIM_ON);
		}
	}
	else if(event == LV_EVENT_CLICKED)
	{
		lv_group_t *g = get_key_group();
		lv_group_remove_all_objs(g);
		menu_level = 2;
		if(fc_list_cont == para.ui.display_cont)
		{
			lv_group_add_obj(g, para.ui.slider_backlight);
			lv_group_add_obj(g, para.ui.switch_enhance);
			lv_group_add_obj(g, para.ui.slider_saturation);
			lv_group_add_obj(g, para.ui.slider_contrast);
			lv_group_add_obj(g, para.ui.slider_bright);
		}
		else if(fc_list_cont == para.ui.general_cont)
		{
			__s32 index = 0;
			for (index = 0; index < sizeof(para.drop_down_list)/sizeof(para.drop_down_list[0]); index++) {
				lv_group_add_obj(g, para.drop_down_list[index].drop_down);
			}
			lv_group_add_obj(g, para.ui.button_backlight);
			lv_group_add_obj(g, para.ui.slider_volume);
		}
		else if(fc_list_cont == para.language_cont)
		{
			lv_group_add_obj(g, para.drop_down_list_language);
		}
		else if(fc_list_cont == para.ui.infomation_cont)
		{

		}
		else if(fc_list_cont == para.ui.button_def_setting)
		{
			lv_group_add_obj(g, para.ui.button_def_setting);
		}
		else if(fc_list_cont == para.ui.cont_time_set)
		{
			lv_group_add_obj(g, para.ui.button_hour_up);
			lv_group_add_obj(g, para.ui.button_hour_down);
			lv_group_add_obj(g, para.ui.button_min_up);
			lv_group_add_obj(g, para.ui.button_min_down);
		}
		else
		{
			//com_err("this key err.");
		}
	}
	else if(event == LV_EVENT_KEY)
	{
		const int *key_val = NULL;
		key_val = lv_event_get_data();
		// com_info("key val = %d",*key_val);
		if(*key_val == LV_KEY_RETURN)//回到上一级列表
		{
			com_info("LV_KEY_RETURN!");
			lv_group_t *g = get_key_group();
			lv_group_remove_all_objs(g);
			if(menu_level == 2)
			{
				lv_list_set_btn_selected(para.ui.list_1, btn);
				com_info("btn name = %s",lv_list_get_btn_text(btn));
				lv_event_send(btn, LV_EVENT_FOCUSED, NULL);
				list_keygroup_add(para.ui.list_1, g);
				lv_group_focus_obj(btn);
				lv_btn_set_state(btn,LV_BTN_STATE_REL);
				menu_level = 1;
			}
			else if (menu_level == 1)
			{
				com_info("back to home!");
				switch_page(PAGE_SETTING, PAGE_HOME);
			}
		}
	}
}
static void sleep_event(lv_obj_t *obj, lv_event_t event)
{
	uint8_t select = 0;
	int sleep_timeout;

//	printf("%s %d event:%d\n", __FUNCTION__, __LINE__, event);
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		char buf[32];
		lv_ddlist_get_selected_str(obj, buf, sizeof(buf));
		select = lv_ddlist_get_selected(obj);
		sleep_timeout = sleep_listid_to_time(select);
		com_info("set sleep time =%d\n", sleep_timeout);
		auto_sleep_timer_modify(sleep_timeout);
		write_int_type_param(PUBLIC_SCENE, AUTO_SLEEP, (int)select);
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_POWEROFF].drop_down, false);
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_POWEROFF].label, false);
	} else if (event == LV_EVENT_PRESSED) {
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_POWEROFF].drop_down, true);
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_POWEROFF].label, true);
	}
}

static void close_screen_event(lv_obj_t *obj, lv_event_t event)
{
	uint8_t select = 0;
	int sleep_timeout;

//	printf("%s %d event:%d\n", __FUNCTION__, __LINE__, event);
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		char buf[32];
		lv_ddlist_get_selected_str(obj, buf, sizeof(buf));
		select = lv_ddlist_get_selected(obj);
		sleep_timeout = sleep_listid_to_time(select);
		com_info("set sleep time =%d\n", sleep_timeout);
		auto_close_screen_timer_modify(sleep_timeout);
		write_int_type_param(PUBLIC_SCENE, AUTO_CLOSE_SCREEN, (int)select);

		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_SLEEP].drop_down, false);
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_SLEEP].label, false);
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_POWEROFF].drop_down, false);
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_POWEROFF].label, false);
	} else if (LV_EVENT_PRESSED == event) {
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_SLEEP].drop_down, true);
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_SLEEP].label, true);
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_POWEROFF].drop_down, true);
		lv_obj_set_hidden(para.drop_down_list[DROP_DOWN_AUTO_POWEROFF].label, true);
	}
}

static void auto_power_off_event(lv_obj_t *obj, lv_event_t event)
{
	uint8_t select = 0;
	int sleep_timeout;

//	printf("%s %d event:%d\n", __FUNCTION__, __LINE__, event);
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		char buf[32];
		lv_ddlist_get_selected_str(obj, buf, sizeof(buf));
		select = lv_ddlist_get_selected(obj);
		sleep_timeout = sleep_listid_to_time(select);
		com_info("set power_off time =%d\n", sleep_timeout);
		auto_power_off_timer_modify(sleep_timeout);
		write_int_type_param(PUBLIC_SCENE, AUTO_POWEROFF, (int)select);
	}
}

static int list_cb_setting(const lv_obj_t * list, lv_event_cb_t event_cb)
{
	lv_obj_t *firstBtn;
	lv_obj_t *nextBtn;
	int i, item_num;

	item_num = lv_list_get_size(list);
	firstBtn = lv_list_get_next_btn(list, NULL);
	nextBtn = firstBtn;
	for(i = 0; i < item_num; i++){
		lv_obj_set_event_cb(nextBtn, event_cb);
		nextBtn = lv_list_get_next_btn(list, nextBtn);
	}

	return 0;
}

static void sys_dialog(struct _lv_task_t *task)
{
	if(dialog_show_time == 0){
		lv_obj_set_hidden(para.ui.cont_dialog, true);
	}
	else{
		dialog_show_time--;
	}
}

static void ui_language_drop_list(void)
{
	#ifdef LV_USE_CONT
	static lv_style_t style0_language_cont;
	lv_style_copy(&style0_language_cont, &lv_style_pretty);
	style0_language_cont.body.opa = 0;
	style0_language_cont.body.border.opa = 0;

	para.language_cont = lv_cont_create(para.ui.cont, NULL);
	lv_obj_set_pos(para.language_cont, 262, 106);
	lv_obj_set_size(para.language_cont, 407, 301);
	lv_cont_set_fit4(para.language_cont, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(para.language_cont, LV_CONT_STYLE_MAIN, &style0_language_cont);
	#endif // LV_USE_CONT

	#ifdef LV_USE_DDLIST
	static lv_style_t style0_drop_down_list_language;
	lv_style_copy(&style0_drop_down_list_language, &lv_style_pretty);
	#if CONFIG_FONT_ENABLE
	style0_drop_down_list_language.text.font = get_font_lib()->msyh_16;
	#endif

	para.drop_down_list_language = lv_ddlist_create(para.language_cont, NULL);
	lv_obj_set_pos(para.drop_down_list_language, 96, 91);
	lv_obj_set_size(para.drop_down_list_language, 250, 100);
	lv_ddlist_set_align(para.drop_down_list_language, 0);
	lv_ddlist_set_fix_width(para.drop_down_list_language, 200);
	lv_ddlist_set_fix_height(para.drop_down_list_language, 0);
	lv_ddlist_set_sb_mode(para.drop_down_list_language, 3);
	lv_ddlist_set_anim_time(para.drop_down_list_language, 200);
	lv_ddlist_set_draw_arrow(para.drop_down_list_language, 1);
	lv_ddlist_set_stay_open(para.drop_down_list_language, 0);
	lv_ddlist_set_style(para.drop_down_list_language, LV_DDLIST_STYLE_BG, &style0_drop_down_list_language);

#if CONFIG_FONT_ENABLE
	lv_ddlist_set_options(para.drop_down_list_language, get_text_by_id(LANG_SET_LANG_VAL));
	lv_ddlist_set_selected(para.drop_down_list_language, (int)get_language());
	#else

	lv_ddlist_set_options(para.drop_down_list_language, "English");
	lv_ddlist_set_selected(para.drop_down_list_language, 0);
#endif
	//lv_ddlist_set_anim_time(para.drop_down_list_language, 100);
	//lv_ddlist_set_draw_arrow(para.drop_down_list_language, 1);
	//lv_ddlist_set_fix_width(para.drop_down_list_language, 200);
	//lv_ddlist_set_stay_open(para.drop_down_list_language, 0);
	#endif // LV_USE_DDLIST
}

static void ui_drop_down_list_init(void)
{
	int val, index = 0;
	#ifdef LV_USE_CONT
	static lv_style_t style0_sleep_cont;
	lv_style_copy(&style0_sleep_cont, &lv_style_pretty);
	style0_sleep_cont.body.opa = 0;
	style0_sleep_cont.body.border.opa = 0;

	para.sleep_cont = lv_cont_create(para.ui.cont, NULL);
	lv_obj_set_pos(para.sleep_cont, 362, 106);
	lv_obj_set_size(para.sleep_cont, 407, 301);
	lv_cont_set_fit4(para.sleep_cont, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(para.sleep_cont, LV_CONT_STYLE_MAIN, &style0_sleep_cont);
	#endif // LV_USE_CONT

	for (index = 0; index < sizeof(para.drop_down_list)/sizeof(para.drop_down_list[0]); index++) {
		para.drop_down_list[index].label_pos.x = 30;
		para.drop_down_list[index].label_pos.y = 0 + 70*index;
		para.drop_down_list[index].label_w = 255;
		para.drop_down_list[index].label_h = 22;

		para.drop_down_list[index].drop_down_pos.x = 30;
		para.drop_down_list[index].drop_down_pos.y = 30 + 70*index;
		para.drop_down_list[index].drop_down_w = 255;
		para.drop_down_list[index].drop_down_h = 255;
	}
#ifdef LV_USE_LABEL
	static lv_style_t style0_label_sleep;
	lv_style_copy(&style0_label_sleep, &lv_style_transp);
#if CONFIG_FONT_ENABLE
	style0_label_sleep.text.font = get_font_lib()->msyh_16;
#endif
#endif // LV_USE_LABEL

#ifdef LV_USE_DDLIST
	static lv_style_t style0_drop_down_list_sleep;
	lv_style_copy(&style0_drop_down_list_sleep, &lv_style_pretty);
	style0_drop_down_list_sleep.text.line_space = 10;
	style0_drop_down_list_sleep.body.padding.top = 10;
#if CONFIG_FONT_ENABLE
	style0_drop_down_list_sleep.text.font = get_font_lib()->msyh_16;
#endif
	for (index = 0; index < sizeof(para.drop_down_list)/sizeof(para.drop_down_list[0]); index++) {
		lv_obj_t *drop_down_obj = NULL;
#ifdef LV_USE_LABEL
		lv_obj_t *label = NULL;
		para.drop_down_list[index].label = lv_label_create(para.ui.general_cont, NULL);
		label = para.drop_down_list[index].label;
		lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);
		lv_obj_set_pos(label, para.drop_down_list[index].label_pos.x, para.drop_down_list[index].label_pos.y);
		lv_obj_set_size(label, para.drop_down_list[index].label_w, para.drop_down_list[index].label_h);
		lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style0_label_sleep);
		lv_label_set_body_draw(label,1);
#endif

		para.drop_down_list[index].drop_down = lv_ddlist_create(para.ui.general_cont, NULL);
		drop_down_obj = para.drop_down_list[index].drop_down;
		lv_obj_set_pos(drop_down_obj, para.drop_down_list[index].drop_down_pos.x, para.drop_down_list[index].drop_down_pos.y);
		lv_obj_set_size(drop_down_obj, para.drop_down_list[index].drop_down_w, para.drop_down_list[index].drop_down_h);
		lv_ddlist_set_align(drop_down_obj, LV_LABEL_ALIGN_LEFT);
		lv_ddlist_set_fix_width(drop_down_obj, 250);
		lv_ddlist_set_fix_height(drop_down_obj, 0);
		lv_ddlist_set_sb_mode(drop_down_obj, 3);
		lv_ddlist_set_anim_time(drop_down_obj, 200);
		lv_ddlist_set_draw_arrow(drop_down_obj, 1);
		lv_ddlist_set_stay_open(drop_down_obj, 0);
		lv_ddlist_set_style(drop_down_obj, LV_DDLIST_STYLE_BG, &style0_drop_down_list_sleep);
#if CONFIG_FONT_ENABLE
		lv_ddlist_set_options(drop_down_obj, get_text_by_id(LANG_SET_SLEEP_TIME_VAL));
#else
		lv_ddlist_set_options(drop_down_obj, "10 sec\n20 sec\n30 sec\n60 sec\n120 sec\nnever");
#endif
		if (index == DROP_DOWN_AUTO_CLOSE_SCREEN) {
#if CONFIG_FONT_ENABLE
			lv_label_set_text(label, get_text_by_id(LANG_SET_AUTO_CLOSE_SCRREN_TIME));
#else
			lv_label_set_text(label, "auto close screen time");
#endif
			read_int_type_param(PUBLIC_SCENE, AUTO_CLOSE_SCREEN, &val);
		} else if (index == DROP_DOWN_AUTO_SLEEP) {
#if CONFIG_FONT_ENABLE
			lv_label_set_text(label, get_text_by_id(LANG_SET_SLEEP_TIME));
#else
			lv_label_set_text(label, "auto sleep time");
#endif
			read_int_type_param(PUBLIC_SCENE, AUTO_SLEEP, &val);
		} else if (index == DROP_DOWN_AUTO_POWEROFF) {
#if CONFIG_FONT_ENABLE
			lv_label_set_text(label, get_text_by_id(LANG_SET_POWEROFF_TIME));
#else
			lv_label_set_text(label, "auto power off time");
#endif
			read_int_type_param(PUBLIC_SCENE, AUTO_POWEROFF, &val);
		}
		lv_ddlist_set_selected(drop_down_obj, val);
	}
#endif // LV_USE_DDLIST
}

#if CONFIG_FONT_ENABLE
static void setting_label_text_init(setting_para_t *para)
{
	static lv_style_t style0_label_1;

	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.line_space = 2;
	style0_label_1.text.font = get_font_lib()->msyh_16;

	static lv_style_t style0_label_2;
	lv_style_copy(&style0_label_2, &lv_style_transp);
	style0_label_2.text.color = lv_color_hex(0x000000);
	style0_label_2.text.line_space = 2;
	style0_label_2.text.font = get_font_lib()->msyh_20;

	lv_label_set_text(para->ui.label_setting, get_text_by_id(LANG_SET_SETTING));
	lv_label_set_text(para->ui.label_volume, get_text_by_id(LANG_SET_VOLUME));
	lv_label_set_text(para->ui.label_btn_backlight, get_text_by_id(LANG_SET_BACKLIGHT));
	lv_label_set_text(para->ui.label_btn_def_setting, get_text_by_id(LANG_SET_DEFAULT));
	lv_label_set_text(para->ui.label_backlight, get_text_by_id(LANG_SET_BACKLIGHT_VAL));
	lv_label_set_text(para->ui.label_enhance, get_text_by_id(LANG_SET_ENHANCE));
	lv_label_set_text(para->ui.label_saturation, get_text_by_id(LANG_SET_SATURATION));
	lv_label_set_text(para->ui.label_contrast, get_text_by_id(LANG_SET_CONTRAST));
	lv_label_set_text(para->ui.label_bright, get_text_by_id(LANG_SET_BRIGHT));
	lv_label_set_text(para->ui.label_dialog, get_text_by_id(LANG_SET_DEFAULT_ING));
	lv_label_set_text(para->ui.label_sdk_version, get_text_by_id(LANG_SET_SDK_VER));//sdk version
	lv_label_set_text(para->ui.label_sdk_val, SDK_VERSION);
	lv_label_set_text(para->ui.label_os_version, get_text_by_id(LANG_SET_OS_VER));//os version
	lv_label_set_text(para->ui.label_os_val, OS_VERSION);
	lv_label_set_text(para->ui.label_ui_version, get_text_by_id(LANG_SET_UI_VER));//ui version
	lv_label_set_text(para->ui.label_ui_val, UI_VERSION);
	lv_label_set_text(para->ui.label_screen_info, get_text_by_id(LANG_SET_SCN_INFO));//scene info

	lv_label_set_style(para->ui.label_setting, LV_LABEL_STYLE_MAIN, &style0_label_2);
	lv_label_set_style(para->ui.label_volume, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_btn_backlight, LV_LABEL_STYLE_MAIN, &style0_label_2);
	lv_label_set_style(para->ui.label_btn_def_setting, LV_LABEL_STYLE_MAIN, &style0_label_2);
	lv_label_set_style(para->ui.label_backlight, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_enhance, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_saturation, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_contrast, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_bright, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_dialog, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_sdk_version, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_os_version, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_ui_version, LV_LABEL_STYLE_MAIN, &style0_label_1);
	lv_label_set_style(para->ui.label_screen_info, LV_LABEL_STYLE_MAIN, &style0_label_1);

	static lv_style_t style;
	style.text.font = get_font_lib()->msyh_16;
	list_label_set_text(para->ui.list_1, &style, list_txt);

}
#endif

static int list_keygroup_add(const lv_obj_t * list, lv_group_t *g)
{
	lv_obj_t *firstBtn;
	lv_obj_t *nextBtn;
	int i, item_num;

	item_num = lv_list_get_size(list);
	firstBtn = lv_list_get_next_btn(list, NULL);
	nextBtn = firstBtn;
	for(i = 0; i < item_num; i++){
		lv_group_add_obj(g, nextBtn);
		nextBtn = lv_list_get_next_btn(list, nextBtn);
	}
	return 0;
}

static int create_setting(void)
{
	char str[10] = {0};
	__s32 index = 0, val = 0;
	com_info("############\n");
	para.ui.cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(para.ui.cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color = LV_COLOR_BLUE;
	cont_style.body.grad_color = LV_COLOR_BLUE;
	lv_cont_set_style(para.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
	lv_cont_set_layout(para.ui.cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(para.ui.cont, LV_FIT_NONE);
	setting_auto_ui_create(&para.ui);
	#if CONFIG_FONT_ENABLE
	setting_label_text_init(&para);
	#endif
	ui_language_drop_list();
	ui_drop_down_list_init();
	hidden_all_contents();
	lv_obj_set_hidden(para.ui.general_cont, false);
	configure_display();
	configure_time_set();
	read_int_type_param(PUBLIC_SCENE, VOLUME, &val);
	sprintf(str, "%d%%", val);
	lv_label_set_text(para.ui.label_volume_num, str);
	lv_slider_set_value(para.ui.slider_volume, val, LV_ANIM_ON);
	lv_obj_set_event_cb(para.ui.slider_volume, volume_cb);
	list_cb_setting(para.ui.list_1, setting_cb);
	lv_obj_set_event_cb(para.ui.image_btn_return, back_btn_event);

	for (index = 0; index < sizeof(para.drop_down_list)/sizeof(para.drop_down_list[0]); index++) {
		if (index == DROP_DOWN_AUTO_CLOSE_SCREEN) {
			lv_obj_set_event_cb(para.drop_down_list[index].drop_down, close_screen_event);
		} else if (index == DROP_DOWN_AUTO_SLEEP) {
			lv_obj_set_event_cb(para.drop_down_list[index].drop_down, sleep_event);
		} else if (index == DROP_DOWN_AUTO_POWEROFF) {
			lv_obj_set_event_cb(para.drop_down_list[index].drop_down, auto_power_off_event);
		}
	}
	lv_obj_set_event_cb(para.drop_down_list_language, language_event);
	lv_obj_set_event_cb(para.ui.button_def_setting, reset_setting_event);
	lv_obj_set_event_cb(para.ui.button_backlight, button_backlight_cb);
	dialog_task = lv_task_create(sys_dialog, 1000, LV_TASK_PRIO_MID, NULL);
	#if USE_SUNXI_KEY
	lv_group_t *g = get_key_group();
	if(g != NULL) {
	menu_level = 1;
	list_keygroup_add(para.ui.list_1, g);
	}
	#endif
	return 0;
}

static int destory_setting(void)
{
	menu_level = 0;
#if USE_SUNXI_KEY
	lv_group_t *g = get_key_group();
	if(g != NULL) {
		lv_group_remove_all_objs(g);
	}
#endif
	setting_auto_ui_destory(&para.ui);
	time_pic_res_uninit();
	lv_obj_del(para.ui.cont);
	lv_task_del(dialog_task);
	return 0;
}

static int show_setting(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);
	return 0;
}

static int hide_setting(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);

	return 0;
}

static int msg_proc_setting(MsgDataInfo *msg)
{
	int param;

	switch (msg->type)
	{
	case MSG_KEY:
		if(!msg->status)
		return 0;
		if(msg->value == LV_KEY_RETURN)
		{
			param = LV_KEY_RETURN;
			com_info("set key return..");
			lv_event_send(fc_list_btn, LV_EVENT_KEY, (void *)&param);
		}
		break;

	default:
		break;
	}
	return 0;
}

static page_interface_t page_setting =
{
	.ops =
	{
		create_setting,
		destory_setting,
		show_setting,
		hide_setting,
		msg_proc_setting,

	},
	.info =
	{
		.id         = PAGE_SETTING,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_SETTING(void)
{
	reg_page(&page_setting);
}
