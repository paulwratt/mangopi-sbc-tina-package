/**********************
 *      includes
 **********************/
#include "ui_sound.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/
static lv_style_t style_screen;
static lv_style_t style0_label_time;
static lv_style_t style0_media_list;
static lv_style_t style1_media_list;
static lv_style_t style3_media_list;
static lv_style_t style4_media_list;
static lv_style_t style5_media_list;
static lv_style_t style6_media_list;
static lv_style_t style7_media_list;
static lv_style_t style0_container_1;
static lv_style_t style1_progressbar;
static lv_style_t style2_progressbar;
static lv_style_t style1_volume_bar;
static lv_style_t style2_volume_bar;
static lv_style_t style0_curr_time;
static lv_style_t style0_total_time;
static lv_style_t style0_warn_info;
static lv_style_t style0_delete;
static lv_style_t style0_de_label;

/**********************
 *  images and fonts
 **********************/
static void *take_music_play_png_state0 = NULL;
static void *take_music_pause_png_state2 = NULL;
static void *play_music_play_png_state0 = NULL;
static void *play_music_play_png_state1 = NULL;
static void *play_music_pause_png_state2 = NULL;
static void *play_music_pause_png_state3 = NULL;
static void *volume_music_vol_png_state0 = NULL;
static void *volume_music_vol_png_state1 = NULL;
static void *volume_music_vol_png_state2 = NULL;
static void *volume_music_vol_png_state3 = NULL;
static void *volume_music_vol_png_state4 = NULL;
static void *media_list_1_music_item_png = NULL;

/**********************
 *  functions
 **********************/
void sound_auto_ui_create(sound_ui_t *ui)
{
	lv_style_copy(&style_screen, &lv_style_scr);
	style_screen.body.main_color = lv_color_hex(0xaaffff);
	style_screen.body.grad_color = lv_color_hex(0xaaffff);
	lv_obj_set_style(ui->cont, &style_screen);

#ifdef LV_USE_IMGBTN
	ui->take = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->take, 213, 219);
	lv_obj_set_size(ui->take, 48, 48);
	lv_imgbtn_set_state(ui->take, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->take, true);
	take_music_play_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_play.png");
	lv_imgbtn_set_src(ui->take, LV_BTN_STATE_REL, take_music_play_png_state0);
	take_music_pause_png_state2 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_pause.png");
	lv_imgbtn_set_src(ui->take, LV_BTN_STATE_TGL_REL, take_music_pause_png_state2);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_time, &lv_style_transp);
	style0_label_time.text.font = &lv_font_roboto_28;
	style0_label_time.text.line_space = 2;

	ui->label_time = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_time, "00:00:00");
	lv_label_set_align(ui->label_time, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_time, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_time, 179, 168);
	lv_obj_set_size(ui->label_time, 124, 35);
	lv_label_set_style(ui->label_time, LV_LABEL_STYLE_MAIN, &style0_label_time);
#endif // LV_USE_LABEL

#ifdef LV_USE_LIST
	lv_style_copy(&style0_media_list, &lv_style_transp_fit);
	style0_media_list.body.border.color = lv_color_hex(0x5555ff);

	lv_style_copy(&style1_media_list, &lv_style_pretty);
	style1_media_list.body.main_color = lv_color_hex(0x9d9d9d);
	style1_media_list.body.grad_color = lv_color_hex(0x9d9d9d);
	style1_media_list.body.radius = 10;
	style1_media_list.body.opa = 0;
	style1_media_list.body.border.width = 0;
	style1_media_list.body.padding.bottom = 0;
	style1_media_list.body.padding.left = 0;
	style1_media_list.body.padding.right = 0;

	lv_style_copy(&style3_media_list, &lv_style_transp);
	style3_media_list.body.main_color = lv_color_hex(0x5596d8);
	style3_media_list.body.grad_color = lv_color_hex(0x5596d8);
	style3_media_list.body.radius = 0;
	style3_media_list.body.opa = 255;
	style3_media_list.body.border.color = lv_color_hex(0x000000);
	style3_media_list.body.border.width = 0;
	style3_media_list.body.border.part = 15;
	style3_media_list.body.border.opa = 255;
	style3_media_list.body.shadow.color = lv_color_hex(0x808080);
	style3_media_list.body.shadow.width = 0;
	style3_media_list.body.shadow.type = 1;
	style3_media_list.body.padding.top = 5;
	style3_media_list.body.padding.bottom = 5;
	style3_media_list.body.padding.left = 5;
	style3_media_list.body.padding.right = 5;
	style3_media_list.body.padding.inner = 5;
	style3_media_list.text.color = lv_color_hex(0xf0f0f0);
	style3_media_list.text.sel_color = lv_color_hex(0x5596d8);
	style3_media_list.text.font = &lv_font_roboto_16;
	style3_media_list.text.letter_space = 0;
	style3_media_list.text.line_space = 2;
	style3_media_list.text.opa = 255;
	style3_media_list.image.color = lv_color_hex(0xf0f0f0);
	style3_media_list.image.intense = 0;
	style3_media_list.image.opa = 255;

	lv_style_copy(&style4_media_list, &lv_style_btn_rel);
	style4_media_list.body.main_color = lv_color_hex(0xebebeb);
	style4_media_list.body.grad_color = lv_color_hex(0xe5e5e5);
	style4_media_list.body.radius = 20;
	style4_media_list.body.border.color = lv_color_hex(0x5555ff);
	style4_media_list.body.border.width = 0;
	style4_media_list.body.border.part = 1;
	style4_media_list.text.color = lv_color_hex(0x000000);
	style4_media_list.text.line_space = 2;
	style4_media_list.image.color = lv_color_hex(0x00aa7f);
	style4_media_list.image.intense = 255;

	lv_style_copy(&style5_media_list, &lv_style_btn_pr);
	style5_media_list.body.main_color = lv_color_hex(0x55aaff);
	style5_media_list.body.grad_color = lv_color_hex(0x5555ff);
	style5_media_list.body.radius = 20;
	style5_media_list.body.border.width = 0;
	style5_media_list.text.line_space = 2;

	lv_style_copy(&style6_media_list, &lv_style_btn_tgl_rel);
	style6_media_list.body.main_color = lv_color_hex(0x55aaff);
	style6_media_list.body.grad_color = lv_color_hex(0x5555ff);
	style6_media_list.body.radius = 20;
	style6_media_list.body.border.color = lv_color_hex(0xff0000);
	style6_media_list.body.border.width = 0;
	style6_media_list.text.line_space = 2;
	style6_media_list.image.color = lv_color_hex(0xff0000);

	lv_style_copy(&style7_media_list, &lv_style_btn_tgl_pr);
	style7_media_list.body.main_color = lv_color_hex(0x55aaff);
	style7_media_list.body.grad_color = lv_color_hex(0x5555ff);
	style7_media_list.body.radius = 20;
	style7_media_list.body.border.color = lv_color_hex(0xff0000);
	style7_media_list.body.border.width = 0;
	style7_media_list.text.line_space = 2;

	ui->media_list = lv_list_create(ui->cont, NULL);
	lv_obj_set_pos(ui->media_list, 486, 2);
	lv_obj_set_size(ui->media_list, 306, 417);

	media_list_1_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->media_list, media_list_1_music_item_png, "");
	lv_list_set_single_mode(ui->media_list, true);
	lv_list_set_scroll_propagation(ui->media_list, false);
	lv_list_set_edge_flash(ui->media_list, true);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BG, &style0_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_SCRL, &style1_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_EDGE_FLASH, &style3_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_REL, &style4_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_PR, &style5_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_TGL_REL, &style6_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_TGL_PR, &style7_media_list);
#endif // LV_USE_LIST

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_1, &lv_style_pretty);
	style0_container_1.body.radius = 0;

	ui->container_1 = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->container_1, 0, 425);
	lv_obj_set_size(ui->container_1, 800, 55);
	lv_cont_set_fit4(ui->container_1, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->container_1, LV_CONT_STYLE_MAIN, &style0_container_1);
#endif // LV_USE_CONT

#ifdef LV_USE_IMGBTN
	ui->play = lv_imgbtn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->play, 15, 5);
	lv_obj_set_size(ui->play, 48, 48);
	lv_imgbtn_set_state(ui->play, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->play, true);
	play_music_play_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_play.png");
	lv_imgbtn_set_src(ui->play, LV_BTN_STATE_REL, play_music_play_png_state0);
	play_music_play_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_play.png");
	lv_imgbtn_set_src(ui->play, LV_BTN_STATE_PR, play_music_play_png_state1);
	play_music_pause_png_state2 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_pause.png");
	lv_imgbtn_set_src(ui->play, LV_BTN_STATE_TGL_REL, play_music_pause_png_state2);
	play_music_pause_png_state3 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_pause.png");
	lv_imgbtn_set_src(ui->play, LV_BTN_STATE_TGL_PR, play_music_pause_png_state3);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_SLIDER
	lv_style_copy(&style1_progressbar, &lv_style_pretty_color);
	style1_progressbar.body.padding.top = 0;
	style1_progressbar.body.padding.bottom = 0;
	style1_progressbar.body.padding.left = 0;
	style1_progressbar.body.padding.right = 0;
	style1_progressbar.body.padding.inner = 0;

	lv_style_copy(&style2_progressbar, &lv_style_pretty);
	style2_progressbar.body.radius = 15;

	ui->progressbar = lv_slider_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->progressbar, 91, 14);
	lv_obj_set_size(ui->progressbar, 495, 20);
	lv_slider_set_range(ui->progressbar, 0, 1024);
	lv_slider_set_knob_in(ui->progressbar, false);
	lv_slider_set_value(ui->progressbar, 0, LV_ANIM_OFF);
	lv_slider_set_style(ui->progressbar, LV_SLIDER_STYLE_INDIC, &style1_progressbar);
	lv_slider_set_style(ui->progressbar, LV_SLIDER_STYLE_KNOB, &style2_progressbar);
#endif // LV_USE_SLIDER

#ifdef LV_USE_SLIDER
	lv_style_copy(&style1_volume_bar, &lv_style_pretty_color);
	style1_volume_bar.body.padding.top = 0;
	style1_volume_bar.body.padding.bottom = 0;
	style1_volume_bar.body.padding.left = 0;
	style1_volume_bar.body.padding.right = 0;
	style1_volume_bar.body.padding.inner = 0;

	lv_style_copy(&style2_volume_bar, &lv_style_pretty);
	style2_volume_bar.body.radius = 15;

	ui->volume_bar = lv_slider_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->volume_bar, 639, 16);
	lv_obj_set_size(ui->volume_bar, 149, 20);
	lv_slider_set_range(ui->volume_bar, 0, 40);
	lv_slider_set_knob_in(ui->volume_bar, false);
	lv_slider_set_value(ui->volume_bar, 30, LV_ANIM_OFF);
	lv_slider_set_style(ui->volume_bar, LV_SLIDER_STYLE_INDIC, &style1_volume_bar);
	lv_slider_set_style(ui->volume_bar, LV_SLIDER_STYLE_KNOB, &style2_volume_bar);
#endif // LV_USE_SLIDER

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_curr_time, &lv_style_transp);
	style0_curr_time.text.color = lv_color_hex(0x000000);
	style0_curr_time.text.line_space = 2;

	ui->curr_time = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->curr_time, "00:00:00");
	lv_label_set_long_mode(ui->curr_time, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->curr_time, 83, 33);
	lv_obj_set_size(ui->curr_time, 62, 19);
	lv_label_set_style(ui->curr_time, LV_LABEL_STYLE_MAIN, &style0_curr_time);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_total_time, &lv_style_transp);
	style0_total_time.text.color = lv_color_hex(0x000000);
	style0_total_time.text.line_space = 2;

	ui->total_time = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->total_time, "00:00:00");
	lv_label_set_long_mode(ui->total_time, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->total_time, 545, 31);
	lv_obj_set_size(ui->total_time, 62, 19);
	lv_label_set_style(ui->total_time, LV_LABEL_STYLE_MAIN, &style0_total_time);
#endif // LV_USE_LABEL

#ifdef LV_USE_IMGBTN
	ui->volume = lv_imgbtn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->volume, 599, 9);
	lv_obj_set_size(ui->volume, 32, 32);
	lv_imgbtn_set_state(ui->volume, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->volume, true);
	volume_music_vol_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_vol.png");
	lv_imgbtn_set_src(ui->volume, LV_BTN_STATE_REL, volume_music_vol_png_state0);
	volume_music_vol_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_vol.png");
	lv_imgbtn_set_src(ui->volume, LV_BTN_STATE_PR, volume_music_vol_png_state1);
	volume_music_vol_png_state2 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_vol.png");
	lv_imgbtn_set_src(ui->volume, LV_BTN_STATE_TGL_REL, volume_music_vol_png_state2);
	volume_music_vol_png_state3 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_vol.png");
	lv_imgbtn_set_src(ui->volume, LV_BTN_STATE_TGL_PR, volume_music_vol_png_state3);
	volume_music_vol_png_state4 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_vol.png");
	lv_imgbtn_set_src(ui->volume, LV_BTN_STATE_INA, volume_music_vol_png_state4);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_warn_info, &lv_style_transp);
	style0_warn_info.text.font = &microsoft_yahei_en_cn_24_4;
	style0_warn_info.text.line_space = 2;

	ui->warn_info = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->warn_info, "警告：正在录音！！！");
	lv_label_set_long_mode(ui->warn_info, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->warn_info, 116, 115);
	lv_obj_set_size(ui->warn_info, 284, 39);
	lv_obj_set_hidden(ui->warn_info,true);
	lv_label_set_style(ui->warn_info, LV_LABEL_STYLE_MAIN, &style0_warn_info);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_delete, &lv_style_btn_rel);
	style0_delete.body.opa = 0;

	ui->delete = lv_btn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->delete, 410, 14);
	lv_obj_set_size(ui->delete, 64, 33);
	lv_btn_set_style(ui->delete, LV_BTN_STYLE_REL, &style0_delete);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_de_label, &lv_style_transp);
	style0_de_label.text.color = lv_color_hex(0xff0000);
	style0_de_label.text.line_space = 2;

	ui->de_label = lv_label_create(ui->delete, NULL);
	lv_label_set_text(ui->de_label, "delete");
	lv_label_set_align(ui->de_label, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->de_label, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->de_label, 7, 5);
	lv_obj_set_size(ui->de_label, 50, 22);
	lv_obj_set_hidden(ui->de_label,true);
	lv_label_set_style(ui->de_label, LV_LABEL_STYLE_MAIN, &style0_de_label);
#endif // LV_USE_LABEL

}

void sound_auto_ui_destory(sound_ui_t *ui)
{
	lv_obj_clean(ui->cont);
	free_image(take_music_play_png_state0);
	free_image(take_music_pause_png_state2);
	free_image(play_music_play_png_state0);
	free_image(play_music_play_png_state1);
	free_image(play_music_pause_png_state2);
	free_image(play_music_pause_png_state3);
	free_image(volume_music_vol_png_state0);
	free_image(volume_music_vol_png_state1);
	free_image(volume_music_vol_png_state2);
	free_image(volume_music_vol_png_state3);
	free_image(volume_music_vol_png_state4);
	free_image(media_list_1_music_item_png);
}
