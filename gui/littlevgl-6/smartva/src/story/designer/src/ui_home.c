/**********************
 *      includes
 **********************/
#include "ui_home.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/
static lv_style_t style_screen;
static lv_style_t style0_label_1;
static lv_style_t style0_label_2;
static lv_style_t style0_label_3;
static lv_style_t style0_label_4;
static lv_style_t style0_label_5;
static lv_style_t style0_label_6;
static lv_style_t style0_label_7;
static lv_style_t style0_label_8;
static lv_style_t style0_cont_hbar;
static lv_style_t style0_btn_hbar_return;
static lv_style_t style1_btn_hbar_return;
static lv_style_t style0_img_hbar_return;
static lv_style_t style0_btn_hbar_home;
static lv_style_t style1_btn_hbar_home;
static lv_style_t style0_img_hbar_home;
static lv_style_t style0_label_9;
static lv_style_t style0_label_dialog;

/**********************
 *  images and fonts
 **********************/
static void *img_hbar_wifi_wifi_no_connect_png = NULL;
static void *image_button_1_home_movie_png_state0 = NULL;
static void *image_button_1_home_movie_png_state1 = NULL;
static void *image_button_2_home_music_png_state0 = NULL;
static void *image_button_2_home_music_png_state1 = NULL;
static void *image_button_3_home_calendar_png_state0 = NULL;
static void *image_button_3_home_calendar_png_state1 = NULL;
static void *image_button_4_home_timer_png_state0 = NULL;
static void *image_button_4_home_timer_png_state1 = NULL;
static void *image_button_5_home_photo_png_state0 = NULL;
static void *image_button_5_home_photo_png_state1 = NULL;
static void *image_button_6_home_calculator_png_state0 = NULL;
static void *image_button_6_home_calculator_png_state1 = NULL;
static void *image_button_7_home_explorer_png_state0 = NULL;
static void *image_button_7_home_explorer_png_state1 = NULL;
static void *image_button_8_home_setting_png_state0 = NULL;
static void *image_button_8_home_setting_png_state1 = NULL;
static void *image_button_9_slide_home_png_state0 = NULL;
static void *image_button_9_slide_home_png_state1 = NULL;

/**********************
 *  functions
 **********************/
void home_auto_ui_create(home_ui_t *ui)
{
	lv_style_copy(&style_screen, &lv_style_scr);
	style_screen.body.main_color = lv_color_hex(0x55ffff);
	style_screen.body.grad_color = lv_color_hex(0x55ffff);
	lv_obj_set_style(ui->cont, &style_screen);

#ifdef LV_USE_IMGBTN
	ui->image_button_1 = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_button_1, 119, 149);
	lv_obj_set_size(ui->image_button_1, 59, 60);
	lv_imgbtn_set_state(ui->image_button_1, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_1, false);
	image_button_1_home_movie_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_movie.png");
	lv_imgbtn_set_src(ui->image_button_1, LV_BTN_STATE_REL, image_button_1_home_movie_png_state0);
	image_button_1_home_movie_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_movie.png");
	lv_imgbtn_set_src(ui->image_button_1, LV_BTN_STATE_PR, image_button_1_home_movie_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_2 = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_button_2, 240, 150);
	lv_obj_set_size(ui->image_button_2, 59, 60);
	lv_imgbtn_set_state(ui->image_button_2, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_2, false);
	image_button_2_home_music_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_music.png");
	lv_imgbtn_set_src(ui->image_button_2, LV_BTN_STATE_REL, image_button_2_home_music_png_state0);
	image_button_2_home_music_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_music.png");
	lv_imgbtn_set_src(ui->image_button_2, LV_BTN_STATE_PR, image_button_2_home_music_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_3 = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_button_3, 360, 150);
	lv_obj_set_size(ui->image_button_3, 59, 60);
	lv_imgbtn_set_state(ui->image_button_3, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_3, false);
	image_button_3_home_calendar_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_calendar.png");
	lv_imgbtn_set_src(ui->image_button_3, LV_BTN_STATE_REL, image_button_3_home_calendar_png_state0);
	image_button_3_home_calendar_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_calendar.png");
	lv_imgbtn_set_src(ui->image_button_3, LV_BTN_STATE_PR, image_button_3_home_calendar_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_4 = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_button_4, 480, 150);
	lv_obj_set_size(ui->image_button_4, 59, 60);
	lv_imgbtn_set_state(ui->image_button_4, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_4, false);
	image_button_4_home_timer_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_timer.png");
	lv_imgbtn_set_src(ui->image_button_4, LV_BTN_STATE_REL, image_button_4_home_timer_png_state0);
	image_button_4_home_timer_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_timer.png");
	lv_imgbtn_set_src(ui->image_button_4, LV_BTN_STATE_PR, image_button_4_home_timer_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_5 = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_button_5, 600, 150);
	lv_obj_set_size(ui->image_button_5, 59, 60);
	lv_imgbtn_set_state(ui->image_button_5, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_5, false);
	image_button_5_home_photo_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_photo.png");
	lv_imgbtn_set_src(ui->image_button_5, LV_BTN_STATE_REL, image_button_5_home_photo_png_state0);
	image_button_5_home_photo_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_photo.png");
	lv_imgbtn_set_src(ui->image_button_5, LV_BTN_STATE_PR, image_button_5_home_photo_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_6 = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_button_6, 120, 285);
	lv_obj_set_size(ui->image_button_6, 59, 60);
	lv_imgbtn_set_state(ui->image_button_6, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_6, false);
	image_button_6_home_calculator_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_calculator.png");
	lv_imgbtn_set_src(ui->image_button_6, LV_BTN_STATE_REL, image_button_6_home_calculator_png_state0);
	image_button_6_home_calculator_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_calculator.png");
	lv_imgbtn_set_src(ui->image_button_6, LV_BTN_STATE_PR, image_button_6_home_calculator_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_7 = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_button_7, 240, 285);
	lv_obj_set_size(ui->image_button_7, 59, 60);
	lv_imgbtn_set_state(ui->image_button_7, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_7, false);
	image_button_7_home_explorer_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_explorer.png");
	lv_imgbtn_set_src(ui->image_button_7, LV_BTN_STATE_REL, image_button_7_home_explorer_png_state0);
	image_button_7_home_explorer_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_explorer.png");
	lv_imgbtn_set_src(ui->image_button_7, LV_BTN_STATE_PR, image_button_7_home_explorer_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_8 = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_button_8, 360, 285);
	lv_obj_set_size(ui->image_button_8, 59, 60);
	lv_imgbtn_set_state(ui->image_button_8, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_8, false);
	image_button_8_home_setting_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_setting.png");
	lv_imgbtn_set_src(ui->image_button_8, LV_BTN_STATE_REL, image_button_8_home_setting_png_state0);
	image_button_8_home_setting_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_setting.png");
	lv_imgbtn_set_src(ui->image_button_8, LV_BTN_STATE_PR, image_button_8_home_setting_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.line_space = 2;

	ui->label_1 = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_1, "movie");
	lv_label_set_long_mode(ui->label_1, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_1, 127, 213);
	lv_obj_set_size(ui->label_1, 44, 19);
	lv_label_set_style(ui->label_1, LV_LABEL_STYLE_MAIN, &style0_label_1);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_2, &lv_style_transp);
	style0_label_2.text.color = lv_color_hex(0x000000);
	style0_label_2.text.line_space = 2;

	ui->label_2 = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_2, "music");
	lv_label_set_long_mode(ui->label_2, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_2, 248, 214);
	lv_obj_set_size(ui->label_2, 43, 19);
	lv_label_set_style(ui->label_2, LV_LABEL_STYLE_MAIN, &style0_label_2);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_3, &lv_style_transp);
	style0_label_3.text.color = lv_color_hex(0x000000);
	style0_label_3.text.line_space = 2;

	ui->label_3 = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_3, "camera");
	lv_label_set_long_mode(ui->label_3, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_3, 365, 215);
	lv_obj_set_size(ui->label_3, 54, 19);
	lv_label_set_style(ui->label_3, LV_LABEL_STYLE_MAIN, &style0_label_3);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_4, &lv_style_transp);
	style0_label_4.text.color = lv_color_hex(0x000000);
	style0_label_4.text.line_space = 2;

	ui->label_4 = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_4, "sound");
	lv_label_set_long_mode(ui->label_4, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_4, 488, 214);
	lv_obj_set_size(ui->label_4, 44, 19);
	lv_label_set_style(ui->label_4, LV_LABEL_STYLE_MAIN, &style0_label_4);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_5, &lv_style_transp);
	style0_label_5.text.color = lv_color_hex(0x000000);
	style0_label_5.text.line_space = 2;

	ui->label_5 = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_5, "photo");
	lv_label_set_long_mode(ui->label_5, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_5, 609, 213);
	lv_obj_set_size(ui->label_5, 41, 19);
	lv_label_set_style(ui->label_5, LV_LABEL_STYLE_MAIN, &style0_label_5);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_6, &lv_style_transp);
	style0_label_6.text.color = lv_color_hex(0x000000);
	style0_label_6.text.line_space = 2;

	ui->label_6 = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_6, "wlan");
	lv_label_set_long_mode(ui->label_6, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_6, 133, 348);
	lv_obj_set_size(ui->label_6, 34, 19);
	lv_label_set_style(ui->label_6, LV_LABEL_STYLE_MAIN, &style0_label_6);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_7, &lv_style_transp);
	style0_label_7.text.color = lv_color_hex(0x000000);
	style0_label_7.text.line_space = 2;

	ui->label_7 = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_7, "explorer");
	lv_label_set_long_mode(ui->label_7, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_7, 242, 347);
	lv_obj_set_size(ui->label_7, 58, 19);
	lv_label_set_style(ui->label_7, LV_LABEL_STYLE_MAIN, &style0_label_7);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_8, &lv_style_transp);
	style0_label_8.text.color = lv_color_hex(0x000000);
	style0_label_8.text.line_space = 2;

	ui->label_8 = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_8, "setting");
	lv_label_set_long_mode(ui->label_8, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_8, 365, 347);
	lv_obj_set_size(ui->label_8, 49, 19);
	lv_label_set_style(ui->label_8, LV_LABEL_STYLE_MAIN, &style0_label_8);
#endif // LV_USE_LABEL

#ifdef LV_USE_CONT
	lv_style_copy(&style0_cont_hbar, &lv_style_pretty);
	style0_cont_hbar.body.radius = 0;
	style0_cont_hbar.body.opa = 0;
	style0_cont_hbar.body.border.width = 1;

	ui->cont_hbar = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->cont_hbar, 0, 0);
	lv_obj_set_size(ui->cont_hbar, 800, 50);
	lv_cont_set_fit4(ui->cont_hbar, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->cont_hbar, LV_CONT_STYLE_MAIN, &style0_cont_hbar);
#endif // LV_USE_CONT

#ifdef LV_USE_IMG
	ui->img_hbar_power = lv_img_create(ui->cont_hbar, NULL);
	lv_obj_set_pos(ui->img_hbar_power, 758, 13);
	lv_obj_set_size(ui->img_hbar_power, 20, 19);
	lv_img_set_src(ui->img_hbar_power, LV_SYMBOL_BATTERY_2);
#endif // LV_USE_IMG

#ifdef LV_USE_IMG
	ui->img_hbar_wifi = lv_img_create(ui->cont_hbar, NULL);
	lv_obj_set_pos(ui->img_hbar_wifi, 724, 13);
	lv_obj_set_size(ui->img_hbar_wifi, 18, 16);
	img_hbar_wifi_wifi_no_connect_png = (void *)parse_image_from_file(LV_IMAGE_PATH"wifi_no_connect.png");
	lv_img_set_src(ui->img_hbar_wifi, img_hbar_wifi_wifi_no_connect_png);

#endif // LV_USE_IMG

#ifdef LV_USE_LABEL
	ui->label_hbar_timer = lv_label_create(ui->cont_hbar, NULL);
	lv_label_set_text(ui->label_hbar_timer, "02:45");
	lv_label_set_long_mode(ui->label_hbar_timer, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_hbar_timer, 670, 13);
	lv_obj_set_size(ui->label_hbar_timer, 48, 22);
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

#ifdef LV_USE_IMGBTN
	ui->image_button_9 = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_button_9, 478, 286);
	lv_obj_set_size(ui->image_button_9, 61, 62);
	lv_imgbtn_set_state(ui->image_button_9, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_9, false);
	image_button_9_slide_home_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"slide_home.png");
	lv_imgbtn_set_src(ui->image_button_9, LV_BTN_STATE_REL, image_button_9_slide_home_png_state0);
	image_button_9_slide_home_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"slide_home.png");
	lv_imgbtn_set_src(ui->image_button_9, LV_BTN_STATE_PR, image_button_9_slide_home_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_9, &lv_style_transp);
	style0_label_9.text.color = lv_color_hex(0x000000);
	style0_label_9.text.line_space = 2;

	ui->label_9 = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_9, "shome");
	lv_label_set_long_mode(ui->label_9, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_9, 482, 347);
	lv_obj_set_size(ui->label_9, 49, 19);
	lv_label_set_style(ui->label_9, LV_LABEL_STYLE_MAIN, &style0_label_9);
#endif // LV_USE_LABEL

#ifdef LV_USE_CONT
	ui->cont_dialog = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->cont_dialog, 277, 179);
	lv_obj_set_size(ui->cont_dialog, 249, 100);
	lv_cont_set_fit4(ui->cont_dialog, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_hidden(ui->cont_dialog,true);
#endif // LV_USE_CONT

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_dialog, &lv_style_transp);
	style0_label_dialog.text.color = lv_color_hex(0x000000);
	style0_label_dialog.text.line_space = 2;

	ui->label_dialog = lv_label_create(ui->cont_dialog, NULL);
	lv_label_set_text(ui->label_dialog, "no disk insert !");
	lv_label_set_align(ui->label_dialog, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_dialog, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_dialog, 71, 41);
	lv_obj_set_size(ui->label_dialog, 104, 19);
	lv_obj_set_hidden(ui->label_dialog,true);
	lv_label_set_style(ui->label_dialog, LV_LABEL_STYLE_MAIN, &style0_label_dialog);
#endif // LV_USE_LABEL

}

void home_auto_ui_destory(home_ui_t *ui)
{
	lv_obj_clean(ui->cont);
	free_image(img_hbar_wifi_wifi_no_connect_png);
	free_image(image_button_1_home_movie_png_state0);
	free_image(image_button_1_home_movie_png_state1);
	free_image(image_button_2_home_music_png_state0);
	free_image(image_button_2_home_music_png_state1);
	free_image(image_button_3_home_calendar_png_state0);
	free_image(image_button_3_home_calendar_png_state1);
	free_image(image_button_4_home_timer_png_state0);
	free_image(image_button_4_home_timer_png_state1);
	free_image(image_button_5_home_photo_png_state0);
	free_image(image_button_5_home_photo_png_state1);
	free_image(image_button_6_home_calculator_png_state0);
	free_image(image_button_6_home_calculator_png_state1);
	free_image(image_button_7_home_explorer_png_state0);
	free_image(image_button_7_home_explorer_png_state1);
	free_image(image_button_8_home_setting_png_state0);
	free_image(image_button_8_home_setting_png_state1);
	free_image(image_button_9_slide_home_png_state0);
	free_image(image_button_9_slide_home_png_state1);
}
