/**********************
 *      includes
 **********************/
#include <stdio.h>
#include <stdlib.h>
#include "moc_photo.h"
#include "ui_photo.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "media_load_file.h"
#include "image.h"
//#include "media_ui.h"
#include "bs_widget.h"
#include "app_config_interface.h"



#define IMAGE_PLAY_TIME_STEP 10  //图片播放检测间隔，单位: ms

typedef enum {
	IMAGE_PLAY_STOP,
	IMAGE_PLAY_START,
} image_play_state_t;

typedef enum {
	IMAGE_DIR_UP,
	IMAGE_DIR_DOWN,
	IMAGE_DIR_LEFT,
	IMAGE_DIR_RIGHT,
	IMAGE_DIR_UNKNOWN
} image_dir_t;

typedef struct {
	photo_ui_t ui;
	pthread_t id;
	pthread_t gif_id;
	pthread_t auto_play_id;
	sem_t sem;
	volatile image_play_state_t play_state;
	char *file_path;
	int file_index;
	int file_total_num;
	int setting_interface_hidden;
	int scaler_ratio; /* in percentage */
	image_rotate_angle_t rotate_angle;
	int  effect;
	int  speed;
	int  is_busy;
	int  is_early_exit;
	int  is_auto_play;
	photo_app_data_t app_data;
} image_mgr_t;

static image_mgr_t image_mgr;
static volatile int window_level = 0;
static pthread_cond_t  image_wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t image_wait_mutex;
static pthread_mutex_t image_busy_mutex;
static media_file_list_t *media_file_list = NULL;

#if 0
extern image_buffer_t image;
static lv_img_dsc_t dsc;

void *image_button_redraw(void)
{
	memset(&dsc, 0, sizeof(lv_img_dsc_t));

	dsc.header.w = image.dst.width;
	dsc.header.h = image.dst.height;
	dsc.header.always_zero = 0;
	dsc.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	dsc.data_size = image.dst.width * image.dst.height * image.dst.comp;
	dsc.data = image.dst.vir_addr;
	return &dsc;
}
#endif

static void button_setting_interface_ui_hide(bool en)
{
	// lv_obj_set_hidden(image_mgr.ui.image_button_rotate_right, en);
	// lv_obj_set_hidden(image_mgr.ui.image_button_rotate_left, en);
	// lv_obj_set_hidden(image_mgr.ui.image_button_scaler_up, en);
	// lv_obj_set_hidden(image_mgr.ui.image_button_scaler_down, en);
	// lv_obj_set_hidden(image_mgr.ui.image_button_play_start, en);
	lv_obj_set_hidden(image_mgr.ui.container_control, en);
	lv_obj_set_hidden(image_mgr.ui.back, en);
	if (en) {
		lv_obj_move_foreground(image_mgr.ui.button_setting_interface);
		if(!lv_obj_get_hidden(image_mgr.ui.container_sel_effect)){
			lv_obj_set_hidden(image_mgr.ui.container_sel_effect, en);
		}
		if(!lv_obj_get_hidden(image_mgr.ui.container_sel_speed)){
			lv_obj_set_hidden(image_mgr.ui.container_sel_speed, en);
		}
		if(!lv_obj_get_hidden(image_mgr.ui.container_info)){
			lv_obj_set_hidden(image_mgr.ui.container_info, en);
		}
	} else {
		lv_obj_move_background(image_mgr.ui.button_setting_interface);
	}
	image_mgr.setting_interface_hidden = en;
}

static void image_thumb_window_set(void)
{
	image_rect_t rect;

	rect.x = lv_obj_get_x(image_mgr.ui.image_button_preview);
	rect.y = lv_obj_get_y(image_mgr.ui.image_button_preview);
	rect.w = lv_obj_get_width(image_mgr.ui.image_button_preview);
	rect.h = lv_obj_get_height(image_mgr.ui.image_button_preview);
	image_disp_rect_set(&rect);
}

static int id2speed(int id)
{
	int speed;
	switch(id){
		case 0:
			speed = 5;
			break;
		case 1:
			speed = 3;
			break;
		case 2:
			speed = 1;
			break;
		default:
			speed = -1;
			break;
	}
	return speed;
}
static int speed2id(int speed)
{
	int id;
	switch(speed){
		case 5:
			id = 0;
			break;
		case 3:
			id = 1;
			break;
		case 1:
			id = 2;
			break;
		default:
			id = -1;
			break;
	}
	return id;
}


static int id2effect(int rotate,int id)
{
	switch(id){
		case 0:
			image_mgr.effect = IMAGE_SHOW_NORMAL;
			break;
		case 1:
			image_mgr.effect = IMAGE_SHOW_RANDOM;
			break;
		case 2:
			image_mgr.effect = IMAGE_SHOW_FADE;
			break;
		case 3:
			image_mgr.effect = IMAGE_SHOW_MOSIAC;
			break;
		case 4:
			image_mgr.effect = IMAGE_SHOW_SLIDE_UP;
			break;
		case 5:
			image_mgr.effect = IMAGE_SHOW_SLIDE_DOWN;
			break;
		case 6:
			image_mgr.effect = IMAGE_SHOW_SLIDE_LEFT;
			break;
		case 7:
			image_mgr.effect = IMAGE_SHOW_SLIDE_RIGHT;
			break;
		case 8:
			image_mgr.effect = IMAGE_SHOW_STRETCH_UP;
			break;
		case 9:
			image_mgr.effect = IMAGE_SHOW_STRETCH_DOWN;
			break;
		case 10:
			image_mgr.effect = IMAGE_SHOW_STRETCH_LEFT;
			break;
		case 11:
			image_mgr.effect = IMAGE_SHOW_STRETCH_RIGHT;
			break;
		case 12:
			image_mgr.effect = IMAGE_SHOW_ZOOM_IN;
			break;
		case 13:
			image_mgr.effect = IMAGE_SHOW_ZOOM_OUT;
			break;
		case 14:
			image_mgr.effect = IMAGE_SHOW_PERSIANBLIND_H;
			break;
		case 15:
			image_mgr.effect = IMAGE_SHOW_PERSIANBLIND_V;
			break;
		default:
			image_mgr.effect = IMAGE_SHOW_NORMAL;
			break;
	}
	return image_mgr.effect;
}
static void image_confirm_event(lv_obj_t * btn, lv_event_t event)
{
	int btn_id;
	const char * btn_text;
	if (event == LV_EVENT_CLICKED) {
		if(btn == image_mgr.ui.image_set_effect)
		{
			btn_id   = lv_btnm_get_active_btn(image_mgr.ui.button_matrix_effect);
			btn_text = lv_btnm_get_active_btn_text(image_mgr.ui.button_matrix_effect);
			lv_label_set_text(image_mgr.ui.label_effect,btn_text);
			lv_obj_set_hidden(image_mgr.ui.container_sel_effect,1);
			image_mgr.effect = id2effect(0,btn_id);


		}
		else if(btn == image_mgr.ui.image_set_speed)
		{
			btn_id   = lv_btnm_get_active_btn(image_mgr.ui.button_matrix_speed);
			btn_text = lv_btnm_get_active_btn_text(image_mgr.ui.button_matrix_speed);
			lv_label_set_text(image_mgr.ui.label_speed,btn_text);
			lv_obj_set_hidden(image_mgr.ui.container_sel_speed,1);
			image_mgr.speed = id2speed(btn_id);
		}
		else if(btn == image_mgr.ui.image_get_info)
		{
			lv_obj_set_hidden(image_mgr.ui.container_info,1);
		}
	}
}

static void btnm_click_event(lv_obj_t * btn, lv_event_t event)
{
	int btn_id;
	// static int last_id = -1;
	const char * btn_text;

	if(event == LV_EVENT_VALUE_CHANGED)
	{
		if(btn == image_mgr.ui.button_matrix_effect)
		{
			btn_id   = lv_btnm_get_active_btn(image_mgr.ui.button_matrix_effect);
			btn_text = lv_btnm_get_active_btn_text(image_mgr.ui.button_matrix_effect);
			lv_label_set_text(image_mgr.ui.label_effect,btn_text);
			//lv_obj_set_hidden(image_mgr.ui.container_sel_effect,1);
			image_mgr.effect = id2effect(0,btn_id);
		}
		else if(btn == image_mgr.ui.button_matrix_speed)
		{
			btn_id   = lv_btnm_get_active_btn(image_mgr.ui.button_matrix_speed);
			btn_text = lv_btnm_get_active_btn_text(image_mgr.ui.button_matrix_speed);
			lv_label_set_text(image_mgr.ui.label_speed,btn_text);
			//lv_obj_set_hidden(image_mgr.ui.container_sel_speed,1);
			image_mgr.speed = id2speed(btn_id);
		}
	}
}

static void btn_effect_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		if(lv_obj_get_hidden(image_mgr.ui.container_sel_effect))
		{
			lv_obj_set_hidden(image_mgr.ui.container_sel_effect,0);
		}
		else
		{
			lv_obj_set_hidden(image_mgr.ui.container_sel_effect,1);
		}

		if(!lv_obj_get_hidden(image_mgr.ui.container_sel_speed))
		{
			lv_obj_set_hidden(image_mgr.ui.container_sel_speed,1);
		}
		if(!lv_obj_get_hidden(image_mgr.ui.container_info))
		{
			lv_obj_set_hidden(image_mgr.ui.container_info,1);
		}
	}
}
static void setting_dialog_hide(void)
{
	if(!lv_obj_get_hidden(image_mgr.ui.container_sel_speed))
	{
		lv_obj_set_hidden(image_mgr.ui.container_sel_speed,1);
	}

	if(!lv_obj_get_hidden(image_mgr.ui.container_sel_effect))
	{
		lv_obj_set_hidden(image_mgr.ui.container_sel_effect,1);
	}

	if(!lv_obj_get_hidden(image_mgr.ui.container_info))
	{
		lv_obj_set_hidden(image_mgr.ui.container_info,1);
	}
}
static void btn_speed_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		if(lv_obj_get_hidden(image_mgr.ui.container_sel_speed))
		{
			lv_obj_set_hidden(image_mgr.ui.container_sel_speed,0);
		}
		else
		{
			lv_obj_set_hidden(image_mgr.ui.container_sel_speed,1);
		}

		if(!lv_obj_get_hidden(image_mgr.ui.container_sel_effect))
		{
			lv_obj_set_hidden(image_mgr.ui.container_sel_effect,1);
		}
		if(!lv_obj_get_hidden(image_mgr.ui.container_info))
		{
			lv_obj_set_hidden(image_mgr.ui.container_info,1);
		}
	}
}

static void size2str(__u32 size, char *str) {
	if (size < 1024) {
		sprintf(str, "%d B",size);
	}
	else if ( size < (1024*1024)) {
		sprintf(str, "%d.%dK",size/1024, size%1024*10/1024);
	}
	else if (size < (1024*1024*1024)) {
		sprintf(str, "%d.%d%d M",size/(1024*1024), ((size%(1024*1024))/1024)*1000/1024/100,((size%(1024*1024))/1024)*1000/1024%100/10);	//������λС��
	}
	else {
		sprintf(str, "%d.%d%d G",size/(1024*1024*1024),(size%(1024*1024*1024))/(1024*1024)*1000/1024/100,(size%(1024*1024*1024))/(1024*1024)*1000/1024%100/10);
	}
}

static __s32 mtime_to_time_string(time_t *mtime, char *string) {
	struct tm *p = NULL;
	__s32 Index = 0;

	p = gmtime(mtime);

	string[Index++] = (1900 + p->tm_year)/1000 + '0';
	string[Index++] = (1900 + p->tm_year)/100%10 + '0';
	string[Index++] = (1900 + p->tm_year)/10%10 + '0';
	string[Index++] = (1900 + p->tm_year)%10 + '0';

	string[Index++] = '-';
	string[Index++] = p->tm_mon/10 + '0';
	string[Index++] = p->tm_mon%10 + '0';

	string[Index++] = '-';
	string[Index++] = p->tm_mday/10 + '0';
	string[Index++] = p->tm_mday%10 + '0';

	string[Index++] = ' ';
	string[Index++] = p->tm_hour / 10 + '0';
	string[Index++] = p->tm_hour % 10 + '0';
	string[Index++] = ':';
	string[Index++] = p->tm_min / 10 + '0';
	string[Index++] = p->tm_min % 10 + '0';
	string[Index++] = ':';
	string[Index++] = p->tm_sec / 10 + '0';
	string[Index++] = p->tm_sec % 10 + '0';
	string[Index++] = '\0';
	return 0;
}

static void btn_info_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		char*filename;
		char str[32];
		int width,height;
		struct stat file_stat;
		filename = media_get_file_path(media_file_list,image_mgr.file_index);
		lv_label_set_text(image_mgr.ui.label_filename, media_get_path_to_name(filename));
		memset(&file_stat, 0x00, sizeof(struct stat));

		stat(filename, &file_stat);
		size2str(file_stat.st_size, str);
		lv_label_set_text(image_mgr.ui.label_file_size, str);
		memset(str, 0x00, sizeof(str));

		mtime_to_time_string(&file_stat.st_mtime, str);
		lv_label_set_text(image_mgr.ui.label_file_time, str);
		memset(str, 0x00, sizeof(str));


		width  = image_get_width();
		height = image_get_height();
		sprintf(str,"%d x %d",width,height);
		lv_label_set_text(image_mgr.ui.label_w_h, str);


		if(lv_obj_get_hidden(image_mgr.ui.container_info))
		{
			lv_obj_set_hidden(image_mgr.ui.container_info,0);
		}
		else
		{
			lv_obj_set_hidden(image_mgr.ui.container_info,1);
		}

		if(!lv_obj_get_hidden(image_mgr.ui.container_sel_effect))
		{
			lv_obj_set_hidden(image_mgr.ui.container_sel_effect,1);
		}
		if(!lv_obj_get_hidden(image_mgr.ui.container_sel_speed))
		{
			lv_obj_set_hidden(image_mgr.ui.container_sel_speed,1);
		}
	}
}



static void image_rotate_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		setting_dialog_hide();
		if (btn == image_mgr.ui.image_button_rotate_left) {
			if(image_mgr.rotate_angle > 0)
				image_mgr.rotate_angle--;
			else
				image_mgr.rotate_angle = 3;
		} else if (btn == image_mgr.ui.image_button_rotate_right) {
			image_mgr.rotate_angle = (image_mgr.rotate_angle + 1) % 4;
		}
		image_rotate(image_mgr.rotate_angle);
	}
}

static void image_rotate_event1(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		if (btn == image_mgr.ui.image_button_rotate_left) {
				image_mgr.rotate_angle = IMAGE_ROTATE_270;
		} else if (btn == image_mgr.ui.image_button_rotate_right) {
			image_mgr.rotate_angle = IMAGE_ROTATE_90;
		}
		image_rotate(image_mgr.rotate_angle);
	}
}

static void image_scaler_event(lv_obj_t * btn, lv_event_t event)
{
	if ((event == LV_EVENT_CLICKED) || (event == LV_EVENT_LONG_PRESSED_REPEAT)) {
		setting_dialog_hide();
		if (btn == image_mgr.ui.image_button_scaler_up) {
			image_mgr.scaler_ratio += 20;
		} else if (btn == image_mgr.ui.image_button_scaler_down) {
			image_mgr.scaler_ratio -= 20;
			if (image_mgr.scaler_ratio < 20)
				image_mgr.scaler_ratio = 20;
		}
		image_scaler(&image_mgr.scaler_ratio);
	}
}

static void image_list_event(lv_obj_t * btn, lv_event_t event)
{
	image_mgr.file_index = lv_list_get_btn_index(image_mgr.ui.media_list, btn);
	if (event == LV_EVENT_CLICKED) {
		image_mgr.file_path = media_get_file_path(media_file_list,image_mgr.file_index);
		lv_label_set_text(image_mgr.ui.file_name, media_get_path_to_name(image_mgr.file_path));
		printf("%s %d file_index = %d, image_mgr.file_path = %s\n", __func__, __LINE__, image_mgr.file_index, image_mgr.file_path);
		image_disp_cache_enable(1);
		image_thumb_window_set();

		image_show(image_mgr.file_path, IMAGE_FULL_SCREEN_SCLAER);
		image_disp_cache_enable(0);
		usleep(20000);
	}
}

static void back_btn_event(lv_obj_t *btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		if (window_level == 0) {
			//image_exit();
			//destory_page(PAGE_PHOTO);
			//create_page(PAGE_HOME);
			switch_page(PAGE_PHOTO, PAGE_HOME);
			//media_unload_file();
		} else if (window_level == 1) {
			button_setting_interface_ui_hide(1);
			lv_obj_set_hidden(image_mgr.ui.back, 0);
			lv_obj_set_hidden(image_mgr.ui.media_list, 0);
			lv_obj_set_hidden(image_mgr.ui.image_button_preview, 0);
			lv_obj_set_hidden(image_mgr.ui.file_name, 0);
			lv_obj_set_hidden(image_mgr.ui.button_setting_interface, 1);
			window_level = 0;
			image_disp_cache_enable(1);
			image_thumb_window_set();
			image_show(image_mgr.file_path, IMAGE_FULL_SCREEN_SCLAER);
			image_disp_cache_enable(0);
			usleep(20000);
		}
	}
}

static void image_preview_full_screen_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		window_level = 1;
		image_mgr.rotate_angle = 0;
		image_mgr.scaler_ratio = 100;
		lv_obj_set_hidden(image_mgr.ui.media_list, 1);
		lv_obj_set_hidden(image_mgr.ui.image_button_preview, 1);
		lv_obj_set_hidden(image_mgr.ui.file_name, 1);

		image_disp_cache_enable(1);
		image_disp_rect_set(IMAGE_FULL_SCREEN);
		image_show(image_mgr.file_path, IMAGE_FULL_SCREEN_SCLAER);
		image_disp_cache_enable(0);
		usleep(20000);
		button_setting_interface_ui_hide(1);
//		image_button_redraw();
//		lv_imgbtn_set_src(image_mgr.ui.button_setting_interface, LV_BTN_STATE_REL, &dsc);
		lv_obj_set_hidden(image_mgr.ui.button_setting_interface, 0);
		// photo_user_data_t *user_data = get_page_user_data(PAGE_PHOTO);
		// if(user_data && user_data->is_auto_play)
		// {
		//	lv_event_send(image_mgr.ui.image_button_play_start,LV_EVENT_CLICKED,NULL);
		// }
	}
}



static void image_play_phread_down(void)
{
	pthread_mutex_lock(&image_wait_mutex);
	pthread_cond_wait(&image_wait_cond, &image_wait_mutex);
	pthread_mutex_unlock(&image_wait_mutex);
}
static void image_play_thread_up(void)
{
	pthread_mutex_lock(&image_wait_mutex);
	pthread_cond_signal(&image_wait_cond);
	pthread_mutex_unlock(&image_wait_mutex);
}

static void image_play_stop(void)
{
	if (image_mgr.play_state != IMAGE_PLAY_STOP) {
		image_mgr.play_state = IMAGE_PLAY_STOP;
		pthread_join(image_mgr.id, NULL);
		sem_destroy(&image_mgr.sem);
	}
}

static void image_play_next(void)
{

}
static void image_play_prev(void)
{

}
static void image_auto_play_start(void)
{
	button_setting_interface_ui_hide(1);
	image_disp_cache_enable(1);
	image_mgr.rotate_angle = 0;
	image_mgr.scaler_ratio = 100;
	image_disp_rect_set(IMAGE_FULL_SCREEN);
	image_show_normal();
	image_disp_cache_enable(0);
	image_mgr.play_state = IMAGE_PLAY_START;
	image_play_thread_up();

}
static void image_auto_play_stop(void)
{
	image_mgr.play_state = IMAGE_PLAY_STOP;
	button_setting_interface_ui_hide(0);

}
static void image_play_continue(void)
{

}
static void image_play_resume(void)
{

}
static void image_play_prepare(void)
{
	image_mgr.file_index = (image_mgr.file_index + 1) % image_mgr.file_total_num;
	image_mgr.file_path = media_get_file_path(media_file_list,image_mgr.file_index);
}
static int  image_play_get_status(void)
{
	return image_mgr.play_state;
}
static int  image_play_get_interval(void)
{
	return image_mgr.speed;
}



static void *image_auto_play_pthread(void *arg)
{
	int interval,delay_cnt;
	int ret = 0;
	while(1)
	{
		if(image_play_get_status() == IMAGE_PLAY_START)
		{
			pthread_mutex_lock(&image_busy_mutex);
			image_mgr.is_busy = 1;
			pthread_mutex_unlock(&image_busy_mutex);
			interval = image_play_get_interval();//单位：s
			delay_cnt = (interval) * 1000 / IMAGE_PLAY_TIME_STEP;
			//printf("delay_cnt = %d\n",delay_cnt);
			if(ret < 0)
			{
				delay_cnt = 0;//播放失败，跳过这张图片
			}
			while (delay_cnt)
			{
				usleep(IMAGE_PLAY_TIME_STEP*1000);

				if (image_play_get_status() != IMAGE_PLAY_START)
				{
					break ;
				}
				else
				{
					delay_cnt-- ;
				}
			}
		}

		if(image_play_get_status() == IMAGE_PLAY_START)
		{
			pthread_mutex_lock(&image_busy_mutex);
			image_mgr.is_busy = 1;
			pthread_mutex_unlock(&image_busy_mutex);
			ret = image_effect_show(image_mgr.file_path, image_mgr.effect);
			image_play_prepare();
		}
		if(image_play_get_status() == IMAGE_PLAY_STOP)
		{
			printf("down!!!\n");
			pthread_mutex_lock(&image_busy_mutex);
			image_mgr.is_busy = 0;
			pthread_mutex_unlock(&image_busy_mutex);
			image_play_phread_down();
			printf("up!!!!\n");
		}
		if(image_play_get_status() == IMAGE_PLAY_STOP)
		{
			pthread_exit(0);
		}
	}

}


static void *image_play_pthread2(void *arg)
{
#if 1
	int i;

	button_setting_interface_ui_hide(1);
	image_disp_rect_set(IMAGE_FULL_SCREEN);
	for (i = 0; i < 2; i++) {
		image_mgr.file_index = (image_mgr.file_index + 1) % image_mgr.file_total_num;
		image_mgr.file_path = media_get_file_path(media_file_list,image_mgr.file_index);
		printf("%s: file = %s\n", __func__, image_mgr.file_path);
		image_effect_show(image_mgr.file_path, image_mgr.effect);
		//image_effect_show(image_mgr.file_path, IMAGE_SHOW_ROOM);
		//image_effect_show(image_mgr.file_path, IMAGE_SHOW_FILL);
		// if (image_mgr.play_state == IMAGE_SHOW_STRETCH) {
		//	return NULL;
		// }
		sleep(image_mgr.speed);
	}
	button_setting_interface_ui_hide(0);
	image_mgr.play_state = IMAGE_PLAY_STOP;
#endif
	return NULL;
}

static int image_play_start(void)
{
	int ret = -1;

	if (image_mgr.play_state != IMAGE_PLAY_START) {
		image_mgr.play_state = IMAGE_PLAY_START;
		ret = pthread_create(&image_mgr.id, NULL, image_play_pthread2, NULL);
		if (ret != 0) {
			printf("%s: pthread create fail!\n", __func__);
		}
	}
	image_mgr.scaler_ratio = 100;
	image_mgr.rotate_angle = 0;
	return ret;

	// pthread_mutex_init(&load_mutex, NULL);
	// pthread_mutex_init(&play_mutex, NULL);
}

static void image_play_event(lv_obj_t *btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		//image_play_start();
		setting_dialog_hide();
		image_auto_play_start();
	}
}

static image_dir_t image_button_dir_get(lv_obj_t *btn, lv_point_t *point)
{
	lv_area_t cords;
	image_dir_t dir;

	lv_obj_get_coords(btn, &cords);

	if (point->x < (cords.x2 / 3)) {
		dir =  IMAGE_DIR_LEFT;
	} else if (point->x > (cords.x2 * 2 / 3)) {
		dir = IMAGE_DIR_RIGHT;
	} else if (point->y < (cords.y2 / 3)) {
		dir = IMAGE_DIR_UP;
	} else if (point->y > (cords.y2 * 2 / 3)) {
		dir = IMAGE_DIR_DOWN;
	} else {
		dir = IMAGE_DIR_UNKNOWN;
	}

	return dir;
}

//lv_indev_get_vect instead
static void image_drag(struct _lv_task_t *task)
{
	image_dir_t dir;
	int x = 0, y = 0;
	lv_point_t *point = (lv_point_t *)task->user_data;

	if (point == NULL) {
		printf("%s: point = %p\n", __func__, point);
		return;
	}
	dir = image_button_dir_get(image_mgr.ui.button_setting_interface, point);
	if (dir == IMAGE_DIR_LEFT) {
		x = -1;
	} else if (dir == IMAGE_DIR_RIGHT) {
		x = 1;
	} else if (dir == IMAGE_DIR_UP) {
		y = -1;
	} else if (dir == IMAGE_DIR_DOWN) {
		y = 1;
	}
	printf("move x = %d y = %d\n",x,y);
	image_move(x, y);
}

static int image_switch(image_dir_t dir)
{
	int ret = 0;
	image_disp_cache_enable(1);
	image_disp_rect_set(IMAGE_FULL_SCREEN);
	if (dir == IMAGE_DIR_LEFT) {
		if(image_mgr.file_index > 0)
			image_mgr.file_index--;
		else
			image_mgr.file_index = image_mgr.file_total_num - 1;
		image_mgr.file_path = media_get_file_path(media_file_list,image_mgr.file_index);
	} else if (dir == IMAGE_DIR_RIGHT) {
		image_mgr.file_index = (image_mgr.file_index + 1) % image_mgr.file_total_num;
		image_mgr.file_path = media_get_file_path(media_file_list,image_mgr.file_index);
	}
	image_mgr.rotate_angle = 0;
	image_mgr.scaler_ratio = 100;

	ret = image_show(image_mgr.file_path, IMAGE_FULL_SCREEN_SCLAER);
	image_disp_cache_enable(0);
	usleep(20000);/*延时20ms，确保cache参数生效*/
	return ret;
}

static void button_setting_interface_event(lv_obj_t *btn, lv_event_t event)
{
	int dir;
	static int is_dragging = 0;
	static lv_point_t point;
	static lv_task_t *image_drag_task = NULL;

	if (event == LV_EVENT_CLICKED) {
		if (image_mgr.setting_interface_hidden == 1) {
			 switch (image_mgr.play_state) {
				case IMAGE_PLAY_STOP:
					lv_indev_get_point(lv_indev_get_act(), &point);
					dir = image_button_dir_get(btn, &point);
					if ((dir == IMAGE_DIR_LEFT) || (dir == IMAGE_DIR_RIGHT)) {
						image_switch(dir);
					} else {
						button_setting_interface_ui_hide(0);
					}
					break;
				case IMAGE_PLAY_START:
					//image_play_stop();
					image_auto_play_stop();
					button_setting_interface_ui_hide(0);
					break;
				default:
					break;
			 }
		} else {
			if (!is_dragging)
				button_setting_interface_ui_hide(1);
		}
	} else if (event == LV_EVENT_LONG_PRESSED_REPEAT) {
		if (image_mgr.setting_interface_hidden == 0) {
			lv_indev_get_point(lv_indev_get_act(), &point);
			if (!is_dragging) {
				is_dragging = 1;
				image_drag_task = lv_task_create(image_drag, 10, LV_TASK_PRIO_MID, &point);
			}
		}
	} else if (event == LV_EVENT_RELEASED) {
		if (is_dragging) {
			is_dragging = 0;
			lv_task_del(image_drag_task);
		}
	}
}

static void photo_hide_dialog(void)
{
	lv_obj_set_hidden(image_mgr.ui.container_dialog,1);
}

static void photo_dialog_event(lv_obj_t *btn, lv_event_t event)
{
	if(event == LV_EVENT_CLICKED)
	{
		photo_hide_dialog();
		printf("photo end!\n");
		switch_page(PAGE_PHOTO, PAGE_HOME);
	}
}

static void dialog_task(lv_task_t *task)
{
	photo_hide_dialog();
	printf("photo end!\n");
	switch_page(PAGE_PHOTO, PAGE_HOME);
}

static void photo_show_dialog(const char* message, bool is_btn ,int time)
{
	if(is_btn)
	{
		lv_label_set_text(image_mgr.ui.label_dialog_info,message);
		lv_label_set_long_mode(image_mgr.ui.label_dialog_info,LV_LABEL_LONG_EXPAND);
		lv_obj_align(image_mgr.ui.label_dialog_info,NULL,LV_ALIGN_CENTER,0,-30);
		lv_obj_set_event_cb(image_mgr.ui.button_1, photo_dialog_event);
	}
	else
	{
		lv_obj_set_hidden(image_mgr.ui.button_1,1);
		lv_label_set_text(image_mgr.ui.label_dialog_info,message);
		lv_label_set_long_mode(image_mgr.ui.label_dialog_info,LV_LABEL_LONG_EXPAND);
		lv_obj_align(image_mgr.ui.label_dialog_info,NULL,LV_ALIGN_CENTER,0,0);
	}

	lv_obj_set_hidden(image_mgr.ui.container_dialog,0);
	if(time > 0)
	{
		lv_task_t *task  = lv_task_create(dialog_task, time*1000, LV_TASK_PRIO_MID, NULL);
		lv_task_once(task);
	}
}

static void list_label_set_text1(lv_obj_t * list, lv_font_t *font)
{
	lv_obj_t *list_btn[16];
	lv_obj_t *list_label[16];
	unsigned int i = 0;
	const lv_style_t *style_get = NULL;//fix warnning
	static lv_style_t style;
	list_btn[0] = lv_list_get_next_btn(list, NULL);
	while(list_btn[i])
	{
		list_label[i] = lv_list_get_btn_label(list_btn[i]);
		style_get = lv_label_get_style(list_label[i],LV_LABEL_STYLE_MAIN);
		memcpy(&style, style_get, sizeof(lv_style_t));
		style.text.font = font;
		lv_label_set_style(list_label[i], LV_LABEL_STYLE_MAIN, &style);
		lv_label_set_long_mode(list_label[i],LV_LABEL_LONG_CROP);
		// lv_label_set_text(list_label[i], get_text_by_id(text[i]));
		// com_info("list_label[%d] == %s",i,get_text_by_id(text[i]));
		i++;
		list_btn[i] = lv_list_get_next_btn(list, list_btn[i-1]);
	}
}
#if 1
static int font_change(void)
{
	static lv_style_t effetc_btnm_style_rel;
	static lv_style_t effetc_btnm_style_pr;
	static lv_style_t effetc_btnm_style_tgl_rel;
	static lv_style_t effetc_btnm_style_tgl_pr;
	static lv_style_t speed_btnm_style_rel;
	static lv_style_t speed_btnm_style_pr;
	static lv_style_t speed_btnm_style_tgl_rel;
	static lv_style_t speed_btnm_style_tgl_pr;
	// static lv_style_t media_list_style_rel;
	// static lv_style_t media_list_style_pr;
	// static lv_style_t media_list_style_tgl_rel;
	// static lv_style_t media_list_style_tgl_pr;
	static lv_style_t text_style;

	memcpy(&text_style,&lv_style_transp,sizeof(lv_style_t));
	text_style.text.color = lv_color_hex(0xffffff);
	text_style.text.font = get_font_lib()->msyh_20;
	lv_label_set_style(image_mgr.ui.file_name, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_filename, LV_LABEL_STYLE_MAIN, &text_style);
	// lv_label_set_style(image_mgr.ui.label_dialog_info, LV_LABEL_STYLE_MAIN, &text_style);
	// lv_label_set_style(image_mgr.ui.label_dialog_ok, LV_LABEL_STYLE_MAIN, &text_style);

	if(get_language() == LANG_EN)
	{
		return 0;
	}

	memcpy(&effetc_btnm_style_rel,lv_btnm_get_style(image_mgr.ui.button_matrix_effect,LV_BTNM_STYLE_BTN_REL),sizeof(lv_style_t));
	memcpy(&effetc_btnm_style_pr,lv_btnm_get_style(image_mgr.ui.button_matrix_effect,LV_BTNM_STYLE_BTN_PR),sizeof(lv_style_t));
	memcpy(&effetc_btnm_style_tgl_rel,lv_btnm_get_style(image_mgr.ui.button_matrix_effect,LV_BTNM_STYLE_BTN_TGL_REL),sizeof(lv_style_t));
	memcpy(&effetc_btnm_style_tgl_pr,lv_btnm_get_style(image_mgr.ui.button_matrix_effect,LV_BTNM_STYLE_BTN_TGL_PR),sizeof(lv_style_t));
	memcpy(&speed_btnm_style_rel,lv_btnm_get_style(image_mgr.ui.button_matrix_speed,LV_BTNM_STYLE_BTN_REL),sizeof(lv_style_t));
	memcpy(&speed_btnm_style_pr,lv_btnm_get_style(image_mgr.ui.button_matrix_speed,LV_BTNM_STYLE_BTN_PR),sizeof(lv_style_t));
	memcpy(&speed_btnm_style_tgl_rel,lv_btnm_get_style(image_mgr.ui.button_matrix_speed,LV_BTNM_STYLE_BTN_TGL_REL),sizeof(lv_style_t));
	memcpy(&speed_btnm_style_tgl_pr,lv_btnm_get_style(image_mgr.ui.button_matrix_speed,LV_BTNM_STYLE_BTN_TGL_PR),sizeof(lv_style_t));
	// memcpy(&media_list_style_rel,lv_list_get_style(image_mgr.ui.media_list,LV_LIST_STYLE_BTN_REL),sizeof(lv_style_t));
	// memcpy(&media_list_style_pr,lv_list_get_style(image_mgr.ui.media_list,LV_LIST_STYLE_BTN_PR),sizeof(lv_style_t));
	// memcpy(&media_list_style_tgl_rel,lv_list_get_style(image_mgr.ui.media_list,LV_LIST_STYLE_BTN_TGL_REL),sizeof(lv_style_t));
	// memcpy(&media_list_style_tgl_pr,lv_list_get_style(image_mgr.ui.media_list,LV_LIST_STYLE_BTN_TGL_PR),sizeof(lv_style_t));
	memcpy(&text_style,&lv_style_transp,sizeof(lv_style_t));

	text_style.text.color = lv_color_hex(0xffffff);
	text_style.text.font = get_font_lib()->msyh_20;
	effetc_btnm_style_rel.text.font = get_font_lib()->msyh_20;
	effetc_btnm_style_pr.text.font = get_font_lib()->msyh_20;
	effetc_btnm_style_tgl_rel.text.font = get_font_lib()->msyh_20;
	effetc_btnm_style_tgl_pr.text.font = get_font_lib()->msyh_20;
	speed_btnm_style_rel.text.font = get_font_lib()->msyh_20;
	speed_btnm_style_pr.text.font = get_font_lib()->msyh_20;
	speed_btnm_style_tgl_rel.text.font = get_font_lib()->msyh_20;
	speed_btnm_style_tgl_pr.text.font = get_font_lib()->msyh_20;
	// media_list_style_rel.text.font = get_font_lib()->msyh_20;
	// media_list_style_pr.text.font = get_font_lib()->msyh_20;
	// media_list_style_tgl_rel.text.font = get_font_lib()->msyh_20;
	// media_list_style_tgl_pr.text.font = get_font_lib()->msyh_20;

	// static const char *map_button_matrix_effect[] = {
	//	"无特效","随机","淡入淡出","马赛克","\n",
	//	"向上滑动","向下滑动","向左滑动","向右滑动","\n",
	//	"向上展开","向下展开","向左展开","向右展开","\n",
	//	"缩小","放大","水平百叶窗","垂直百叶窗",""
	// };
	static const char *map_button_matrix_effect[20];
	map_button_matrix_effect[0] = get_text_by_id(LANG_PHOTO_EFFECT_NO_EFFECT);
	map_button_matrix_effect[1] = get_text_by_id(LANG_PHOTO_EFFECT_RANDOM);
	map_button_matrix_effect[2] = get_text_by_id(LANG_PHOTO_EFFECT_FADE);
	map_button_matrix_effect[3] = get_text_by_id(LANG_PHOTO_EFFECT_MOSAIC);
	map_button_matrix_effect[4] = "\n";
	map_button_matrix_effect[5] = get_text_by_id(LANG_PHOTO_EFFECT_SLIDE_UP);
	map_button_matrix_effect[6] = get_text_by_id(LANG_PHOTO_EFFECT_SLIDE_DOWN);
	map_button_matrix_effect[7] = get_text_by_id(LANG_PHOTO_EFFECT_SLIDE_LEFT);
	map_button_matrix_effect[8] = get_text_by_id(LANG_PHOTO_EFFECT_SLIDE_RIGHT);
	map_button_matrix_effect[9] = "\n";
	map_button_matrix_effect[10] = get_text_by_id(LANG_PHOTO_EFFECT_STRETCH_UP);
	map_button_matrix_effect[11] = get_text_by_id(LANG_PHOTO_EFFECT_STRETCH_DOWN);
	map_button_matrix_effect[12] = get_text_by_id(LANG_PHOTO_EFFECT_STRETCH_LEFT);
	map_button_matrix_effect[13] = get_text_by_id(LANG_PHOTO_EFFECT_STRETCH_RIGHT);
	map_button_matrix_effect[14] = "\n";
	map_button_matrix_effect[15] = get_text_by_id(LANG_PHOTO_EFFECT_ROOM_IN);
	map_button_matrix_effect[16] = get_text_by_id(LANG_PHOTO_EFFECT_ROOM_OUT);
	map_button_matrix_effect[17] = get_text_by_id(LANG_PHOTO_EFFECT_PERSIAN_H);
	map_button_matrix_effect[18] = get_text_by_id(LANG_PHOTO_EFFECT_PERSIAN_V);
	map_button_matrix_effect[19] = "";

	lv_btnm_set_map(image_mgr.ui.button_matrix_effect, map_button_matrix_effect);

	// static const char *map_button_matrix_speed[] = {
	//	"慢速","正常","快速",""
	// };
	static const char *map_button_matrix_speed[4];
	map_button_matrix_speed[0] = get_text_by_id(LANG_PHOTO_SPEED_SLOW);
	map_button_matrix_speed[1] = get_text_by_id(LANG_PHOTO_SPEED_NORMAL);
	map_button_matrix_speed[2] = get_text_by_id(LANG_PHOTO_SPEED_FAST);
	map_button_matrix_speed[3] = "";

	lv_btnm_set_map(image_mgr.ui.button_matrix_speed, map_button_matrix_speed);

	// lv_label_set_text(image_mgr.ui.file_name, get_text_by_id(LANG_SET_UI_VER));
	// lv_label_set_text(image_mgr.ui.label_speed, get_text_by_id(LANG_SET_UI_VER));
	// lv_label_set_text(image_mgr.ui.label_effect, get_text_by_id(LANG_SET_UI_VER));

	lv_label_set_text(image_mgr.ui.label_info,get_text_by_id(LANG_PHOTO_SHOW_INFO));
	lv_label_set_text(image_mgr.ui.label_title_filename,get_text_by_id(LANG_PHOTO_TITLE_FILENAME));
	lv_label_set_text(image_mgr.ui.label_title_file_size,get_text_by_id(LANG_PHOTO_TITLE_FILESIZE));
	lv_label_set_text(image_mgr.ui.label_title_w_h,get_text_by_id(LANG_PHOTO_TITLE_W_H));
	// lv_label_set_text(image_mgr.ui.label_filename, get_text_by_id(LANG_SET_UI_VER));
	lv_label_set_text(image_mgr.ui.label_title_file_time,get_text_by_id(LANG_PHOTO_TITLE_TIME));
	// lv_label_set_text(image_mgr.ui.label_dialog_info, get_text_by_id(LANG_SET_UI_VER));
	lv_label_set_text(image_mgr.ui.label_dialog_ok,get_text_by_id(LANG_PHOTO_DIALOG_OK));

	lv_btnm_set_style(image_mgr.ui.button_matrix_effect, LV_BTNM_STYLE_BTN_REL, &effetc_btnm_style_rel);
	lv_btnm_set_style(image_mgr.ui.button_matrix_effect, LV_BTNM_STYLE_BTN_PR, &effetc_btnm_style_pr);
	lv_btnm_set_style(image_mgr.ui.button_matrix_effect, LV_BTNM_STYLE_BTN_TGL_REL, &effetc_btnm_style_tgl_rel);
	lv_btnm_set_style(image_mgr.ui.button_matrix_effect, LV_BTNM_STYLE_BTN_TGL_PR, &effetc_btnm_style_tgl_pr);
	lv_btnm_set_style(image_mgr.ui.button_matrix_speed, LV_BTNM_STYLE_BTN_REL,&speed_btnm_style_rel);
	lv_btnm_set_style(image_mgr.ui.button_matrix_speed, LV_BTNM_STYLE_BTN_PR, &speed_btnm_style_pr);
	lv_btnm_set_style(image_mgr.ui.button_matrix_speed, LV_BTNM_STYLE_BTN_TGL_REL,&speed_btnm_style_tgl_rel);
	lv_btnm_set_style(image_mgr.ui.button_matrix_speed, LV_BTNM_STYLE_BTN_TGL_PR, &speed_btnm_style_tgl_pr);

	lv_label_set_style(image_mgr.ui.file_name, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_speed, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_effect, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_info, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_title_filename, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_title_file_size, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_title_w_h, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_filename, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_title_file_time, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_dialog_info, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_style(image_mgr.ui.label_dialog_ok, LV_LABEL_STYLE_MAIN, &text_style);


	// lv_list_set_style(image_mgr.ui.media_list, LV_LIST_STYLE_BTN_REL, &media_list_style_rel);
	// lv_list_set_style(image_mgr.ui.media_list, LV_LIST_STYLE_BTN_PR, &media_list_style_pr);
	// lv_list_set_style(image_mgr.ui.media_list, LV_LIST_STYLE_BTN_TGL_REL, &media_list_style_tgl_rel);
	// lv_list_set_style(image_mgr.ui.media_list, LV_LIST_STYLE_BTN_TGL_PR, &media_list_style_tgl_pr);

	// list_label_set_text1(image_mgr.ui.media_list,get_font_lib()->msyh_20);
	return 0;
}
#endif //CONFIG_FONT_ENABLE

static int photo_read_breaktag(photo_app_data_t *app_data)
{
	int ret;
	ret = read_int_type_param(PHOTO_SCENE, "speed", &app_data->speed);
	if(ret<0)
	{
		app_data->speed = 1;
	}
	ret = read_int_type_param(PHOTO_SCENE, "effect", &app_data->effect);
	if(ret<0)
	{
		app_data->effect = IMAGE_SHOW_NORMAL;
	}
	ret = read_string_type_param(PHOTO_SCENE, "filename", app_data->filename,sizeof(app_data->filename));
	if(ret < 0)
	{
		memset(app_data->filename,0,strlen(app_data->filename));
	}
	ret = read_string_type_param(PHOTO_SCENE, "mountpoint", app_data->mountpoint,sizeof(app_data->mountpoint));
	if(ret < 0)
	{
		memset(app_data->mountpoint,0,strlen(app_data->mountpoint));
	}
	printf("read effect = %d speed = %d \n",app_data->effect,app_data->speed);
	return 0;
}

static int photo_write_breaktag(photo_app_data_t *app_data)
{
	int ret;
	ret = write_int_type_param(PHOTO_SCENE, "speed", app_data->speed);
	if(ret < 0)
	{
		printf("photo save speed err!\n");
	}
	ret = write_int_type_param(PHOTO_SCENE, "effect", app_data->effect);
	if(ret < 0)
	{
		printf("photo save effect err!\n");
	}
	ret = write_string_type_param(PHOTO_SCENE, "filename", app_data->filename,strlen(app_data->filename));
	if(ret < 0)
	{
		printf("photo save filename err!\n");
	}
	ret = write_string_type_param(PHOTO_SCENE, "mountpoint", app_data->mountpoint,strlen(app_data->mountpoint));
	if(ret < 0)
	{
		printf("photo save filename err!\n");
	}
	return 0;
}

static int create_photo(void)
{
	int ret;

	memset(&image_mgr, 0, sizeof(image_mgr));
	image_mgr.ui.cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(image_mgr.ui.cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color = LV_COLOR_BLUE;
	cont_style.body.grad_color = LV_COLOR_BLUE;
	lv_cont_set_style(image_mgr.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
	lv_cont_set_layout(image_mgr.ui.cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(image_mgr.ui.cont, LV_FIT_NONE);
	photo_auto_ui_create(&image_mgr.ui);

	lv_obj_t *scn = lv_scr_act();					// clear screen for alpha setting
	static lv_style_t scn_style;
	lv_style_copy(&scn_style, &lv_style_plain);
	scn_style.body.main_color.full = 0x00000000;
	scn_style.body.grad_color.full = 0x00000000;
	lv_obj_set_style(scn, &scn_style);

	cont_style.body.main_color = LV_COLOR_WHITE;
	cont_style.body.grad_color = LV_COLOR_WHITE;
//	cont_style.body.main_color.full = 0x00000000;	// clear container for alpha setting
//	cont_style.body.grad_color.full = 0x00000000;
	cont_style.body.opa = 0;
	lv_cont_set_style(image_mgr.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
	lv_obj_move_background(image_mgr.ui.button_setting_interface);
//	lv_cont_set_layout(image_mgr.ui.button_setting_interface, LV_LAYOUT_ROW_M);
	lv_obj_set_hidden(image_mgr.ui.button_setting_interface, 1);
	lv_obj_set_event_cb(image_mgr.ui.back, back_btn_event);
	lv_obj_set_event_cb(image_mgr.ui.image_button_preview, image_preview_full_screen_event);
	lv_obj_set_event_cb(image_mgr.ui.button_setting_interface, button_setting_interface_event);
	lv_obj_set_event_cb(image_mgr.ui.image_button_rotate_right, image_rotate_event);
	lv_obj_set_event_cb(image_mgr.ui.image_button_rotate_left, image_rotate_event);
	lv_obj_set_event_cb(image_mgr.ui.image_button_scaler_up, image_scaler_event);
	lv_obj_set_event_cb(image_mgr.ui.image_button_scaler_down, image_scaler_event);
	lv_obj_set_event_cb(image_mgr.ui.image_button_play_start, image_play_event);
	lv_obj_set_event_cb(image_mgr.ui.button_show_effect, btn_effect_event);
	lv_obj_set_event_cb(image_mgr.ui.button_show_speed, btn_speed_event);
	lv_obj_set_event_cb(image_mgr.ui.button_info, btn_info_event);
	lv_obj_set_event_cb(image_mgr.ui.image_set_effect, image_confirm_event);
	lv_obj_set_event_cb(image_mgr.ui.image_set_speed, image_confirm_event);
	lv_obj_set_event_cb(image_mgr.ui.image_get_info, image_confirm_event);
	lv_obj_set_event_cb(image_mgr.ui.button_matrix_effect, btnm_click_event);
	lv_obj_set_event_cb(image_mgr.ui.button_matrix_speed, btnm_click_event);

	photo_read_breaktag(&image_mgr.app_data);//尝试读取断点信息
	// media_file_list = media_get_file_list(RAT_MEDIA_TYPE_PIC);
	photo_user_data_t *user_data = get_page_user_data(PAGE_PHOTO);//尝试获取用户数据
	if(user_data)
	{
		media_file_list = media_load_file(RAT_MEDIA_TYPE_PIC,user_data->mountpoint);//尝试搜索用户数据中指定的挂载点
		strcpy(image_mgr.app_data.mountpoint,user_data->mountpoint);
	}
	else
	{
		media_file_list = media_load_file(RAT_MEDIA_TYPE_PIC,image_mgr.app_data.mountpoint);//尝试搜索断点信息存储的挂载点
		if(media_file_list == NULL)
		{
			int i;
			int disk_num;
			disk_num = DiskManager_GetDiskNum();
			for (i = 0; i < disk_num; i++) //尝试搜索现在所有的挂载点
			{
				DiskInfo_t *disk_tmp = NULL;
				disk_tmp = DiskManager_GetDiskInfoByIndex(i);
				media_file_list = media_load_file(RAT_MEDIA_TYPE_PIC, disk_tmp->MountPoint);
				if (NULL != media_file_list)
				{
					strcpy(image_mgr.app_data.mountpoint,disk_tmp->MountPoint);
					break;
				}
			}
		}
	}


#if CONFIG_FONT_ENABLE
	font_change();
#endif //CONFIG_FONT_ENABLE
	if (media_file_list == NULL) {
#if CONFIG_FONT_ENABLE
		photo_show_dialog(get_text_by_id(LANG_PHOTO_DIALOG_INFO),0,1);
#else
		photo_show_dialog("No picture!",1,1);
#endif
		image_mgr.is_early_exit = 1;
		return -1;
	}
	media_update_file_list(image_mgr.ui.media_list,media_file_list,image_list_event);

	if(user_data && user_data->index >= 0)
	{
		image_mgr.file_index = user_data->index;
		image_mgr.file_path  = user_data->filename;
		media_set_list_focus(image_mgr.ui.media_list,user_data->index);
	}
	else
	{
		image_mgr.file_index = 0;
		image_mgr.file_path  = media_get_file_path(media_file_list,image_mgr.file_index);
		media_set_list_focus(image_mgr.ui.media_list,0);
	}
	if(user_data)
	{
		image_mgr.is_auto_play = user_data->is_auto_play;
	}
	image_mgr.setting_interface_hidden = 1;
	image_mgr.file_total_num = media_file_list->total_num;
	// image_mgr.file_index = 0;
	// image_mgr.file_path = media_get_file_path(media_file_list,image_mgr.file_index);
	lv_obj_set_ext_click_area(image_mgr.ui.back,10,10,10,10);

	lv_btnm_set_btn_ctrl_all(image_mgr.ui.button_matrix_effect,LV_BTNM_CTRL_TGL_ENABLE);
	lv_btnm_set_btn_ctrl_all(image_mgr.ui.button_matrix_speed,LV_BTNM_CTRL_TGL_ENABLE);
	lv_btnm_set_one_toggle(image_mgr.ui.button_matrix_effect,true);
	lv_btnm_set_one_toggle(image_mgr.ui.button_matrix_speed,true);

	lv_obj_set_click(image_mgr.ui.image_set_effect,true);
	lv_obj_set_click(image_mgr.ui.image_set_speed,true);
	lv_obj_set_click(image_mgr.ui.image_get_info,true);

	image_mgr.effect = image_mgr.app_data.effect;
	image_mgr.speed  = image_mgr.app_data.speed;;
	// printf("effect = %d speed = %d %d\n",image_mgr.effect,image_mgr.speed,speed2id(image_mgr.speed));

	lv_btnm_set_pressed(image_mgr.ui.button_matrix_effect,image_mgr.effect);
	lv_btnm_set_pressed(image_mgr.ui.button_matrix_speed,speed2id(image_mgr.speed));
	lv_btnm_set_btn_ctrl(image_mgr.ui.button_matrix_effect,image_mgr.effect,LV_BTNM_CTRL_TGL_STATE);
	lv_btnm_set_btn_ctrl(image_mgr.ui.button_matrix_speed,speed2id(image_mgr.speed),LV_BTNM_CTRL_TGL_STATE);
	lv_label_set_text(image_mgr.ui.label_effect,lv_btnm_get_btn_text(image_mgr.ui.button_matrix_effect,image_mgr.effect));
	lv_label_set_text(image_mgr.ui.label_speed,lv_btnm_get_btn_text(image_mgr.ui.button_matrix_speed,speed2id(image_mgr.speed)));

	image_init();
	if(!image_mgr.is_auto_play)
	{
		image_thumb_window_set();
		image_show(image_mgr.file_path, IMAGE_FULL_SCREEN_SCLAER);
	}

	pthread_mutex_init(&image_busy_mutex, NULL);
	pthread_mutex_init(&image_wait_mutex, NULL);
	pthread_cond_init(&image_wait_cond, NULL);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr,0x4000);

	ret = pthread_create(&image_mgr.auto_play_id, NULL, image_auto_play_pthread, NULL);
	if(ret < 0)
	{
		com_err("create thread auto play err!\n");
	}

	lv_label_set_text(image_mgr.ui.file_name, media_get_path_to_name(image_mgr.file_path));


	strcpy(image_mgr.app_data.filename,image_mgr.file_path);
	photo_write_breaktag(&image_mgr.app_data);

	if(image_mgr.is_auto_play)
	{
		lv_event_send(image_mgr.ui.image_button_preview,LV_EVENT_CLICKED,NULL);
	}
	if(user_data)
	{
		free(user_data);
		set_page_user_data(PAGE_PHOTO,NULL);
	}

	return 0;
}

static int destory_photo(void)
{
	photo_auto_ui_destory(&image_mgr.ui);
	lv_obj_del(image_mgr.ui.cont);

	if(image_mgr.is_early_exit == 1)
	{
		return 0;
	}

	if(image_mgr.play_state == IMAGE_PLAY_START)
	{
		image_mgr.play_state = IMAGE_PLAY_STOP;
	}
	while(1)
	{
		pthread_mutex_lock(&image_busy_mutex);
		if(image_mgr.is_busy == 0)
		{
			com_info("we get busy!!!\n");
			break;
		}
		pthread_mutex_unlock(&image_busy_mutex);
		usleep(100);
	}
	image_play_thread_up();
	pthread_join(image_mgr.auto_play_id, NULL);

	image_exit();
	media_unload_file(media_file_list);
	photo_write_breaktag(&image_mgr.app_data);

	return 0;
}

static int show_photo(void)
{
	lv_obj_set_hidden(image_mgr.ui.cont, 0);

	return 0;
}

static int hide_photo(void)
{
	lv_obj_set_hidden(image_mgr.ui.cont, 1);

	return 0;
}

static int msg_proc_photo(MsgDataInfo *msg)
{
	return 0;
}

static page_interface_t page_photo =
{
	.ops =
	{
		create_photo,
		destory_photo,
		show_photo,
		hide_photo,
		msg_proc_photo,
	},
	.info =
	{
		.id         = PAGE_PHOTO,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_PHOTO(void)
{
	reg_page(&page_photo);
}
