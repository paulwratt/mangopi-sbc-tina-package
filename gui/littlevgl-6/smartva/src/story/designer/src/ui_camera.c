/**********************
 *      includes
 **********************/
#include "ui_camera.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/
static lv_style_t style_screen;
static lv_style_t style0_back;
static lv_style_t style0_page_1;
static lv_style_t style1_page_1;
static lv_style_t style0_label_mode;
static lv_style_t style0_disk_info;

/**********************
 *  images and fonts
 **********************/
static void *back_home_return_png_state0 = NULL;
static void *back_home_return_png_state1 = NULL;
static void *back_home_return_png_state2 = NULL;
static void *back_home_return_png_state3 = NULL;
static void *back_home_return_png_state4 = NULL;
static void *take_music_play_png_state0 = NULL;
static void *take_music_pause_png_state2 = NULL;
static void *menu_menu_png_state0 = NULL;
static void *mode_setting_password_png_state0 = NULL;

/**********************
 *  functions
 **********************/
void camera_auto_ui_create(camera_ui_t *ui)
{
	lv_style_copy(&style_screen, &lv_style_scr);
	style_screen.body.main_color = lv_color_hex(0x55ffff);
	style_screen.body.grad_color = lv_color_hex(0x55ffff);
	lv_obj_set_style(ui->cont, &style_screen);

#ifdef LV_USE_IMGBTN
	lv_style_copy(&style0_back, &lv_style_btn_rel);

	ui->back = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->back, 18, 14);
	lv_obj_set_size(ui->back, 32, 32);
	lv_imgbtn_set_state(ui->back, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->back, false);
	back_home_return_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_return.png");
	lv_imgbtn_set_src(ui->back, LV_BTN_STATE_REL, back_home_return_png_state0);
	back_home_return_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_return.png");
	lv_imgbtn_set_src(ui->back, LV_BTN_STATE_PR, back_home_return_png_state1);
	back_home_return_png_state2 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_return.png");
	lv_imgbtn_set_src(ui->back, LV_BTN_STATE_TGL_REL, back_home_return_png_state2);
	back_home_return_png_state3 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_return.png");
	lv_imgbtn_set_src(ui->back, LV_BTN_STATE_TGL_PR, back_home_return_png_state3);
	back_home_return_png_state4 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_return.png");
	lv_imgbtn_set_src(ui->back, LV_BTN_STATE_INA, back_home_return_png_state4);
	lv_imgbtn_set_style(ui->back, LV_IMGBTN_STYLE_REL, &style0_back);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_PAGE
	lv_style_copy(&style0_page_1, &lv_style_pretty_color);
	style0_page_1.body.opa = 0;
	style0_page_1.body.border.opa = 0;

	lv_style_copy(&style1_page_1, &lv_style_pretty);
	style1_page_1.body.opa = 0;
	style1_page_1.body.border.opa = 0;

	ui->page_1 = lv_page_create(ui->cont, NULL);
	lv_obj_set_pos(ui->page_1, 253, 412);
	lv_obj_set_size(ui->page_1, 310, 67);
	lv_page_set_sb_mode(ui->page_1, LV_SB_MODE_OFF);
	lv_page_set_edge_flash(ui->page_1, false);
	lv_page_set_scroll_propagation(ui->page_1, false);
	lv_page_set_style(ui->page_1, LV_PAGE_STYLE_BG, &style0_page_1);
	lv_page_set_style(ui->page_1, LV_PAGE_STYLE_SCRL, &style1_page_1);
#endif // LV_USE_PAGE

#ifdef LV_USE_IMGBTN
	ui->take = lv_imgbtn_create(ui->page_1, NULL);
	lv_obj_set_pos(ui->take, 86, 7);
	lv_obj_set_size(ui->take, 48, 48);
	lv_imgbtn_set_state(ui->take, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->take, false);
	take_music_play_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_play.png");
	lv_imgbtn_set_src(ui->take, LV_BTN_STATE_REL, take_music_play_png_state0);
	take_music_pause_png_state2 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_pause.png");
	lv_imgbtn_set_src(ui->take, LV_BTN_STATE_TGL_REL, take_music_pause_png_state2);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->menu = lv_imgbtn_create(ui->page_1, NULL);
	lv_obj_set_pos(ui->menu, 243, 16);
	lv_obj_set_size(ui->menu, 32, 32);
	lv_imgbtn_set_state(ui->menu, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->menu, false);
	menu_menu_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"menu.png");
	lv_imgbtn_set_src(ui->menu, LV_BTN_STATE_REL, menu_menu_png_state0);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_LABEL
	ui->label_time = lv_label_create(ui->page_1, NULL);
	lv_label_set_text(ui->label_time, "00:00:00");
	lv_label_set_long_mode(ui->label_time, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_time, 154, 22);
	lv_obj_set_size(ui->label_time, 72, 20);
#endif // LV_USE_LABEL

#ifdef LV_USE_IMGBTN
	ui->mode = lv_imgbtn_create(ui->page_1, NULL);
	lv_obj_set_pos(ui->mode, 30, 17);
	lv_obj_set_size(ui->mode, 32, 32);
	lv_imgbtn_set_state(ui->mode, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->mode, true);
	mode_setting_password_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"setting_password.png");
	lv_imgbtn_set_src(ui->mode, LV_BTN_STATE_REL, mode_setting_password_png_state0);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_mode, &lv_style_transp);
	style0_label_mode.text.color = lv_color_hex(0xff0000);
	style0_label_mode.text.font = &lv_font_roboto_28;
	style0_label_mode.text.line_space = 2;

	ui->label_mode = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_mode, "recording...");
	lv_label_set_align(ui->label_mode, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_mode, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_mode, 282, 34);
	lv_obj_set_size(ui->label_mode, 175, 38);
	lv_obj_set_hidden(ui->label_mode,true);
	lv_label_set_style(ui->label_mode, LV_LABEL_STYLE_MAIN, &style0_label_mode);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_disk_info, &lv_style_transp);
	style0_disk_info.text.color = lv_color_hex(0xffff00);
	style0_disk_info.text.font = &microsoft_yahei_en_cn_24_4;
	style0_disk_info.text.line_space = 2;

	ui->disk_info = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->disk_info, "提示：请插入磁盘!!!");
	lv_label_set_align(ui->disk_info, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->disk_info, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->disk_info, 244, 208);
	lv_obj_set_size(ui->disk_info, 278, 44);
	lv_obj_set_hidden(ui->disk_info,true);
	lv_label_set_style(ui->disk_info, LV_LABEL_STYLE_MAIN, &style0_disk_info);
#endif // LV_USE_LABEL

}

void camera_auto_ui_destory(camera_ui_t *ui)
{
	lv_obj_clean(ui->cont);
	free_image(back_home_return_png_state0);
	free_image(back_home_return_png_state1);
	free_image(back_home_return_png_state2);
	free_image(back_home_return_png_state3);
	free_image(back_home_return_png_state4);
	free_image(take_music_play_png_state0);
	free_image(take_music_pause_png_state2);
	free_image(menu_menu_png_state0);
	free_image(mode_setting_password_png_state0);
}
