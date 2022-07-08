/**********************
 *      includes
 **********************/
#include "moc_wlan_set1.h"
#include "ui_wlan_set1.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "bs_widget.h"
#include "app_config_interface.h"


/**********************
 *       variables
 **********************/
typedef struct
{
	//lv_task_t *wifi_info_update_tid;
	lv_task_t *list_update_tid;

	pthread_t wifi_scan_tid;
	int scan_quit_flag;
	pthread_mutex_t scan_mutex;

	char scan_results[WIFI_MAX_SCAN_SIZE];
	int scan_valid_len;
	net_wifi_scan_info_t p_scan_info[WIFI_MAX_SCAN_NUM];
	int scan_num;
	connection_status connect_info;

	lv_obj_t *on_off_sw;
	char tmp_ssid[WIFI_MAX_SSID_SIZE];		/* ssid */
	char tmp_password[WIFI_MAX_PASSWORD_SIZE];		/* password */
} wlan_set1_moc_t;

typedef struct
{
	wlan_set1_ui_t ui;
	wlan_set1_moc_t moc;
} wlan_set1_para_t;
static wlan_set1_para_t para;

static lv_style_t style0_switch_4;
static lv_style_t style1_switch_4;
static lv_style_t style2_switch_4;
static lv_style_t style3_switch_4;
static lv_style_t style0_button_3;

typedef enum {
	WIFI_CYCLE_IMAGE = 0,
	WIFI_NO_SIGNAL_IMAGE,
	WIFI_NEXT_IMAGE,
	WIFI_LEVEL1_IMAGE,
	WIFI_LEVEL2_IMAGE,
	WIFI_LEVEL3_IMAGE,
	WIFI_LEVEL4_IMAGE,
	WIFI_IMAGE_NUM
} wifi_image_t;

static ui_image_t wifi_image_list[WIFI_IMAGE_NUM] = {
	{NULL, LV_IMAGE_PATH"wifi_cycle.png"},
	{NULL, LV_IMAGE_PATH"wifi_no_connect.png"},
	{NULL, LV_IMAGE_PATH"wifi_next.png"},
	{NULL, LV_IMAGE_PATH"wifi_level1.png"},
	{NULL, LV_IMAGE_PATH"wifi_level2.png"},
	{NULL, LV_IMAGE_PATH"wifi_level3.png"},
	{NULL, LV_IMAGE_PATH"wifi_level4.png"}
};




/**********************
 *  functions
 **********************/
static void update_list_context(wlan_set1_para_t *para);

static void hidden_obj_and_all_child(lv_obj_t *obj, bool en)
{
	#if 0
	lv_obj_set_hidden(para.ui.keyboard_1, en);
	lv_obj_set_hidden(para.ui.label_2, en);
	lv_obj_set_hidden(para.ui.text_area_1, en);
	lv_obj_set_hidden(para.ui.label_3, en);
	lv_obj_set_hidden(para.ui.button_1, en);
	lv_obj_set_hidden(para.ui.label_4, en);
	lv_obj_set_hidden(para.ui.button_2, en);
	lv_obj_set_hidden(para.ui.container_1, en);
	#endif
	#if 1
    lv_obj_t * child = lv_obj_get_child(obj, NULL);
    lv_obj_t * child_next;
    while(child) {
		lv_obj_set_hidden(child, en);
        child_next = lv_obj_get_child(obj, child);
        child = child_next;
    }
	lv_obj_set_hidden(obj, en);
	#endif
}

static bool __is_in_obj(lv_obj_t * obj, __u32 x, __u32 y)
{
	lv_area_t  obj_area;
	lv_obj_get_coords(obj, &obj_area);

	if (x > obj_area.x2 || x < obj_area.x1 || y > obj_area.y2 || y < obj_area.y1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void *get_img_by_level(int level)
{
	if (level >= 75) {
		return get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_LEVEL4_IMAGE);
	} else if (level >= 50) {
		return get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_LEVEL3_IMAGE);
	} else if (level >= 25) {
		return get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_LEVEL2_IMAGE);
	} else if (level == -1) {
		return get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_NO_SIGNAL_IMAGE);
	} else {
		return get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_LEVEL1_IMAGE);
	}
}

static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_WLAN_SET1);
		//create_page(PAGE_HOME);
		switch_page(PAGE_WLAN_SET1, PAGE_HOME);
	}
}

static void btn_hbar_return_event_cb(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_WLAN_SET1);
		//create_page(PAGE_HOME);
		switch_page(PAGE_WLAN_SET1, PAGE_HOME);
	}
}

static lv_obj_t * on_off_sw_create(lv_obj_t * par)
{
	lv_obj_t *switch_4;

	lv_style_copy(&style0_switch_4, &lv_style_pretty);
	style0_switch_4.body.main_color = lv_color_hex(0xdfdfdf);
	style0_switch_4.body.grad_color = lv_color_hex(0xdfdfdf);
	style0_switch_4.body.radius = 20;
	style0_switch_4.body.border.width = 0;
	style0_switch_4.body.border.opa = 0;
	style0_switch_4.body.padding.top = -5;
	style0_switch_4.body.padding.bottom = -5;
	style0_switch_4.body.padding.left = -5;
	style0_switch_4.body.padding.right = -5;
	style0_switch_4.body.padding.inner = -5;

	lv_style_copy(&style1_switch_4, &lv_style_pretty_color);
	style1_switch_4.body.main_color = lv_color_hex(0x0055ff);
	style1_switch_4.body.grad_color = lv_color_hex(0x55ffff);
	style1_switch_4.body.radius = 20;
	style1_switch_4.body.padding.top = 0;
	style1_switch_4.body.padding.bottom = 0;
	style1_switch_4.body.padding.left = 0;
	style1_switch_4.body.padding.right = 0;
	style1_switch_4.body.padding.inner = 0;

	lv_style_copy(&style2_switch_4, &lv_style_pretty);
	style2_switch_4.body.grad_color = lv_color_hex(0xffffff);
	style2_switch_4.body.border.width = 0;
	style2_switch_4.body.border.opa = 0;
	style2_switch_4.body.padding.top = 0;
	style2_switch_4.body.padding.bottom = 0;
	style2_switch_4.body.padding.left = 0;
	style2_switch_4.body.padding.right = 0;
	style2_switch_4.body.padding.inner = 0;

	lv_style_copy(&style3_switch_4, &lv_style_pretty);
	style3_switch_4.body.grad_color = lv_color_hex(0xffffff);

	switch_4 = lv_sw_create(par, NULL);
	lv_obj_set_pos(switch_4, 800-33-30, 0);
	lv_obj_set_size(switch_4, 33, 14);
	lv_sw_on(switch_4, LV_ANIM_OFF);
	lv_sw_set_style(switch_4, LV_SW_STYLE_BG, &style0_switch_4);
	lv_sw_set_style(switch_4, LV_SW_STYLE_INDIC, &style1_switch_4);
	lv_sw_set_style(switch_4, LV_SW_STYLE_KNOB_OFF, &style2_switch_4);
	lv_sw_set_style(switch_4, LV_SW_STYLE_KNOB_ON, &style3_switch_4);

	return switch_4;
}

static void on_off_sw_destory(lv_obj_t *obj)
{
	if(obj != NULL) {
		lv_obj_del(obj);
	}
}

static void wifi_manu_param_save(wifi_data_t wifi)
{
	int param;

	param = (int)(wifi.manu_on);
	write_int_type_param(WLAN_SCENE, WLAN_MANU_ON, param);
	param = (int)(wifi.manu_connected);
	write_int_type_param(WLAN_SCENE, WLAN_MANU_CONNECTED, param);
	write_string_type_param(WLAN_SCENE, WLAN_MANU_SSID, wifi.manu_ssid,
		sizeof(wifi.manu_ssid));
	write_string_type_param(WLAN_SCENE, WLAN_MANU_PASSWORD, wifi.manu_password,
		sizeof(wifi.manu_password));
}

static void select_event_cb(lv_obj_t * btn, lv_event_t event)
{
	const char *str = NULL;
	if (btn != NULL && event == LV_EVENT_CLICKED) {
		hidden_obj_and_all_child(para.ui.container_1, 0);
		lv_obj_move_foreground(para.ui.container_1);
		hidden_obj_and_all_child(para.ui.container_mark, 0);
		memset(para.moc.tmp_ssid, 0, WIFI_MAX_SSID_SIZE);
		str = lv_list_get_btn_text(btn);
		memcpy(para.moc.tmp_ssid, str, strlen(str));
		lv_label_set_text(para.ui.label_2, para.moc.tmp_ssid);
	}
}

static void connect_event_cb(lv_obj_t * btn, lv_event_t event)
{
	int ret;
	const char *str = NULL;
	wifi_data_t wifi;
	static lv_style_t style0_label_1;
	get_wifi_data(&wifi);

	lv_style_copy(&style0_label_1, &lv_style_transp);
	if (btn != NULL && event == LV_EVENT_CLICKED) {

		if(wifi.is_on != 1){
			return;
		}

		memset(para.moc.tmp_password, 0, WIFI_MAX_PASSWORD_SIZE);
		str = lv_ta_get_text(para.ui.text_area_1);
		memcpy(para.moc.tmp_password, str, strlen(str));

		com_info("connecting:%s, %s\n", para.moc.tmp_ssid, para.moc.tmp_password);
		#if CONFIG_FONT_ENABLE
		lv_style_copy(&style0_label_1, &lv_style_transp);
		style0_label_1.text.color = lv_color_hex(0x000000);
		style0_label_1.text.line_space = 2;
		style0_label_1.text.font = get_font_lib()->msyh_16;

		lv_label_set_text(para.ui.label_5, get_text_by_id(LANG_WLAN_SELECT_CONNECTING));
		lv_label_set_style(para.ui.label_5, LV_LABEL_STYLE_MAIN, &style0_label_1);
		#else
		lv_label_set_text(para.ui.label_5, "connecting...");
		#endif

		lv_obj_invalidate(para.ui.label_5);
		ret = net_wifi_connect_ap(wifi.p_wifi_hd, para.moc.tmp_ssid,
				para.moc.tmp_password, wifi.event_label);
		if(ret == 1) {
			com_info("connect new wifi:%s, %s ok\n", para.moc.tmp_ssid, para.moc.tmp_password);
			wifi.is_connected = 1;
			wifi.manu_connected = 1;
			memcpy(wifi.ssid, para.moc.tmp_ssid, WIFI_MAX_SSID_SIZE);
			memcpy(wifi.password, para.moc.tmp_password, WIFI_MAX_PASSWORD_SIZE);
			memcpy(wifi.manu_ssid, para.moc.tmp_ssid, WIFI_MAX_SSID_SIZE);
			memcpy(wifi.manu_password, para.moc.tmp_password, WIFI_MAX_PASSWORD_SIZE);
			set_wifi_data(&wifi);
			wifi_manu_param_save(wifi);
			update_list_context(&para);
			#if CONFIG_FONT_ENABLE
			lv_style_copy(&style0_label_1, &lv_style_transp);
			style0_label_1.text.color = lv_color_hex(0x000000);
			style0_label_1.text.line_space = 2;
			style0_label_1.text.font = get_font_lib()->msyh_16;

			lv_label_set_text(para.ui.label_5, get_text_by_id(LANG_WLAN_SELECT_CONNECTOK));
			lv_label_set_style(para.ui.label_5, LV_LABEL_STYLE_MAIN, &style0_label_1);
			#else
			lv_label_set_text(para.ui.label_5, "connect ok");
			#endif

			lv_obj_invalidate(para.ui.label_5);
		}
		else {
			#if CONFIG_FONT_ENABLE
			lv_style_copy(&style0_label_1, &lv_style_transp);
			style0_label_1.text.color = lv_color_hex(0x000000);
			style0_label_1.text.line_space = 2;
			style0_label_1.text.font = get_font_lib()->msyh_16;

			lv_label_set_text(para.ui.label_5, get_text_by_id(LANG_WLAN_SELECT_PS_ERR));
			lv_label_set_style(para.ui.label_5, LV_LABEL_STYLE_MAIN, &style0_label_1);
			#else
			lv_label_set_text(para.ui.label_5, "password err");
			#endif

			lv_obj_invalidate(para.ui.label_5);
		}
	}
}

static void disconnect_event_cb(lv_obj_t * btn, lv_event_t event)
{
	int ret;
	wifi_data_t wifi;
	static lv_style_t style0_label_1;
	get_wifi_data(&wifi);

	lv_style_copy(&style0_label_1, &lv_style_transp);
	if (btn != NULL && event == LV_EVENT_CLICKED) {
		if(wifi.is_on != 1){
			return;
		}

		#if CONFIG_FONT_ENABLE
		lv_style_copy(&style0_label_1, &lv_style_transp);
		style0_label_1.text.color = lv_color_hex(0x000000);
		style0_label_1.text.line_space = 2;
		style0_label_1.text.font = get_font_lib()->msyh_16;

		lv_label_set_text(para.ui.label_5, get_text_by_id(LANG_WLAN_SELECT_DISCONNECTING));
		lv_label_set_style(para.ui.label_5, LV_LABEL_STYLE_MAIN, &style0_label_1);
		#else
		lv_label_set_text(para.ui.label_5, "disconnecting...");
		#endif

		ret = net_wifi_disconnect_ap(wifi.p_wifi_hd, wifi.event_label);
		if(ret == 0) {
			com_info("disconnect wifi ok\n");
			net_wifi_remove_all_networks(wifi.p_wifi_hd);
			wifi.is_connected = 0;
			wifi.manu_connected = 0;
			memset(wifi.ssid, 0, WIFI_MAX_SSID_SIZE);
			memset(wifi.password, 0, WIFI_MAX_PASSWORD_SIZE);
			memset(wifi.manu_ssid, 0, WIFI_MAX_SSID_SIZE);
			memset(wifi.manu_password, 0, WIFI_MAX_PASSWORD_SIZE);
			set_wifi_data(&wifi);
			wifi_manu_param_save(wifi);
			update_list_context(&para);
			#if CONFIG_FONT_ENABLE
			lv_style_copy(&style0_label_1, &lv_style_transp);
			style0_label_1.text.color = lv_color_hex(0x000000);
			style0_label_1.text.line_space = 2;
			style0_label_1.text.font = get_font_lib()->msyh_16;

			lv_label_set_text(para.ui.label_5, get_text_by_id(LANG_WLAN_SELECT_DISCONNECT_OK));
			lv_label_set_style(para.ui.label_5, LV_LABEL_STYLE_MAIN, &style0_label_1);
			#else
			lv_label_set_text(para.ui.label_5, "disconnect ok");
			#endif
		}
		else {
			#if CONFIG_FONT_ENABLE
			lv_style_copy(&style0_label_1, &lv_style_transp);
			style0_label_1.text.color = lv_color_hex(0x000000);
			style0_label_1.text.line_space = 2;
			style0_label_1.text.font = get_font_lib()->msyh_16;

			lv_label_set_text(para.ui.label_5, get_text_by_id(LANG_WLAN_SELECT_DISCONNECT_ERR));
			lv_label_set_style(para.ui.label_5, LV_LABEL_STYLE_MAIN, &style0_label_1);
			#else
			lv_label_set_text(para.ui.label_5, "disconnect err");
			#endif

		}
	}
}

static void out_kb_area_event_cb(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		hidden_obj_and_all_child(para.ui.container_1, 1);
		hidden_obj_and_all_child(para.ui.container_mark, 1);
		update_list_context(&para);
	}
	#if 0
	lv_indev_t * indev = lv_indev_get_act();
	if (__is_in_obj(para.ui.container_1, indev->proc.types.pointer.act_point.x, indev->proc.types.pointer.act_point.y)) {
		;
	}
	else {
		hidden_obj_and_all_child(para.ui.container_1, 1);
		hidden_obj_and_all_child(para.ui.container_mark, 1);
		update_list_context(&para);
	}
	#endif
}

static void on_off_event_cb(lv_obj_t * btn, lv_event_t event)
{
	int ret;
	wifi_data_t wifi;
	get_wifi_data(&wifi);

	if (event == LV_EVENT_CLICKED)
	{
		if(wifi.is_on != 1) {
			wifi.event_label += 1;
			wifi.p_wifi_hd = net_wifi_on(wifi.event_label);
			if(wifi.p_wifi_hd != NULL) {
				com_info("open wifi ok\n");
				wifi.is_on = 1;
				wifi.manu_on = 1;

				#if 0
				if(wifi.manu_connected == 1 && (strlen(wifi.manu_ssid) != 0)) {
					ret = net_wifi_connect_ap(wifi.p_wifi_hd, wifi.manu_ssid,
						wifi.manu_password, wifi.event_label);
					if(ret == 1) {
						com_info("connect wifi ok\n");
						wifi.is_connected = 1;
					}
				}
				#endif
			}
		}
		else
		{
			if(wifi.is_connected) {
				ret = net_wifi_disconnect_ap(wifi.p_wifi_hd, wifi.event_label);
				if(ret == 0) {
					wifi.is_connected = 0;
					wifi.manu_connected = 0;
				}
			}

			ret = net_wifi_off(wifi.p_wifi_hd);		/* 先断开在关闭wifi */
			if(ret == 0) {
				com_info("close wifi ok\n");
				wifi.is_on = 0;
				wifi.manu_on = 0;
				wifi.p_wifi_hd = NULL;
			}
		}

		set_wifi_data(&wifi);
		wifi_manu_param_save(wifi);
		update_list_context(&para);
	}
}

static void update_list_context(wlan_set1_para_t *para)
{
	int i;
	lv_obj_t *tmp_btn;
	lv_obj_t *label;
	lv_obj_t *image;
	lv_obj_t *next_image;
	wifi_data_t wifi;
	int scan_num;
	static lv_style_t style0_label_1;
	net_wifi_scan_info_t p_scan_info[WIFI_MAX_SCAN_NUM];

	lv_style_copy(&style0_label_1, &lv_style_transp);
	lv_list_clean(para->ui.list_1);
	get_wifi_data(&wifi);

	pthread_mutex_lock(&para->moc.scan_mutex);
	scan_num = para->moc.scan_num;
	memcpy(p_scan_info, para->moc.p_scan_info, sizeof(para->moc.p_scan_info));
	pthread_mutex_unlock(&para->moc.scan_mutex);

	// btn1
	tmp_btn = lv_list_add_btn(para->ui.list_1, NULL, NULL);
	lv_btn_set_layout(tmp_btn, LV_LAYOUT_OFF);

	label = lv_label_create(tmp_btn, NULL);
	#if CONFIG_FONT_ENABLE
	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.line_space = 2;
	style0_label_1.text.font = get_font_lib()->msyh_16;

	lv_label_set_text(label, get_text_by_id(LANG_WLAN_OPEN_WLAN));
	lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style0_label_1);
	#else
	lv_label_set_text(label, "open wlan");
	#endif
	lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);

	para->moc.on_off_sw = on_off_sw_create(tmp_btn);
	lv_obj_align(para->moc.on_off_sw, label, LV_ALIGN_OUT_RIGHT_MID, 625, 0);
	lv_obj_set_event_cb(tmp_btn, on_off_event_cb);
	lv_obj_set_event_cb(para->moc.on_off_sw, on_off_event_cb);
	if(wifi.is_on == 1) {
		lv_sw_on(para->moc.on_off_sw, LV_ANIM_OFF);
	}
	else {
		lv_sw_off(para->moc.on_off_sw, LV_ANIM_OFF);
	}

	// btn2
	if(wifi.is_on == 1 && wifi.is_connected) {
		tmp_btn = lv_list_add_btn(para->ui.list_1, NULL, NULL);
		lv_btn_set_layout(tmp_btn, LV_LAYOUT_OFF);

		image = lv_img_create(tmp_btn, NULL);
		lv_img_set_src(image, get_img_by_level(wifi.rssi));
		lv_obj_align(image, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);

		if(strlen(wifi.ssid) != 0) {
			label = lv_label_create(tmp_btn, NULL);

			#if CONFIG_FONT_ENABLE
			lv_style_copy(&style0_label_1, &lv_style_transp);
			style0_label_1.text.color = lv_color_hex(0x000000);
			style0_label_1.text.line_space = 2;
			style0_label_1.text.font = get_font_lib()->msyh_16;

			lv_label_set_text(label, wifi.ssid);
			lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style0_label_1);
			#else
			lv_label_set_text(label, wifi.ssid);
			#endif

			lv_obj_align(label, image, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
		}

		lv_style_copy(&style0_button_3, &lv_style_btn_rel);
		style0_button_3.body.main_color = lv_color_hex(0x55aaff);
		style0_button_3.body.grad_color = lv_color_hex(0x55aaff);
		style0_button_3.body.radius = 0;

		lv_btn_set_style(tmp_btn, LV_BTN_STYLE_REL, &style0_button_3);
		lv_btn_set_style(tmp_btn, LV_BTN_STYLE_PR, &style0_button_3);
	}

	// btn3
	if(wifi.is_on == 1) {
		tmp_btn = lv_list_add_btn(para->ui.list_1, NULL, NULL);
		lv_btn_set_layout(tmp_btn, LV_LAYOUT_OFF);

		label = lv_label_create(tmp_btn, NULL);

		#if CONFIG_FONT_ENABLE
		lv_style_copy(&style0_label_1, &lv_style_transp);
		style0_label_1.text.color = lv_color_hex(0x000000);
		style0_label_1.text.line_space = 2;
		style0_label_1.text.font = get_font_lib()->msyh_16;

		lv_label_set_text(label, get_text_by_id(LANG_WLAN_SELECT_WLAN));
		lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style0_label_1);
		#else
		lv_label_set_text(label, "select the wlan");
		#endif

		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);

		image = lv_img_create(tmp_btn, NULL);
		lv_img_set_src(image, get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_CYCLE_IMAGE));
		lv_obj_align(image, label, LV_ALIGN_OUT_RIGHT_MID, 590, 0);
	}

	// btn more
	if(wifi.is_on == 1) {
		for(i=0; i<scan_num; i++) {
			tmp_btn = lv_list_add_btn(para->ui.list_1, NULL, NULL);
			lv_btn_set_layout(tmp_btn, LV_LAYOUT_OFF);

			image = lv_img_create(tmp_btn, NULL);
			lv_img_set_src(image, get_img_by_level(p_scan_info[i].level));
			lv_obj_align(image, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);

			label = lv_label_create(tmp_btn, NULL);

			#if CONFIG_FONT_ENABLE
			lv_style_copy(&style0_label_1, &lv_style_transp);
			style0_label_1.text.color = lv_color_hex(0x000000);
			style0_label_1.text.line_space = 2;
			style0_label_1.text.font = get_font_lib()->msyh_16;

			lv_label_set_text(label, p_scan_info[i].ssid);
			lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style0_label_1);
			#else
			lv_label_set_text(label, p_scan_info[i].ssid);
			#endif

			lv_obj_align(label, image, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

			next_image = lv_img_create(tmp_btn, NULL);
			lv_img_set_src(next_image, get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_NEXT_IMAGE));
			lv_obj_align(next_image, image, LV_ALIGN_OUT_RIGHT_MID, 665, 0);

			lv_obj_set_event_cb(tmp_btn, select_event_cb);
		}
	}
}

static void list_update_task(lv_task_t * task)
{
	wifi_data_t wifi;
	wlan_set1_para_t *para;

	get_wifi_data(&wifi);
	para = (wlan_set1_para_t *)task->user_data;
	if(wifi.update_flag) {
		update_list_context(para);

		wifi.update_flag = 0;
		set_wifi_data(&wifi);
	}
}

void* wifi_scan_thread(void *arg)
{
	int counter = 0;
	wifi_data_t wifi;
	wlan_set1_para_t *para;
	int scan_valid_len;
	char scan_results[WIFI_MAX_SCAN_SIZE];
	int scan_num;
	net_wifi_scan_info_t p_scan_info[WIFI_MAX_SCAN_NUM];

	while(1)
	{
		get_wifi_data(&wifi);
		para = (wlan_set1_para_t *)arg;

		if(para->moc.scan_quit_flag == 1) {
			pthread_exit((void*)0);
		}

		// 15秒扫描一次
		if(counter == 0) {
			if(wifi.is_on) {
			    memset(scan_results, 0, WIFI_MAX_SCAN_SIZE);
			    scan_valid_len = net_wifi_get_scan_results(wifi.p_wifi_hd, scan_results);

				memset(p_scan_info, 0, sizeof(p_scan_info));
				scan_num = net_wifi_parse_scan_results(p_scan_info,
					WIFI_MAX_SCAN_NUM, scan_results, scan_valid_len);

				pthread_mutex_lock(&para->moc.scan_mutex);
				memset(para->moc.scan_results, 0, WIFI_MAX_SCAN_SIZE);
				memcpy(para->moc.scan_results, scan_results, WIFI_MAX_SCAN_SIZE);
				para->moc.scan_valid_len = scan_valid_len;
				memset(para->moc.p_scan_info, 0, sizeof(para->moc.p_scan_info));
				memcpy(para->moc.p_scan_info, p_scan_info, sizeof(p_scan_info));
				para->moc.scan_num = scan_num;
				pthread_mutex_unlock(&para->moc.scan_mutex);

				wifi.update_flag = 1;
				set_wifi_data(&wifi);
			}
		}

		counter++;
		if(counter >= 150) {
			counter = 0;
		}

		usleep(100 * 1000);
	}
}

static void wifi_info_update_task(lv_task_t * task)
{
	wifi_data_t wifi;
	wlan_set1_para_t *para;
	int is_wifi_connected = 0;
	connection_status info = {0};

	get_wifi_data(&wifi);
	para = (wlan_set1_para_t *)task->user_data;

	if(wifi.is_on != 1) {
		return;
	}

	if(net_wifi_get_wifi_state() == NETWORK_CONNECTED)
	{
		is_wifi_connected = 1;
	}
	else
	{
		is_wifi_connected = 0;
	}

	wifi.is_connected = is_wifi_connected;
	if(is_wifi_connected)
	{
		memset(&info, 0, sizeof(connection_status));
		net_wifi_get_connect_info(wifi.p_wifi_hd, &info);

		wifi.rssi = 100 + info.rssi;
		memcpy(wifi.ssid, info.ssid, strlen(info.ssid));
		memcpy(&para->moc.connect_info, &info, sizeof(connection_status));
	}
	else
	{
		wifi.rssi = 0;
		memset(wifi.ssid, 0, sizeof(wifi.ssid));
		memset(&para->moc.connect_info, 0, sizeof(connection_status));
	}
	set_wifi_data(&wifi);
}

void wifi_pic_res_uninit(void)
{
	free_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]));
}

static void update_common_font(void)
{
	static lv_style_t style0_label_1;
	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.line_space = 2;
	style0_label_1.text.font = get_font_lib()->msyh_16;

	lv_label_set_text(para.ui.label_1, get_text_by_id(LANG_WLAN_WLAN));
	lv_label_set_style(para.ui.label_1, LV_LABEL_STYLE_MAIN, &style0_label_1);

	lv_label_set_text(para.ui.label_3, get_text_by_id(LANG_WLAN_CONNECT));
	lv_label_set_style(para.ui.label_3, LV_LABEL_STYLE_MAIN, &style0_label_1);

	lv_label_set_text(para.ui.label_4, get_text_by_id(LANG_WLAN_DISCONNECT));
	lv_label_set_style(para.ui.label_4, LV_LABEL_STYLE_MAIN, &style0_label_1);
}

static void wlan_set1_moc_create(void)
{
	//lv_task_t *scan_once_tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_attr_setstacksize(&attr, 0x40000);
	pthread_mutex_init(&para.moc.scan_mutex, NULL);
	//para.moc.wifi_info_update_tid = lv_task_create(wifi_info_update_task, 500, LV_TASK_PRIO_MID, &para);
	//lv_task_ready(para.moc.wifi_info_update_tid);

	para.moc.list_update_tid = lv_task_create(list_update_task, 100, LV_TASK_PRIO_MID, &para);
	lv_task_ready(para.moc.list_update_tid);

	pthread_create(&para.moc.wifi_scan_tid, &attr, wifi_scan_thread, &para);
	pthread_attr_destroy(&attr);

	update_list_context(&para);
	hidden_obj_and_all_child(para.ui.container_1, 1);
	hidden_obj_and_all_child(para.ui.container_mark, 1);
	lv_kb_set_ta(para.ui.keyboard_1, para.ui.text_area_1);
	lv_obj_set_event_cb(para.ui.button_1, connect_event_cb);
	lv_obj_set_event_cb(para.ui.button_2, disconnect_event_cb);
	lv_obj_set_event_cb(para.ui.container_mark, out_kb_area_event_cb);
	lv_obj_set_event_cb(para.ui.btn_hbar_return, btn_hbar_return_event_cb);
	lv_obj_set_event_cb(para.ui.btn_hbar_home, btn_hbar_return_event_cb);

	#if CONFIG_FONT_ENABLE
	update_common_font();
	#endif
}

static void wlan_set1_moc_destory(void)
{
	para.moc.scan_quit_flag = 1;
	pthread_join(para.moc.wifi_scan_tid, NULL);

	lv_task_del(para.moc.list_update_tid);
	//lv_task_del(para.moc.wifi_info_update_tid);
	pthread_mutex_destroy(&para.moc.scan_mutex);
	lv_list_clean(para.ui.list_1);
}

static int create_wlan_set1(void)
{
	memset(&para, 0, sizeof(wlan_set1_para_t));
	para.ui.cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(para.ui.cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color = LV_COLOR_BLUE;
	cont_style.body.grad_color = LV_COLOR_BLUE;
	lv_cont_set_style(para.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
	lv_cont_set_layout(para.ui.cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(para.ui.cont, LV_FIT_NONE);

	#if 0
	static lv_style_t back_btn_style;
	lv_style_copy(&back_btn_style, &lv_style_pretty);
	back_btn_style.text.font = &lv_font_roboto_28;
	lv_obj_t *back_btn = lv_btn_create(para.ui.cont, NULL);
	lv_obj_align(back_btn, para.ui.cont, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_t *back_lable = lv_label_create(back_btn, NULL);
	lv_label_set_text(back_lable, LV_SYMBOL_LEFT);
	lv_obj_set_event_cb(back_btn, back_btn_event);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_REL, &back_btn_style);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_PR, &back_btn_style);
	#endif

	wlan_set1_auto_ui_create(&para.ui);
	wlan_set1_moc_create();

	return 0;
}

static int destory_wlan_set1(void)
{
	wlan_set1_moc_destory();
	wlan_set1_auto_ui_destory(&para.ui);
	lv_obj_del(para.ui.cont);

	return 0;
}

static int show_wlan_set1(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);

	return 0;
}

static int hide_wlan_set1(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);

	return 0;
}

static int msg_proc_wlan_set1(MsgDataInfo *msg)
{
	return 0;
}

static page_interface_t page_wlan_set1 =
{
	.ops =
	{
		create_wlan_set1,
		destory_wlan_set1,
		show_wlan_set1,
		hide_wlan_set1,
		msg_proc_wlan_set1,
	},
	.info =
	{
		.id         = PAGE_WLAN_SET1,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_WLAN_SET1(void)
{
	reg_page(&page_wlan_set1);
}
