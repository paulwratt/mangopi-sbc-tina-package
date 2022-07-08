#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <malloc.h>

#include "lvgl/lvgl.h"
#include "common.h"
#include "bs_widget.h"
#include "app_config_interface.h"


pthread_t tick_id;
lv_task_t *page_monitor_task_id;

void *tick_thread(void * data)
{
    (void)data;

    while(1) {
        usleep(10000);   /*Sleep for 10 millisecond*/
        lv_tick_inc(10); /*Tell LittelvGL that 10 milliseconds were elapsed*/
    }

    return 0;
}


static void page_monitor_task(lv_task_t *task)
{
	(void)task;
	update_page();
}
static void hal_init(void)
{
	int ret;

	/*Linux frame buffer device init*/
    fbdev_init();
    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[LV_HOR_RES_MAX*LV_VER_RES_MAX];
    /*Initialize a descriptor for the buffer*/
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX*LV_VER_RES_MAX);
	//printf("L:%d F=%s: buf=%x\n", __LINE__, __FUNCTION__, buf);

    /*Initialize and register a display driver*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer   = &disp_buf;
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

#if USE_SUNXI_KEY
	keydev_init();
    lv_indev_drv_t key_drv;
    lv_indev_drv_init(&key_drv);
    key_drv.type = LV_INDEV_TYPE_KEYPAD;
    key_drv.read_cb = keydev_read;
    lv_indev_t *key_dev = lv_indev_drv_register(&key_drv);
	lv_group_t *g = key_group_create();
	lv_indev_set_group(key_dev, g);
#endif

	ret = pthread_create(&tick_id, NULL, tick_thread, NULL);
	if (ret == -1) {
		printf("create thread fail\n");
		return ;
	}

    //lv_task_create(memory_monitor, 3000, LV_TASK_PRIO_MID, NULL);
}

static void hal_uninit(void) {
	pthread_join(tick_id, NULL);
	fbdev_exit();
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
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
	int param;

	/* audio */
	read_int_type_param(PUBLIC_SCENE, VOLUME, &param);
}

void va_driver_uninit(void)
{
	;
}

int main(void)
{
	DiskInfo_t Static_DeviceInfo;
	extern int ota_check_file(char *file_path);
	//lv_task_t *task_id;
	int start, end, inter;
	com_info("####################");
	com_info("####################");
	com_info("####################");
	com_info("main start V1.0");

	app_config_param_init(0);
	middle_ware_init();
	va_driver_init();
	app_param_effect(0);

	memset(&Static_DeviceInfo, 0x00, sizeof(Static_DeviceInfo));
	strncpy(Static_DeviceInfo.DeviceName, "/dev/ubi0_8", strlen("/dev/ubi0_8"));
	strncpy(Static_DeviceInfo.MountPoint, "/mnt/UDISK", strlen("/mnt/UDISK"));
	DiskManager_Register_StaticDisk(&Static_DeviceInfo);
	DiskManager_detect();

	REG_PAGE(PAGE_EXPLORER);
	REG_PAGE(PAGE_OTA);
	if (access(getenv("swu_software"), F_OK|R_OK) == 0 && ota_check_file(getenv("swu_software")) == 0) {
		printf("%s %d %s %s\n", __FILE__, __LINE__, __func__, getenv("swu_software"));
		create_page(PAGE_OTA);
	} else {
		create_page(PAGE_EXPLORER);
	}
	page_monitor_task_id = lv_task_create(page_monitor_task, 5, LV_TASK_PRIO_HIGH, NULL);
	/* main loop */
    while(1)
	{
		start = lv_tick_get();
        lv_task_handler();
		end = lv_tick_get();

		#if 1
		/* Adjust the sleep time according to the main task processing time */
		inter = end - start;
		if (inter >= 40)
			;
		else if(inter >= 20)
			usleep(5000);
		else
			usleep(10000);
		#endif
    }
	middle_ware_uninit();
	va_driver_uninit();
	va_save_param_init();
	va_power_power_off();
    return 0;
}
