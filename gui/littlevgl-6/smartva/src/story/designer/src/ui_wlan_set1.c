/**********************
 *      includes
 **********************/
#include "ui_wlan_set1.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/
static lv_style_t style0_container_bg;
static lv_style_t style0_cont_hbar;
static lv_style_t style0_img_hbar_power;
static lv_style_t style0_img_hbar_wifi;
static lv_style_t style0_label_hbar_timer;
static lv_style_t style0_btn_hbar_return;
static lv_style_t style1_btn_hbar_return;
static lv_style_t style0_img_hbar_return;
static lv_style_t style0_btn_hbar_home;
static lv_style_t style1_btn_hbar_home;
static lv_style_t style0_img_hbar_home;
static lv_style_t style0_label_1;
static lv_style_t style0_list_1;
static lv_style_t style1_list_1;
static lv_style_t style2_list_1;
static lv_style_t style3_list_1;
static lv_style_t style4_list_1;
static lv_style_t style5_list_1;
static lv_style_t style0_container_1;
static lv_style_t style0_keyboard_1;
static lv_style_t style1_keyboard_1;
static lv_style_t style2_keyboard_1;
static lv_style_t style0_label_2;
static lv_style_t style0_text_area_1;
static lv_style_t style0_button_1;
static lv_style_t style1_button_1;
static lv_style_t style0_label_3;
static lv_style_t style0_button_2;
static lv_style_t style1_button_2;
static lv_style_t style0_label_4;
static lv_style_t style0_label_5;
static lv_style_t style0_container_mark;

/**********************
 *  images and fonts
 **********************/
static void *img_hbar_wifi_wifi_no_connect_png = NULL;

/**********************
 *  functions
 **********************/
void wlan_set1_auto_ui_create(wlan_set1_ui_t *ui)
{

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_bg, &lv_style_pretty);
	style0_container_bg.body.main_color = lv_color_hex(0xfefefe);
	style0_container_bg.body.grad_color = lv_color_hex(0xfefefe);
	style0_container_bg.body.radius = 0;

	ui->container_bg = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->container_bg, 0, 0);
	lv_obj_set_size(ui->container_bg, 800, 480);
	lv_cont_set_fit4(ui->container_bg, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->container_bg, LV_CONT_STYLE_MAIN, &style0_container_bg);
#endif // LV_USE_CONT

#ifdef LV_USE_CONT
	lv_style_copy(&style0_cont_hbar, &lv_style_pretty);
	style0_cont_hbar.body.radius = 0;
	style0_cont_hbar.body.opa = 0;
	style0_cont_hbar.body.border.width = 1;

	ui->cont_hbar = lv_cont_create(ui->container_bg, NULL);
	lv_obj_set_pos(ui->cont_hbar, 0, 0);
	lv_obj_set_size(ui->cont_hbar, 800, 50);
	lv_cont_set_fit4(ui->cont_hbar, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->cont_hbar, LV_CONT_STYLE_MAIN, &style0_cont_hbar);
#endif // LV_USE_CONT

#ifdef LV_USE_IMG
	lv_style_copy(&style0_img_hbar_power, &lv_style_plain);

	ui->img_hbar_power = lv_img_create(ui->cont_hbar, NULL);
	lv_obj_set_pos(ui->img_hbar_power, 758, 13);
	lv_obj_set_size(ui->img_hbar_power, 20, 19);
	lv_img_set_src(ui->img_hbar_power, LV_SYMBOL_BATTERY_2);
	lv_img_set_style(ui->img_hbar_power, LV_IMG_STYLE_MAIN, &style0_img_hbar_power);
#endif // LV_USE_IMG

#ifdef LV_USE_IMG
	lv_style_copy(&style0_img_hbar_wifi, &lv_style_plain);

	ui->img_hbar_wifi = lv_img_create(ui->cont_hbar, NULL);
	lv_obj_set_pos(ui->img_hbar_wifi, 724, 13);
	lv_obj_set_size(ui->img_hbar_wifi, 18, 16);
	img_hbar_wifi_wifi_no_connect_png = (void *)parse_image_from_file(LV_IMAGE_PATH"wifi_no_connect.png");
	lv_img_set_src(ui->img_hbar_wifi, img_hbar_wifi_wifi_no_connect_png);

	lv_img_set_style(ui->img_hbar_wifi, LV_IMG_STYLE_MAIN, &style0_img_hbar_wifi);
#endif // LV_USE_IMG

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_hbar_timer, &lv_style_transp);

	ui->label_hbar_timer = lv_label_create(ui->cont_hbar, NULL);
	lv_label_set_text(ui->label_hbar_timer, "02:45");
	lv_label_set_long_mode(ui->label_hbar_timer, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_hbar_timer, 670, 13);
	lv_obj_set_size(ui->label_hbar_timer, 48, 22);
	lv_label_set_style(ui->label_hbar_timer, LV_LABEL_STYLE_MAIN, &style0_label_hbar_timer);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_btn_hbar_return, &lv_style_btn_rel);
	style0_btn_hbar_return.body.main_color = lv_color_hex(0xf9eef1);
	style0_btn_hbar_return.body.grad_color = lv_color_hex(0xf9eef1);
	style0_btn_hbar_return.body.radius = 12;
	style0_btn_hbar_return.body.opa = 0;
	style0_btn_hbar_return.body.border.width = 1;
	style0_btn_hbar_return.body.shadow.width = 1;
	style0_btn_hbar_return.body.padding.top = 0;
	style0_btn_hbar_return.body.padding.bottom = 0;
	style0_btn_hbar_return.body.padding.left = 0;

	lv_style_copy(&style1_btn_hbar_return, &lv_style_btn_pr);
	style1_btn_hbar_return.body.main_color = lv_color_hex(0xe5d6dc);
	style1_btn_hbar_return.body.grad_color = lv_color_hex(0xe5d6dc);
	style1_btn_hbar_return.body.radius = 12;
	style1_btn_hbar_return.body.border.width = 1;

	ui->btn_hbar_return = lv_btn_create(ui->cont_hbar, NULL);
	lv_obj_set_pos(ui->btn_hbar_return, 23, 7);
	lv_obj_set_size(ui->btn_hbar_return, 57, 35);
	lv_btn_set_style(ui->btn_hbar_return, LV_BTN_STYLE_REL, &style0_btn_hbar_return);
	lv_btn_set_style(ui->btn_hbar_return, LV_BTN_STYLE_PR, &style1_btn_hbar_return);
#endif // LV_USE_BTN

#ifdef LV_USE_IMG
	lv_style_copy(&style0_img_hbar_return, &lv_style_plain);
	style0_img_hbar_return.image.color = lv_color_hex(0x000000);

	ui->img_hbar_return = lv_img_create(ui->btn_hbar_return, NULL);
	lv_obj_set_pos(ui->img_hbar_return, 23, 8);
	lv_obj_set_size(ui->img_hbar_return, 10, 19);
	lv_img_set_src(ui->img_hbar_return, LV_SYMBOL_LEFT);
	lv_img_set_style(ui->img_hbar_return, LV_IMG_STYLE_MAIN, &style0_img_hbar_return);
#endif // LV_USE_IMG

#ifdef LV_USE_BTN
	lv_style_copy(&style0_btn_hbar_home, &lv_style_btn_rel);
	style0_btn_hbar_home.body.radius = 12;
	style0_btn_hbar_home.body.opa = 0;
	style0_btn_hbar_home.body.border.width = 1;

	lv_style_copy(&style1_btn_hbar_home, &lv_style_btn_pr);
	style1_btn_hbar_home.body.main_color = lv_color_hex(0xe5d6dc);
	style1_btn_hbar_home.body.grad_color = lv_color_hex(0xe5d6dc);
	style1_btn_hbar_home.body.radius = 12;
	style1_btn_hbar_home.body.border.width = 1;

	ui->btn_hbar_home = lv_btn_create(ui->cont_hbar, NULL);
	lv_obj_set_pos(ui->btn_hbar_home, 374, 8);
	lv_obj_set_size(ui->btn_hbar_home, 56, 35);
	lv_btn_set_style(ui->btn_hbar_home, LV_BTN_STYLE_REL, &style0_btn_hbar_home);
	lv_btn_set_style(ui->btn_hbar_home, LV_BTN_STYLE_PR, &style1_btn_hbar_home);
#endif // LV_USE_BTN

#ifdef LV_USE_IMG
	lv_style_copy(&style0_img_hbar_home, &lv_style_plain);
	style0_img_hbar_home.image.color = lv_color_hex(0x000000);

	ui->img_hbar_home = lv_img_create(ui->btn_hbar_home, NULL);
	lv_obj_set_pos(ui->img_hbar_home, 19, 8);
	lv_obj_set_size(ui->img_hbar_home, 18, 19);
	lv_img_set_src(ui->img_hbar_home, LV_SYMBOL_HOME);
	lv_img_set_style(ui->img_hbar_home, LV_IMG_STYLE_MAIN, &style0_img_hbar_home);
#endif // LV_USE_IMG

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_1, &lv_style_transp);

	ui->label_1 = lv_label_create(ui->container_bg, NULL);
	lv_label_set_text(ui->label_1, "WLAN");
	lv_label_set_long_mode(ui->label_1, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_1, 28, 65);
	lv_obj_set_size(ui->label_1, 69, 22);
	lv_label_set_style(ui->label_1, LV_LABEL_STYLE_MAIN, &style0_label_1);
#endif // LV_USE_LABEL

#ifdef LV_USE_LIST
	lv_style_copy(&style0_list_1, &lv_style_transp_fit);
	style0_list_1.body.main_color = lv_color_hex(0xfefefe);
	style0_list_1.body.grad_color = lv_color_hex(0xfefefe);
	style0_list_1.body.border.color = lv_color_hex(0xffffff);
	style0_list_1.body.border.opa = 0;
	style0_list_1.body.shadow.color = lv_color_hex(0xffffff);

	lv_style_copy(&style1_list_1, &lv_style_pretty);
	style1_list_1.body.grad_color = lv_color_hex(0xffffff);
	style1_list_1.body.radius = 0;
	style1_list_1.body.opa = 0;
	style1_list_1.body.border.color = lv_color_hex(0xffffff);
	style1_list_1.body.shadow.color = lv_color_hex(0xffffff);

	lv_style_copy(&style2_list_1, &lv_style_pretty_color);
	style2_list_1.body.main_color = lv_color_hex(0xc1c1c1);
	style2_list_1.body.grad_color = lv_color_hex(0xc1c1c1);
	style2_list_1.body.opa = 128;
	style2_list_1.body.border.color = lv_color_hex(0xffffff);
	style2_list_1.body.shadow.color = lv_color_hex(0xffffff);

	lv_style_copy(&style3_list_1, &lv_style_transp);
	style3_list_1.body.main_color = lv_color_hex(0xc1c1c1);
	style3_list_1.body.grad_color = lv_color_hex(0xc1c1c1);
	style3_list_1.body.radius = 0;
	style3_list_1.body.opa = 128;
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
	style3_list_1.text.color = lv_color_hex(0xcccccc);
	style3_list_1.text.sel_color = lv_color_hex(0x5596d8);
	style3_list_1.text.font = &lv_font_roboto_16;
	style3_list_1.text.letter_space = 0;
	style3_list_1.text.line_space = 2;
	style3_list_1.text.opa = 255;
	style3_list_1.image.color = lv_color_hex(0xf0f0f0);
	style3_list_1.image.intense = 0;
	style3_list_1.image.opa = 255;

	lv_style_copy(&style4_list_1, &lv_style_btn_rel);
	style4_list_1.body.main_color = lv_color_hex(0xffaaff);
	style4_list_1.body.grad_color = lv_color_hex(0xffaaff);
	style4_list_1.body.opa = 0;
	style4_list_1.body.border.width = 0;
	style4_list_1.body.border.opa = 0;
	style4_list_1.text.color = lv_color_hex(0x000000);
	style4_list_1.text.sel_color = lv_color_hex(0x000000);
	style4_list_1.text.line_space = 2;
	style4_list_1.image.color = lv_color_hex(0x000000);

	lv_style_copy(&style5_list_1, &lv_style_btn_pr);
	style5_list_1.body.main_color = lv_color_hex(0xc1c1c1);
	style5_list_1.body.grad_color = lv_color_hex(0xc1c1c1);
	style5_list_1.body.opa = 128;
	style5_list_1.body.border.width = 0;
	style5_list_1.body.border.opa = 0;
	style5_list_1.text.color = lv_color_hex(0x000000);
	style5_list_1.text.sel_color = lv_color_hex(0x000000);
	style5_list_1.text.line_space = 2;
	style5_list_1.image.color = lv_color_hex(0x000000);

	ui->list_1 = lv_list_create(ui->container_bg, NULL);
	lv_obj_set_pos(ui->list_1, 0, 98);
	lv_obj_set_size(ui->list_1, 800, 382);
	lv_list_add_btn(ui->list_1, NULL, "open wlan");
	lv_list_add_btn(ui->list_1, LV_SYMBOL_WIFI, "AW-Guest");
	lv_list_set_single_mode(ui->list_1, false);
	lv_list_set_scroll_propagation(ui->list_1, false);
	lv_list_set_edge_flash(ui->list_1, true);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BG, &style0_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_SCRL, &style1_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_SB, &style2_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_EDGE_FLASH, &style3_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BTN_REL, &style4_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BTN_PR, &style5_list_1);
#endif // LV_USE_LIST

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_1, &lv_style_pretty);
	style0_container_1.body.grad_color = lv_color_hex(0xffffff);
	style0_container_1.body.radius = 12;

	ui->container_1 = lv_cont_create(ui->container_bg, NULL);
	lv_obj_set_pos(ui->container_1, 0, 100);
	lv_obj_set_size(ui->container_1, 800, 368);
	lv_cont_set_fit4(ui->container_1, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->container_1, LV_CONT_STYLE_MAIN, &style0_container_1);
#endif // LV_USE_CONT

#ifdef LV_USE_KB
	lv_style_copy(&style0_keyboard_1, &lv_style_pretty);
	style0_keyboard_1.body.main_color = lv_color_hex(0xc0c0c0);
	style0_keyboard_1.body.radius = 0;

	lv_style_copy(&style1_keyboard_1, &lv_style_btn_rel);
	style1_keyboard_1.body.main_color = lv_color_hex(0xffffff);
	style1_keyboard_1.body.grad_color = lv_color_hex(0xffffff);
	style1_keyboard_1.body.border.width = 0;
	style1_keyboard_1.body.border.opa = 255;
	style1_keyboard_1.text.color = lv_color_hex(0x000000);
	style1_keyboard_1.text.sel_color = lv_color_hex(0x000000);
	style1_keyboard_1.text.line_space = 2;

	lv_style_copy(&style2_keyboard_1, &lv_style_btn_pr);
	style2_keyboard_1.body.main_color = lv_color_hex(0xffffff);
	style2_keyboard_1.body.grad_color = lv_color_hex(0xffffff);
	style2_keyboard_1.body.border.width = 0;
	style2_keyboard_1.body.border.opa = 255;
	style2_keyboard_1.text.color = lv_color_hex(0x000000);
	style2_keyboard_1.text.sel_color = lv_color_hex(0x000000);
	style2_keyboard_1.text.line_space = 2;

	ui->keyboard_1 = lv_kb_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->keyboard_1, 148, 170);
	lv_obj_set_size(ui->keyboard_1, 516, 198);
	lv_kb_set_mode(ui->keyboard_1, LV_KB_MODE_TEXT);
	lv_kb_set_cursor_manage(ui->keyboard_1, false);
	lv_kb_set_style(ui->keyboard_1, LV_KB_STYLE_BG, &style0_keyboard_1);
	lv_kb_set_style(ui->keyboard_1, LV_KB_STYLE_BTN_REL, &style1_keyboard_1);
	lv_kb_set_style(ui->keyboard_1, LV_KB_STYLE_BTN_PR, &style2_keyboard_1);
#endif // LV_USE_KB

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_2, &lv_style_transp);

	ui->label_2 = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->label_2, "AW_gesut");
	lv_label_set_align(ui->label_2, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_2, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_2, 300, 22);
	lv_obj_set_size(ui->label_2, 199, 25);
	lv_label_set_style(ui->label_2, LV_LABEL_STYLE_MAIN, &style0_label_2);
#endif // LV_USE_LABEL

#ifdef LV_USE_TA
	lv_style_copy(&style0_text_area_1, &lv_style_pretty);

	ui->text_area_1 = lv_ta_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->text_area_1, 258, 57);
	lv_obj_set_size(ui->text_area_1, 280, 29);
	lv_ta_set_text(ui->text_area_1, "");
	lv_ta_set_placeholder_text(ui->text_area_1, "");
	lv_ta_set_cursor_type(ui->text_area_1, LV_CURSOR_LINE);
	lv_ta_set_cursor_blink_time(ui->text_area_1, 250);
	lv_ta_set_one_line(ui->text_area_1, true);
	lv_ta_set_pwd_mode(ui->text_area_1, true);
	lv_ta_set_max_length(ui->text_area_1, 0);
	lv_ta_set_text_align(ui->text_area_1, 0);
	lv_ta_set_sb_mode(ui->text_area_1, 2);
	lv_ta_set_scroll_propagation(ui->text_area_1, false);
	lv_ta_set_edge_flash(ui->text_area_1, false);
	lv_ta_set_style(ui->text_area_1, LV_TA_STYLE_BG, &style0_text_area_1);
#endif // LV_USE_TA

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_1, &lv_style_btn_rel);
	style0_button_1.body.main_color = lv_color_hex(0xffffff);
	style0_button_1.body.grad_color = lv_color_hex(0xffffff);
	style0_button_1.body.radius = 12;

	lv_style_copy(&style1_button_1, &lv_style_btn_pr);
	style1_button_1.body.main_color = lv_color_hex(0xbababa);
	style1_button_1.body.grad_color = lv_color_hex(0xbfbfbf);
	style1_button_1.body.radius = 12;

	ui->button_1 = lv_btn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->button_1, 434, 103);
	lv_obj_set_size(ui->button_1, 101, 35);
	lv_btn_set_layout(ui->button_1, LV_LAYOUT_OFF);
	lv_btn_set_style(ui->button_1, LV_BTN_STYLE_REL, &style0_button_1);
	lv_btn_set_style(ui->button_1, LV_BTN_STYLE_PR, &style1_button_1);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_3, &lv_style_transp);
	style0_label_3.text.color = lv_color_hex(0x000000);
	style0_label_3.text.sel_color = lv_color_hex(0x000000);
	style0_label_3.text.line_space = 2;

	ui->label_3 = lv_label_create(ui->button_1, NULL);
	lv_label_set_text(ui->label_3, "connect");
	lv_label_set_align(ui->label_3, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_3, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_3, 23, 7);
	lv_obj_set_size(ui->label_3, 64, 22);
	lv_label_set_style(ui->label_3, LV_LABEL_STYLE_MAIN, &style0_label_3);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_2, &lv_style_btn_rel);
	style0_button_2.body.main_color = lv_color_hex(0xffffff);
	style0_button_2.body.grad_color = lv_color_hex(0xffffff);
	style0_button_2.body.radius = 12;

	lv_style_copy(&style1_button_2, &lv_style_btn_pr);
	style1_button_2.body.main_color = lv_color_hex(0xbababa);
	style1_button_2.body.grad_color = lv_color_hex(0xbfbfbf);
	style1_button_2.body.radius = 12;

	ui->button_2 = lv_btn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->button_2, 264, 104);
	lv_obj_set_size(ui->button_2, 101, 35);
	lv_btn_set_layout(ui->button_2, LV_LAYOUT_OFF);
	lv_btn_set_style(ui->button_2, LV_BTN_STYLE_REL, &style0_button_2);
	lv_btn_set_style(ui->button_2, LV_BTN_STYLE_PR, &style1_button_2);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_4, &lv_style_transp);
	style0_label_4.text.color = lv_color_hex(0x000000);
	style0_label_4.text.sel_color = lv_color_hex(0x000000);
	style0_label_4.text.line_space = 2;

	ui->label_4 = lv_label_create(ui->button_2, NULL);
	lv_label_set_text(ui->label_4, "disconnect");
	lv_label_set_align(ui->label_4, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_4, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_4, 11, 7);
	lv_obj_set_size(ui->label_4, 89, 22);
	lv_label_set_style(ui->label_4, LV_LABEL_STYLE_MAIN, &style0_label_4);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_5, &lv_style_transp);

	ui->label_5 = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->label_5, "");
	lv_label_set_align(ui->label_5, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_5, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_5, 321, 151);
	lv_obj_set_size(ui->label_5, 160, 22);
	lv_label_set_style(ui->label_5, LV_LABEL_STYLE_MAIN, &style0_label_5);
#endif // LV_USE_LABEL

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_mark, &lv_style_pretty);
	style0_container_mark.body.main_color = lv_color_hex(0xf1f1f1);
	style0_container_mark.body.grad_color = lv_color_hex(0xefefef);
	style0_container_mark.body.radius = 0;
	style0_container_mark.body.opa = 128;

	ui->container_mark = lv_cont_create(ui->container_bg, NULL);
	lv_obj_set_pos(ui->container_mark, 0, 0);
	lv_obj_set_size(ui->container_mark, 800, 143);
	lv_cont_set_fit4(ui->container_mark, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_hidden(ui->container_mark,true);
	lv_cont_set_style(ui->container_mark, LV_CONT_STYLE_MAIN, &style0_container_mark);
#endif // LV_USE_CONT

}

void wlan_set1_auto_ui_destory(wlan_set1_ui_t *ui)
{
	lv_obj_clean(ui->cont);
	free_image(img_hbar_wifi_wifi_no_connect_png);
}
