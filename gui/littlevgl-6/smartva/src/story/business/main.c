#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <malloc.h>

#include "lvgl/lvgl.h"
#include "common.h"
#include "bs_widget.h"
#include "ui_timer.h"
#include "app_config_interface.h"

pthread_t tick_id;
lv_task_t *main_task_id;
static hotplug_message_focus_win_t *RegisterInfo = NULL;

void *tick_thread(void *data)
{
	(void)data;

	while (1) {
		usleep(10000); /*Sleep for 10 millisecond*/
		lv_tick_inc(
			10); /*Tell LittelvGL that 10 milliseconds were elapsed*/
	}

	return 0;
}

void update_wifi_info(void)
{
	wifi_data_t wifi;
	int is_wifi_connected = 0;
	connection_status info = { 0 };
	static int wifi_count = 0;

	// update wifi rssi each 500ms
	if ((++wifi_count) >= 5) {
		wifi_count = 0;

		get_wifi_data(&wifi);
		if (wifi.is_on == 1) {
			if (net_wifi_get_wifi_state() == NETWORK_CONNECTED) {
				is_wifi_connected = 1;
			} else {
				is_wifi_connected = 0;
			}

			wifi.is_connected = is_wifi_connected;
			if (is_wifi_connected) {
				memset(&info, 0, sizeof(connection_status));
				net_wifi_get_connect_info(wifi.p_wifi_hd,
							  &info);

				wifi.rssi = 100 + info.rssi;
				memcpy(wifi.ssid, info.ssid, strlen(info.ssid));
			} else {
				wifi.rssi = 0;
				memset(wifi.ssid, 0, sizeof(wifi.ssid));
			}
		}
		set_wifi_data(&wifi);
	}
}
static void check_backlight_status_and_reset_timer(void) {
	if (0 == va_display_lcd_backlight_status()) {
		com_info("open lcd backlight");
		va_display_lcd_backlight_onoff(1);
	}
	reset_default_timer();
}
static void backlight_auto_ctl(void)
{
	int inactive_time = lv_disp_get_inactive_time(NULL) / 100;

	if (inactive_time == 1) {
		check_backlight_status_and_reset_timer();
	}
	return;
}

static void main_task(lv_task_t *task)
{
	(void)task;
	update_wifi_info();
	backlight_auto_ctl();
	volume_ctl_hide();
}

void *delay_process_thread(void *arg)
{
	int ret;
	int param;
	wifi_data_t wifi;

	/* 待主界面显示之后联网 */
	sleep(5); //sleep 5s
	memset(&wifi, 0, sizeof(wifi_data_t));
	read_int_type_param(WLAN_SCENE, WLAN_MANU_ON, &param);
	wifi.manu_on = (bool)param;

	if (wifi.manu_on) {
		read_int_type_param(WLAN_SCENE, WLAN_MANU_CONNECTED, &param);
		wifi.manu_connected = (bool)param;
		read_string_type_param(WLAN_SCENE, WLAN_MANU_SSID, wifi.manu_ssid,
					   sizeof(wifi.manu_ssid));
		read_string_type_param(WLAN_SCENE, WLAN_MANU_PASSWORD, wifi.manu_password,
					   sizeof(wifi.manu_password));
		wifi.event_label = rand();
		wifi.p_wifi_hd = net_wifi_on(wifi.event_label);
		if (wifi.p_wifi_hd != NULL) {
			wifi.is_on = 1;
		} else {
			wifi.is_on = 0;
		}
	} else {
		wifi.is_on = 0;
	}

	wifi.update_flag = 1;
	set_wifi_data(&wifi);

	if (wifi.is_on && wifi.manu_connected) {
		ret = net_wifi_connect_ap(wifi.p_wifi_hd, wifi.manu_ssid,
					  wifi.manu_password, wifi.event_label);
		if (ret == 1) {
			wifi.is_connected = 1;
		} else {
			wifi.is_connected = 0;
		}
	}
	set_wifi_data(&wifi);
	pthread_exit(NULL);
}

#if 0
static void memory_monitor(lv_task_t * param)
{
    (void) param; /*Unused*/

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n", (int)mon.total_size - mon.free_size,
            mon.used_pct,
            mon.frag_pct,
            (int)mon.free_biggest_size);

}
#endif

/* deal key which lvgl not supported */
#if USE_SUNXI_KEY
void global_key_callback(uint32_t key, lv_indev_state_t state)
{
    MsgDataInfo msg;
	// pmsg("(%d, %d)\n", key, state);
	msg.status = state;
    msg.type = MSG_KEY;
    msg.to = current_page();
    msg.value = key;
    msg.used = 1;
    sent_msg(msg);
	return;
}
#endif

static void hal_init(void)
{
	int ret;

	/*Linux frame buffer device init*/
	fbdev_init();
	/*A small buffer for LittlevGL to draw the screen's content*/
	static lv_color_t buf[LV_HOR_RES_MAX * LV_VER_RES_MAX];
	/*Initialize a descriptor for the buffer*/
	static lv_disp_buf_t disp_buf;
	lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);
	//printf("L:%d F=%s: buf=%x\n", __LINE__, __FUNCTION__, buf);

	/*Initialize and register a display driver*/
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.buffer = &disp_buf;
	disp_drv.flush_cb = fbdev_flush;
	lv_disp_drv_register(&disp_drv);

#ifdef __CTP_USE__
	ctpdev_init();
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = ctpdev_read;
	lv_indev_drv_register(&indev_drv);
#endif

#ifdef __RTP_USE__
	rtpdev_init();
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = rtpdev_read;
	lv_indev_drv_register(&indev_drv);
#endif

#if USE_SUNXI_KEY
	keydev_init();
	lv_indev_drv_t key_drv;
	lv_indev_drv_init(&key_drv);
	key_drv.type = LV_INDEV_TYPE_KEYPAD;
	key_drv.read_cb = keydev_read;
	lv_indev_t *key_dev = lv_indev_drv_register(&key_drv);
	keydev_register_hook(global_key_callback);
	lv_group_t *g = key_group_create();
	lv_indev_set_group(key_dev, g);
#endif

	ret = pthread_create(&tick_id, NULL, tick_thread, NULL);
	if (ret == -1) {
		printf("create thread fail\n");
		return;
	}

	//lv_task_create(memory_monitor, 3000, LV_TASK_PRIO_MID, NULL);
}

static void hal_uninit(void)
{
	pthread_join(tick_id, NULL);

#if USE_SUNXI_KEY
	key_group_del();
	keydev_uninit();
#endif

	fbdev_exit();
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
	static uint64_t start_ms = 0;
	if (start_ms == 0) {
		struct timeval tv_start;
		gettimeofday(&tv_start, NULL);
		start_ms =
			(tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
	}

	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	uint64_t now_ms;
	now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

	uint32_t time_ms = now_ms - start_ms;
	return time_ms;
}

void middle_ware_init(void)
{
	int param;

	param = 0;
	lv_init();
	hal_init();
	page_init();
	rat_init();
	DiskManager_Init();
	va_audio_init();
	va_audio_play_init();
	timer_init();
	#if CONFIG_FONT_ENABLE
	font_init();
	#endif

}

void middle_ware_uninit(void)
{
#if CONFIG_FONT_ENABLE
	lang_and_text_uninit();
	font_uninit();
#endif
	rat_deinit();
	page_uninit();
	hal_uninit();
#if LV_ENABLE_GC || !LV_MEM_CUSTOM
	lv_deinit();
#endif
}

void va_driver_init(void)
{
	;
}

void va_driver_uninit(void)
{
	;
}

static int root_key_proc(MsgDataInfo *msg)
{
	// com_info("key: %d state:%d",msg->value,msg->status);
	if(msg->status == 0)//keyup
		return 0;

	switch (msg->value)
	{
	case LV_KEY_VOLUME_DOWN:
		// com_info("LV_KEY_VOLUME_DOWN");
		volume_ctl('-');
		break;
	case LV_KEY_VOLUME_UP:
		// com_info("LV_KEY_VOLUME_UP");
		volume_ctl('+');
		break;

	default:
		break;
	}
	return 0;
}

static void main_diskhotplugcallback(DiskInfo_t *DiskInfo) {
	check_backlight_status_and_reset_timer();
}

int root_msg_callback(MsgDataInfo *msg)
{
	switch (msg->type)
	{
	case MSG_KEY:
		//com_info("get a key msg val(%d) status(%d)",msg->value,msg->status);
		if(msg->status)
			root_key_proc(msg);
		break;

	default:
		break;
	}

	if(msg->to == PAGE_NONE)
		return 0;

	sent_page_msg(msg);
	return 0;
}

int main(void)
{
	//lv_task_t *task_id;
#ifdef CONFIG_F133
	mallopt(M_TRIM_THRESHOLD, 64*1024);
#endif

	DiskInfo_t Static_DeviceInfo;
	pthread_t task_id;
	pthread_attr_t attr;
	int start, end, inter;
	com_info("####################");
	com_info("####################");
	com_info("####################");
	com_info("main start V1.0");
	app_config_param_init(0);
	middle_ware_init();

	va_driver_init();
	wifi_data_init();
	message_sys_init();
	init_root_callback(root_msg_callback);
	app_param_effect(0);
	
	REG_PAGE(PAGE_HOME);
	REG_PAGE(PAGE_MUSIC);
	REG_PAGE(PAGE_MOVIE);
	REG_PAGE(PAGE_SOUND);
	REG_PAGE(PAGE_CAMERA);
	REG_PAGE(PAGE_CALCULATOR);
	REG_PAGE(PAGE_CALENDAR);
	REG_PAGE(PAGE_EXPLORER);
	REG_PAGE(PAGE_FOLDER);
	REG_PAGE(PAGE_PHOTO);
	REG_PAGE(PAGE_SETTING);
	REG_PAGE(PAGE_EXAMPLE);
	REG_PAGE(PAGE_WLAN_SET1);
	//REG_PAGE(PAGE_WLAN_SET2);
	REG_PAGE(PAGE_SLIDE_HOME);
	REG_PAGE(PAGE_OTA);
	create_page(PAGE_HOME);
	//demo_create();
	//create_page(PAGE_SLIDE_HOME);
	
	RegisterInfo = malloc(sizeof(hotplug_message_focus_win_t));
	if (RegisterInfo != NULL) {
		memset(RegisterInfo, 0x00, sizeof(hotplug_message_focus_win_t));
		strcpy(RegisterInfo->Cur_Win, "main");
		RegisterInfo->CallBackFunction = main_diskhotplugcallback;
		DiskManager_Register(RegisterInfo);
	}
	media_init();
	memset(&Static_DeviceInfo, 0x00, sizeof(Static_DeviceInfo));
	Static_DeviceInfo.MediaType = MEDIUM_LOCAL_DISK;
	strncpy(Static_DeviceInfo.DeviceName, "/dev/ubi0_8",
		strlen("/dev/by-name/UDISK") + 1);
	strncpy(Static_DeviceInfo.MountPoint, "/mnt/UDISK",
		strlen("/mnt/UDISK") + 1);
	DiskManager_Register_StaticDisk(&Static_DeviceInfo);
	DiskManager_detect();
	/* main task It is used to handle common business */
	main_task_id = lv_task_create(main_task, 100, LV_TASK_PRIO_MID, NULL);
	volume_ctl_create();
	
	// 待显示主界面完联网
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x40000);
	pthread_create(&task_id, &attr, delay_process_thread, NULL);
	pthread_attr_destroy(&attr);
	/* main loop */
	while (1) {
		start = lv_tick_get();
		lv_task_handler();
		end = lv_tick_get();

#if 1
		/* Adjust the sleep time according to the main task processing time */
		inter = end - start;
		if (inter >= 40)
			;
		else if (inter >= 20)
			usleep(5000);
		else
			usleep(10000);
#endif
	}
	volume_ctl_destory();
	lv_task_del(main_task_id);
	media_uninit(media_get_player_data());
	message_sys_unit();
	wifi_data_uninit();

	middle_ware_uninit();
	va_driver_uninit();
	va_save_param_init();
	va_power_power_off();
	return 0;
}
