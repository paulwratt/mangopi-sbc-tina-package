/**********************
 *      includes
 **********************/
#include "ui_music.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/
static lv_style_t style_screen;
static lv_style_t style0_media_list;
static lv_style_t style1_media_list;
static lv_style_t style3_media_list;
static lv_style_t style4_media_list;
static lv_style_t style5_media_list;
static lv_style_t style6_media_list;
static lv_style_t style7_media_list;
static lv_style_t style0_list_1;
static lv_style_t style1_list_1;
static lv_style_t style3_list_1;
static lv_style_t style4_list_1;
static lv_style_t style5_list_1;
static lv_style_t style6_list_1;
static lv_style_t style7_list_1;
static lv_style_t style0_list_2;
static lv_style_t style1_list_2;
static lv_style_t style3_list_2;
static lv_style_t style4_list_2;
static lv_style_t style5_list_2;
static lv_style_t style6_list_2;
static lv_style_t style7_list_2;
static lv_style_t style0_list_3;
static lv_style_t style1_list_3;
static lv_style_t style3_list_3;
static lv_style_t style4_list_3;
static lv_style_t style5_list_3;
static lv_style_t style6_list_3;
static lv_style_t style7_list_3;
static lv_style_t style0_container_1;
static lv_style_t style0_btn_play;
static lv_style_t style1_progressbar;
static lv_style_t style2_progressbar;
static lv_style_t style1_volume_bar;
static lv_style_t style2_volume_bar;
static lv_style_t style0_curr_time;
static lv_style_t style0_total_time;
static lv_style_t style0_file_name;
static lv_style_t style0_lrc;
static lv_style_t style0_lrc_no;
static lv_style_t style0_online;
static lv_style_t style0_lrc_list;
static lv_style_t style1_lrc_list;
static lv_style_t style2_lrc_list;
static lv_style_t style3_lrc_list;
static lv_style_t style4_lrc_list;
static lv_style_t style5_lrc_list;
static lv_style_t style6_lrc_list;
static lv_style_t style7_lrc_list;
static lv_style_t style0_spectrum;
static lv_style_t style1_spectrum;
static lv_style_t style2_spectrum;
static lv_style_t style0_download;
static lv_style_t style0_page_menu;
static lv_style_t style0_label_sound_effect;
static lv_style_t style0_label_song_information;
static lv_style_t style0_button_ab;
static lv_style_t style0_label_ab;
static lv_style_t style0_button_page_menu_exit;
static lv_style_t style0_label_menu_esc;
static lv_style_t style0_list_sound_effect;
static lv_style_t style1_list_sound_effect;
static lv_style_t style3_list_sound_effect;
static lv_style_t style4_list_sound_effect;
static lv_style_t style5_list_sound_effect;
static lv_style_t style6_list_sound_effect;
static lv_style_t style7_list_sound_effect;
static lv_style_t style0_list_song_information;
static lv_style_t style1_list_song_information;
static lv_style_t style3_list_song_information;
static lv_style_t style4_list_song_information;
static lv_style_t style5_list_song_information;
static lv_style_t style6_list_song_information;
static lv_style_t style7_list_song_information;
static lv_style_t style0_button_file_source;
static lv_style_t style0_label_file_source;

/**********************
 *  images and fonts
 **********************/
static void *image_1_music_album_png = NULL;
static void *image_2_music_album_out_png = NULL;
static void *btn_play_music_play_png_state0 = NULL;
static void *btn_play_music_play_png_state1 = NULL;
static void *btn_play_music_pause_png_state2 = NULL;
static void *btn_play_music_pause_png_state3 = NULL;
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
static void *media_list_1_music_item_png = NULL;
static void *media_list_2_music_item_png = NULL;
static void *media_list_3_music_item_png = NULL;
static void *media_list_4_music_item_png = NULL;
static void *media_list_5_music_item_png = NULL;
static void *media_list_6_music_item_png = NULL;
static void *list_1_1_music_item_png = NULL;
static void *list_1_2_music_item_png = NULL;
static void *list_1_3_music_item_png = NULL;
static void *list_1_4_music_item_png = NULL;
static void *list_1_5_music_item_png = NULL;
static void *list_1_6_music_item_png = NULL;
static void *list_2_1_music_item_png = NULL;
static void *list_2_2_music_item_png = NULL;
static void *list_2_3_music_item_png = NULL;
static void *list_2_4_music_item_png = NULL;
static void *list_2_5_music_item_png = NULL;
static void *list_2_6_music_item_png = NULL;
static void *list_3_1_music_item_png = NULL;
static void *list_3_2_music_item_png = NULL;
static void *list_3_3_music_item_png = NULL;
static void *list_3_4_music_item_png = NULL;
static void *list_3_5_music_item_png = NULL;
static void *list_3_6_music_item_png = NULL;

/**********************
 *  functions
 **********************/
void music_auto_ui_create(music_ui_t *ui)
{
	lv_style_copy(&style_screen, &lv_style_scr);
	style_screen.body.main_color = lv_color_hex(0x9d9d9d);
	style_screen.body.grad_color = lv_color_hex(0x9d9d9d);
	lv_obj_set_style(ui->cont, &style_screen);

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
	lv_obj_set_pos(ui->media_list, 494, 5);
	lv_obj_set_size(ui->media_list, 306, 417);

	media_list_1_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->media_list, media_list_1_music_item_png, "1");

	media_list_2_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->media_list, media_list_2_music_item_png, "2");

	media_list_3_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->media_list, media_list_3_music_item_png, "3");

	media_list_4_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->media_list, media_list_4_music_item_png, "4");

	media_list_5_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->media_list, media_list_5_music_item_png, "5");

	media_list_6_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->media_list, media_list_6_music_item_png, "6");
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

#ifdef LV_USE_LIST
	lv_style_copy(&style0_list_1, &lv_style_transp_fit);
	style0_list_1.body.border.color = lv_color_hex(0x5555ff);

	lv_style_copy(&style1_list_1, &lv_style_pretty);
	style1_list_1.body.main_color = lv_color_hex(0x9d9d9d);
	style1_list_1.body.grad_color = lv_color_hex(0x9d9d9d);
	style1_list_1.body.radius = 10;
	style1_list_1.body.opa = 0;
	style1_list_1.body.border.width = 0;
	style1_list_1.body.padding.bottom = 0;
	style1_list_1.body.padding.left = 0;
	style1_list_1.body.padding.right = 0;

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
	style3_list_1.text.color = lv_color_hex(0xf0f0f0);
	style3_list_1.text.sel_color = lv_color_hex(0x5596d8);
	style3_list_1.text.font = &lv_font_roboto_16;
	style3_list_1.text.letter_space = 0;
	style3_list_1.text.line_space = 2;
	style3_list_1.text.opa = 255;
	style3_list_1.image.color = lv_color_hex(0xf0f0f0);
	style3_list_1.image.intense = 0;
	style3_list_1.image.opa = 255;

	lv_style_copy(&style4_list_1, &lv_style_btn_rel);
	style4_list_1.body.main_color = lv_color_hex(0xebebeb);
	style4_list_1.body.grad_color = lv_color_hex(0xe5e5e5);
	style4_list_1.body.radius = 20;
	style4_list_1.body.border.color = lv_color_hex(0x5555ff);
	style4_list_1.body.border.width = 0;
	style4_list_1.body.border.part = 1;
	style4_list_1.text.color = lv_color_hex(0x000000);
	style4_list_1.text.line_space = 2;
	style4_list_1.image.color = lv_color_hex(0x00aa7f);
	style4_list_1.image.intense = 255;

	lv_style_copy(&style5_list_1, &lv_style_btn_pr);
	style5_list_1.body.main_color = lv_color_hex(0x55aaff);
	style5_list_1.body.grad_color = lv_color_hex(0x5555ff);
	style5_list_1.body.radius = 20;
	style5_list_1.body.border.width = 0;
	style5_list_1.text.line_space = 2;

	lv_style_copy(&style6_list_1, &lv_style_btn_tgl_rel);
	style6_list_1.body.main_color = lv_color_hex(0x55aaff);
	style6_list_1.body.grad_color = lv_color_hex(0x5555ff);
	style6_list_1.body.radius = 20;
	style6_list_1.body.border.color = lv_color_hex(0xff0000);
	style6_list_1.body.border.width = 0;
	style6_list_1.text.line_space = 2;
	style6_list_1.image.color = lv_color_hex(0xff0000);

	lv_style_copy(&style7_list_1, &lv_style_btn_tgl_pr);
	style7_list_1.body.main_color = lv_color_hex(0x55aaff);
	style7_list_1.body.grad_color = lv_color_hex(0x5555ff);
	style7_list_1.body.radius = 20;
	style7_list_1.body.border.color = lv_color_hex(0xff0000);
	style7_list_1.body.border.width = 0;
	style7_list_1.text.line_space = 2;

	ui->list_1 = lv_list_create(ui->media_list, NULL);
	lv_obj_set_pos(ui->list_1, 0, 419);
	lv_obj_set_size(ui->list_1, 306, 417);

	list_1_1_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_1, list_1_1_music_item_png, "1");

	list_1_2_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_1, list_1_2_music_item_png, "2");

	list_1_3_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_1, list_1_3_music_item_png, "3");

	list_1_4_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_1, list_1_4_music_item_png, "4");

	list_1_5_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_1, list_1_5_music_item_png, "5");

	list_1_6_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_1, list_1_6_music_item_png, "6");
	lv_list_set_single_mode(ui->list_1, true);
	lv_list_set_scroll_propagation(ui->list_1, false);
	lv_list_set_edge_flash(ui->list_1, true);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BG, &style0_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_SCRL, &style1_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_EDGE_FLASH, &style3_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BTN_REL, &style4_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BTN_PR, &style5_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BTN_TGL_REL, &style6_list_1);
	lv_list_set_style(ui->list_1, LV_LIST_STYLE_BTN_TGL_PR, &style7_list_1);
#endif // LV_USE_LIST

#ifdef LV_USE_LIST
	lv_style_copy(&style0_list_2, &lv_style_transp_fit);
	style0_list_2.body.border.color = lv_color_hex(0x5555ff);

	lv_style_copy(&style1_list_2, &lv_style_pretty);
	style1_list_2.body.main_color = lv_color_hex(0x9d9d9d);
	style1_list_2.body.grad_color = lv_color_hex(0x9d9d9d);
	style1_list_2.body.radius = 10;
	style1_list_2.body.opa = 0;
	style1_list_2.body.border.width = 0;
	style1_list_2.body.padding.bottom = 0;
	style1_list_2.body.padding.left = 0;
	style1_list_2.body.padding.right = 0;

	lv_style_copy(&style3_list_2, &lv_style_transp);
	style3_list_2.body.main_color = lv_color_hex(0x5596d8);
	style3_list_2.body.grad_color = lv_color_hex(0x5596d8);
	style3_list_2.body.radius = 0;
	style3_list_2.body.opa = 255;
	style3_list_2.body.border.color = lv_color_hex(0x000000);
	style3_list_2.body.border.width = 0;
	style3_list_2.body.border.part = 15;
	style3_list_2.body.border.opa = 255;
	style3_list_2.body.shadow.color = lv_color_hex(0x808080);
	style3_list_2.body.shadow.width = 0;
	style3_list_2.body.shadow.type = 1;
	style3_list_2.body.padding.top = 5;
	style3_list_2.body.padding.bottom = 5;
	style3_list_2.body.padding.left = 5;
	style3_list_2.body.padding.right = 5;
	style3_list_2.body.padding.inner = 5;
	style3_list_2.text.color = lv_color_hex(0xf0f0f0);
	style3_list_2.text.sel_color = lv_color_hex(0x5596d8);
	style3_list_2.text.font = &lv_font_roboto_16;
	style3_list_2.text.letter_space = 0;
	style3_list_2.text.line_space = 2;
	style3_list_2.text.opa = 255;
	style3_list_2.image.color = lv_color_hex(0xf0f0f0);
	style3_list_2.image.intense = 0;
	style3_list_2.image.opa = 255;

	lv_style_copy(&style4_list_2, &lv_style_btn_rel);
	style4_list_2.body.main_color = lv_color_hex(0xebebeb);
	style4_list_2.body.grad_color = lv_color_hex(0xe5e5e5);
	style4_list_2.body.radius = 20;
	style4_list_2.body.border.color = lv_color_hex(0x5555ff);
	style4_list_2.body.border.width = 0;
	style4_list_2.body.border.part = 1;
	style4_list_2.text.color = lv_color_hex(0x000000);
	style4_list_2.text.line_space = 2;
	style4_list_2.image.color = lv_color_hex(0x00aa7f);
	style4_list_2.image.intense = 255;

	lv_style_copy(&style5_list_2, &lv_style_btn_pr);
	style5_list_2.body.main_color = lv_color_hex(0x55aaff);
	style5_list_2.body.grad_color = lv_color_hex(0x5555ff);
	style5_list_2.body.radius = 20;
	style5_list_2.body.border.width = 0;
	style5_list_2.text.line_space = 2;

	lv_style_copy(&style6_list_2, &lv_style_btn_tgl_rel);
	style6_list_2.body.main_color = lv_color_hex(0x55aaff);
	style6_list_2.body.grad_color = lv_color_hex(0x5555ff);
	style6_list_2.body.radius = 20;
	style6_list_2.body.border.color = lv_color_hex(0xff0000);
	style6_list_2.body.border.width = 0;
	style6_list_2.text.line_space = 2;
	style6_list_2.image.color = lv_color_hex(0xff0000);

	lv_style_copy(&style7_list_2, &lv_style_btn_tgl_pr);
	style7_list_2.body.main_color = lv_color_hex(0x55aaff);
	style7_list_2.body.grad_color = lv_color_hex(0x5555ff);
	style7_list_2.body.radius = 20;
	style7_list_2.body.border.color = lv_color_hex(0xff0000);
	style7_list_2.body.border.width = 0;
	style7_list_2.text.line_space = 2;

	ui->list_2 = lv_list_create(ui->media_list, NULL);
	lv_obj_set_pos(ui->list_2, 0, 841);
	lv_obj_set_size(ui->list_2, 306, 417);

	list_2_1_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_2, list_2_1_music_item_png, "1");

	list_2_2_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_2, list_2_2_music_item_png, "2");

	list_2_3_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_2, list_2_3_music_item_png, "3");

	list_2_4_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_2, list_2_4_music_item_png, "4");

	list_2_5_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_2, list_2_5_music_item_png, "5");

	list_2_6_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_2, list_2_6_music_item_png, "6");
	lv_list_set_single_mode(ui->list_2, true);
	lv_list_set_scroll_propagation(ui->list_2, false);
	lv_list_set_edge_flash(ui->list_2, true);
	lv_list_set_style(ui->list_2, LV_LIST_STYLE_BG, &style0_list_2);
	lv_list_set_style(ui->list_2, LV_LIST_STYLE_SCRL, &style1_list_2);
	lv_list_set_style(ui->list_2, LV_LIST_STYLE_EDGE_FLASH, &style3_list_2);
	lv_list_set_style(ui->list_2, LV_LIST_STYLE_BTN_REL, &style4_list_2);
	lv_list_set_style(ui->list_2, LV_LIST_STYLE_BTN_PR, &style5_list_2);
	lv_list_set_style(ui->list_2, LV_LIST_STYLE_BTN_TGL_REL, &style6_list_2);
	lv_list_set_style(ui->list_2, LV_LIST_STYLE_BTN_TGL_PR, &style7_list_2);
#endif // LV_USE_LIST

#ifdef LV_USE_LIST
	lv_style_copy(&style0_list_3, &lv_style_transp_fit);
	style0_list_3.body.border.color = lv_color_hex(0x5555ff);

	lv_style_copy(&style1_list_3, &lv_style_pretty);
	style1_list_3.body.main_color = lv_color_hex(0x9d9d9d);
	style1_list_3.body.grad_color = lv_color_hex(0x9d9d9d);
	style1_list_3.body.radius = 10;
	style1_list_3.body.opa = 0;
	style1_list_3.body.border.width = 0;
	style1_list_3.body.padding.bottom = 0;
	style1_list_3.body.padding.left = 0;
	style1_list_3.body.padding.right = 0;

	lv_style_copy(&style3_list_3, &lv_style_transp);
	style3_list_3.body.main_color = lv_color_hex(0x5596d8);
	style3_list_3.body.grad_color = lv_color_hex(0x5596d8);
	style3_list_3.body.radius = 0;
	style3_list_3.body.opa = 255;
	style3_list_3.body.border.color = lv_color_hex(0x000000);
	style3_list_3.body.border.width = 0;
	style3_list_3.body.border.part = 15;
	style3_list_3.body.border.opa = 255;
	style3_list_3.body.shadow.color = lv_color_hex(0x808080);
	style3_list_3.body.shadow.width = 0;
	style3_list_3.body.shadow.type = 1;
	style3_list_3.body.padding.top = 5;
	style3_list_3.body.padding.bottom = 5;
	style3_list_3.body.padding.left = 5;
	style3_list_3.body.padding.right = 5;
	style3_list_3.body.padding.inner = 5;
	style3_list_3.text.color = lv_color_hex(0xf0f0f0);
	style3_list_3.text.sel_color = lv_color_hex(0x5596d8);
	style3_list_3.text.font = &lv_font_roboto_16;
	style3_list_3.text.letter_space = 0;
	style3_list_3.text.line_space = 2;
	style3_list_3.text.opa = 255;
	style3_list_3.image.color = lv_color_hex(0xf0f0f0);
	style3_list_3.image.intense = 0;
	style3_list_3.image.opa = 255;

	lv_style_copy(&style4_list_3, &lv_style_btn_rel);
	style4_list_3.body.main_color = lv_color_hex(0xebebeb);
	style4_list_3.body.grad_color = lv_color_hex(0xe5e5e5);
	style4_list_3.body.radius = 20;
	style4_list_3.body.border.color = lv_color_hex(0x5555ff);
	style4_list_3.body.border.width = 0;
	style4_list_3.body.border.part = 1;
	style4_list_3.text.color = lv_color_hex(0x000000);
	style4_list_3.text.line_space = 2;
	style4_list_3.image.color = lv_color_hex(0x00aa7f);
	style4_list_3.image.intense = 255;

	lv_style_copy(&style5_list_3, &lv_style_btn_pr);
	style5_list_3.body.main_color = lv_color_hex(0x55aaff);
	style5_list_3.body.grad_color = lv_color_hex(0x5555ff);
	style5_list_3.body.radius = 20;
	style5_list_3.body.border.width = 0;
	style5_list_3.text.line_space = 2;

	lv_style_copy(&style6_list_3, &lv_style_btn_tgl_rel);
	style6_list_3.body.main_color = lv_color_hex(0x55aaff);
	style6_list_3.body.grad_color = lv_color_hex(0x5555ff);
	style6_list_3.body.radius = 20;
	style6_list_3.body.border.color = lv_color_hex(0xff0000);
	style6_list_3.body.border.width = 0;
	style6_list_3.text.line_space = 2;
	style6_list_3.image.color = lv_color_hex(0xff0000);

	lv_style_copy(&style7_list_3, &lv_style_btn_tgl_pr);
	style7_list_3.body.main_color = lv_color_hex(0x55aaff);
	style7_list_3.body.grad_color = lv_color_hex(0x5555ff);
	style7_list_3.body.radius = 20;
	style7_list_3.body.border.color = lv_color_hex(0xff0000);
	style7_list_3.body.border.width = 0;
	style7_list_3.text.line_space = 2;

	ui->list_3 = lv_list_create(ui->list_2, NULL);
	lv_obj_set_pos(ui->list_3, 0, 419);
	lv_obj_set_size(ui->list_3, 306, 417);

	list_3_1_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_3, list_3_1_music_item_png, "1");

	list_3_2_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_3, list_3_2_music_item_png, "2");

	list_3_3_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_3, list_3_3_music_item_png, "3");

	list_3_4_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_3, list_3_4_music_item_png, "4");

	list_3_5_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_3, list_3_5_music_item_png, "5");

	list_3_6_music_item_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_item.png");
	lv_list_add_btn(ui->list_3, list_3_6_music_item_png, "6");
	lv_list_set_single_mode(ui->list_3, true);
	lv_list_set_scroll_propagation(ui->list_3, false);
	lv_list_set_edge_flash(ui->list_3, true);
	lv_list_set_style(ui->list_3, LV_LIST_STYLE_BG, &style0_list_3);
	lv_list_set_style(ui->list_3, LV_LIST_STYLE_SCRL, &style1_list_3);
	lv_list_set_style(ui->list_3, LV_LIST_STYLE_EDGE_FLASH, &style3_list_3);
	lv_list_set_style(ui->list_3, LV_LIST_STYLE_BTN_REL, &style4_list_3);
	lv_list_set_style(ui->list_3, LV_LIST_STYLE_BTN_PR, &style5_list_3);
	lv_list_set_style(ui->list_3, LV_LIST_STYLE_BTN_TGL_REL, &style6_list_3);
	lv_list_set_style(ui->list_3, LV_LIST_STYLE_BTN_TGL_PR, &style7_list_3);
#endif // LV_USE_LIST

#ifdef LV_USE_IMG
	ui->image_1 = lv_img_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_1, 142, 129);
	lv_obj_set_size(ui->image_1, 170, 170);
	image_1_music_album_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_album.png");
	lv_img_set_src(ui->image_1, image_1_music_album_png);

#endif // LV_USE_IMG

#ifdef LV_USE_IMG
	ui->image_2 = lv_img_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_2, 102, 91);
	lv_obj_set_size(ui->image_2, 250, 250);
	image_2_music_album_out_png = (void *)parse_image_from_file(LV_IMAGE_PATH"music_album_out.png");
	lv_img_set_src(ui->image_2, image_2_music_album_out_png);

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
	lv_style_copy(&style0_btn_play, &lv_style_btn_rel);

	ui->btn_play = lv_imgbtn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->btn_play, 71, 5);
	lv_obj_set_size(ui->btn_play, 48, 48);
	lv_imgbtn_set_state(ui->btn_play, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->btn_play, true);
	btn_play_music_play_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_play.png");
	lv_imgbtn_set_src(ui->btn_play, LV_BTN_STATE_REL, btn_play_music_play_png_state0);
	btn_play_music_play_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_play.png");
	lv_imgbtn_set_src(ui->btn_play, LV_BTN_STATE_PR, btn_play_music_play_png_state1);
	btn_play_music_pause_png_state2 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_pause.png");
	lv_imgbtn_set_src(ui->btn_play, LV_BTN_STATE_TGL_REL, btn_play_music_pause_png_state2);
	btn_play_music_pause_png_state3 = (void*)parse_image_from_file(LV_IMAGE_PATH"music_pause.png");
	lv_imgbtn_set_src(ui->btn_play, LV_BTN_STATE_TGL_PR, btn_play_music_pause_png_state3);
	lv_imgbtn_set_style(ui->btn_play, LV_IMGBTN_STYLE_REL, &style0_btn_play);
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
	lv_obj_set_pos(ui->progressbar, 210, 14);
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
	lv_obj_set_pos(ui->total_time, 453, 32);
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
	lv_obj_set_pos(ui->play_mode, 582, 11);
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
	lv_obj_set_pos(ui->volume, 618, 10);
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
	lv_obj_set_pos(ui->button_menu, 520, 13);
	lv_obj_set_size(ui->button_menu, 59, 28);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->menu_label = lv_label_create(ui->button_menu, NULL);
	lv_label_set_text(ui->menu_label, "menu");
	lv_label_set_long_mode(ui->menu_label, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->menu_label, 8, 3);
	lv_obj_set_size(ui->menu_label, 43, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_file_name, &lv_style_transp);
	style0_file_name.text.color = lv_color_hex(0xffffff);
	style0_file_name.text.line_space = 2;

	ui->file_name = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->file_name, "");
	lv_label_set_align(ui->file_name, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->file_name, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->file_name, 102, 10);
	lv_obj_set_size(ui->file_name, 234, 29);
	lv_label_set_style(ui->file_name, LV_LABEL_STYLE_MAIN, &style0_file_name);
#endif // LV_USE_LABEL

#ifdef LV_USE_TA
	lv_style_copy(&style0_lrc, &lv_style_pretty);
	style0_lrc.body.opa = 0;
	style0_lrc.body.border.opa = 0;
	style0_lrc.text.color = lv_color_hex(0xffff00);
	style0_lrc.text.sel_color = lv_color_hex(0x000000);
	style0_lrc.text.line_space = 2;

	ui->lrc = lv_ta_create(ui->cont, NULL);
	lv_obj_set_pos(ui->lrc, 4, 377);
	lv_obj_set_size(ui->lrc, 33, 40);
	lv_ta_set_text(ui->lrc, "");
	lv_ta_set_placeholder_text(ui->lrc, "");
	lv_ta_set_cursor_type(ui->lrc, LV_CURSOR_NONE);
	lv_ta_set_cursor_blink_time(ui->lrc, 144);
	lv_ta_set_one_line(ui->lrc, false);
	lv_ta_set_pwd_mode(ui->lrc, false);
	lv_ta_set_max_length(ui->lrc, 0);
	lv_ta_set_text_align(ui->lrc, 1);
	lv_ta_set_sb_mode(ui->lrc, 0);
	lv_ta_set_scroll_propagation(ui->lrc, false);
	lv_ta_set_edge_flash(ui->lrc, false);
	lv_ta_set_style(ui->lrc, LV_TA_STYLE_BG, &style0_lrc);
#endif // LV_USE_TA

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_lrc_no, &lv_style_transp);
	style0_lrc_no.body.border.color = lv_color_hex(0xffff00);
	style0_lrc_no.body.shadow.color = lv_color_hex(0xffff7f);
	style0_lrc_no.text.color = lv_color_hex(0xffff00);
	style0_lrc_no.text.font = &microsoft_yahei_en_cn_24_4;
	style0_lrc_no.text.line_space = 2;

	ui->lrc_no = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->lrc_no, "没有歌词");
	lv_label_set_align(ui->lrc_no, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->lrc_no, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->lrc_no, 113, 365);
	lv_obj_set_size(ui->lrc_no, 206, 44);
	lv_obj_set_hidden(ui->lrc_no,true);
	lv_label_set_style(ui->lrc_no, LV_LABEL_STYLE_MAIN, &style0_lrc_no);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_online, &lv_style_btn_rel);
	style0_online.body.main_color = lv_color_hex(0xffffff);
	style0_online.body.grad_color = lv_color_hex(0xffffff);
	style0_online.body.opa = 128;

	ui->online = lv_btn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->online, 439, 3);
	lv_obj_set_size(ui->online, 48, 30);
	lv_btn_set_style(ui->online, LV_BTN_STYLE_REL, &style0_online);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_online = lv_label_create(ui->online, NULL);
	lv_label_set_text(ui->label_online, "online");
	lv_label_set_align(ui->label_online, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_online, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_online, 3, 5);
	lv_obj_set_size(ui->label_online, 42, 20);
#endif // LV_USE_LABEL

#ifdef LV_USE_LIST
	lv_style_copy(&style0_lrc_list, &lv_style_transp_fit);
	style0_lrc_list.body.border.color = lv_color_hex(0xffffff);
	style0_lrc_list.body.border.opa = 0;
	style0_lrc_list.body.shadow.color = lv_color_hex(0x178038);

	lv_style_copy(&style1_lrc_list, &lv_style_pretty);
	style1_lrc_list.body.opa = 0;
	style1_lrc_list.body.border.opa = 0;

	lv_style_copy(&style2_lrc_list, &lv_style_pretty_color);
	style2_lrc_list.body.opa = 0;
	style2_lrc_list.body.border.opa = 0;

	lv_style_copy(&style3_lrc_list, &lv_style_transp);
	style3_lrc_list.body.main_color = lv_color_hex(0x5596d8);
	style3_lrc_list.body.grad_color = lv_color_hex(0x5596d8);
	style3_lrc_list.body.radius = 0;
	style3_lrc_list.body.opa = 0;
	style3_lrc_list.body.border.color = lv_color_hex(0x000000);
	style3_lrc_list.body.border.width = 0;
	style3_lrc_list.body.border.part = 15;
	style3_lrc_list.body.border.opa = 0;
	style3_lrc_list.body.shadow.color = lv_color_hex(0x808080);
	style3_lrc_list.body.shadow.width = 0;
	style3_lrc_list.body.shadow.type = 1;
	style3_lrc_list.body.padding.top = 5;
	style3_lrc_list.body.padding.bottom = 5;
	style3_lrc_list.body.padding.left = 5;
	style3_lrc_list.body.padding.right = 5;
	style3_lrc_list.body.padding.inner = 5;
	style3_lrc_list.text.color = lv_color_hex(0xf0f0f0);
	style3_lrc_list.text.sel_color = lv_color_hex(0x5596d8);
	style3_lrc_list.text.font = &lv_font_roboto_16;
	style3_lrc_list.text.letter_space = 0;
	style3_lrc_list.text.line_space = 2;
	style3_lrc_list.text.opa = 255;
	style3_lrc_list.image.color = lv_color_hex(0xf0f0f0);
	style3_lrc_list.image.intense = 0;
	style3_lrc_list.image.opa = 255;

	lv_style_copy(&style4_lrc_list, &lv_style_btn_rel);
	style4_lrc_list.body.opa = 0;
	style4_lrc_list.body.border.opa = 0;
	style4_lrc_list.text.line_space = 2;

	lv_style_copy(&style5_lrc_list, &lv_style_btn_pr);
	style5_lrc_list.body.opa = 0;
	style5_lrc_list.body.border.opa = 0;
	style5_lrc_list.text.line_space = 2;

	lv_style_copy(&style6_lrc_list, &lv_style_btn_tgl_rel);
	style6_lrc_list.body.opa = 0;
	style6_lrc_list.body.border.opa = 0;
	style6_lrc_list.text.line_space = 2;

	lv_style_copy(&style7_lrc_list, &lv_style_btn_tgl_pr);
	style7_lrc_list.body.opa = 0;
	style7_lrc_list.body.border.opa = 0;
	style7_lrc_list.text.line_space = 2;

	ui->lrc_list = lv_list_create(ui->cont, NULL);
	lv_obj_set_pos(ui->lrc_list, 21, 360);
	lv_obj_set_size(ui->lrc_list, 424, 56);
	lv_list_set_single_mode(ui->lrc_list, false);
	lv_list_set_scroll_propagation(ui->lrc_list, false);
	lv_list_set_edge_flash(ui->lrc_list, false);
	lv_list_set_style(ui->lrc_list, LV_LIST_STYLE_BG, &style0_lrc_list);
	lv_list_set_style(ui->lrc_list, LV_LIST_STYLE_SCRL, &style1_lrc_list);
	lv_list_set_style(ui->lrc_list, LV_LIST_STYLE_SB, &style2_lrc_list);
	lv_list_set_style(ui->lrc_list, LV_LIST_STYLE_EDGE_FLASH, &style3_lrc_list);
	lv_list_set_style(ui->lrc_list, LV_LIST_STYLE_BTN_REL, &style4_lrc_list);
	lv_list_set_style(ui->lrc_list, LV_LIST_STYLE_BTN_PR, &style5_lrc_list);
	lv_list_set_style(ui->lrc_list, LV_LIST_STYLE_BTN_TGL_REL, &style6_lrc_list);
	lv_list_set_style(ui->lrc_list, LV_LIST_STYLE_BTN_TGL_PR, &style7_lrc_list);
#endif // LV_USE_LIST

#ifdef LV_USE_PAGE
	lv_style_copy(&style0_spectrum, &lv_style_pretty_color);
	style0_spectrum.body.opa = 0;
	style0_spectrum.body.border.opa = 0;

	lv_style_copy(&style1_spectrum, &lv_style_pretty);
	style1_spectrum.body.opa = 64;
	style1_spectrum.body.border.opa = 0;

	lv_style_copy(&style2_spectrum, &lv_style_pretty_color);
	style2_spectrum.body.opa = 0;
	style2_spectrum.body.border.opa = 0;

	ui->spectrum = lv_page_create(ui->cont, NULL);
	lv_obj_set_pos(ui->spectrum, 0, 86);
	lv_obj_set_size(ui->spectrum, 497, 262);
	lv_page_set_sb_mode(ui->spectrum, LV_SB_MODE_AUTO);
	lv_page_set_edge_flash(ui->spectrum, false);
	lv_page_set_scroll_propagation(ui->spectrum, false);
	lv_page_set_style(ui->spectrum, LV_PAGE_STYLE_BG, &style0_spectrum);
	lv_page_set_style(ui->spectrum, LV_PAGE_STYLE_SCRL, &style1_spectrum);
	lv_page_set_style(ui->spectrum, LV_PAGE_STYLE_SB, &style2_spectrum);
#endif // LV_USE_PAGE

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_download, &lv_style_transp);
	style0_download.text.color = lv_color_hex(0xffff7f);
	style0_download.text.font = &lv_font_roboto_28;
	style0_download.text.line_space = 2;

	ui->download = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->download, "");
	lv_label_set_long_mode(ui->download, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->download, 126, 190);
	lv_obj_set_size(ui->download, 223, 38);
	lv_obj_set_hidden(ui->download,true);
	lv_label_set_style(ui->download, LV_LABEL_STYLE_MAIN, &style0_download);
#endif // LV_USE_LABEL

#ifdef LV_USE_PAGE
	ui->ab_page = lv_page_create(ui->cont, NULL);
	lv_obj_set_pos(ui->ab_page, 283, 229);
	lv_obj_set_size(ui->ab_page, 266, 191);
	lv_page_set_sb_mode(ui->ab_page, LV_SB_MODE_AUTO);
	lv_page_set_edge_flash(ui->ab_page, false);
	lv_page_set_scroll_propagation(ui->ab_page, false);
	lv_obj_set_hidden(ui->ab_page,true);
#endif // LV_USE_PAGE

#ifdef LV_USE_LABEL
	ui->label_1 = lv_label_create(ui->ab_page, NULL);
	lv_label_set_text(ui->label_1, "A");
	lv_label_set_long_mode(ui->label_1, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_1, 8, 32);
	lv_obj_set_size(ui->label_1, 22, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_SLIDER
	ui->slider_a = lv_slider_create(ui->ab_page, NULL);
	lv_obj_set_pos(ui->slider_a, 28, 31);
	lv_obj_set_size(ui->slider_a, 198, 25);
	lv_slider_set_range(ui->slider_a, 0, 1024);
	lv_slider_set_knob_in(ui->slider_a, false);
	lv_slider_set_value(ui->slider_a, 19, LV_ANIM_OFF);
#endif // LV_USE_SLIDER

#ifdef LV_USE_LABEL
	ui->label_slider_a = lv_label_create(ui->ab_page, NULL);
	lv_label_set_text(ui->label_slider_a, "00:00:00");
	lv_label_set_long_mode(ui->label_slider_a, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_slider_a, 175, 17);
	lv_obj_set_size(ui->label_slider_a, 62, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	ui->label_2 = lv_label_create(ui->ab_page, NULL);
	lv_label_set_text(ui->label_2, "B");
	lv_label_set_long_mode(ui->label_2, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_2, 5, 97);
	lv_obj_set_size(ui->label_2, 22, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_SLIDER
	ui->slider_b = lv_slider_create(ui->ab_page, NULL);
	lv_obj_set_pos(ui->slider_b, 28, 89);
	lv_obj_set_size(ui->slider_b, 198, 25);
	lv_slider_set_range(ui->slider_b, 0, 1024);
	lv_slider_set_knob_in(ui->slider_b, false);
	lv_slider_set_value(ui->slider_b, 68, LV_ANIM_OFF);
#endif // LV_USE_SLIDER

#ifdef LV_USE_LABEL
	ui->label_slider_b = lv_label_create(ui->ab_page, NULL);
	lv_label_set_text(ui->label_slider_b, "00:00:00");
	lv_label_set_long_mode(ui->label_slider_b, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_slider_b, 174, 72);
	lv_obj_set_size(ui->label_slider_b, 62, 19);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_ab_ok = lv_btn_create(ui->ab_page, NULL);
	lv_obj_set_pos(ui->button_ab_ok, 49, 137);
	lv_obj_set_size(ui->button_ab_ok, 64, 25);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_AB_OK = lv_label_create(ui->button_ab_ok, NULL);
	lv_label_set_text(ui->label_AB_OK, "OK");
	lv_label_set_align(ui->label_AB_OK, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_AB_OK, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_AB_OK, 14, 1);
	lv_obj_set_size(ui->label_AB_OK, 36, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_ab_esc = lv_btn_create(ui->ab_page, NULL);
	lv_obj_set_pos(ui->button_ab_esc, 140, 137);
	lv_obj_set_size(ui->button_ab_esc, 64, 25);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	ui->label_AB_ESC = lv_label_create(ui->button_ab_esc, NULL);
	lv_label_set_text(ui->label_AB_ESC, "Esc");
	lv_label_set_align(ui->label_AB_ESC, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_AB_ESC, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_AB_ESC, 14, 1);
	lv_obj_set_size(ui->label_AB_ESC, 36, 22);
#endif // LV_USE_LABEL

#ifdef LV_USE_PAGE
	lv_style_copy(&style0_page_menu, &lv_style_pretty_color);

	ui->page_menu = lv_page_create(ui->cont, NULL);
	lv_obj_set_pos(ui->page_menu, 554, 193);
	lv_obj_set_size(ui->page_menu, 164, 224);
	lv_page_set_sb_mode(ui->page_menu, LV_SB_MODE_AUTO);
	lv_page_set_edge_flash(ui->page_menu, false);
	lv_page_set_scroll_propagation(ui->page_menu, false);
	lv_obj_set_hidden(ui->page_menu,true);
	lv_page_set_style(ui->page_menu, LV_PAGE_STYLE_BG, &style0_page_menu);
#endif // LV_USE_PAGE

#ifdef LV_USE_BTN
	ui->button_sound_effect = lv_btn_create(ui->page_menu, NULL);
	lv_obj_set_pos(ui->button_sound_effect, 18, 7);
	lv_obj_set_size(ui->button_sound_effect, 118, 35);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_sound_effect, &lv_style_transp);
	style0_label_sound_effect.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_sound_effect.text.line_space = 2;

	ui->label_sound_effect = lv_label_create(ui->button_sound_effect, NULL);
	lv_label_set_text(ui->label_sound_effect, "音效");
	lv_label_set_align(ui->label_sound_effect, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_sound_effect, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_sound_effect, 33, 0);
	lv_obj_set_size(ui->label_sound_effect, 52, 34);
	lv_label_set_style(ui->label_sound_effect, LV_LABEL_STYLE_MAIN, &style0_label_sound_effect);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	ui->button_song_information = lv_btn_create(ui->page_menu, NULL);
	lv_obj_set_pos(ui->button_song_information, 18, 46);
	lv_obj_set_size(ui->button_song_information, 118, 35);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_song_information, &lv_style_transp);
	style0_label_song_information.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_song_information.text.line_space = 2;

	ui->label_song_information = lv_label_create(ui->button_song_information, NULL);
	lv_label_set_text(ui->label_song_information, "歌曲信息");
	lv_label_set_align(ui->label_song_information, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_song_information, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_song_information, 11, 0);
	lv_obj_set_size(ui->label_song_information, 97, 34);
	lv_label_set_style(ui->label_song_information, LV_LABEL_STYLE_MAIN, &style0_label_song_information);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_ab, &lv_style_btn_rel);

	ui->button_ab = lv_btn_create(ui->page_menu, NULL);
	lv_obj_set_pos(ui->button_ab, 18, 86);
	lv_obj_set_size(ui->button_ab, 118, 35);
	lv_btn_set_style(ui->button_ab, LV_BTN_STYLE_REL, &style0_button_ab);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_ab, &lv_style_transp);

	ui->label_AB = lv_label_create(ui->button_ab, NULL);
	lv_label_set_text(ui->label_AB, "A/B");
	lv_label_set_align(ui->label_AB, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_AB, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_AB, 29, 5);
	lv_obj_set_size(ui->label_AB, 61, 24);
	lv_label_set_style(ui->label_AB, LV_LABEL_STYLE_MAIN, &style0_label_ab);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_page_menu_exit, &lv_style_btn_rel);

	ui->button_page_menu_exit = lv_btn_create(ui->page_menu, NULL);
	lv_obj_set_pos(ui->button_page_menu_exit, 18, 167);
	lv_obj_set_size(ui->button_page_menu_exit, 118, 35);
	lv_btn_set_style(ui->button_page_menu_exit, LV_BTN_STYLE_REL, &style0_button_page_menu_exit);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_menu_esc, &lv_style_transp);
	style0_label_menu_esc.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_menu_esc.text.line_space = 2;

	ui->label_menu_esc = lv_label_create(ui->button_page_menu_exit, NULL);
	lv_label_set_text(ui->label_menu_esc, "退出");
	lv_label_set_align(ui->label_menu_esc, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(ui->label_menu_esc, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_menu_esc, 30, 0);
	lv_obj_set_size(ui->label_menu_esc, 58, 34);
	lv_label_set_style(ui->label_menu_esc, LV_LABEL_STYLE_MAIN, &style0_label_menu_esc);
#endif // LV_USE_LABEL

#ifdef LV_USE_LIST
	lv_style_copy(&style0_list_sound_effect, &lv_style_transp_fit);
	style0_list_sound_effect.body.border.color = lv_color_hex(0x5555ff);

	lv_style_copy(&style1_list_sound_effect, &lv_style_pretty);
	style1_list_sound_effect.body.main_color = lv_color_hex(0x9d9d9d);
	style1_list_sound_effect.body.grad_color = lv_color_hex(0x9d9d9d);
	style1_list_sound_effect.body.radius = 10;
	style1_list_sound_effect.body.opa = 0;
	style1_list_sound_effect.body.border.width = 0;
	style1_list_sound_effect.body.padding.bottom = 0;
	style1_list_sound_effect.body.padding.left = 0;
	style1_list_sound_effect.body.padding.right = 0;

	lv_style_copy(&style3_list_sound_effect, &lv_style_transp);
	style3_list_sound_effect.body.main_color = lv_color_hex(0x5596d8);
	style3_list_sound_effect.body.grad_color = lv_color_hex(0x5596d8);
	style3_list_sound_effect.body.radius = 0;
	style3_list_sound_effect.body.opa = 255;
	style3_list_sound_effect.body.border.color = lv_color_hex(0x000000);
	style3_list_sound_effect.body.border.width = 0;
	style3_list_sound_effect.body.border.part = 15;
	style3_list_sound_effect.body.border.opa = 255;
	style3_list_sound_effect.body.shadow.color = lv_color_hex(0x808080);
	style3_list_sound_effect.body.shadow.width = 0;
	style3_list_sound_effect.body.shadow.type = 1;
	style3_list_sound_effect.body.padding.top = 5;
	style3_list_sound_effect.body.padding.bottom = 5;
	style3_list_sound_effect.body.padding.left = 5;
	style3_list_sound_effect.body.padding.right = 5;
	style3_list_sound_effect.body.padding.inner = 5;
	style3_list_sound_effect.text.color = lv_color_hex(0xf0f0f0);
	style3_list_sound_effect.text.sel_color = lv_color_hex(0x5596d8);
	style3_list_sound_effect.text.font = &lv_font_roboto_16;
	style3_list_sound_effect.text.letter_space = 0;
	style3_list_sound_effect.text.line_space = 2;
	style3_list_sound_effect.text.opa = 255;
	style3_list_sound_effect.image.color = lv_color_hex(0xf0f0f0);
	style3_list_sound_effect.image.intense = 0;
	style3_list_sound_effect.image.opa = 255;

	lv_style_copy(&style4_list_sound_effect, &lv_style_btn_rel);
	style4_list_sound_effect.body.main_color = lv_color_hex(0xebebeb);
	style4_list_sound_effect.body.grad_color = lv_color_hex(0xe5e5e5);
	style4_list_sound_effect.body.radius = 20;
	style4_list_sound_effect.body.border.color = lv_color_hex(0x5555ff);
	style4_list_sound_effect.body.border.width = 0;
	style4_list_sound_effect.body.border.part = 1;
	style4_list_sound_effect.text.color = lv_color_hex(0x000000);
	style4_list_sound_effect.text.line_space = 2;
	style4_list_sound_effect.image.color = lv_color_hex(0x00aa7f);
	style4_list_sound_effect.image.intense = 255;

	lv_style_copy(&style5_list_sound_effect, &lv_style_btn_pr);
	style5_list_sound_effect.body.main_color = lv_color_hex(0x55aaff);
	style5_list_sound_effect.body.grad_color = lv_color_hex(0x5555ff);
	style5_list_sound_effect.body.radius = 20;
	style5_list_sound_effect.body.border.width = 0;
	style5_list_sound_effect.text.line_space = 2;

	lv_style_copy(&style6_list_sound_effect, &lv_style_btn_tgl_rel);
	style6_list_sound_effect.body.main_color = lv_color_hex(0x55aaff);
	style6_list_sound_effect.body.grad_color = lv_color_hex(0x5555ff);
	style6_list_sound_effect.body.radius = 20;
	style6_list_sound_effect.body.border.color = lv_color_hex(0xff0000);
	style6_list_sound_effect.body.border.width = 0;
	style6_list_sound_effect.text.line_space = 2;
	style6_list_sound_effect.image.color = lv_color_hex(0xff0000);

	lv_style_copy(&style7_list_sound_effect, &lv_style_btn_tgl_pr);
	style7_list_sound_effect.body.main_color = lv_color_hex(0x55aaff);
	style7_list_sound_effect.body.grad_color = lv_color_hex(0x5555ff);
	style7_list_sound_effect.body.radius = 20;
	style7_list_sound_effect.body.border.color = lv_color_hex(0xff0000);
	style7_list_sound_effect.body.border.width = 0;
	style7_list_sound_effect.text.line_space = 2;

	ui->list_sound_effect = lv_list_create(ui->cont, NULL);
	lv_obj_set_pos(ui->list_sound_effect, 349, 232);
	lv_obj_set_size(ui->list_sound_effect, 198, 171);
	lv_list_add_btn(ui->list_sound_effect, NULL, "1");
	lv_list_add_btn(ui->list_sound_effect, NULL, "2");
	lv_list_add_btn(ui->list_sound_effect, NULL, "3");
	lv_list_add_btn(ui->list_sound_effect, NULL, "4");
	lv_list_add_btn(ui->list_sound_effect, NULL, "5");
	lv_list_add_btn(ui->list_sound_effect, NULL, "6");
	lv_list_set_single_mode(ui->list_sound_effect, true);
	lv_list_set_scroll_propagation(ui->list_sound_effect, false);
	lv_list_set_edge_flash(ui->list_sound_effect, true);
	lv_obj_set_hidden(ui->list_sound_effect,true);
	lv_list_set_style(ui->list_sound_effect, LV_LIST_STYLE_BG, &style0_list_sound_effect);
	lv_list_set_style(ui->list_sound_effect, LV_LIST_STYLE_SCRL, &style1_list_sound_effect);
	lv_list_set_style(ui->list_sound_effect, LV_LIST_STYLE_EDGE_FLASH, &style3_list_sound_effect);
	lv_list_set_style(ui->list_sound_effect, LV_LIST_STYLE_BTN_REL, &style4_list_sound_effect);
	lv_list_set_style(ui->list_sound_effect, LV_LIST_STYLE_BTN_PR, &style5_list_sound_effect);
	lv_list_set_style(ui->list_sound_effect, LV_LIST_STYLE_BTN_TGL_REL, &style6_list_sound_effect);
	lv_list_set_style(ui->list_sound_effect, LV_LIST_STYLE_BTN_TGL_PR, &style7_list_sound_effect);
#endif // LV_USE_LIST

#ifdef LV_USE_LIST
	lv_style_copy(&style0_list_song_information, &lv_style_transp_fit);
	style0_list_song_information.body.border.color = lv_color_hex(0x5555ff);

	lv_style_copy(&style1_list_song_information, &lv_style_pretty);
	style1_list_song_information.body.main_color = lv_color_hex(0x9d9d9d);
	style1_list_song_information.body.grad_color = lv_color_hex(0x9d9d9d);
	style1_list_song_information.body.radius = 10;
	style1_list_song_information.body.opa = 0;
	style1_list_song_information.body.border.width = 0;
	style1_list_song_information.body.padding.bottom = 0;
	style1_list_song_information.body.padding.left = 0;
	style1_list_song_information.body.padding.right = 0;

	lv_style_copy(&style3_list_song_information, &lv_style_transp);
	style3_list_song_information.body.main_color = lv_color_hex(0x5596d8);
	style3_list_song_information.body.grad_color = lv_color_hex(0x5596d8);
	style3_list_song_information.body.radius = 0;
	style3_list_song_information.body.opa = 255;
	style3_list_song_information.body.border.color = lv_color_hex(0x000000);
	style3_list_song_information.body.border.width = 0;
	style3_list_song_information.body.border.part = 15;
	style3_list_song_information.body.border.opa = 255;
	style3_list_song_information.body.shadow.color = lv_color_hex(0x808080);
	style3_list_song_information.body.shadow.width = 0;
	style3_list_song_information.body.shadow.type = 1;
	style3_list_song_information.body.padding.top = 5;
	style3_list_song_information.body.padding.bottom = 5;
	style3_list_song_information.body.padding.left = 5;
	style3_list_song_information.body.padding.right = 5;
	style3_list_song_information.body.padding.inner = 5;
	style3_list_song_information.text.color = lv_color_hex(0xf0f0f0);
	style3_list_song_information.text.sel_color = lv_color_hex(0x5596d8);
	style3_list_song_information.text.font = &lv_font_roboto_16;
	style3_list_song_information.text.letter_space = 0;
	style3_list_song_information.text.line_space = 2;
	style3_list_song_information.text.opa = 255;
	style3_list_song_information.image.color = lv_color_hex(0xf0f0f0);
	style3_list_song_information.image.intense = 0;
	style3_list_song_information.image.opa = 255;

	lv_style_copy(&style4_list_song_information, &lv_style_btn_rel);
	style4_list_song_information.body.main_color = lv_color_hex(0xebebeb);
	style4_list_song_information.body.grad_color = lv_color_hex(0xe5e5e5);
	style4_list_song_information.body.radius = 20;
	style4_list_song_information.body.border.color = lv_color_hex(0x5555ff);
	style4_list_song_information.body.border.width = 0;
	style4_list_song_information.body.border.part = 1;
	style4_list_song_information.text.color = lv_color_hex(0x000000);
	style4_list_song_information.text.line_space = 2;
	style4_list_song_information.image.color = lv_color_hex(0x00aa7f);
	style4_list_song_information.image.intense = 255;

	lv_style_copy(&style5_list_song_information, &lv_style_btn_pr);
	style5_list_song_information.body.main_color = lv_color_hex(0x55aaff);
	style5_list_song_information.body.grad_color = lv_color_hex(0x5555ff);
	style5_list_song_information.body.radius = 20;
	style5_list_song_information.body.border.width = 0;
	style5_list_song_information.text.line_space = 2;

	lv_style_copy(&style6_list_song_information, &lv_style_btn_tgl_rel);
	style6_list_song_information.body.main_color = lv_color_hex(0x55aaff);
	style6_list_song_information.body.grad_color = lv_color_hex(0x5555ff);
	style6_list_song_information.body.radius = 20;
	style6_list_song_information.body.border.color = lv_color_hex(0xff0000);
	style6_list_song_information.body.border.width = 0;
	style6_list_song_information.text.line_space = 2;
	style6_list_song_information.image.color = lv_color_hex(0xff0000);

	lv_style_copy(&style7_list_song_information, &lv_style_btn_tgl_pr);
	style7_list_song_information.body.main_color = lv_color_hex(0x55aaff);
	style7_list_song_information.body.grad_color = lv_color_hex(0x5555ff);
	style7_list_song_information.body.radius = 20;
	style7_list_song_information.body.border.color = lv_color_hex(0xff0000);
	style7_list_song_information.body.border.width = 0;
	style7_list_song_information.text.line_space = 2;

	ui->list_song_information = lv_list_create(ui->cont, NULL);
	lv_obj_set_pos(ui->list_song_information, 352, 234);
	lv_obj_set_size(ui->list_song_information, 198, 171);
	lv_list_add_btn(ui->list_song_information, NULL, "1");
	lv_list_add_btn(ui->list_song_information, NULL, "2");
	lv_list_add_btn(ui->list_song_information, NULL, "3");
	lv_list_add_btn(ui->list_song_information, NULL, "4");
	lv_list_add_btn(ui->list_song_information, NULL, "5");
	lv_list_add_btn(ui->list_song_information, NULL, "6");
	lv_list_set_single_mode(ui->list_song_information, true);
	lv_list_set_scroll_propagation(ui->list_song_information, false);
	lv_list_set_edge_flash(ui->list_song_information, true);
	lv_obj_set_hidden(ui->list_song_information,true);
	lv_list_set_style(ui->list_song_information, LV_LIST_STYLE_BG, &style0_list_song_information);
	lv_list_set_style(ui->list_song_information, LV_LIST_STYLE_SCRL, &style1_list_song_information);
	lv_list_set_style(ui->list_song_information, LV_LIST_STYLE_EDGE_FLASH, &style3_list_song_information);
	lv_list_set_style(ui->list_song_information, LV_LIST_STYLE_BTN_REL, &style4_list_song_information);
	lv_list_set_style(ui->list_song_information, LV_LIST_STYLE_BTN_PR, &style5_list_song_information);
	lv_list_set_style(ui->list_song_information, LV_LIST_STYLE_BTN_TGL_REL, &style6_list_song_information);
	lv_list_set_style(ui->list_song_information, LV_LIST_STYLE_BTN_TGL_PR, &style7_list_song_information);
#endif // LV_USE_LIST

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_file_source, &lv_style_btn_rel);

	ui->button_file_source = lv_btn_create(ui->page_menu, NULL);
	lv_obj_set_pos(ui->button_file_source, 18, 127);
	lv_obj_set_size(ui->button_file_source, 118, 35);
	lv_btn_set_style(ui->button_file_source, LV_BTN_STYLE_REL, &style0_button_file_source);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_file_source, &lv_style_transp);

	ui->label_file_source = lv_label_create(ui->button_file_source, NULL);
	lv_label_set_text(ui->label_file_source, "udisk/sd");
	lv_label_set_long_mode(ui->label_file_source, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_file_source, 28, 8);
	lv_obj_set_size(ui->label_file_source, 62, 19);
	lv_label_set_style(ui->label_file_source, LV_LABEL_STYLE_MAIN, &style0_label_file_source);
#endif // LV_USE_LABEL

}

void music_auto_ui_destory(music_ui_t *ui)
{
	lv_obj_clean(ui->cont);
	free_image(image_1_music_album_png);
	free_image(image_2_music_album_out_png);
	free_image(btn_play_music_play_png_state0);
	free_image(btn_play_music_play_png_state1);
	free_image(btn_play_music_pause_png_state2);
	free_image(btn_play_music_pause_png_state3);
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
	free_image(media_list_1_music_item_png);
	free_image(media_list_2_music_item_png);
	free_image(media_list_3_music_item_png);
	free_image(media_list_4_music_item_png);
	free_image(media_list_5_music_item_png);
	free_image(media_list_6_music_item_png);
	free_image(list_1_1_music_item_png);
	free_image(list_1_2_music_item_png);
	free_image(list_1_3_music_item_png);
	free_image(list_1_4_music_item_png);
	free_image(list_1_5_music_item_png);
	free_image(list_1_6_music_item_png);
	free_image(list_2_1_music_item_png);
	free_image(list_2_2_music_item_png);
	free_image(list_2_3_music_item_png);
	free_image(list_2_4_music_item_png);
	free_image(list_2_5_music_item_png);
	free_image(list_2_6_music_item_png);
	free_image(list_3_1_music_item_png);
	free_image(list_3_2_music_item_png);
	free_image(list_3_3_music_item_png);
	free_image(list_3_4_music_item_png);
	free_image(list_3_5_music_item_png);
	free_image(list_3_6_music_item_png);
}
