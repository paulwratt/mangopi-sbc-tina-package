/**********************
 *      includes
 **********************/
#include "ui_movie.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/
static lv_style_t style_screen;
static lv_style_t style0_media_list;
static lv_style_t style1_media_list;
static lv_style_t style2_media_list;
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
static lv_style_t style0_download;
static lv_style_t style0_full_button;
static lv_style_t style1_full_button;
static lv_style_t style2_full_button;
static lv_style_t style3_full_button;
static lv_style_t style0_online;
static lv_style_t style0_label_a;
static lv_style_t style0_label_b;
static lv_style_t style0_label_ab;
static lv_style_t style0_label_ratio;
static lv_style_t style0_label_sound_channel;
static lv_style_t style0_label_audio_track;
static lv_style_t style0_label_menu_esc;
static lv_style_t style0_label_ratio_video;
static lv_style_t style0_label_ratio_screen;
static lv_style_t style0_label_ratio_original;
static lv_style_t style0_label_ratio_43;
static lv_style_t style0_label_ratio_169;
static lv_style_t style0_label_sound_stereo;
static lv_style_t style0_label_sound_left;
static lv_style_t style0_label_sound_right;
static lv_style_t style3_audio_track_list;

/**********************
 *  images and fonts
 **********************/
static void *image_4_movie_big_png = NULL;
static void *play_music_play_png_state0 = NULL;
static void *play_music_play_png_state1 = NULL;
static void *play_music_pause_png_state2 = NULL;
static void *play_music_pause_png_state3 = NULL;
static void *next_music_next_png_state0 = NULL;
static void *next_music_next_png_state1 = NULL;
static void *last_music_prev_png_state0 = NULL;
static void *last_music_prev_png_state1 = NULL;
static void *play_mode_all_cycle_png_state0 = NULL;
static void *play_mode_all_cycle_png_state1 = NULL;
static void *play_mode_all_cycle_png_state2 = NULL;
static void *play_mode_all_cycle_png_state3 = NULL;
static void *volume_music_vol_png_state0 = NULL;
static void *volume_music_vol_png_state1 = NULL;
static void *volume_music_vol_png_state2 = NULL;
static void *volume_music_vol_png_state3 = NULL;
static void *volume_music_vol_png_state4 = NULL;
static void *order_explorer_prev_png_state0 = NULL;
static void *order_music_order_png_state1 = NULL;
static void *media_list_1_movie_item_png = NULL;
static void *media_list_2_movie_item_png = NULL;
static void *media_list_3_movie_item_png = NULL;
static void *media_list_4_movie_item_png = NULL;
static void *media_list_5_movie_item_png = NULL;
static void *media_list_6_movie_item_png = NULL;

/**********************
 *  functions
 **********************/
void movie_auto_ui_create(movie_ui_t *ui)
{
	lv_style_copy(&style_screen, &lv_style_scr);
	style_screen.body.main_color = lv_color_hex(0xff557f);
	style_screen.body.grad_color = lv_color_hex(0xff557f);
	lv_obj_set_style(ui->cont, &style_screen);

#ifdef LV_USE_LIST
	lv_style_copy(&style0_media_list, &lv_style_transp_fit);
	style0_media_list.body.main_color = lv_color_hex(0xff55ff);

	lv_style_copy(&style1_media_list, &lv_style_pretty);
	style1_media_list.body.main_color = lv_color_hex(0xff557f);
	style1_media_list.body.grad_color = lv_color_hex(0xff557f);
	style1_media_list.body.opa = 0;
	style1_media_list.body.border.width = 0;

	lv_style_copy(&style2_media_list, &lv_style_pretty_color);
	style2_media_list.body.main_color = lv_color_hex(0xff557f);

	lv_style_copy(&style4_media_list, &lv_style_btn_rel);
	style4_media_list.body.main_color = lv_color_hex(0xffaaff);
	style4_media_list.body.grad_color = lv_color_hex(0xff55ff);
	style4_media_list.body.radius = 20;
	style4_media_list.text.line_space = 2;

	lv_style_copy(&style5_media_list, &lv_style_btn_pr);
	style5_media_list.body.main_color = lv_color_hex(0x55aaff);
	style5_media_list.body.grad_color = lv_color_hex(0x55aaff);
	style5_media_list.body.radius = 20;
	style5_media_list.text.line_space = 2;

	lv_style_copy(&style6_media_list, &lv_style_btn_tgl_rel);
	style6_media_list.body.main_color = lv_color_hex(0x55aaff);
	style6_media_list.body.grad_color = lv_color_hex(0x55aaff);
	style6_media_list.body.radius = 20;
	style6_media_list.text.line_space = 2;

	lv_style_copy(&style7_media_list, &lv_style_btn_tgl_pr);
	style7_media_list.body.main_color = lv_color_hex(0x55aaff);
	style7_media_list.body.grad_color = lv_color_hex(0x55aaff);
	style7_media_list.body.radius = 20;
	style7_media_list.text.line_space = 2;

	ui->media_list = lv_list_create(ui->cont, NULL);
	lv_obj_set_pos(ui->media_list, 382, 9);
	lv_obj_set_size(ui->media_list, 418, 417);

	media_list_1_movie_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"movie_item.png");
	lv_list_add_btn(ui->media_list, media_list_1_movie_item_png, "1");

	media_list_2_movie_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"movie_item.png");
	lv_list_add_btn(ui->media_list, media_list_2_movie_item_png, "2");

	media_list_3_movie_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"movie_item.png");
	lv_list_add_btn(ui->media_list, media_list_3_movie_item_png, "3");

	media_list_4_movie_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"movie_item.png");
	lv_list_add_btn(ui->media_list, media_list_4_movie_item_png, "4");

	media_list_5_movie_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"movie_item.png");
	lv_list_add_btn(ui->media_list, media_list_5_movie_item_png, "5");

	media_list_6_movie_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"movie_item.png");
	lv_list_add_btn(ui->media_list, media_list_6_movie_item_png, "6");
	lv_list_set_single_mode(ui->media_list, true);
	lv_list_set_scroll_propagation(ui->media_list, false);
	lv_list_set_edge_flash(ui->media_list, true);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BG, &style0_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_SCRL, &style1_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_SB, &style2_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_REL, &style4_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_PR, &style5_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_TGL_REL, &style6_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_TGL_PR, &style7_media_list);
#endif // LV_USE_LIST

#ifdef LV_USE_IMG
	ui->image_4 = lv_img_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_4, 121, 93);
	lv_obj_set_size(ui->image_4, 128, 128);
	image_4_movie_big_png = (void *)parse_image_from_file(LV_IMAGE_PATH"movie_big.png");
	lv_img_set_src(ui->image_4, image_4_movie_big_png);

#endif // LV_USE_IMG

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
	lv_obj_set_pos(ui->play, 70, 3);
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
	lv_obj_set_pos(ui->progressbar, 207, 13);
	lv_obj_set_size(ui->progressbar, 299, 20);
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
	lv_obj_set_pos(ui->volume_bar, 652, 16);
	lv_obj_set_size(ui->volume_bar, 136, 20);
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
	lv_obj_set_pos(ui->curr_time, 177, 35);
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
	lv_obj_set_pos(ui->total_time, 453, 33);
	lv_obj_set_size(ui->total_time, 62, 19);
	lv_label_set_style(ui->total_time, LV_LABEL_STYLE_MAIN, &style0_total_time);
#endif // LV_USE_LABEL

#ifdef LV_USE_IMGBTN
	ui->next = lv_imgbtn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->next, 128, 3);
	lv_obj_set_size(ui->next, 48, 48);
	lv_imgbtn_set_state(ui->next, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->next, false);
	next_music_next_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_next.png");
	lv_imgbtn_set_src(ui->next, LV_BTN_STATE_REL, next_music_next_png_state0);
	next_music_next_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_next.png");
	lv_imgbtn_set_src(ui->next, LV_BTN_STATE_PR, next_music_next_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->last = lv_imgbtn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->last, 14, 4);
	lv_obj_set_size(ui->last, 48, 48);
	lv_imgbtn_set_state(ui->last, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->last, false);
	last_music_prev_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_prev.png");
	lv_imgbtn_set_src(ui->last, LV_BTN_STATE_REL, last_music_prev_png_state0);
	last_music_prev_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_prev.png");
	lv_imgbtn_set_src(ui->last, LV_BTN_STATE_PR, last_music_prev_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->play_mode = lv_imgbtn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->play_mode, 567, 11);
	lv_obj_set_size(ui->play_mode, 32, 32);
	lv_imgbtn_set_state(ui->play_mode, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->play_mode, false);
	play_mode_all_cycle_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"all_cycle.png");
	lv_imgbtn_set_src(ui->play_mode, LV_BTN_STATE_REL, play_mode_all_cycle_png_state0);
	play_mode_all_cycle_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"all_cycle.png");
	lv_imgbtn_set_src(ui->play_mode, LV_BTN_STATE_PR, play_mode_all_cycle_png_state1);
	play_mode_all_cycle_png_state2 = (void*)parse_image_from_file(LV_IMAGE_PATH"all_cycle.png");
	lv_imgbtn_set_src(ui->play_mode, LV_BTN_STATE_TGL_REL, play_mode_all_cycle_png_state2);
	play_mode_all_cycle_png_state3 = (void*)parse_image_from_file(LV_IMAGE_PATH"all_cycle.png");
	lv_imgbtn_set_src(ui->play_mode, LV_BTN_STATE_TGL_PR, play_mode_all_cycle_png_state3);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->volume = lv_imgbtn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->volume, 610, 11);
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

#ifdef LV_USE_BTN
	ui->button_menu = lv_btn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->button_menu, 513, 15);
	lv_obj_set_size(ui->button_menu, 50, 23);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_menu = lv_label_create(ui->button_menu, NULL);
	lv_label_set_text(ui->label_menu, "menu");
	lv_label_set_long_mode(ui->label_menu, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_menu, 5, 0);
	lv_obj_set_size(ui->label_menu, 40, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->file_name = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->file_name, "");
	lv_label_set_align(ui->file_name, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->file_name, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->file_name, 303, 0);
	lv_obj_set_size(ui->file_name, 162, 37);
	lv_obj_set_hidden(ui->file_name,true);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_download, &lv_style_transp);
	style0_download.text.color = lv_color_hex(0xffff7f);
	style0_download.text.font = &lv_font_roboto_28;
	style0_download.text.line_space = 2;

	ui->download = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->download, "");
	lv_label_set_long_mode(ui->download, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->download, 281, 202);
	lv_obj_set_size(ui->download, 223, 38);
	lv_obj_set_hidden(ui->download,true);
	lv_label_set_style(ui->download, LV_LABEL_STYLE_MAIN, &style0_download);
#endif // LV_USE_LABEL

#ifdef LV_USE_IMGBTN
	ui->order = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->order, 700, 207);
	lv_obj_set_size(ui->order, 32, 32);
	lv_imgbtn_set_state(ui->order, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->order, false);
	order_explorer_prev_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"explorer_prev.png");
	lv_imgbtn_set_src(ui->order, LV_BTN_STATE_REL, order_explorer_prev_png_state0);
	order_music_order_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_order.png");
	lv_imgbtn_set_src(ui->order, LV_BTN_STATE_PR, order_music_order_png_state1);
	lv_obj_set_hidden(ui->order,true);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_BTN
	lv_style_copy(&style0_full_button, &lv_style_btn_rel);
	style0_full_button.body.main_color = lv_color_hex(0x000000);
	style0_full_button.body.grad_color = lv_color_hex(0x000000);
	style0_full_button.body.opa = 0;
	style0_full_button.body.border.opa = 0;

	lv_style_copy(&style1_full_button, &lv_style_btn_pr);
	style1_full_button.body.opa = 0;
	style1_full_button.body.border.opa = 0;

	lv_style_copy(&style2_full_button, &lv_style_btn_tgl_rel);
	style2_full_button.body.opa = 0;
	style2_full_button.body.border.opa = 0;

	lv_style_copy(&style3_full_button, &lv_style_btn_tgl_pr);
	style3_full_button.body.opa = 0;
	style3_full_button.body.border.opa = 0;

	ui->full_button = lv_btn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->full_button, 3, 81);
	lv_obj_set_size(ui->full_button, 797, 342);
	lv_btn_set_style(ui->full_button, LV_BTN_STYLE_REL, &style0_full_button);
	lv_btn_set_style(ui->full_button, LV_BTN_STYLE_PR, &style1_full_button);
	lv_btn_set_style(ui->full_button, LV_BTN_STYLE_TGL_REL, &style2_full_button);
	lv_btn_set_style(ui->full_button, LV_BTN_STYLE_TGL_PR, &style3_full_button);
#endif // LV_USE_BTN

#ifdef LV_USE_BTN
	lv_style_copy(&style0_online, &lv_style_btn_rel);
	style0_online.body.main_color = lv_color_hex(0xff00ff);
	style0_online.body.grad_color = lv_color_hex(0xff00ff);
	style0_online.body.opa = 128;

	ui->online = lv_btn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->online, 337, 2);
	lv_obj_set_size(ui->online, 55, 31);
	lv_btn_set_style(ui->online, LV_BTN_STYLE_REL, &style0_online);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_online = lv_label_create(ui->online, NULL);
	lv_label_set_text(ui->label_online, "online");
	lv_label_set_align(ui->label_online, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_online, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_online, 4, 4);
	lv_obj_set_size(ui->label_online, 47, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_PAGE
	ui->ab_page = lv_page_create(ui->cont, NULL);
	lv_obj_set_pos(ui->ab_page, 267, 195);
	lv_obj_set_size(ui->ab_page, 266, 191);
	lv_page_set_sb_mode(ui->ab_page, LV_SB_MODE_AUTO);
	lv_page_set_edge_flash(ui->ab_page, false);
	lv_page_set_scroll_propagation(ui->ab_page, false);
	lv_obj_set_hidden(ui->ab_page,true);
#endif // LV_USE_PAGE

#ifdef LV_USE_SLIDER
	ui->slider_a = lv_slider_create(ui->ab_page, NULL);
	lv_obj_set_pos(ui->slider_a, 27, 36);
	lv_obj_set_size(ui->slider_a, 197, 25);
	lv_slider_set_range(ui->slider_a, 0, 1024);
	lv_slider_set_knob_in(ui->slider_a, false);
	lv_slider_set_value(ui->slider_a, 19, LV_ANIM_OFF);
#endif // LV_USE_SLIDER

#ifdef LV_USE_SLIDER
	ui->slider_b = lv_slider_create(ui->ab_page, NULL);
	lv_obj_set_pos(ui->slider_b, 28, 98);
	lv_obj_set_size(ui->slider_b, 198, 25);
	lv_slider_set_range(ui->slider_b, 0, 1024);
	lv_slider_set_knob_in(ui->slider_b, false);
	lv_slider_set_value(ui->slider_b, 68, LV_ANIM_OFF);
#endif // LV_USE_SLIDER

#ifdef LV_USE_BTN
	ui->button_ok = lv_btn_create(ui->ab_page, NULL);
	lv_obj_set_pos(ui->button_ok, 33, 141);
	lv_obj_set_size(ui->button_ok, 64, 25);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_AB_OK = lv_label_create(ui->button_ok, NULL);
	lv_label_set_text(ui->label_AB_OK, "OK");
	lv_label_set_long_mode(ui->label_AB_OK, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_AB_OK, 14, 1);
	lv_obj_set_size(ui->label_AB_OK, 36, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_1 = lv_label_create(ui->ab_page, NULL);
	lv_label_set_text(ui->label_1, "A");
	lv_label_set_long_mode(ui->label_1, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_1, 8, 40);
	lv_obj_set_size(ui->label_1, 20, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_2 = lv_label_create(ui->ab_page, NULL);
	lv_label_set_text(ui->label_2, "B");
	lv_label_set_long_mode(ui->label_2, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_2, 5, 103);
	lv_obj_set_size(ui->label_2, 22, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_a, &lv_style_transp);
	style0_label_a.text.color = lv_color_hex(0x000000);
	style0_label_a.text.line_space = 2;

	ui->label_a = lv_label_create(ui->ab_page, NULL);
	lv_label_set_text(ui->label_a, "00:00:00");
	lv_label_set_long_mode(ui->label_a, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_a, 176, 25);
	lv_obj_set_size(ui->label_a, 62, 19);
	lv_label_set_style(ui->label_a, LV_LABEL_STYLE_MAIN, &style0_label_a);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_b, &lv_style_transp);
	style0_label_b.text.color = lv_color_hex(0x000000);
	style0_label_b.text.line_space = 2;

	ui->label_b = lv_label_create(ui->ab_page, NULL);
	lv_label_set_text(ui->label_b, "00:00:00");
	lv_label_set_long_mode(ui->label_b, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_b, 175, 81);
	lv_obj_set_size(ui->label_b, 62, 19);
	lv_label_set_style(ui->label_b, LV_LABEL_STYLE_MAIN, &style0_label_b);
#endif // LV_USE_LABEL

#ifdef LV_USE_TA
	ui->text_area_1 = lv_ta_create(ui->ab_page, NULL);
	lv_obj_set_pos(ui->text_area_1, 110, 5);
	lv_obj_set_size(ui->text_area_1, 37, 26);
	lv_ta_set_text(ui->text_area_1, "A/B");
	lv_ta_set_placeholder_text(ui->text_area_1, "");
	lv_ta_set_cursor_type(ui->text_area_1, LV_CURSOR_NONE);
	lv_ta_set_cursor_blink_time(ui->text_area_1, 144);
	lv_ta_set_one_line(ui->text_area_1, false);
	lv_ta_set_pwd_mode(ui->text_area_1, false);
	lv_ta_set_max_length(ui->text_area_1, 0);
	lv_ta_set_text_align(ui->text_area_1, 0);
	lv_ta_set_sb_mode(ui->text_area_1, 2);
	lv_ta_set_scroll_propagation(ui->text_area_1, false);
	lv_ta_set_edge_flash(ui->text_area_1, false);
#endif // LV_USE_TA

#ifdef LV_USE_BTN
	ui->button_esc = lv_btn_create(ui->ab_page, NULL);
	lv_obj_set_pos(ui->button_esc, 145, 141);
	lv_obj_set_size(ui->button_esc, 64, 25);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_AB_ESC = lv_label_create(ui->button_esc, NULL);
	lv_label_set_text(ui->label_AB_ESC, "ESC");
	lv_label_set_long_mode(ui->label_AB_ESC, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_AB_ESC, 14, 1);
	lv_obj_set_size(ui->label_AB_ESC, 36, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_PAGE
	ui->menu_page = lv_page_create(ui->cont, NULL);
	lv_obj_set_pos(ui->menu_page, 537, 196);
	lv_obj_set_size(ui->menu_page, 187, 227);
	lv_page_set_sb_mode(ui->menu_page, LV_SB_MODE_AUTO);
	lv_page_set_edge_flash(ui->menu_page, false);
	lv_page_set_scroll_propagation(ui->menu_page, false);
	lv_obj_set_hidden(ui->menu_page,true);
#endif // LV_USE_PAGE

#ifdef LV_USE_BTN
	ui->button_ab = lv_btn_create(ui->menu_page, NULL);
	lv_obj_set_pos(ui->button_ab, 7, 5);
	lv_obj_set_size(ui->button_ab, 163, 40);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_ab, &lv_style_transp);
	style0_label_ab.body.shadow.color = lv_color_hex(0xaa0000);
	style0_label_ab.text.color = lv_color_hex(0x55aaff);
	style0_label_ab.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_ab.text.line_space = 2;

	ui->label_AB = lv_label_create(ui->button_ab, NULL);
	lv_label_set_text(ui->label_AB, "A/B");
	lv_label_set_align(ui->label_AB, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_AB, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_AB, 47, 0);
	lv_obj_set_size(ui->label_AB, 68, 40);
	lv_label_set_style(ui->label_AB, LV_LABEL_STYLE_MAIN, &style0_label_ab);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_ratio = lv_btn_create(ui->menu_page, NULL);
	lv_obj_set_pos(ui->button_ratio, 7, 46);
	lv_obj_set_size(ui->button_ratio, 163, 40);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_ratio, &lv_style_transp);
	style0_label_ratio.body.shadow.color = lv_color_hex(0xaa0000);
	style0_label_ratio.text.color = lv_color_hex(0x55aaff);
	style0_label_ratio.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_ratio.text.line_space = 2;

	ui->label_ratio = lv_label_create(ui->button_ratio, NULL);
	lv_label_set_text(ui->label_ratio, "比例");
	lv_label_set_align(ui->label_ratio, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_ratio, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_ratio, 49, 0);
	lv_obj_set_size(ui->label_ratio, 64, 40);
	lv_label_set_style(ui->label_ratio, LV_LABEL_STYLE_MAIN, &style0_label_ratio);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_sound_track = lv_btn_create(ui->menu_page, NULL);
	lv_obj_set_pos(ui->button_sound_track, 7, 87);
	lv_obj_set_size(ui->button_sound_track, 163, 40);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_sound_channel, &lv_style_transp);
	style0_label_sound_channel.body.shadow.color = lv_color_hex(0xaa0000);
	style0_label_sound_channel.text.color = lv_color_hex(0x55aaff);
	style0_label_sound_channel.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_sound_channel.text.line_space = 2;

	ui->label_sound_channel = lv_label_create(ui->button_sound_track, NULL);
	lv_label_set_text(ui->label_sound_channel, "声道");
	lv_label_set_align(ui->label_sound_channel, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_sound_channel, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_sound_channel, 49, 0);
	lv_obj_set_size(ui->label_sound_channel, 64, 40);
	lv_label_set_style(ui->label_sound_channel, LV_LABEL_STYLE_MAIN, &style0_label_sound_channel);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_audio_track = lv_btn_create(ui->menu_page, NULL);
	lv_obj_set_pos(ui->button_audio_track, 8, 128);
	lv_obj_set_size(ui->button_audio_track, 163, 40);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_audio_track, &lv_style_transp);
	style0_label_audio_track.body.shadow.color = lv_color_hex(0xaa0000);
	style0_label_audio_track.text.color = lv_color_hex(0x55aaff);
	style0_label_audio_track.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_audio_track.text.line_space = 2;

	ui->label_audio_track = lv_label_create(ui->button_audio_track, NULL);
	lv_label_set_text(ui->label_audio_track, "音轨");
	lv_label_set_align(ui->label_audio_track, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_audio_track, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_audio_track, 49, 0);
	lv_obj_set_size(ui->label_audio_track, 64, 40);
	lv_label_set_style(ui->label_audio_track, LV_LABEL_STYLE_MAIN, &style0_label_audio_track);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_exit = lv_btn_create(ui->menu_page, NULL);
	lv_obj_set_pos(ui->button_exit, 7, 169);
	lv_obj_set_size(ui->button_exit, 163, 40);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_menu_esc, &lv_style_transp);
	style0_label_menu_esc.body.shadow.color = lv_color_hex(0xaa0000);
	style0_label_menu_esc.text.color = lv_color_hex(0x55aaff);
	style0_label_menu_esc.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_menu_esc.text.line_space = 2;

	ui->label_menu_esc = lv_label_create(ui->button_exit, NULL);
	lv_label_set_text(ui->label_menu_esc, "退出");
	lv_label_set_align(ui->label_menu_esc, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_menu_esc, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_menu_esc, 49, 0);
	lv_obj_set_size(ui->label_menu_esc, 64, 40);
	lv_label_set_style(ui->label_menu_esc, LV_LABEL_STYLE_MAIN, &style0_label_menu_esc);
#endif // LV_USE_LABEL

#ifdef LV_USE_PAGE
	ui->ratio_page = lv_page_create(ui->cont, NULL);
	lv_obj_set_pos(ui->ratio_page, 392, 195);
	lv_obj_set_size(ui->ratio_page, 138, 205);
	lv_page_set_sb_mode(ui->ratio_page, LV_SB_MODE_AUTO);
	lv_page_set_edge_flash(ui->ratio_page, false);
	lv_page_set_scroll_propagation(ui->ratio_page, false);
	lv_obj_set_hidden(ui->ratio_page,true);
#endif // LV_USE_PAGE

#ifdef LV_USE_BTN
	ui->button_video_ratio = lv_btn_create(ui->ratio_page, NULL);
	lv_obj_set_pos(ui->button_video_ratio, 7, 6);
	lv_obj_set_size(ui->button_video_ratio, 115, 36);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_ratio_video, &lv_style_transp);
	style0_label_ratio_video.text.color = lv_color_hex(0x55ff7f);
	style0_label_ratio_video.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_ratio_video.text.line_space = 2;

	ui->label_ratio_video = lv_label_create(ui->button_video_ratio, NULL);
	lv_label_set_text(ui->label_ratio_video, "视频比例");
	lv_label_set_align(ui->label_ratio_video, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_ratio_video, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_ratio_video, 2, -1);
	lv_obj_set_size(ui->label_ratio_video, 110, 39);
	lv_label_set_style(ui->label_ratio_video, LV_LABEL_STYLE_MAIN, &style0_label_ratio_video);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_screen_ratio = lv_btn_create(ui->ratio_page, NULL);
	lv_obj_set_pos(ui->button_screen_ratio, 7, 43);
	lv_obj_set_size(ui->button_screen_ratio, 115, 36);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_ratio_screen, &lv_style_transp);
	style0_label_ratio_screen.text.color = lv_color_hex(0x55ff7f);
	style0_label_ratio_screen.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_ratio_screen.text.line_space = 2;

	ui->label_ratio_screen = lv_label_create(ui->button_screen_ratio, NULL);
	lv_label_set_text(ui->label_ratio_screen, "屏幕比例");
	lv_label_set_align(ui->label_ratio_screen, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_ratio_screen, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_ratio_screen, 2, -1);
	lv_obj_set_size(ui->label_ratio_screen, 110, 38);
	lv_label_set_style(ui->label_ratio_screen, LV_LABEL_STYLE_MAIN, &style0_label_ratio_screen);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_original_ratio = lv_btn_create(ui->ratio_page, NULL);
	lv_obj_set_pos(ui->button_original_ratio, 7, 80);
	lv_obj_set_size(ui->button_original_ratio, 115, 36);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_ratio_original, &lv_style_transp);
	style0_label_ratio_original.text.color = lv_color_hex(0x55ff7f);
	style0_label_ratio_original.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_ratio_original.text.line_space = 2;

	ui->label_ratio_original = lv_label_create(ui->button_original_ratio, NULL);
	lv_label_set_text(ui->label_ratio_original, "原始比例");
	lv_label_set_align(ui->label_ratio_original, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_ratio_original, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_ratio_original, 2, -1);
	lv_obj_set_size(ui->label_ratio_original, 110, 38);
	lv_label_set_style(ui->label_ratio_original, LV_LABEL_STYLE_MAIN, &style0_label_ratio_original);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_43_ratio = lv_btn_create(ui->ratio_page, NULL);
	lv_obj_set_pos(ui->button_43_ratio, 7, 117);
	lv_obj_set_size(ui->button_43_ratio, 115, 36);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_ratio_43, &lv_style_transp);
	style0_label_ratio_43.text.color = lv_color_hex(0x55ff7f);
	style0_label_ratio_43.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_ratio_43.text.line_space = 2;

	ui->label_ratio_43 = lv_label_create(ui->button_43_ratio, NULL);
	lv_label_set_text(ui->label_ratio_43, "4比3");
	lv_label_set_align(ui->label_ratio_43, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_ratio_43, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_ratio_43, 27, -1);
	lv_obj_set_size(ui->label_ratio_43, 61, 38);
	lv_label_set_style(ui->label_ratio_43, LV_LABEL_STYLE_MAIN, &style0_label_ratio_43);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_169_ratio = lv_btn_create(ui->ratio_page, NULL);
	lv_obj_set_pos(ui->button_169_ratio, 8, 154);
	lv_obj_set_size(ui->button_169_ratio, 114, 36);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_ratio_169, &lv_style_transp);
	style0_label_ratio_169.text.color = lv_color_hex(0x55ff7f);
	style0_label_ratio_169.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_ratio_169.text.line_space = 2;

	ui->label_ratio_169 = lv_label_create(ui->button_169_ratio, NULL);
	lv_label_set_text(ui->label_ratio_169, "16比9");
	lv_label_set_align(ui->label_ratio_169, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_ratio_169, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_ratio_169, 12, -1);
	lv_obj_set_size(ui->label_ratio_169, 90, 38);
	lv_label_set_style(ui->label_ratio_169, LV_LABEL_STYLE_MAIN, &style0_label_ratio_169);
#endif // LV_USE_LABEL

#ifdef LV_USE_PAGE
	ui->sound_track_page = lv_page_create(ui->cont, NULL);
	lv_obj_set_pos(ui->sound_track_page, 397, 198);
	lv_obj_set_size(ui->sound_track_page, 135, 131);
	lv_page_set_sb_mode(ui->sound_track_page, LV_SB_MODE_AUTO);
	lv_page_set_edge_flash(ui->sound_track_page, false);
	lv_page_set_scroll_propagation(ui->sound_track_page, false);
	lv_obj_set_hidden(ui->sound_track_page,true);
#endif // LV_USE_PAGE

#ifdef LV_USE_BTN
	ui->button_stereo = lv_btn_create(ui->sound_track_page, NULL);
	lv_obj_set_pos(ui->button_stereo, 6, 5);
	lv_obj_set_size(ui->button_stereo, 112, 36);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_sound_stereo, &lv_style_transp);
	style0_label_sound_stereo.text.color = lv_color_hex(0x55ff7f);
	style0_label_sound_stereo.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_sound_stereo.text.line_space = 2;

	ui->label_sound_stereo = lv_label_create(ui->button_stereo, NULL);
	lv_label_set_text(ui->label_sound_stereo, "立体声");
	lv_label_set_align(ui->label_sound_stereo, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_sound_stereo, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_sound_stereo, 9, 0);
	lv_obj_set_size(ui->label_sound_stereo, 95, 36);
	lv_label_set_style(ui->label_sound_stereo, LV_LABEL_STYLE_MAIN, &style0_label_sound_stereo);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_left_channel = lv_btn_create(ui->sound_track_page, NULL);
	lv_obj_set_pos(ui->button_left_channel, 6, 42);
	lv_obj_set_size(ui->button_left_channel, 112, 36);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_sound_left, &lv_style_transp);
	style0_label_sound_left.text.color = lv_color_hex(0x55ff7f);
	style0_label_sound_left.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_sound_left.text.line_space = 2;

	ui->label_sound_left = lv_label_create(ui->button_left_channel, NULL);
	lv_label_set_text(ui->label_sound_left, "左声道");
	lv_label_set_align(ui->label_sound_left, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_sound_left, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_sound_left, 9, 0);
	lv_obj_set_size(ui->label_sound_left, 95, 36);
	lv_label_set_style(ui->label_sound_left, LV_LABEL_STYLE_MAIN, &style0_label_sound_left);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_right_channel = lv_btn_create(ui->sound_track_page, NULL);
	lv_obj_set_pos(ui->button_right_channel, 7, 79);
	lv_obj_set_size(ui->button_right_channel, 111, 36);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_sound_right, &lv_style_transp);
	style0_label_sound_right.text.color = lv_color_hex(0x55ff7f);
	style0_label_sound_right.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_sound_right.text.line_space = 2;

	ui->label_sound_right = lv_label_create(ui->button_right_channel, NULL);
	lv_label_set_text(ui->label_sound_right, "右声道");
	lv_label_set_align(ui->label_sound_right, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_sound_right, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_sound_right, 8, 0);
	lv_obj_set_size(ui->label_sound_right, 95, 36);
	lv_label_set_style(ui->label_sound_right, LV_LABEL_STYLE_MAIN, &style0_label_sound_right);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_file_size = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_file_size, "size ");
	lv_label_set_long_mode(ui->label_file_size, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_file_size, 107, 244);
	lv_obj_set_size(ui->label_file_size, 176, 32);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_file_time = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->label_file_time, "time 1900-1-1");
	lv_label_set_long_mode(ui->label_file_time, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_file_time, 107, 282);
	lv_obj_set_size(ui->label_file_time, 176, 32);
#endif // LV_USE_LABEL

#ifdef LV_USE_LIST
	lv_style_copy(&style3_audio_track_list, &lv_style_transp);
	style3_audio_track_list.body.main_color = lv_color_hex(0x5596d8);
	style3_audio_track_list.body.grad_color = lv_color_hex(0x5596d8);
	style3_audio_track_list.body.radius = 0;
	style3_audio_track_list.body.opa = 255;
	style3_audio_track_list.body.border.color = lv_color_hex(0x000000);
	style3_audio_track_list.body.border.width = 0;
	style3_audio_track_list.body.border.part = 15;
	style3_audio_track_list.body.border.opa = 255;
	style3_audio_track_list.body.shadow.color = lv_color_hex(0x808080);
	style3_audio_track_list.body.shadow.width = 0;
	style3_audio_track_list.body.shadow.type = 1;
	style3_audio_track_list.body.padding.top = 5;
	style3_audio_track_list.body.padding.bottom = 5;
	style3_audio_track_list.body.padding.left = 5;
	style3_audio_track_list.body.padding.right = 5;
	style3_audio_track_list.body.padding.inner = 5;
	style3_audio_track_list.text.color = lv_color_hex(0xf0f0f0);
	style3_audio_track_list.text.sel_color = lv_color_hex(0x5596d8);
	style3_audio_track_list.text.font = &lv_font_roboto_16;
	style3_audio_track_list.text.letter_space = 0;
	style3_audio_track_list.text.line_space = 2;
	style3_audio_track_list.text.opa = 255;
	style3_audio_track_list.image.color = lv_color_hex(0xf0f0f0);
	style3_audio_track_list.image.intense = 0;
	style3_audio_track_list.image.opa = 255;

	ui->audio_track_list = lv_list_create(ui->cont, NULL);
	lv_obj_set_pos(ui->audio_track_list, 380, 194);
	lv_obj_set_size(ui->audio_track_list, 152, 143);
	lv_list_add_btn(ui->audio_track_list, NULL, "track-1");
	lv_list_add_btn(ui->audio_track_list, NULL, "track-2");
	lv_list_add_btn(ui->audio_track_list, NULL, "track-3");
	lv_list_set_single_mode(ui->audio_track_list, false);
	lv_list_set_scroll_propagation(ui->audio_track_list, false);
	lv_list_set_edge_flash(ui->audio_track_list, false);
	lv_obj_set_hidden(ui->audio_track_list,true);
	lv_list_set_style(ui->audio_track_list, LV_LIST_STYLE_EDGE_FLASH, &style3_audio_track_list);
#endif // LV_USE_LIST

}

void movie_auto_ui_destory(movie_ui_t *ui)
{
	lv_obj_clean(ui->cont);
	free_image(image_4_movie_big_png);
	free_image(play_music_play_png_state0);
	free_image(play_music_play_png_state1);
	free_image(play_music_pause_png_state2);
	free_image(play_music_pause_png_state3);
	free_image(next_music_next_png_state0);
	free_image(next_music_next_png_state1);
	free_image(last_music_prev_png_state0);
	free_image(last_music_prev_png_state1);
	free_image(play_mode_all_cycle_png_state0);
	free_image(play_mode_all_cycle_png_state1);
	free_image(play_mode_all_cycle_png_state2);
	free_image(play_mode_all_cycle_png_state3);
	free_image(volume_music_vol_png_state0);
	free_image(volume_music_vol_png_state1);
	free_image(volume_music_vol_png_state2);
	free_image(volume_music_vol_png_state3);
	free_image(volume_music_vol_png_state4);
	free_image(order_explorer_prev_png_state0);
	free_image(order_music_order_png_state1);
	free_image(media_list_1_movie_item_png);
	free_image(media_list_2_movie_item_png);
	free_image(media_list_3_movie_item_png);
	free_image(media_list_4_movie_item_png);
	free_image(media_list_5_movie_item_png);
	free_image(media_list_6_movie_item_png);
}
