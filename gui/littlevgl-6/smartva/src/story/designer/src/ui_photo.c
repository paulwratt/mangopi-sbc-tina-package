/**********************
 *      includes
 **********************/
#include "ui_photo.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/
static lv_style_t style1_media_list;
static lv_style_t style2_media_list;
static lv_style_t style4_media_list;
static lv_style_t style5_media_list;
static lv_style_t style6_media_list;
static lv_style_t style7_media_list;
static lv_style_t style1_back;
static lv_style_t style0_file_name;
static lv_style_t style0_image_button_preview;
static lv_style_t style1_image_button_preview;
static lv_style_t style0_button_setting_interface;
static lv_style_t style1_button_setting_interface;
static lv_style_t style0_container_control;
static lv_style_t style0_button_show_effect;
static lv_style_t style1_button_show_effect;
static lv_style_t style0_label_effect;
static lv_style_t style0_button_show_speed;
static lv_style_t style1_button_show_speed;
static lv_style_t style0_label_speed;
static lv_style_t style0_button_info;
static lv_style_t style1_button_info;
static lv_style_t style0_label_info;
static lv_style_t style0_container_sel_effect;
static lv_style_t style0_button_matrix_effect;
static lv_style_t style1_button_matrix_effect;
static lv_style_t style2_button_matrix_effect;
static lv_style_t style3_button_matrix_effect;
static lv_style_t style4_button_matrix_effect;
static lv_style_t style5_button_matrix_effect;
static lv_style_t style0_container_sel_speed;
static lv_style_t style0_button_matrix_speed;
static lv_style_t style1_button_matrix_speed;
static lv_style_t style2_button_matrix_speed;
static lv_style_t style3_button_matrix_speed;
static lv_style_t style4_button_matrix_speed;
static lv_style_t style0_image_set_speed;
static lv_style_t style0_container_dialog;
static lv_style_t style0_container_1;
static lv_style_t style0_button_1;
static lv_style_t style1_button_1;
static lv_style_t style4_button_1;
static lv_style_t style0_label_dialog_ok;
static lv_style_t style0_label_dialog_info;
static lv_style_t style0_container_info;
static lv_style_t style0_label_title_filename;
static lv_style_t style0_label_title_file_size;
static lv_style_t style0_label_title_w_h;
static lv_style_t style0_label_filename;
static lv_style_t style0_label_file_size;
static lv_style_t style0_label_w_h;
static lv_style_t style0_label_title_file_time;
static lv_style_t style0_label_file_time;

/**********************
 *  images and fonts
 **********************/
static void *image_set_effect_setting_choose_png = NULL;
static void *image_set_speed_setting_choose_png = NULL;
static void *image_get_info_setting_choose_png = NULL;
static void *back_home_return_png_state0 = NULL;
static void *back_home_return_png_state1 = NULL;
static void *image_button_preview_background31_jpg_state0 = NULL;
static void *image_button_preview_background31_jpg_state1 = NULL;
static void *image_button_play_start_play_start_jpeg_state0 = NULL;
static void *image_button_play_start_play_start_jpeg_state1 = NULL;
static void *image_button_rotate_left_rotate_left_png_state0 = NULL;
static void *image_button_rotate_left_rotate_left_png_state1 = NULL;
static void *image_button_rotate_right_rotate_right_png_state0 = NULL;
static void *image_button_rotate_right_rotate_right_png_state1 = NULL;
static void *image_button_scaler_down_scaler_down_jpg_state0 = NULL;
static void *image_button_scaler_down_scaler_down_jpg_state1 = NULL;
static void *image_button_scaler_down_scaler_down_jpg_state2 = NULL;
static void *image_button_scaler_up_scaler_up_jpg_state0 = NULL;
static void *image_button_scaler_up_scaler_up_jpg_state1 = NULL;
static void *media_list_1_movie_item_png = NULL;
static void *media_list_2_movie_item_png = NULL;
static void *media_list_3_movie_item_png = NULL;
static void *media_list_4_movie_item_png = NULL;
static void *media_list_5_movie_item_png = NULL;
static void *media_list_6_movie_item_png = NULL;

/**********************
 *  functions
 **********************/
void photo_auto_ui_create(photo_ui_t *ui)
{

#ifdef LV_USE_LIST
	lv_style_copy(&style1_media_list, &lv_style_pretty);
	style1_media_list.body.main_color = lv_color_hex(0xff557f);
	style1_media_list.body.grad_color = lv_color_hex(0xff557f);
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
	lv_obj_set_pos(ui->media_list, 361, 35);
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
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_SCRL, &style1_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_SB, &style2_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_REL, &style4_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_PR, &style5_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_TGL_REL, &style6_media_list);
	lv_list_set_style(ui->media_list, LV_LIST_STYLE_BTN_TGL_PR, &style7_media_list);
#endif // LV_USE_LIST

#ifdef LV_USE_IMGBTN
	lv_style_copy(&style1_back, &lv_style_btn_pr);
	style1_back.image.color = lv_color_hex(0xffffff);
	style1_back.image.opa = 128;

	ui->back = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->back, 11, 18);
	lv_obj_set_size(ui->back, 32, 32);
	lv_imgbtn_set_state(ui->back, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->back, false);
	back_home_return_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_return.png");
	lv_imgbtn_set_src(ui->back, LV_BTN_STATE_REL, back_home_return_png_state0);
	back_home_return_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"home_return.png");
	lv_imgbtn_set_src(ui->back, LV_BTN_STATE_PR, back_home_return_png_state1);
	lv_imgbtn_set_style(ui->back, LV_IMGBTN_STYLE_PR, &style1_back);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_file_name, &lv_style_transp);
	style0_file_name.text.color = lv_color_hex(0xffffff);
	style0_file_name.text.sel_color = lv_color_hex(0xffffff);
	style0_file_name.text.line_space = 2;

	ui->file_name = lv_label_create(ui->cont, NULL);
	lv_label_set_text(ui->file_name, "");
	lv_label_set_long_mode(ui->file_name, LV_LABEL_LONG_BREAK);
	lv_obj_set_pos(ui->file_name, 55, 242);
	lv_obj_set_size(ui->file_name, 300, 19);
	lv_label_set_style(ui->file_name, LV_LABEL_STYLE_MAIN, &style0_file_name);
#endif // LV_USE_LABEL

#ifdef LV_USE_IMGBTN
	lv_style_copy(&style0_image_button_preview, &lv_style_btn_rel);
	style0_image_button_preview.image.opa = 0;

	lv_style_copy(&style1_image_button_preview, &lv_style_btn_pr);
	style1_image_button_preview.image.opa = 0;

	ui->image_button_preview = lv_imgbtn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->image_button_preview, 53, 123);
	lv_obj_set_size(ui->image_button_preview, 150, 90);
	lv_imgbtn_set_state(ui->image_button_preview, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_preview, false);
	image_button_preview_background31_jpg_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"background31.jpg");
	lv_imgbtn_set_src(ui->image_button_preview, LV_BTN_STATE_REL, image_button_preview_background31_jpg_state0);
	image_button_preview_background31_jpg_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"background31.jpg");
	lv_imgbtn_set_src(ui->image_button_preview, LV_BTN_STATE_PR, image_button_preview_background31_jpg_state1);
	lv_imgbtn_set_style(ui->image_button_preview, LV_IMGBTN_STYLE_REL, &style0_image_button_preview);
	lv_imgbtn_set_style(ui->image_button_preview, LV_IMGBTN_STYLE_PR, &style1_image_button_preview);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_setting_interface, &lv_style_btn_rel);
	style0_button_setting_interface.body.main_color = lv_color_hex(0x00aaff);
	style0_button_setting_interface.body.grad_color = lv_color_hex(0x00aaff);
	style0_button_setting_interface.body.radius = 0;
	style0_button_setting_interface.body.opa = 0;
	style0_button_setting_interface.body.border.color = lv_color_hex(0x55aaff);
	style0_button_setting_interface.body.border.width = 0;
	style0_button_setting_interface.body.border.opa = 0;

	lv_style_copy(&style1_button_setting_interface, &lv_style_btn_pr);
	style1_button_setting_interface.body.main_color = lv_color_hex(0x00aaff);
	style1_button_setting_interface.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_setting_interface.body.radius = 0;
	style1_button_setting_interface.body.opa = 0;
	style1_button_setting_interface.body.border.color = lv_color_hex(0x0000ff);
	style1_button_setting_interface.body.border.width = 0;
	style1_button_setting_interface.body.border.opa = 0;

	ui->button_setting_interface = lv_btn_create(ui->cont, NULL);
	lv_obj_set_pos(ui->button_setting_interface, 0, 0);
	lv_obj_set_size(ui->button_setting_interface, 800, 480);
	lv_btn_set_style(ui->button_setting_interface, LV_BTN_STYLE_REL, &style0_button_setting_interface);
	lv_btn_set_style(ui->button_setting_interface, LV_BTN_STYLE_PR, &style1_button_setting_interface);
#endif // LV_USE_BTN

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_control, &lv_style_pretty);
	style0_container_control.body.opa = 0;
	style0_container_control.body.border.width = 0;
	style0_container_control.body.border.opa = 0;

	ui->container_control = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->container_control, 0, 262);
	lv_obj_set_size(ui->container_control, 364, 218);
	lv_cont_set_fit4(ui->container_control, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_hidden(ui->container_control,true);
	lv_cont_set_style(ui->container_control, LV_CONT_STYLE_MAIN, &style0_container_control);
#endif // LV_USE_CONT

#ifdef LV_USE_IMGBTN
	ui->image_button_play_start = lv_imgbtn_create(ui->container_control, NULL);
	lv_obj_set_pos(ui->image_button_play_start, 27, 63);
	lv_obj_set_size(ui->image_button_play_start, 48, 48);
	lv_imgbtn_set_state(ui->image_button_play_start, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_play_start, false);
	image_button_play_start_play_start_jpeg_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"play_start.jpeg");
	lv_imgbtn_set_src(ui->image_button_play_start, LV_BTN_STATE_REL, image_button_play_start_play_start_jpeg_state0);
	image_button_play_start_play_start_jpeg_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"play_start.jpeg");
	lv_imgbtn_set_src(ui->image_button_play_start, LV_BTN_STATE_PR, image_button_play_start_play_start_jpeg_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_rotate_left = lv_imgbtn_create(ui->container_control, NULL);
	lv_obj_set_pos(ui->image_button_rotate_left, 33, 140);
	lv_obj_set_size(ui->image_button_rotate_left, 48, 48);
	lv_imgbtn_set_state(ui->image_button_rotate_left, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_rotate_left, false);
	image_button_rotate_left_rotate_left_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"rotate_left.png");
	lv_imgbtn_set_src(ui->image_button_rotate_left, LV_BTN_STATE_REL, image_button_rotate_left_rotate_left_png_state0);
	image_button_rotate_left_rotate_left_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"rotate_left.png");
	lv_imgbtn_set_src(ui->image_button_rotate_left, LV_BTN_STATE_PR, image_button_rotate_left_rotate_left_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_rotate_right = lv_imgbtn_create(ui->container_control, NULL);
	lv_obj_set_pos(ui->image_button_rotate_right, 133, 141);
	lv_obj_set_size(ui->image_button_rotate_right, 48, 48);
	lv_imgbtn_set_state(ui->image_button_rotate_right, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_rotate_right, false);
	image_button_rotate_right_rotate_right_png_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"rotate_right.png");
	lv_imgbtn_set_src(ui->image_button_rotate_right, LV_BTN_STATE_REL, image_button_rotate_right_rotate_right_png_state0);
	image_button_rotate_right_rotate_right_png_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"rotate_right.png");
	lv_imgbtn_set_src(ui->image_button_rotate_right, LV_BTN_STATE_PR, image_button_rotate_right_rotate_right_png_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_scaler_down = lv_imgbtn_create(ui->container_control, NULL);
	lv_obj_set_pos(ui->image_button_scaler_down, 229, 140);
	lv_obj_set_size(ui->image_button_scaler_down, 48, 48);
	lv_imgbtn_set_state(ui->image_button_scaler_down, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_scaler_down, false);
	image_button_scaler_down_scaler_down_jpg_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"scaler_down.jpg");
	lv_imgbtn_set_src(ui->image_button_scaler_down, LV_BTN_STATE_REL, image_button_scaler_down_scaler_down_jpg_state0);
	image_button_scaler_down_scaler_down_jpg_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"scaler_down.jpg");
	lv_imgbtn_set_src(ui->image_button_scaler_down, LV_BTN_STATE_PR, image_button_scaler_down_scaler_down_jpg_state1);
	image_button_scaler_down_scaler_down_jpg_state2 = (void*)parse_image_from_file(LV_IMAGE_PATH"scaler_down.jpg");
	lv_imgbtn_set_src(ui->image_button_scaler_down, LV_BTN_STATE_TGL_REL, image_button_scaler_down_scaler_down_jpg_state2);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_IMGBTN
	ui->image_button_scaler_up = lv_imgbtn_create(ui->container_control, NULL);
	lv_obj_set_pos(ui->image_button_scaler_up, 311, 140);
	lv_obj_set_size(ui->image_button_scaler_up, 48, 48);
	lv_imgbtn_set_state(ui->image_button_scaler_up, LV_BTN_STATE_REL);
	lv_imgbtn_set_toggle(ui->image_button_scaler_up, false);
	image_button_scaler_up_scaler_up_jpg_state0 = (void*)parse_image_from_file(LV_IMAGE_PATH"scaler_up.jpg");
	lv_imgbtn_set_src(ui->image_button_scaler_up, LV_BTN_STATE_REL, image_button_scaler_up_scaler_up_jpg_state0);
	image_button_scaler_up_scaler_up_jpg_state1 = (void*)parse_image_from_file(LV_IMAGE_PATH"scaler_up.jpg");
	lv_imgbtn_set_src(ui->image_button_scaler_up, LV_BTN_STATE_PR, image_button_scaler_up_scaler_up_jpg_state1);
#endif // LV_USE_IMGBTN

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_show_effect, &lv_style_btn_rel);
	style0_button_show_effect.body.main_color = lv_color_hex(0x00aaff);
	style0_button_show_effect.body.grad_color = lv_color_hex(0x00aaff);
	style0_button_show_effect.body.border.color = lv_color_hex(0x55aaff);
	style0_button_show_effect.body.border.width = 1;

	lv_style_copy(&style1_button_show_effect, &lv_style_btn_pr);
	style1_button_show_effect.body.main_color = lv_color_hex(0x00aaff);
	style1_button_show_effect.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_show_effect.body.border.color = lv_color_hex(0x0000ff);

	ui->button_show_effect = lv_btn_create(ui->container_control, NULL);
	lv_obj_set_pos(ui->button_show_effect, 157, 66);
	lv_obj_set_size(ui->button_show_effect, 135, 42);
	lv_btn_set_style(ui->button_show_effect, LV_BTN_STYLE_REL, &style0_button_show_effect);
	lv_btn_set_style(ui->button_show_effect, LV_BTN_STYLE_PR, &style1_button_show_effect);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_effect, &lv_style_transp);
	style0_label_effect.text.color = lv_color_hex(0xffffff);
	style0_label_effect.text.line_space = 2;

	ui->label_effect = lv_label_create(ui->button_show_effect, NULL);
	lv_label_set_text(ui->label_effect, "persian blind H");
	lv_label_set_long_mode(ui->label_effect, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_effect, 14, 12);
	lv_obj_set_size(ui->label_effect, 107, 19);
	lv_label_set_style(ui->label_effect, LV_LABEL_STYLE_MAIN, &style0_label_effect);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_show_speed, &lv_style_btn_rel);
	style0_button_show_speed.body.main_color = lv_color_hex(0x00aaff);
	style0_button_show_speed.body.grad_color = lv_color_hex(0x00aaff);
	style0_button_show_speed.body.border.color = lv_color_hex(0x55aaff);
	style0_button_show_speed.body.border.width = 1;

	lv_style_copy(&style1_button_show_speed, &lv_style_btn_pr);
	style1_button_show_speed.body.main_color = lv_color_hex(0x00aaff);
	style1_button_show_speed.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_show_speed.body.border.color = lv_color_hex(0x0000ff);

	ui->button_show_speed = lv_btn_create(ui->container_control, NULL);
	lv_obj_set_pos(ui->button_show_speed, 86, 67);
	lv_obj_set_size(ui->button_show_speed, 67, 41);
	lv_btn_set_style(ui->button_show_speed, LV_BTN_STYLE_REL, &style0_button_show_speed);
	lv_btn_set_style(ui->button_show_speed, LV_BTN_STYLE_PR, &style1_button_show_speed);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_speed, &lv_style_transp);
	style0_label_speed.text.color = lv_color_hex(0xffffff);
	style0_label_speed.text.line_space = 2;

	ui->label_speed = lv_label_create(ui->button_show_speed, NULL);
	lv_label_set_text(ui->label_speed, "normal");
	lv_label_set_long_mode(ui->label_speed, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_speed, 8, 11);
	lv_obj_set_size(ui->label_speed, 50, 19);
	lv_label_set_style(ui->label_speed, LV_LABEL_STYLE_MAIN, &style0_label_speed);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_info, &lv_style_btn_rel);
	style0_button_info.body.main_color = lv_color_hex(0x00aaff);
	style0_button_info.body.grad_color = lv_color_hex(0x00aaff);
	style0_button_info.body.border.color = lv_color_hex(0x55aaff);

	lv_style_copy(&style1_button_info, &lv_style_btn_pr);
	style1_button_info.body.main_color = lv_color_hex(0x00aaff);
	style1_button_info.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_info.body.border.color = lv_color_hex(0x0000ff);

	ui->button_info = lv_btn_create(ui->container_control, NULL);
	lv_obj_set_pos(ui->button_info, 297, 66);
	lv_obj_set_size(ui->button_info, 66, 43);
	lv_btn_set_style(ui->button_info, LV_BTN_STYLE_REL, &style0_button_info);
	lv_btn_set_style(ui->button_info, LV_BTN_STYLE_PR, &style1_button_info);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_info, &lv_style_transp);
	style0_label_info.text.color = lv_color_hex(0xffffff);
	style0_label_info.text.line_space = 2;

	ui->label_info = lv_label_create(ui->button_info, NULL);
	lv_label_set_text(ui->label_info, "info");
	lv_label_set_long_mode(ui->label_info, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_info, 19, 12);
	lv_obj_set_size(ui->label_info, 28, 19);
	lv_label_set_style(ui->label_info, LV_LABEL_STYLE_MAIN, &style0_label_info);
#endif // LV_USE_LABEL

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_sel_effect, &lv_style_pretty);
	style0_container_sel_effect.body.opa = 200;
	style0_container_sel_effect.body.border.width = 1;

	ui->container_sel_effect = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->container_sel_effect, 137, 18);
	lv_obj_set_size(ui->container_sel_effect, 536, 302);
	lv_cont_set_fit4(ui->container_sel_effect, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_hidden(ui->container_sel_effect,true);
	lv_cont_set_style(ui->container_sel_effect, LV_CONT_STYLE_MAIN, &style0_container_sel_effect);
#endif // LV_USE_CONT

#ifdef LV_USE_BTNM
	lv_style_copy(&style0_button_matrix_effect, &lv_style_pretty);
	style0_button_matrix_effect.body.opa = 0;
	style0_button_matrix_effect.body.border.opa = 0;

	lv_style_copy(&style1_button_matrix_effect, &lv_style_btn_rel);
	style1_button_matrix_effect.body.main_color = lv_color_hex(0x00aaff);
	style1_button_matrix_effect.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_matrix_effect.body.border.color = lv_color_hex(0x55aaff);
	style1_button_matrix_effect.body.border.width = 1;

	lv_style_copy(&style2_button_matrix_effect, &lv_style_btn_pr);
	style2_button_matrix_effect.body.main_color = lv_color_hex(0x00aaff);
	style2_button_matrix_effect.body.grad_color = lv_color_hex(0x00aaff);
	style2_button_matrix_effect.body.border.color = lv_color_hex(0x55aaff);
	style2_button_matrix_effect.body.border.width = 0;

	lv_style_copy(&style3_button_matrix_effect, &lv_style_btn_tgl_rel);
	style3_button_matrix_effect.body.main_color = lv_color_hex(0xff557f);
	style3_button_matrix_effect.body.grad_color = lv_color_hex(0xff557f);
	style3_button_matrix_effect.body.border.color = lv_color_hex(0x000000);
	style3_button_matrix_effect.body.border.width = 0;

	lv_style_copy(&style4_button_matrix_effect, &lv_style_btn_tgl_pr);
	style4_button_matrix_effect.body.main_color = lv_color_hex(0xff557f);
	style4_button_matrix_effect.body.grad_color = lv_color_hex(0xff557f);
	style4_button_matrix_effect.body.border.color = lv_color_hex(0xff55ff);
	style4_button_matrix_effect.body.border.width = 0;

	lv_style_copy(&style5_button_matrix_effect, &lv_style_btn_ina);
	style5_button_matrix_effect.body.main_color = lv_color_hex(0xff557f);
	style5_button_matrix_effect.body.grad_color = lv_color_hex(0xff557f);
	style5_button_matrix_effect.body.border.color = lv_color_hex(0xff55ff);
	style5_button_matrix_effect.body.border.width = 1;

	ui->button_matrix_effect = lv_btnm_create(ui->container_sel_effect, NULL);
	lv_obj_set_pos(ui->button_matrix_effect, 12, 6);
	lv_obj_set_size(ui->button_matrix_effect, 516, 246);
	static const char *map_button_matrix_effect[] = {
		"no effect","random","fade","mosaic","\n",
		"slide up","slide down","slide left","slide right","\n",
		"stretch up","stretch down","stretch left","stretch right","\n",
		"room in","room out","persian blind H","persian blind V",""
	};
	lv_btnm_set_map(ui->button_matrix_effect, map_button_matrix_effect);
	lv_btnm_set_style(ui->button_matrix_effect, LV_BTNM_STYLE_BG, &style0_button_matrix_effect);
	lv_btnm_set_style(ui->button_matrix_effect, LV_BTNM_STYLE_BTN_REL, &style1_button_matrix_effect);
	lv_btnm_set_style(ui->button_matrix_effect, LV_BTNM_STYLE_BTN_PR, &style2_button_matrix_effect);
	lv_btnm_set_style(ui->button_matrix_effect, LV_BTNM_STYLE_BTN_TGL_REL, &style3_button_matrix_effect);
	lv_btnm_set_style(ui->button_matrix_effect, LV_BTNM_STYLE_BTN_TGL_PR, &style4_button_matrix_effect);
	lv_btnm_set_style(ui->button_matrix_effect, LV_BTNM_STYLE_BTN_INA, &style5_button_matrix_effect);
#endif // LV_USE_BTNM

#ifdef LV_USE_IMG
	ui->image_set_effect = lv_img_create(ui->container_sel_effect, NULL);
	lv_obj_set_pos(ui->image_set_effect, 253, 257);
	lv_obj_set_size(ui->image_set_effect, 32, 32);
	image_set_effect_setting_choose_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_choose.png");
	lv_img_set_src(ui->image_set_effect, image_set_effect_setting_choose_png);

#endif // LV_USE_IMG

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_sel_speed, &lv_style_pretty);
	style0_container_sel_speed.body.opa = 200;
	style0_container_sel_speed.body.border.width = 1;

	ui->container_sel_speed = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->container_sel_speed, 137, 17);
	lv_obj_set_size(ui->container_sel_speed, 536, 302);
	lv_cont_set_fit4(ui->container_sel_speed, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_hidden(ui->container_sel_speed,true);
	lv_cont_set_style(ui->container_sel_speed, LV_CONT_STYLE_MAIN, &style0_container_sel_speed);
#endif // LV_USE_CONT

#ifdef LV_USE_BTNM
	lv_style_copy(&style0_button_matrix_speed, &lv_style_pretty);
	style0_button_matrix_speed.body.opa = 0;
	style0_button_matrix_speed.body.border.opa = 0;

	lv_style_copy(&style1_button_matrix_speed, &lv_style_btn_rel);
	style1_button_matrix_speed.body.main_color = lv_color_hex(0x00aaff);
	style1_button_matrix_speed.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_matrix_speed.body.border.color = lv_color_hex(0x55aaff);
	style1_button_matrix_speed.body.border.width = 1;

	lv_style_copy(&style2_button_matrix_speed, &lv_style_btn_pr);
	style2_button_matrix_speed.body.main_color = lv_color_hex(0x00aaff);
	style2_button_matrix_speed.body.grad_color = lv_color_hex(0x00aaff);
	style2_button_matrix_speed.body.border.color = lv_color_hex(0x55aaff);
	style2_button_matrix_speed.body.border.width = 1;

	lv_style_copy(&style3_button_matrix_speed, &lv_style_btn_tgl_rel);
	style3_button_matrix_speed.body.main_color = lv_color_hex(0xff557f);
	style3_button_matrix_speed.body.grad_color = lv_color_hex(0xff557f);
	style3_button_matrix_speed.body.border.color = lv_color_hex(0xff55ff);
	style3_button_matrix_speed.body.border.width = 1;

	lv_style_copy(&style4_button_matrix_speed, &lv_style_btn_tgl_pr);
	style4_button_matrix_speed.body.main_color = lv_color_hex(0xff557f);
	style4_button_matrix_speed.body.grad_color = lv_color_hex(0xff557f);
	style4_button_matrix_speed.body.border.color = lv_color_hex(0xff55ff);
	style4_button_matrix_speed.body.border.width = 1;

	ui->button_matrix_speed = lv_btnm_create(ui->container_sel_speed, NULL);
	lv_obj_set_pos(ui->button_matrix_speed, 125, 90);
	lv_obj_set_size(ui->button_matrix_speed, 289, 90);
	static const char *map_button_matrix_speed[] = {
		"slow","normal","fast",""
	};
	lv_btnm_set_map(ui->button_matrix_speed, map_button_matrix_speed);
	lv_btnm_set_style(ui->button_matrix_speed, LV_BTNM_STYLE_BG, &style0_button_matrix_speed);
	lv_btnm_set_style(ui->button_matrix_speed, LV_BTNM_STYLE_BTN_REL, &style1_button_matrix_speed);
	lv_btnm_set_style(ui->button_matrix_speed, LV_BTNM_STYLE_BTN_PR, &style2_button_matrix_speed);
	lv_btnm_set_style(ui->button_matrix_speed, LV_BTNM_STYLE_BTN_TGL_REL, &style3_button_matrix_speed);
	lv_btnm_set_style(ui->button_matrix_speed, LV_BTNM_STYLE_BTN_TGL_PR, &style4_button_matrix_speed);
#endif // LV_USE_BTNM

#ifdef LV_USE_IMG
	lv_style_copy(&style0_image_set_speed, &lv_style_plain);

	ui->image_set_speed = lv_img_create(ui->container_sel_speed, NULL);
	lv_obj_set_pos(ui->image_set_speed, 253, 257);
	lv_obj_set_size(ui->image_set_speed, 32, 32);
	image_set_speed_setting_choose_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_choose.png");
	lv_img_set_src(ui->image_set_speed, image_set_speed_setting_choose_png);

	lv_img_set_style(ui->image_set_speed, LV_IMG_STYLE_MAIN, &style0_image_set_speed);
#endif // LV_USE_IMG

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_dialog, &lv_style_pretty);
	style0_container_dialog.body.grad_color = lv_color_hex(0xffffff);
	style0_container_dialog.body.radius = 0;
	style0_container_dialog.body.opa = 100;
	style0_container_dialog.body.border.width = 0;

	ui->container_dialog = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->container_dialog, 0, 0);
	lv_obj_set_size(ui->container_dialog, 800, 480);
	lv_cont_set_fit4(ui->container_dialog, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_hidden(ui->container_dialog,true);
	lv_cont_set_style(ui->container_dialog, LV_CONT_STYLE_MAIN, &style0_container_dialog);
#endif // LV_USE_CONT

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_1, &lv_style_pretty);
	style0_container_1.body.main_color = lv_color_hex(0x55aaff);
	style0_container_1.body.grad_color = lv_color_hex(0xffffff);
	style0_container_1.body.border.width = 1;

	ui->container_1 = lv_cont_create(ui->container_dialog, NULL);
	lv_obj_set_pos(ui->container_1, 277, 145);
	lv_obj_set_size(ui->container_1, 214, 116);
	lv_cont_set_fit4(ui->container_1, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_cont_set_style(ui->container_1, LV_CONT_STYLE_MAIN, &style0_container_1);
#endif // LV_USE_CONT

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_1, &lv_style_btn_rel);
	style0_button_1.body.main_color = lv_color_hex(0x00aaff);
	style0_button_1.body.grad_color = lv_color_hex(0x00aaff);
	style0_button_1.body.border.color = lv_color_hex(0x55aaff);
	style0_button_1.body.border.width = 1;

	lv_style_copy(&style1_button_1, &lv_style_btn_pr);
	style1_button_1.body.main_color = lv_color_hex(0x5999ff);
	style1_button_1.body.grad_color = lv_color_hex(0x55aaff);
	style1_button_1.body.border.color = lv_color_hex(0x55aaff);
	style1_button_1.body.border.width = 1;

	lv_style_copy(&style4_button_1, &lv_style_btn_ina);
	style4_button_1.body.opa = 0;
	style4_button_1.body.border.opa = 0;

	ui->button_1 = lv_btn_create(ui->container_1, NULL);
	lv_obj_set_pos(ui->button_1, 58, 74);
	lv_obj_set_size(ui->button_1, 100, 35);
	lv_btn_set_state(ui->button_1, LV_BTN_STATE_INA);
	lv_btn_set_style(ui->button_1, LV_BTN_STYLE_REL, &style0_button_1);
	lv_btn_set_style(ui->button_1, LV_BTN_STYLE_PR, &style1_button_1);
	lv_btn_set_style(ui->button_1, LV_BTN_STYLE_INA, &style4_button_1);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_dialog_ok, &lv_style_transp);
	style0_label_dialog_ok.text.line_space = 2;
	style0_label_dialog_ok.text.opa = 0;

	ui->label_dialog_ok = lv_label_create(ui->button_1, NULL);
	lv_label_set_text(ui->label_dialog_ok, "OK");
	lv_label_set_long_mode(ui->label_dialog_ok, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_dialog_ok, 40, 8);
	lv_obj_set_size(ui->label_dialog_ok, 21, 19);
	lv_obj_set_hidden(ui->label_dialog_ok,true);
	lv_label_set_style(ui->label_dialog_ok, LV_LABEL_STYLE_MAIN, &style0_label_dialog_ok);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_dialog_info, &lv_style_transp);

	ui->label_dialog_info = lv_label_create(ui->container_1, NULL);
	lv_label_set_text(ui->label_dialog_info, "No picture!");
	lv_label_set_long_mode(ui->label_dialog_info, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_dialog_info, 69, 44);
	lv_obj_set_size(ui->label_dialog_info, 77, 19);
	lv_label_set_style(ui->label_dialog_info, LV_LABEL_STYLE_MAIN, &style0_label_dialog_info);
#endif // LV_USE_LABEL

#ifdef LV_USE_CONT
	lv_style_copy(&style0_container_info, &lv_style_pretty);
	style0_container_info.body.opa = 200;
	style0_container_info.body.border.width = 1;

	ui->container_info = lv_cont_create(ui->cont, NULL);
	lv_obj_set_pos(ui->container_info, 137, 17);
	lv_obj_set_size(ui->container_info, 536, 302);
	lv_cont_set_fit4(ui->container_info, LV_FIT_NONE, LV_FIT_NONE ,LV_FIT_NONE ,LV_FIT_NONE);
	lv_obj_set_hidden(ui->container_info,true);
	lv_cont_set_style(ui->container_info, LV_CONT_STYLE_MAIN, &style0_container_info);
#endif // LV_USE_CONT

#ifdef LV_USE_IMG
	ui->image_get_info = lv_img_create(ui->container_info, NULL);
	lv_obj_set_pos(ui->image_get_info, 253, 257);
	lv_obj_set_size(ui->image_get_info, 32, 32);
	image_get_info_setting_choose_png = (void *)parse_image_from_file(LV_IMAGE_PATH"setting_choose.png");
	lv_img_set_src(ui->image_get_info, image_get_info_setting_choose_png);

#endif // LV_USE_IMG

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_title_filename, &lv_style_transp);
	style0_label_title_filename.body.shadow.color = lv_color_hex(0xffffff);
	style0_label_title_filename.text.color = lv_color_hex(0xffffff);
	style0_label_title_filename.text.line_space = 2;

	ui->label_title_filename = lv_label_create(ui->container_info, NULL);
	lv_label_set_text(ui->label_title_filename, "Filename:");
	lv_label_set_long_mode(ui->label_title_filename, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_title_filename, 23, 33);
	lv_obj_set_size(ui->label_title_filename, 71, 19);
	lv_label_set_style(ui->label_title_filename, LV_LABEL_STYLE_MAIN, &style0_label_title_filename);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_title_file_size, &lv_style_transp);
	style0_label_title_file_size.text.color = lv_color_hex(0xffffff);
	style0_label_title_file_size.text.line_space = 2;

	ui->label_title_file_size = lv_label_create(ui->container_info, NULL);
	lv_label_set_text(ui->label_title_file_size, "Size:");
	lv_label_set_long_mode(ui->label_title_file_size, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_title_file_size, 24, 95);
	lv_obj_set_size(ui->label_title_file_size, 35, 19);
	lv_label_set_style(ui->label_title_file_size, LV_LABEL_STYLE_MAIN, &style0_label_title_file_size);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_title_w_h, &lv_style_transp);
	style0_label_title_w_h.text.color = lv_color_hex(0xffffff);
	style0_label_title_w_h.text.line_space = 2;

	ui->label_title_w_h = lv_label_create(ui->container_info, NULL);
	lv_label_set_text(ui->label_title_w_h, "W&H:");
	lv_label_set_long_mode(ui->label_title_w_h, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_title_w_h, 24, 154);
	lv_obj_set_size(ui->label_title_w_h, 39, 19);
	lv_label_set_style(ui->label_title_w_h, LV_LABEL_STYLE_MAIN, &style0_label_title_w_h);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_filename, &lv_style_transp);
	style0_label_filename.text.color = lv_color_hex(0xffffff);
	style0_label_filename.text.line_space = 2;

	ui->label_filename = lv_label_create(ui->container_info, NULL);
	lv_label_set_text(ui->label_filename, "asdasd");
	lv_label_set_long_mode(ui->label_filename, LV_LABEL_LONG_BREAK);
	lv_obj_set_pos(ui->label_filename, 120, 25);
	lv_obj_set_size(ui->label_filename, 403, 19);
	lv_label_set_style(ui->label_filename, LV_LABEL_STYLE_MAIN, &style0_label_filename);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_file_size, &lv_style_transp);
	style0_label_file_size.text.color = lv_color_hex(0xffffff);
	style0_label_file_size.text.line_space = 2;

	ui->label_file_size = lv_label_create(ui->container_info, NULL);
	lv_label_set_text(ui->label_file_size, "Text");
	lv_label_set_long_mode(ui->label_file_size, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_file_size, 116, 97);
	lv_obj_set_size(ui->label_file_size, 406, 22);
	lv_label_set_style(ui->label_file_size, LV_LABEL_STYLE_MAIN, &style0_label_file_size);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_w_h, &lv_style_transp);
	style0_label_w_h.text.color = lv_color_hex(0xffffff);
	style0_label_w_h.text.line_space = 2;

	ui->label_w_h = lv_label_create(ui->container_info, NULL);
	lv_label_set_text(ui->label_w_h, "Text");
	lv_label_set_long_mode(ui->label_w_h, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_w_h, 116, 157);
	lv_obj_set_size(ui->label_w_h, 399, 22);
	lv_label_set_style(ui->label_w_h, LV_LABEL_STYLE_MAIN, &style0_label_w_h);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_title_file_time, &lv_style_transp);
	style0_label_title_file_time.text.color = lv_color_hex(0xffffff);
	style0_label_title_file_time.text.line_space = 2;

	ui->label_title_file_time = lv_label_create(ui->container_info, NULL);
	lv_label_set_text(ui->label_title_file_time, "Time:");
	lv_label_set_long_mode(ui->label_title_file_time, LV_LABEL_LONG_EXPAND);
	lv_obj_set_pos(ui->label_title_file_time, 24, 212);
	lv_obj_set_size(ui->label_title_file_time, 41, 19);
	lv_label_set_style(ui->label_title_file_time, LV_LABEL_STYLE_MAIN, &style0_label_title_file_time);
#endif // LV_USE_LABEL

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_file_time, &lv_style_transp);
	style0_label_file_time.text.color = lv_color_hex(0xffffff);
	style0_label_file_time.text.line_space = 2;

	ui->label_file_time = lv_label_create(ui->container_info, NULL);
	lv_label_set_text(ui->label_file_time, "Text");
	lv_label_set_long_mode(ui->label_file_time, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(ui->label_file_time, 116, 215);
	lv_obj_set_size(ui->label_file_time, 395, 22);
	lv_label_set_style(ui->label_file_time, LV_LABEL_STYLE_MAIN, &style0_label_file_time);
#endif // LV_USE_LABEL

}

void photo_auto_ui_destory(photo_ui_t *ui)
{
	lv_obj_clean(ui->cont);
	free_image(image_set_effect_setting_choose_png);
	free_image(image_set_speed_setting_choose_png);
	free_image(image_get_info_setting_choose_png);
	free_image(back_home_return_png_state0);
	free_image(back_home_return_png_state1);
	free_image(image_button_preview_background31_jpg_state0);
	free_image(image_button_preview_background31_jpg_state1);
	free_image(image_button_play_start_play_start_jpeg_state0);
	free_image(image_button_play_start_play_start_jpeg_state1);
	free_image(image_button_rotate_left_rotate_left_png_state0);
	free_image(image_button_rotate_left_rotate_left_png_state1);
	free_image(image_button_rotate_right_rotate_right_png_state0);
	free_image(image_button_rotate_right_rotate_right_png_state1);
	free_image(image_button_scaler_down_scaler_down_jpg_state0);
	free_image(image_button_scaler_down_scaler_down_jpg_state1);
	free_image(image_button_scaler_down_scaler_down_jpg_state2);
	free_image(image_button_scaler_up_scaler_up_jpg_state0);
	free_image(image_button_scaler_up_scaler_up_jpg_state1);
	free_image(media_list_1_movie_item_png);
	free_image(media_list_2_movie_item_png);
	free_image(media_list_3_movie_item_png);
	free_image(media_list_4_movie_item_png);
	free_image(media_list_5_movie_item_png);
	free_image(media_list_6_movie_item_png);
}
