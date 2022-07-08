/**********************
 *      includes
 **********************/
#include "ui_ota.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/
static lv_style_t style0_progressbar;
static lv_style_t style1_progressbar;
static lv_style_t style2_progressbar;
static lv_style_t style0_label_progress;
static lv_style_t style0_prompt_lable;

/**********************
 *  images and fonts
 **********************/

/**********************
 *  functions
 **********************/
void ota_auto_ui_create(ota_ui_t *ui)
{

#ifdef LV_USE_SLIDER
	lv_style_copy(&style0_progressbar, &lv_style_pretty);

	lv_style_copy(&style1_progressbar, &lv_style_pretty_color);
	style1_progressbar.body.padding.top = 0;
	style1_progressbar.body.padding.bottom = 0;
	style1_progressbar.body.padding.left = 0;
	style1_progressbar.body.padding.right = 0;
	style1_progressbar.body.padding.inner = 0;

	lv_style_copy(&style2_progressbar, &lv_style_pretty);
	style2_progressbar.body.radius = 15;

	ui->progressbar = lv_slider_create(ui->cont, NULL);
	lv_obj_set_pos(ui->progressbar, 121, 232);
	lv_obj_set_size(ui->progressbar, 495, 20);
	lv_slider_set_range(ui->progressbar, 0, 100);
	lv_slider_set_knob_in(ui->progressbar, false);
	lv_slider_set_value(ui->progressbar, 0, LV_ANIM_OFF);
	lv_slider_set_style(ui->progressbar, LV_SLIDER_STYLE_BG, &style0_progressbar);
	lv_slider_set_style(ui->progressbar, LV_SLIDER_STYLE_INDIC, &style1_progressbar);
	lv_slider_set_style(ui->progressbar, LV_SLIDER_STYLE_KNOB, &style2_progressbar);
#endif // LV_USE_SLIDER

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_progress, &lv_style_transp);

	ui->label_progress = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_progress, "0%");
	lv_label_set_long_mode(ui->label_progress, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_progress, 636, 232);
	lv_obj_set_size(ui->label_progress, 150, 29);
	lv_label_set_style(ui->label_progress, LV_LABEL_STYLE_MAIN, &style0_label_progress);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_prompt_lable, &lv_style_transp);

	ui->prompt_lable = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->prompt_lable, "updating please wating");
	lv_label_set_long_mode(ui->prompt_lable, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->prompt_lable, 117, 189);
	lv_obj_set_size(ui->prompt_lable, 300, 22);
	lv_label_set_style(ui->prompt_lable, LV_LABEL_STYLE_MAIN, &style0_prompt_lable);
#endif // LV_USE_LABEL

}

void ota_auto_ui_destory(ota_ui_t *ui)
{
	lv_obj_clean(ui->cont);
}
