/**********************
 *      includes
 **********************/
#include "ui_slide_home.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/
static lv_style_t style_screen;
static lv_style_t style0_cont_hbar;
static lv_style_t style0_btn_hbar_return;
static lv_style_t style1_btn_hbar_return;
static lv_style_t style0_img_hbar_return;
static lv_style_t style0_btn_hbar_home;
static lv_style_t style1_btn_hbar_home;
static lv_style_t style0_img_hbar_home;

/**********************
 *  images and fonts
 **********************/

/**********************
 *  functions
 **********************/
void slide_home_auto_ui_create(slide_home_ui_t *ui)
{
	lv_style_copy(&style_screen, &lv_style_scr);
	style_screen.body.main_color = lv_color_hex(0x00ffff);
	style_screen.body.grad_color = lv_color_hex(0x00ffff);
	lv_obj_set_style(ui->cont, &style_screen);

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
	lv_obj_set_size(ui->img_hbar_wifi, 20, 19);
	lv_img_set_src(ui->img_hbar_wifi, LV_SYMBOL_WIFI);
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

}

void slide_home_auto_ui_destory(slide_home_ui_t *ui)
{
	lv_obj_clean(ui->cont);
}
