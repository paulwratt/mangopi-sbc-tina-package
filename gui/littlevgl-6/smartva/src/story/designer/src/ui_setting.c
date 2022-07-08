/**********************
 *      includes
 **********************/
#include "ui_setting.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/
static lv_style_t style_screen;
static lv_style_t style0_line_1;
static lv_style_t style0_line_2;
static lv_style_t style0_label_setting;
static lv_style_t style1_list_1;
static lv_style_t style3_list_1;
static lv_style_t style4_list_1;
static lv_style_t style5_list_1;
static lv_style_t style6_list_1;
static lv_style_t style7_list_1;
static lv_style_t style0_label_10;
static lv_style_t style0_line_5;
static lv_style_t style0_infomation_cont;
static lv_style_t style0_container_1;
static lv_style_t style0_line_19;
static lv_style_t style0_line_20;
static lv_style_t style0_container_3;
static lv_style_t style0_label_dialog;
static lv_style_t style0_image_btn_return;
static lv_style_t style1_image_btn_return;
static lv_style_t style0_img_sett_return;
static lv_style_t style0_display_cont;
static lv_style_t style0_cont_backlight;
static lv_style_t style1_slider_backlight;
static lv_style_t style2_slider_backlight;
static lv_style_t style0_cont_enhance;
static lv_style_t style0_line_7;
static lv_style_t style0_switch_enhance;
static lv_style_t style1_switch_enhance;
static lv_style_t style2_switch_enhance;
static lv_style_t style3_switch_enhance;
static lv_style_t style1_slider_saturation;
static lv_style_t style2_slider_saturation;
static lv_style_t style1_slider_contrast;
static lv_style_t style2_slider_contrast;
static lv_style_t style1_slider_bright;
static lv_style_t style2_slider_bright;
static lv_style_t style0_line_17;
static lv_style_t style0_line_18;
static lv_style_t style0_general_cont;
static lv_style_t style0_cont_time_set;

/**********************
 *  images and fonts
 **********************/
static void *image_hour1_1_png = NULL;
static void *image_hour2_2_png = NULL;
static void *image_min1_3_png = NULL;
static void *image_min2_2_png = NULL;
static void *image_time_dot_time_dot_png = NULL;
static void *list_1_1_setting_general_png = NULL;
static void *list_1_2_setting_display_png = NULL;
static void *list_1_3_setting_language_png = NULL;
static void *list_1_4_setting_information_png = NULL;
static void *list_1_5_setting_wallpaper_png = NULL;
static void *list_1_6_setting_password_png = NULL;
static void *list_1_7_setting_date_and_time_png = NULL;
static void *list_1_8_setting_date_and_time_png = NULL;

/**********************
 *  functions
 **********************/
void setting_auto_ui_create(setting_ui_t *ui)
{
	lv_style_copy(&style_screen, &lv_style_scr);
	style_screen.body.main_color = lv_color_hex(0xf2f2f2);
	style_screen.body.grad_color = lv_color_hex(0xf2f2f2);
	lv_obj_set_style(ui->cont, &style_screen);

#ifdef LV_USE_LINE
	lv_style_copy(&style0_line_1, &lv_style_pretty);
	style0_line_1.line.color = lv_color_hex(0xb9b8bd);
	style0_line_1.line.width = 1;

	ui->line_1 = lv_line_create(ui->cont, NULL);
	lv_obj_set_pos(ui->line_1, 200, 0);
	lv_obj_set_size(ui->line_1, 2, 482);
	static lv_point_t line_1_points[2] = {
		{0,0},
		{0,480},
		};
	lv_line_set_points(ui->line_1, line_1_points, 2);
	lv_line_set_style(ui->line_1, LV_LINE_STYLE_MAIN, &style0_line_1);
#endif // LV_USE_LINE

#ifdef LV_USE_LINE
	lv_style_copy(&style0_line_2, &lv_style_pretty);
	style0_line_2.line.color = lv_color_hex(0xb9b8bd);
	style0_line_2.line.width = 1;

	ui->line_2 = lv_line_create(ui->cont, NULL);
	lv_obj_set_pos(ui->line_2, 0, 54);
	lv_obj_set_size(ui->line_2, 202, 17);
	static lv_point_t line_2_points[2] = {
		{0,15},
		{200,15},
		};
	lv_line_set_points(ui->line_2, line_2_points, 2);
	lv_line_set_style(ui->line_2, LV_LINE_STYLE_MAIN, &style0_line_2);
#endif // LV_USE_LINE

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_setting, &lv_style_transp);
	style0_label_setting.text.color = lv_color_hex(0x3d3d3d);
	style0_label_setting.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_setting.text.line_space = 2;

	ui->label_setting = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_setting, "Setting");
	lv_label_set_long_mode(ui->label_setting, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_setting, 86, 16);
	lv_obj_set_size(ui->label_setting, 82, 32);
	lv_label_set_style(ui->label_setting, LV_LABEL_STYLE_MAIN, &style0_label_setting);
#endif // LV_USE_LABEL

#ifdef LV_USE_LIST
	lv_style_copy(&style1_list_1, &lv_style_pretty);
	style1_list_1.body.padding.top = 0;
	style1_list_1.body.padding.bottom = 0;
	style1_list_1.body.padding.left = 0;
	style1_list_1.body.padding.right = 0;
	style1_list_1.body.padding.inner = 0;

	lv_style_copy(&style3_list_1, &lv_style_transp);
	style3_list_1.body.main_color = lv_color_hex(0x5596d8);
	style3_list_1.body.grad_color = lv_color_hex(0x5596d8);
	style3_list_1.body.radius = 0;
	style3_list_1.body.opa = 255;
	style3_list_1.body.border.color = lv_color_hex(0x000000);
	style3_list_1.body.border.width = 0;
	style3_list_1.body.border.part = 15;
	style3_list_1.body.border.opa = 255;
	style3_list_1.body.shadow.color = lv_color_hex(0x808080);
	style3_list_1.body.shadow.width = 0;
	style3_list_1.body.shadow.type = 1;
	style3_list_1.body.padding.top = 5;
	style3_list_1.body.padding.bottom = 5;
	style3_list_1.body.padding.left = 5;
	style3_list_1.body.padding.right = 5;
	style3_list_1.body.padding.inner = 5;
	style3_list_1.text.color = lv_color_hex(0x000000);
	style3_list_1.text.sel_color = lv_color_hex(0x5596d8);
	style3_list_1.text.font = &lv_font_roboto_16;
	style3_list_1.text.letter_space = 0;
	style3_list_1.text.line_space = 2;
	style3_list_1.text.opa = 255;
	style3_list_1.image.color = lv_color_hex(0xf0f0f0);
	style3_list_1.image.intense = 0;
	style3_list_1.image.opa = 255;

	lv_style_copy(&style4_list_1, &lv_style_btn_rel);
	style4_list_1.body.main_color = lv_color_hex(0xffffff);
	style4_list_1.body.grad_color = lv_color_hex(0xffffff);
	style4_list_1.body.radius = 0;
	style4_list_1.body.border.color = lv_color_hex(0xe3e3e3);
	style4_list_1.body.border.part = 1;
	style4_list_1.text.color = lv_color_hex(0x2d2d2d);
	style4_list_1.text.line_space = 2;

	lv_style_copy(&style5_list_1, &lv_style_btn_pr);
	style5_list_1.body.main_color = lv_color_hex(0xc0c0c0);
	style5_list_1.body.grad_color = lv_color_hex(0xc0c0c0);
	style5_list_1.body.radius = 0;
	style5_list_1.body.border.width = 0;
	style5_list_1.body.padding.inner = 20;
	style5_list_1.text.color = lv_color_hex(0x2d2d2d);
	style5_list_1.text.line_space = 2;

	lv_style_copy(&style6_list_1, &lv_style_btn_tgl_rel);
	style6_list_1.body.main_color = lv_color_hex(0xc0c0c0);
	style6_list_1.body.grad_color = lv_color_hex(0xc0c0c0);
	style6_list_1.body.radius = 0;
	style6_list_1.body.border.width = 0;
	style6_list_1.body.padding.inner = 20;
	style6_list_1.text.color = lv_color_hex(0x2d2d2d);
	style6_list_1.text.line_space = 2;

	lv_style_copy(&style7_list_1, &lv_style_btn_tgl_pr);
	style7_list_1.body.main_color = lv_color_hex(0xc0c0c0);
	style7_list_1.body.grad_color = lv_color_hex(0xc0c0c0);
	style7_list_1.body.radius = 0;
	style7_list_1.body.border.width = 0;
	style7_list_1.body.border.part = 1;
	style7_list_1.body.padding.inner = 20;
	style7_list_1.text.color = lv_color_hex(0x2d2d2d);
	style7_list_1.text.line_space = 2;

	ui->list_1 = lv_list_create(ui->cont, NULL);
	lv_obj_set_pos(ui->list_1, 0, 70);
	lv_obj_set_size(ui->list_1, 197, 410);

	list_1_1_setting_general_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_general.png");
	lv_list_add_btn(ui->list_1, list_1_1_setting_general_png, "General");

	list_1_2_setting_display_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_display.png");
	lv_list_add_btn(ui->list_1, list_1_2_setting_display_png, "Display");

	list_1_3_setting_language_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_language.png");
	lv_list_add_btn(ui->list_1, list_1_3_setting_language_png, "Language");

	list_1_4_setting_information_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_information.png");
	lv_list_add_btn(ui->list_1, list_1_4_setting_information_png, "Information");

	list_1_5_setting_wallpaper_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_wallpaper.png");
	lv_list_add_btn(ui->list_1, list_1_5_setting_wallpaper_png, "Wallpaper");

	list_1_6_setting_password_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_password.png");
	lv_list_add_btn(ui->list_1, list_1_6_setting_password_png, "Password");

	list_1_7_setting_date_and_time_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_date_and_time.png");
	lv_list_add_btn(ui->list_1, list_1_7_setting_date_and_time_png, "Time");

	list_1_8_setting_date_and_time_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_date_and_time.png");
	lv_list_add_btn(ui->list_1, list_1_8_setting_date_and_time_png, "Default setting");
	lv_list_set_single_mode(ui->list_1, true);
	lv_list_set_scroll_propagation(ui->list_1, false);
	lv_list_set_edge_flash(ui->list_1, true);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_SCRL, &style1_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_EDGE_FLASH, &style3_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BTN_REL, &style4_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BTN_PR, &style5_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BTN_TGL_REL, &style6_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BTN_TGL_PR, &style7_list_1);
#endif // LV_USE_LIST

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_10, &lv_style_transp);
	style0_label_10.text.font = &lv_font_roboto_28;
	style0_label_10.text.line_space = 2;

	ui->label_10 = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_10, "General");
	lv_label_set_long_mode(ui->label_10, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_10, 448, 24);
	lv_obj_set_size(ui->label_10, 95, 32);
	lv_label_set_style(ui->label_10, LV_LABEL_STYLE_MAIN, &style0_label_10);
#endif // LV_USE_LABEL

#ifdef LV_USE_LINE
	lv_style_copy(&style0_line_5, &lv_style_pretty);
	style0_line_5.line.color = lv_color_hex(0xb1b1b1);
	style0_line_5.line.width = 1;

	ui->line_5 = lv_line_create(ui->cont, NULL);
	lv_obj_set_pos(ui->line_5, 200, 54);
	lv_obj_set_size(ui->line_5, 602, 17);
	static lv_point_t line_5_points[2] = {
		{0,15},
		{600,15},
		};
	lv_line_set_points(ui->line_5, line_5_points, 2);
	lv_line_set_style(ui->line_5, LV_LINE_STYLE_MAIN, &style0_line_5);
#endif // LV_USE_LINE

#ifdef LV_USE_CONT
	lv_style_copy(&style0_infomation_cont, &lv_style_pretty);
	style0_infomation_cont.body.main_color = lv_color_hex(0xf2f2f2);
	style0_infomation_cont.body.grad_color = lv_color_hex(0xf2f2f2);
	style0_infomation_cont.body.radius = 10;
	style0_infomation_cont.body.border.width = 0;

	ui->infomation_cont = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->infomation_cont, 207, 79);
	lv_obj_set_size(ui->infomation_cont, 580, 389);
	lv_cont_set_fit4(ui->infomation_cont, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_hidden(ui->infomation_cont,true);
	lv_cont_set_style(ui->infomation_cont, LV_CONT_STYLE_MAIN, &style0_infomation_cont);
#endif // LV_USE_CONT

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_1, &lv_style_pretty);
	style0_container_1.body.grad_color = lv_color_hex(0xffffff);
	style0_container_1.body.radius = 10;
	style0_container_1.body.border.width = 1;

	ui->container_1 = lv_cont_create(ui->infomation_cont, NULL);
	lv_obj_set_pos(ui->container_1, 0, 0);
	lv_obj_set_size(ui->container_1, 550, 147);
	lv_cont_set_fit4(ui->container_1, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->container_1, LV_CONT_STYLE_MAIN, &style0_container_1);
#endif // LV_USE_CONT

#ifdef LV_USE_LABEL
	ui->label_sdk_version = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->label_sdk_version, "SDK version");
	lv_label_set_long_mode(ui->label_sdk_version, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_sdk_version, 26, 17);
	lv_obj_set_size(ui->label_sdk_version, 87, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_sdk_val = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->label_sdk_val, "V0.6");
	lv_label_set_long_mode(ui->label_sdk_val, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_sdk_val, 480, 17);
	lv_obj_set_size(ui->label_sdk_val, 32, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_os_version = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->label_os_version, "OS version");
	lv_label_set_long_mode(ui->label_os_version, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_os_version, 27, 65);
	lv_obj_set_size(ui->label_os_version, 77, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_os_val = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->label_os_val, "tina 3.5@2022-01-17");
	lv_label_set_long_mode(ui->label_os_val, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_os_val, 365, 67);
	lv_obj_set_size(ui->label_os_val, 147, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_ui_version = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->label_ui_version, "UI version");
	lv_label_set_long_mode(ui->label_ui_version, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_ui_version, 28, 114);
	lv_obj_set_size(ui->label_ui_version, 70, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_ui_val = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->label_ui_val, "V0.6");
	lv_label_set_long_mode(ui->label_ui_val, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_ui_val, 480, 115);
	lv_obj_set_size(ui->label_ui_val, 32, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LINE
	lv_style_copy(&style0_line_19, &lv_style_pretty);
	style0_line_19.line.color = lv_color_hex(0xb1b1b1);
	style0_line_19.line.width = 1;

	ui->line_19 = lv_line_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->line_19, 28, 34);
	lv_obj_set_size(ui->line_19, 522, 17);
	static lv_point_t line_19_points[2] = {
		{0,15},
		{520,15},
		};
	lv_line_set_points(ui->line_19, line_19_points, 2);
	lv_line_set_style(ui->line_19, LV_LINE_STYLE_MAIN, &style0_line_19);
#endif // LV_USE_LINE

#ifdef LV_USE_LINE
	lv_style_copy(&style0_line_20, &lv_style_pretty);
	style0_line_20.line.color = lv_color_hex(0xb1b1b1);
	style0_line_20.line.width = 1;

	ui->line_20 = lv_line_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->line_20, 29, 84);
	lv_obj_set_size(ui->line_20, 522, 17);
	static lv_point_t line_20_points[2] = {
		{0,15},
		{520,15},
		};
	lv_line_set_points(ui->line_20, line_20_points, 2);
	lv_line_set_style(ui->line_20, LV_LINE_STYLE_MAIN, &style0_line_20);
#endif // LV_USE_LINE

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_3, &lv_style_pretty);
	style0_container_3.body.grad_color = lv_color_hex(0xffffff);
	style0_container_3.body.border.width = 1;

	ui->container_3 = lv_cont_create(ui->infomation_cont, NULL);
	lv_obj_set_pos(ui->container_3, 0, 173);
	lv_obj_set_size(ui->container_3, 550, 50);
	lv_cont_set_fit4(ui->container_3, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->container_3, LV_CONT_STYLE_MAIN, &style0_container_3);
#endif // LV_USE_CONT

#ifdef LV_USE_LABEL
	ui->label_screen_info = lv_label_create(ui->container_3, NULL);
	lv_label_set_text(ui->label_screen_info, "Screen info");
	lv_label_set_long_mode(ui->label_screen_info, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_screen_info, 26, 18);
	lv_obj_set_size(ui->label_screen_info, 82, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_screen_val = lv_label_create(ui->container_3, NULL);
	lv_label_set_text(ui->label_screen_val, "800*480 @60fps");
	lv_label_set_long_mode(ui->label_screen_val, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_screen_val, 416, 19);
	lv_obj_set_size(ui->label_screen_val, 120, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_def_setting = lv_btn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->button_def_setting, 367, 229);
	lv_obj_set_size(ui->button_def_setting, 190, 81);
	lv_obj_set_hidden(ui->button_def_setting,true);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_btn_def_setting = lv_label_create(ui->button_def_setting, NULL);
	lv_label_set_text(ui->label_btn_def_setting, "Default setting");
	lv_label_set_align(ui->label_btn_def_setting, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_btn_def_setting, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_btn_def_setting, 42, 31);
	lv_obj_set_size(ui->label_btn_def_setting, 106, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_CONT
	ui->cont_dialog = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->cont_dialog, 368, 212);
	lv_obj_set_size(ui->cont_dialog, 249, 103);
	lv_cont_set_fit4(ui->cont_dialog, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_hidden(ui->cont_dialog,true);
#endif // LV_USE_CONT

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_dialog, &lv_style_transp);
	style0_label_dialog.text.color = lv_color_hex(0x000000);
	style0_label_dialog.text.line_space = 2;

	ui->label_dialog = lv_label_create(ui->cont_dialog, NULL);
	lv_label_set_text(ui->label_dialog, "reseting ...");
	lv_label_set_align(ui->label_dialog, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_dialog, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_dialog, 91, 50);
	lv_obj_set_size(ui->label_dialog, 74, 19);
	lv_label_set_style(ui->label_dialog, LV_LABEL_STYLE_MAIN, &style0_label_dialog);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_image_btn_return, &lv_style_btn_rel);
	style0_image_btn_return.body.main_color = lv_color_hex(0xf9eef1);
	style0_image_btn_return.body.grad_color = lv_color_hex(0xf9eef1);
	style0_image_btn_return.body.radius = 12;
	style0_image_btn_return.body.opa = 0;
	style0_image_btn_return.body.border.width = 1;
	style0_image_btn_return.body.shadow.width = 1;
	style0_image_btn_return.body.padding.top = 0;
	style0_image_btn_return.body.padding.bottom = 0;
	style0_image_btn_return.body.padding.left = 0;

	lv_style_copy(&style1_image_btn_return, &lv_style_btn_pr);
	style1_image_btn_return.body.main_color = lv_color_hex(0xe5d6dc);
	style1_image_btn_return.body.grad_color = lv_color_hex(0xe5d6dc);
	style1_image_btn_return.body.radius = 12;
	style1_image_btn_return.body.border.width = 1;

	ui->image_btn_return = lv_btn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_btn_return, 24, 14);
	lv_obj_set_size(ui->image_btn_return, 57, 35);
	lv_btn_set_style(ui->image_btn_return, LV_BTN_STYLE_REL, &style0_image_btn_return);
	lv_btn_set_style(ui->image_btn_return, LV_BTN_STYLE_PR, &style1_image_btn_return);
#endif // LV_USE_BTN

#ifdef LV_USE_IMG
	lv_style_copy(&style0_img_sett_return, &lv_style_plain);
	style0_img_sett_return.image.color = lv_color_hex(0x000000);

	ui->img_sett_return = lv_img_create(ui->image_btn_return, NULL);
	lv_obj_set_pos(ui->img_sett_return, 23, 8);
	lv_obj_set_size(ui->img_sett_return, 10, 19);
	lv_img_set_src(ui->img_sett_return, LV_SYMBOL_LEFT);
	lv_img_set_style(ui->img_sett_return, LV_IMG_STYLE_MAIN, &style0_img_sett_return);
#endif // LV_USE_IMG

#ifdef LV_USE_CONT
	lv_style_copy(&style0_display_cont, &lv_style_pretty);
	style0_display_cont.body.main_color = lv_color_hex(0xf2f2f2);
	style0_display_cont.body.grad_color = lv_color_hex(0xf2f2f2);
	style0_display_cont.body.radius = 10;
	style0_display_cont.body.border.width = 0;

	ui->display_cont = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->display_cont, 207, 76);
	lv_obj_set_size(ui->display_cont, 565, 374);
	lv_cont_set_fit4(ui->display_cont, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_hidden(ui->display_cont,true);
	lv_cont_set_style(ui->display_cont, LV_CONT_STYLE_MAIN, &style0_display_cont);
#endif // LV_USE_CONT

#ifdef LV_USE_CONT
	lv_style_copy(&style0_cont_backlight, &lv_style_pretty);
	style0_cont_backlight.body.grad_color = lv_color_hex(0xffffff);
	style0_cont_backlight.body.radius = 10;
	style0_cont_backlight.body.border.width = 1;

	ui->cont_backlight = lv_cont_create(ui->display_cont, NULL);
	lv_obj_set_pos(ui->cont_backlight, 6, 21);
	lv_obj_set_size(ui->cont_backlight, 550, 50);
	lv_cont_set_fit4(ui->cont_backlight, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->cont_backlight, LV_CONT_STYLE_MAIN, &style0_cont_backlight);
#endif // LV_USE_CONT

#ifdef LV_USE_SLIDER
	lv_style_copy(&style1_slider_backlight, &lv_style_pretty_color);
	style1_slider_backlight.body.padding.top = 0;
	style1_slider_backlight.body.padding.bottom = 0;
	style1_slider_backlight.body.padding.left = 0;
	style1_slider_backlight.body.padding.right = 0;
	style1_slider_backlight.body.padding.inner = 0;

	lv_style_copy(&style2_slider_backlight, &lv_style_pretty);
	style2_slider_backlight.body.radius = 20;

	ui->slider_backlight = lv_slider_create(ui->cont_backlight, NULL);
	lv_obj_set_pos(ui->slider_backlight, 130, 13);
	lv_obj_set_size(ui->slider_backlight, 350, 25);
	lv_slider_set_range(ui->slider_backlight, 0, 100);
	lv_slider_set_knob_in(ui->slider_backlight, true);
	lv_slider_set_value(ui->slider_backlight, 0, LV_ANIM_OFF);
	lv_slider_set_style(ui->slider_backlight, LV_SLIDER_STYLE_INDIC, &style1_slider_backlight);
	lv_slider_set_style(ui->slider_backlight, LV_SLIDER_STYLE_KNOB, &style2_slider_backlight);
#endif // LV_USE_SLIDER

#ifdef LV_USE_LABEL
	ui->label_backlight = lv_label_create(ui->cont_backlight, NULL);
	lv_label_set_text(ui->label_backlight, "backlight");
	lv_label_set_long_mode(ui->label_backlight, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_backlight, 15, 16);
	lv_obj_set_size(ui->label_backlight, 65, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_num_backlight = lv_label_create(ui->cont_backlight, NULL);
	lv_label_set_text(ui->label_num_backlight, "50%");
	lv_label_set_long_mode(ui->label_num_backlight, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_num_backlight, 500, 16);
	lv_obj_set_size(ui->label_num_backlight, 30, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_CONT
	lv_style_copy(&style0_cont_enhance, &lv_style_pretty);
	style0_cont_enhance.body.grad_color = lv_color_hex(0xffffff);
	style0_cont_enhance.body.border.width = 1;

	ui->cont_enhance = lv_cont_create(ui->display_cont, NULL);
	lv_obj_set_pos(ui->cont_enhance, 7, 77);
	lv_obj_set_size(ui->cont_enhance, 550, 200);
	lv_cont_set_fit4(ui->cont_enhance, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->cont_enhance, LV_CONT_STYLE_MAIN, &style0_cont_enhance);
#endif // LV_USE_CONT

#ifdef LV_USE_LABEL
	ui->label_enhance = lv_label_create(ui->cont_enhance, NULL);
	lv_label_set_text(ui->label_enhance, "enhance");
	lv_label_set_long_mode(ui->label_enhance, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_enhance, 17, 17);
	lv_obj_set_size(ui->label_enhance, 62, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LINE
	lv_style_copy(&style0_line_7, &lv_style_pretty);
	style0_line_7.line.color = lv_color_hex(0xb1b1b1);
	style0_line_7.line.width = 1;

	ui->line_7 = lv_line_create(ui->cont_enhance, NULL);
	lv_obj_set_pos(ui->line_7, 16, 28);
	lv_obj_set_size(ui->line_7, 532, 17);
	static lv_point_t line_7_points[2] = {
		{0,15},
		{530,15},
		};
	lv_line_set_points(ui->line_7, line_7_points, 2);
	lv_line_set_style(ui->line_7, LV_LINE_STYLE_MAIN, &style0_line_7);
#endif // LV_USE_LINE

#ifdef LV_USE_SW
	lv_style_copy(&style0_switch_enhance, &lv_style_pretty);
	style0_switch_enhance.body.main_color = lv_color_hex(0x55aaff);
	style0_switch_enhance.body.grad_color = lv_color_hex(0x55aaff);
	style0_switch_enhance.body.radius = 40;
	style0_switch_enhance.body.border.width = 0;
	style0_switch_enhance.body.padding.top = 0;
	style0_switch_enhance.body.padding.bottom = 0;
	style0_switch_enhance.body.padding.left = 3;
	style0_switch_enhance.body.padding.right = 1;
	style0_switch_enhance.body.padding.inner = 0;

	lv_style_copy(&style1_switch_enhance, &lv_style_pretty_color);
	style1_switch_enhance.body.opa = 0;
	style1_switch_enhance.body.border.width = 0;

	lv_style_copy(&style2_switch_enhance, &lv_style_pretty);
	style2_switch_enhance.body.main_color = lv_color_hex(0xe8e8e8);
	style2_switch_enhance.body.grad_color = lv_color_hex(0xe8e8e8);
	style2_switch_enhance.body.radius = 40;
	style2_switch_enhance.body.border.width = 0;

	lv_style_copy(&style3_switch_enhance, &lv_style_pretty);
	style3_switch_enhance.body.main_color = lv_color_hex(0xe8e8e8);
	style3_switch_enhance.body.grad_color = lv_color_hex(0xe8e8e8);
	style3_switch_enhance.body.radius = 40;
	style3_switch_enhance.body.border.width = 0;

	ui->switch_enhance = lv_sw_create(ui->cont_enhance, NULL);
	lv_obj_set_pos(ui->switch_enhance, 487, 13);
	lv_obj_set_size(ui->switch_enhance, 50, 25);
	lv_sw_on(ui->switch_enhance, LV_ANIM_OFF);
	lv_sw_set_style(ui->switch_enhance, LV_SW_STYLE_BG, &style0_switch_enhance);
	lv_sw_set_style(ui->switch_enhance, LV_SW_STYLE_INDIC, &style1_switch_enhance);
	lv_sw_set_style(ui->switch_enhance, LV_SW_STYLE_KNOB_OFF, &style2_switch_enhance);
	lv_sw_set_style(ui->switch_enhance, LV_SW_STYLE_KNOB_ON, &style3_switch_enhance);
#endif // LV_USE_SW

#ifdef LV_USE_LABEL
	ui->label_saturation = lv_label_create(ui->cont_enhance, NULL);
	lv_label_set_text(ui->label_saturation, "saturation");
	lv_label_set_long_mode(ui->label_saturation, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_saturation, 16, 65);
	lv_obj_set_size(ui->label_saturation, 72, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_contrast = lv_label_create(ui->cont_enhance, NULL);
	lv_label_set_text(ui->label_contrast, "contrast");
	lv_label_set_long_mode(ui->label_contrast, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_contrast, 19, 113);
	lv_obj_set_size(ui->label_contrast, 58, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_bright = lv_label_create(ui->cont_enhance, NULL);
	lv_label_set_text(ui->label_bright, "bright");
	lv_label_set_long_mode(ui->label_bright, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_bright, 18, 166);
	lv_obj_set_size(ui->label_bright, 41, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_SLIDER
	lv_style_copy(&style1_slider_saturation, &lv_style_pretty_color);
	style1_slider_saturation.body.padding.top = 0;
	style1_slider_saturation.body.padding.bottom = 0;
	style1_slider_saturation.body.padding.left = 0;
	style1_slider_saturation.body.padding.right = 0;
	style1_slider_saturation.body.padding.inner = 0;

	lv_style_copy(&style2_slider_saturation, &lv_style_pretty);
	style2_slider_saturation.body.radius = 10;

	ui->slider_saturation = lv_slider_create(ui->cont_enhance, NULL);
	lv_obj_set_pos(ui->slider_saturation, 131, 63);
	lv_obj_set_size(ui->slider_saturation, 350, 25);
	lv_slider_set_range(ui->slider_saturation, 0, 100);
	lv_slider_set_knob_in(ui->slider_saturation, true);
	lv_slider_set_value(ui->slider_saturation, 0, LV_ANIM_OFF);
	lv_slider_set_style(ui->slider_saturation, LV_SLIDER_STYLE_INDIC, &style1_slider_saturation);
	lv_slider_set_style(ui->slider_saturation, LV_SLIDER_STYLE_KNOB, &style2_slider_saturation);
#endif // LV_USE_SLIDER

#ifdef LV_USE_SLIDER
	lv_style_copy(&style1_slider_contrast, &lv_style_pretty_color);
	style1_slider_contrast.body.padding.top = 0;
	style1_slider_contrast.body.padding.bottom = 0;
	style1_slider_contrast.body.padding.left = 0;
	style1_slider_contrast.body.padding.right = 0;
	style1_slider_contrast.body.padding.inner = 0;

	lv_style_copy(&style2_slider_contrast, &lv_style_pretty);
	style2_slider_contrast.body.radius = 10;

	ui->slider_contrast = lv_slider_create(ui->cont_enhance, NULL);
	lv_obj_set_pos(ui->slider_contrast, 130, 112);
	lv_obj_set_size(ui->slider_contrast, 350, 25);
	lv_slider_set_range(ui->slider_contrast, 0, 100);
	lv_slider_set_knob_in(ui->slider_contrast, true);
	lv_slider_set_value(ui->slider_contrast, 0, LV_ANIM_OFF);
	lv_slider_set_style(ui->slider_contrast, LV_SLIDER_STYLE_INDIC, &style1_slider_contrast);
	lv_slider_set_style(ui->slider_contrast, LV_SLIDER_STYLE_KNOB, &style2_slider_contrast);
#endif // LV_USE_SLIDER

#ifdef LV_USE_SLIDER
	lv_style_copy(&style1_slider_bright, &lv_style_pretty_color);
	style1_slider_bright.body.padding.top = 0;
	style1_slider_bright.body.padding.bottom = 0;
	style1_slider_bright.body.padding.left = 0;
	style1_slider_bright.body.padding.right = 0;
	style1_slider_bright.body.padding.inner = 0;

	lv_style_copy(&style2_slider_bright, &lv_style_pretty);
	style2_slider_bright.body.radius = 10;

	ui->slider_bright = lv_slider_create(ui->cont_enhance, NULL);
	lv_obj_set_pos(ui->slider_bright, 134, 165);
	lv_obj_set_size(ui->slider_bright, 350, 25);
	lv_slider_set_range(ui->slider_bright, 0, 100);
	lv_slider_set_knob_in(ui->slider_bright, true);
	lv_slider_set_value(ui->slider_bright, 28, LV_ANIM_OFF);
	lv_slider_set_style(ui->slider_bright, LV_SLIDER_STYLE_INDIC, &style1_slider_bright);
	lv_slider_set_style(ui->slider_bright, LV_SLIDER_STYLE_KNOB, &style2_slider_bright);
#endif // LV_USE_SLIDER

#ifdef LV_USE_LINE
	lv_style_copy(&style0_line_17, &lv_style_pretty);
	style0_line_17.line.color = lv_color_hex(0xb1b1b1);
	style0_line_17.line.width = 1;

	ui->line_17 = lv_line_create(ui->cont_enhance, NULL);
	lv_obj_set_pos(ui->line_17, 18, 84);
	lv_obj_set_size(ui->line_17, 532, 17);
	static lv_point_t line_17_points[2] = {
		{0,15},
		{530,15},
		};
	lv_line_set_points(ui->line_17, line_17_points, 2);
	lv_line_set_style(ui->line_17, LV_LINE_STYLE_MAIN, &style0_line_17);
#endif // LV_USE_LINE

#ifdef LV_USE_LINE
	lv_style_copy(&style0_line_18, &lv_style_pretty);
	style0_line_18.line.color = lv_color_hex(0xb1b1b1);
	style0_line_18.line.width = 1;

	ui->line_18 = lv_line_create(ui->cont_enhance, NULL);
	lv_obj_set_pos(ui->line_18, 18, 134);
	lv_obj_set_size(ui->line_18, 532, 17);
	static lv_point_t line_18_points[2] = {
		{0,15},
		{530,15},
		};
	lv_line_set_points(ui->line_18, line_18_points, 2);
	lv_line_set_style(ui->line_18, LV_LINE_STYLE_MAIN, &style0_line_18);
#endif // LV_USE_LINE

#ifdef LV_USE_LABEL
	ui->label_num_saturation = lv_label_create(ui->cont_enhance, NULL);
	lv_label_set_text(ui->label_num_saturation, "20%");
	lv_label_set_long_mode(ui->label_num_saturation, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_num_saturation, 499, 65);
	lv_obj_set_size(ui->label_num_saturation, 30, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_num_contrast = lv_label_create(ui->cont_enhance, NULL);
	lv_label_set_text(ui->label_num_contrast, "20%");
	lv_label_set_long_mode(ui->label_num_contrast, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_num_contrast, 499, 115);
	lv_obj_set_size(ui->label_num_contrast, 30, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_num_bright = lv_label_create(ui->cont_enhance, NULL);
	lv_label_set_text(ui->label_num_bright, "20%");
	lv_label_set_long_mode(ui->label_num_bright, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_num_bright, 499, 167);
	lv_obj_set_size(ui->label_num_bright, 30, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_CONT
	lv_style_copy(&style0_general_cont, &lv_style_pretty);
	style0_general_cont.body.main_color = lv_color_hex(0xf2f2f2);
	style0_general_cont.body.grad_color = lv_color_hex(0xf2f2f2);
	style0_general_cont.body.radius = 10;
	style0_general_cont.body.border.width = 0;

	ui->general_cont = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->general_cont, 209, 69);
	lv_obj_set_size(ui->general_cont, 577, 405);
	lv_cont_set_fit4(ui->general_cont, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->general_cont, LV_CONT_STYLE_MAIN, &style0_general_cont);
#endif // LV_USE_CONT

#ifdef LV_USE_LABEL
	ui->label_volume = lv_label_create(ui->general_cont, NULL);
	lv_label_set_text(ui->label_volume, "volume");
	lv_label_set_long_mode(ui->label_volume, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_volume, 25, 282);
	lv_obj_set_size(ui->label_volume, 68, 23);
#endif // LV_USE_LABEL

#ifdef LV_USE_SLIDER
	ui->slider_volume = lv_slider_create(ui->general_cont, NULL);
	lv_obj_set_pos(ui->slider_volume, 33, 309);
	lv_obj_set_size(ui->slider_volume, 434, 26);
	lv_slider_set_range(ui->slider_volume, 0, 100);
	lv_slider_set_knob_in(ui->slider_volume, false);
	lv_slider_set_value(ui->slider_volume, 0, LV_ANIM_OFF);
#endif // LV_USE_SLIDER

#ifdef LV_USE_BTN
	ui->button_backlight = lv_btn_create(ui->general_cont, NULL);
	lv_obj_set_pos(ui->button_backlight, 398, 67);
	lv_obj_set_size(ui->button_backlight, 117, 59);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_btn_backlight = lv_label_create(ui->button_backlight, NULL);
	lv_label_set_text(ui->label_btn_backlight, "Backlight");
	lv_label_set_long_mode(ui->label_btn_backlight, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_btn_backlight, 25, 20);
	lv_obj_set_size(ui->label_btn_backlight, 66, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_volume_num = lv_label_create(ui->general_cont, NULL);
	lv_label_set_text(ui->label_volume_num, "Text");
	lv_label_set_long_mode(ui->label_volume_num, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_volume_num, 472, 309);
	lv_obj_set_size(ui->label_volume_num, 31, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_CONT
	lv_style_copy(&style0_cont_time_set, &lv_style_pretty);
	style0_cont_time_set.body.main_color = lv_color_hex(0xf2f2f2);
	style0_cont_time_set.body.grad_color = lv_color_hex(0xf2f2f2);
	style0_cont_time_set.body.radius = 10;
	style0_cont_time_set.body.opa = 0;
	style0_cont_time_set.body.border.width = 0;
	style0_cont_time_set.body.border.opa = 0;

	ui->cont_time_set = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->cont_time_set, 310, 153);
	lv_obj_set_size(ui->cont_time_set, 366, 249);
	lv_cont_set_fit4(ui->cont_time_set, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_drag(ui->cont_time_set,true);
	lv_obj_set_hidden(ui->cont_time_set,true);
	lv_cont_set_style(ui->cont_time_set, LV_CONT_STYLE_MAIN, &style0_cont_time_set);
#endif // LV_USE_CONT

#ifdef LV_USE_IMG
	ui->image_hour1 = lv_img_create(ui->cont_time_set, NULL);
	lv_obj_set_pos(ui->image_hour1, 5, 75);
	lv_obj_set_size(ui->image_hour1, 43, 98);
	image_hour1_1_png = (void *)parse_image_from_file(LV_IMAGE_PATH"1.png");
	lv_img_set_src(ui->image_hour1, image_hour1_1_png);

#endif // LV_USE_IMG

#ifdef LV_USE_IMG
	ui->image_hour2 = lv_img_create(ui->cont_time_set, NULL);
	lv_obj_set_pos(ui->image_hour2, 71, 70);
	lv_obj_set_size(ui->image_hour2, 70, 100);
	image_hour2_2_png = (void *)parse_image_from_file(LV_IMAGE_PATH"2.png");
	lv_img_set_src(ui->image_hour2, image_hour2_2_png);

#endif // LV_USE_IMG

#ifdef LV_USE_IMG
	ui->image_min1 = lv_img_create(ui->cont_time_set, NULL);
	lv_obj_set_pos(ui->image_min1, 208, 70);
	lv_obj_set_size(ui->image_min1, 72, 102);
	image_min1_3_png = (void *)parse_image_from_file(LV_IMAGE_PATH"3.png");
	lv_img_set_src(ui->image_min1, image_min1_3_png);

#endif // LV_USE_IMG

#ifdef LV_USE_IMG
	ui->image_min2 = lv_img_create(ui->cont_time_set, NULL);
	lv_obj_set_pos(ui->image_min2, 291, 69);
	lv_obj_set_size(ui->image_min2, 70, 100);
	image_min2_2_png = (void *)parse_image_from_file(LV_IMAGE_PATH"2.png");
	lv_img_set_src(ui->image_min2, image_min2_2_png);

#endif // LV_USE_IMG

#ifdef LV_USE_BTN
	ui->button_hour_up = lv_btn_create(ui->cont_time_set, NULL);
	lv_obj_set_pos(ui->button_hour_up, 29, 7);
	lv_obj_set_size(ui->button_hour_up, 90, 35);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_hour_up = lv_label_create(ui->button_hour_up, NULL);
	lv_label_set_text(ui->label_hour_up, "+");
	lv_label_set_long_mode(ui->label_hour_up, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_hour_up, 41, 8);
	lv_obj_set_size(ui->label_hour_up, 9, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_hour_down = lv_btn_create(ui->cont_time_set, NULL);
	lv_obj_set_pos(ui->button_hour_down, 28, 209);
	lv_obj_set_size(ui->button_hour_down, 90, 35);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_hour_down = lv_label_create(ui->button_hour_down, NULL);
	lv_label_set_text(ui->label_hour_down, "-");
	lv_label_set_long_mode(ui->label_hour_down, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_hour_down, 43, 8);
	lv_obj_set_size(ui->label_hour_down, 4, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_min_up = lv_btn_create(ui->cont_time_set, NULL);
	lv_obj_set_pos(ui->button_min_up, 257, 5);
	lv_obj_set_size(ui->button_min_up, 90, 35);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_min_up = lv_label_create(ui->button_min_up, NULL);
	lv_label_set_text(ui->label_min_up, "+");
	lv_label_set_long_mode(ui->label_min_up, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_min_up, 41, 8);
	lv_obj_set_size(ui->label_min_up, 9, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_min_down = lv_btn_create(ui->cont_time_set, NULL);
	lv_obj_set_pos(ui->button_min_down, 257, 209);
	lv_obj_set_size(ui->button_min_down, 90, 35);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_min_down = lv_label_create(ui->button_min_down, NULL);
	lv_label_set_text(ui->label_min_down, "-");
	lv_label_set_long_mode(ui->label_min_down, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_min_down, 43, 8);
	lv_obj_set_size(ui->label_min_down, 4, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_IMG
	ui->image_time_dot = lv_img_create(ui->cont_time_set, NULL);
	lv_obj_set_pos(ui->image_time_dot, 143, 76);
	lv_obj_set_size(ui->image_time_dot, 72, 102);
	image_time_dot_time_dot_png = (void *)parse_image_from_file(LV_IMAGE_PATH"time_dot.png");
	lv_img_set_src(ui->image_time_dot, image_time_dot_time_dot_png);

#endif // LV_USE_IMG

}

void setting_auto_ui_destory(setting_ui_t *ui)
{
	lv_obj_clean(ui->cont);
	free_image(image_hour1_1_png);
	free_image(image_hour2_2_png);
	free_image(image_min1_3_png);
	free_image(image_min2_2_png);
	free_image(image_time_dot_time_dot_png);
	free_image(list_1_1_setting_general_png);
	free_image(list_1_2_setting_display_png);
	free_image(list_1_3_setting_language_png);
	free_image(list_1_4_setting_information_png);
	free_image(list_1_5_setting_wallpaper_png);
	free_image(list_1_6_setting_password_png);
	free_image(list_1_7_setting_date_and_time_png);
	free_image(list_1_8_setting_date_and_time_png);
}
