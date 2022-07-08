#include "va_display.h"

typedef enum{
	DISP_IOCTL_ARG_OUT_SRC,
	DISP_IOCTL_ARG_OUT_LAYER,
	DISP_IOCTL_ARG_OUT_LAYER_PARAM,
	DISP_IOCTL_ARG_DUMMY,
	DISP_IOCTL_ARG_MAX
}DISP_IOCTL_ARG;

typedef enum{
	DISP_OUT_SRC_SEL_LCD = 0x00,
	DISP_OUT_SRC_SEL_TV,
	DISP_OUT_SRC_SEL_LVDS,
	DISP_OUT_SRC_SEL_HDMI,
	DISP_OUT_SRC_SEL_MAX
}DISP_OUT_SRC_SEL;


#define DISP_CLAR(x)  memset(x, 0, sizeof(x))
#define UI_LAYER_NUM	(3)
#define DISP_DEV_NAME "/dev/disp"
static int disp_fd = 0;
static unsigned long int ioctlParam[DISP_IOCTL_ARG_MAX];
static bool bl_status;

static int va_display_device_init(void)
{
	disp_fd = open(DISP_DEV_NAME, O_RDWR);
	if (disp_fd == -1)
	{
		printf("%s:open disp handle is not exist!\r\n", __func__);
		return -1;
	}
	return 0;
}
static int va_display_device_exit(void)
{
	close(disp_fd);
	return 0;
}

int va_display_get_lcd_rect(struct disp_rectsz *rect)
{
	va_display_device_init();
	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = 0;
#ifdef __SUNXI_DISPLAY2__
	rect->width = ioctl(disp_fd, DISP_GET_SCN_WIDTH, ioctlParam);
#else
	rect->width = ioctl(disp_fd, DISP_CMD_GET_SCN_WIDTH, ioctlParam);
#endif
	DISP_CLAR(ioctlParam);
	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = 0;

#ifdef __SUNXI_DISPLAY2__
	rect->height = ioctl(disp_fd, DISP_GET_SCN_HEIGHT, ioctlParam);
#else
	rect->height = ioctl(disp_fd, DISP_CMD_GET_SCN_HEIGHT, ioctlParam);
#endif
	va_display_device_exit();
	return 0;
}

int va_display_set_enhance_mode(uint16_t val)
{
	char buff[64];
	memset(buff, 0, sizeof(buff));
	system("echo 0 > /sys/class/disp/disp/attr/disp");
	sprintf(buff,"echo %d > /sys/class/disp/disp/attr/enhance_mode",val);
	system(buff);
	return 0;
}
int va_display_get_enhance_mode(void)
{
	char buff[64];
	int val;
	memset(buff, 0, sizeof(buff));
	int fd = open("/sys/class/disp/disp/attr/enhance_mode",O_RDONLY);
	read(fd, buff, sizeof(buff));
	com_err("buff = %s",buff);
	close(fd);
	val = atol(buff);
	return val;
}

int va_display_enhance_set_saturation(int16_t val)
{
	char buff[64];
	memset(buff, 0, sizeof(buff));
	system("echo 0 > /sys/class/disp/disp/attr/disp");
	sprintf(buff,"echo %d > /sys/class/disp/disp/attr/enhance_saturation",val);
	system(buff);
	return 0;
}

int va_display_enhance_get_saturation(void)
{
	char buff[64];
	int val;
	memset(buff, 0, sizeof(buff));
	int fd = open("/sys/class/disp/disp/attr/enhance_saturation",O_RDONLY);
	read(fd, buff, sizeof(buff));
	com_err("buff = %s",buff);
	close(fd);
	val = atol(buff);
	return val;
}

int va_display_enhance_set_bright(int16_t val)
{
	char buff[64];
	memset(buff, 0, sizeof(buff));
	system("echo 0 > /sys/class/disp/disp/attr/disp");
	sprintf(buff,"echo %d > /sys/class/disp/disp/attr/enhance_bright",val);
	system(buff);
	return 0;
}

int va_display_enhance_get_bright(void)
{
	char buff[64];
	int val;
	memset(buff, 0, sizeof(buff));
	int fd = open("/sys/class/disp/disp/attr/enhance_bright",O_RDONLY);
	read(fd, buff, sizeof(buff));
	com_err("buff = %s",buff);
	close(fd);
	val = atol(buff);
	return val;
}

int va_display_enhance_set_contrast(int16_t val)
{
	char buff[64];
	memset(buff, 0, sizeof(buff));
	system("echo 0 > /sys/class/disp/disp/attr/disp");
	sprintf(buff,"echo %d > /sys/class/disp/disp/attr/enhance_contrast",val);
	system(buff);
	return 0;
}

int va_display_enhance_get_contrast(void)
{
	char buff[64];
	int val;
	memset(buff, 0, sizeof(buff));
	int fd = open("/sys/class/disp/disp/attr/enhance_contrast",O_RDONLY);
	read(fd, buff, sizeof(buff));
	com_err("buff = %s",buff);
	close(fd);
	val = atol(buff);
	return val;
}

int va_display_enhance_set_edge(int16_t val)
{
	char buff[64];
	memset(buff, 0, sizeof(buff));
	system("echo 0 > /sys/class/disp/disp/attr/disp");
	sprintf(buff,"echo %d > /sys/class/disp/disp/attr/enhance_edge",val);
	system(buff);
	return 0;
}

int va_display_enhance_get_edge(void)
{
	char buff[64];
	int val;
	memset(buff, 0, sizeof(buff));
	int fd = open("/sys/class/disp/disp/attr/enhance_edge",O_RDONLY);
	read(fd, buff, sizeof(buff));
	com_err("buff = %s",buff);
	close(fd);
	val = atol(buff);
	return val;
}

int va_display_enhance_set_detail(int16_t val)
{
	char buff[64];
	memset(buff, 0, sizeof(buff));
	system("echo 0 > /sys/class/disp/disp/attr/disp");
	sprintf(buff,"echo %d > /sys/class/disp/disp/attr/enhance_detail",val);
	system(buff);
	return 0;
}

int va_display_enhance_get_detail(void)
{
	char buff[64];
	int val;
	memset(buff, 0, sizeof(buff));
	int fd = open("/sys/class/disp/disp/attr/enhance_detail",O_RDONLY);
	read(fd, buff, sizeof(buff));
	com_err("buff = %s",buff);
	close(fd);
	val = atol(buff);
	return val;
}

int va_display_enhance_set_denoise(int16_t val)
{
	char buff[64];
	memset(buff, 0, sizeof(buff));
	system("echo 0 > /sys/class/disp/disp/attr/disp");
	sprintf(buff,"echo %d > /sys/class/disp/disp/attr/enhance_denoise",val);
	system(buff);
	return 0;
}

int va_display_enhance_get_denoise(void)
{
	char buff[64];
	int val;
	memset(buff, 0, sizeof(buff));
	int fd = open("/sys/class/disp/disp/attr/enhance_denoise",O_RDONLY);
	read(fd, buff, sizeof(buff));
	com_err("buff = %s",buff);
	close(fd);
	val = atol(buff);
	return val;
}

int va_display_lcd_set_backlight(int16_t val)
{
	int ret = 0;
	val = val * 250 / 100 + 1;//¡À¨¹?a¦Ì¡Â¦Ì??¨¢??¨¨?o¨²
	va_display_device_init();
	DISP_CLAR(ioctlParam);
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = val;
#ifdef __SUNXI_DISPLAY2__
		ret = ioctl(disp_fd, DISP_LCD_SET_BRIGHTNESS, (void*)ioctlParam);
#else
		ret = ioctl(disp_fd, DISP_CMD_LCD_SET_BRIGHTNESS, (void*)ioctlParam);
#endif
	if (ret < 0)
	{
		printf("%s: fail to set backlight!\r\n", __func__);
	}
	va_display_device_exit();
	return 0;
}

int va_display_lcd_get_backlight(void)
{
	int ret = 0;

	va_display_device_init();
	DISP_CLAR(ioctlParam);

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
#ifdef __SUNXI_DISPLAY2__
		ret = ioctl(disp_fd, DISP_LCD_GET_BRIGHTNESS, (void*)ioctlParam);
#else
		ret = ioctl(disp_fd, DISP_CMD_LCD_GET_BRIGHTNESS, (void*)ioctlParam);
#endif
	if (ret < 0)
	{
		printf("%s: fail to GET backlight!\r\n", __func__);
	}
	//printf("get brightness = %d\r\n", ret);
	va_display_device_exit();
	ret = ret * 100 / 255;
	return ret;
}

bool va_display_lcd_backlight_status(void)
{
	return bl_status;
}

int va_display_lcd_backlight_onoff(bool onoff)
{
	int ret = 0;
	unsigned int cmd = {0};

	va_display_device_init();
	DISP_CLAR(ioctlParam);

	if (onoff) {
#ifdef __SUNXI_DISPLAY2__
		cmd = DISP_LCD_BACKLIGHT_ENABLE;
#else
		cmd = DISP_CMD_LCD_BACKLIGHT_ENABLE;
#endif
	} else {
#ifdef __SUNXI_DISPLAY2__
		cmd = DISP_LCD_BACKLIGHT_DISABLE;
#else
		cmd = DISP_CMD_LCD_BACKLIGHT_DISABLE;
#endif
	}
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ret = ioctl(disp_fd, cmd, (void*)ioctlParam);
	if (ret < 0)
	{
		printf("%s: fail to open or close backlight!\r\n", __func__);
	}
	bl_status = onoff;
	va_display_device_exit();
	return 0;
}
int va_display_lcd_onoff(bool onoff)
{
	int ret = 0;
	unsigned int cmd = 0;

	va_display_device_init();
	DISP_CLAR(ioctlParam);

	if(onoff == 1){
#ifdef __SUNXI_DISPLAY2__
		cmd = DISP_LCD_ENABLE;
#else
		cmd = DISP_CMD_LCD_ENABLE;
#endif
	}else{
#ifdef __SUNXI_DISPLAY2__
		cmd = DISP_LCD_DISABLE;
#else
		cmd = DISP_CMD_LCD_DISABLE;
#endif
	}
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = 0;
	ret = ioctl(disp_fd, cmd, (void*)ioctlParam);
	if (ret < 0)
	{
		printf("%s: fail to open backlight!\r\n", __func__);
	}

	va_display_device_exit();
	return 0;
}

int va_display_ui_layer_onoff(bool onoff)
{
	int ret = 0;
	unsigned int cmd = 0;

	va_display_device_init();
	DISP_CLAR(ioctlParam);

	if(onoff == 1){
#ifdef __SUNXI_DISPLAY2__
		cmd = DISP_LAYER_ENABLE;
#else
		cmd = DISP_CMD_LAYER_ENABLE;
#endif
	}else{
#ifdef __SUNXI_DISPLAY2__
		cmd = DISP_LAYER_DISABLE;
#else
		cmd = DISP_CMD_LAYER_DISABLE;
#endif
	}
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = UI_LAYER_NUM;
	ret = ioctl(disp_fd, cmd, (void*)ioctlParam);
	if (ret < 0)
	{
		printf("%s: fail to open backlight!\r\n", __func__);
	}

	va_display_device_exit();
	return 0;
}

int va_display_set_ui_layer_screen(disp_window *screen_win)
{
	int ret = 0;
	struct disp_layer_info layer_info;

	if (screen_win == NULL)
	{
		printf("%s: para null!\r\n", __func__);
		return -1;
	}
	va_display_device_init();
	memset(&layer_info, 0, sizeof(struct disp_layer_info));
	DISP_CLAR(ioctlParam);

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = UI_LAYER_NUM;
	ioctlParam[2] = (unsigned long int)&layer_info;
#ifdef __SUNXI_DISPLAY2__
	ret = ioctl(disp_fd, DISP_LAYER_GET_INFO, (void*)ioctlParam);
#else
	ret = ioctl(disp_fd, DISP_CMD_LAYER_GET_INFO, (void*)ioctlParam);
#endif
	if (ret < 0)
	{
		printf("%s: fail to DISP_CMD_LAYER_GET_INFO!\r\n", __func__);
		return -1;
	}

	layer_info.screen_win.x      = screen_win->x;
	layer_info.screen_win.y      = screen_win->y;
	layer_info.screen_win.width  = screen_win->width;
	layer_info.screen_win.height = screen_win->height;
#ifdef __SUNXI_DISPLAY2__
		ret = ioctl(disp_fd, DISP_LAYER_SET_INFO, (void*)ioctlParam);
#else
		ret = ioctl(disp_fd, DISP_CMD_LAYER_SET_INFO, (void*)ioctlParam);
#endif
	if (ret < 0)
	{
		printf("%s: fail to DISP_CMD_LAYER_SET_INFO!\r\n", __func__);
		return -1;
	}

	va_display_device_exit();
	return 0;
}
int va_display_get_ui_layer_screen(disp_window *screen_win)
{
	int ret = 0;
	struct disp_layer_info layer_info;

	if (screen_win == NULL)
	{
		printf("%s: para null!\r\n", __func__);
		return -1;
	}
	va_display_device_init();
	memset(&layer_info, 0, sizeof(struct disp_layer_info));
	DISP_CLAR(ioctlParam);

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = UI_LAYER_NUM;
	ioctlParam[2] = (unsigned long int)&layer_info;
#ifdef __SUNXI_DISPLAY2__
	ret = ioctl(disp_fd, DISP_LAYER_GET_INFO, (void*)ioctlParam);
#else
	ret = ioctl(disp_fd, DISP_CMD_LAYER_GET_INFO, (void*)ioctlParam);
#endif
	if (ret < 0)
	{
		printf("%s: fail to DISP_CMD_LAYER_GET_INFO!\r\n", __func__);
		return -1;
	}
	screen_win->x = layer_info.screen_win.x;
	screen_win->y = layer_info.screen_win.y;
	screen_win->width = layer_info.screen_win.width;
	screen_win->height = layer_info.screen_win.height;

	va_display_device_exit();
	return 0;
}

int va_display_set_ui_layer_src(disp_window *src)
{
	int ret = 0;
	struct disp_layer_info layer_info;

	if (src == NULL)
	{
		printf("%s: para null!\r\n", __func__);
		return -1;
	}
	va_display_device_init();
	memset(&layer_info, 0, sizeof(struct disp_layer_info));
	DISP_CLAR(ioctlParam);

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = UI_LAYER_NUM;
	ioctlParam[2] = (unsigned long int)&layer_info;
#ifdef __SUNXI_DISPLAY2__
	ret = ioctl(disp_fd, DISP_LAYER_GET_INFO, (void*)ioctlParam);
#else
	ret = ioctl(disp_fd, DISP_CMD_LAYER_GET_INFO, (void*)ioctlParam);
#endif
	if (ret < 0)
	{
		printf("%s: fail to DISP_CMD_LAYER_GET_INFO!\r\n", __func__);
		return -1;
	}
#ifdef __SUNXI_DISPLAY2__
	layer_info.screen_win.x		 = src->x;
	layer_info.screen_win.y		 = src->y;
	layer_info.screen_win.width  = src->width;
	layer_info.screen_win.height = src->height;
#else
	layer_info.fb.src_win.x      = src->x;
	layer_info.fb.src_win.y      = src->y;
	layer_info.fb.src_win.width  = src->width;
	layer_info.fb.src_win.height = src->height;
#endif
#ifdef __SUNXI_DISPLAY2__
	ret = ioctl(disp_fd, DISP_LAYER_SET_INFO, (void*)ioctlParam);
#else
	ret = ioctl(disp_fd, DISP_CMD_LAYER_SET_INFO, (void*)ioctlParam);
#endif
	if (ret < 0)
	{
		printf("%s: fail to DISP_CMD_LAYER_SET_INFO!\r\n", __func__);
		return -1;
	}

	va_display_device_exit();
	return 0;
}
int va_display_get_ui_layer_src(disp_window *src)
{
	int ret = 0;
	struct disp_layer_info layer_info;

	if (src == NULL)
	{
		printf("%s: para null!\r\n", __func__);
		return -1;
	}
	va_display_device_init();
	memset(&layer_info, 0, sizeof(struct disp_layer_info));
	DISP_CLAR(ioctlParam);

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = UI_LAYER_NUM;
	ioctlParam[2] = (unsigned long int)&layer_info;
#ifdef __SUNXI_DISPLAY2__
	ret = ioctl(disp_fd, DISP_LAYER_GET_INFO, (void*)ioctlParam);
#else
	ret = ioctl(disp_fd, DISP_CMD_LAYER_GET_INFO, (void*)ioctlParam);
#endif
	if (ret < 0)
	{
		printf("%s: fail to DISP_CMD_LAYER_GET_INFO!\r\n", __func__);
		return -1;
	}
#ifdef __SUNXI_DISPLAY2__
	src->x = layer_info.screen_win.x;
	src->y = layer_info.screen_win.y;
	src->width = layer_info.screen_win.width;
	src->height = layer_info.screen_win.height;
#else
	src->x = layer_info.fb.src_win.x;
	src->y = layer_info.fb.src_win.y;
	src->width = layer_info.fb.src_win.width;
	src->height = layer_info.fb.src_win.height;
#endif
	va_display_device_exit();
	return 0;
}

#ifdef __SUNXI_DISPLAY__
int va_display_ui_layer_mode_switch(disp_layer_mode mode)
{
	unsigned int args[4] = {0};
	int disp_dev;
	struct disp_layer_info layer_para;

	disp_dev = open(DISP_DEV_NAME, O_RDWR);
	args[0] = 0;
	args[1] = UI_LAYER_NUM;
	args[2] = (unsigned int)&layer_para;
	ioctl(disp_dev, DISP_CMD_LAYER_GET_INFO, (unsigned int)args);
	layer_para.mode = mode;
	ioctl(disp_dev, DISP_CMD_LAYER_SET_INFO, (unsigned int)args);
	close(disp_dev);

	return 0;
}

int va_display_ui_pipe_set(unsigned char pipe)
{
	unsigned int args[4]={0};
	int disp_dev;
	struct disp_layer_info layer_para;

	disp_dev = open(DISP_DEV_NAME, O_RDWR);
	args[0] = 0;
	args[1] = UI_LAYER_NUM;
	args[2] = (unsigned int)&layer_para;
	ioctl(disp_dev, DISP_CMD_LAYER_GET_INFO, (unsigned int)args);
	layer_para.pipe = pipe;
	ioctl(disp_dev, DISP_CMD_LAYER_SET_INFO, (unsigned int)args);
	close(disp_dev);

	return 0;
}
void va_display_ui_zorder_set(unsigned char zorder)
{
	unsigned int args[4]={0};
	int disp_dev;
	struct disp_layer_info layer_para;

	disp_dev = open(DISP_DEV_NAME, O_RDWR);
	args[0] = 0;
	args[1] = UI_LAYER_NUM;
	args[2] = (unsigned int)&layer_para;
	ioctl(disp_dev, DISP_CMD_LAYER_GET_INFO, (unsigned int)args);
	layer_para.zorder = zorder;
	ioctl(disp_dev, DISP_CMD_LAYER_SET_INFO, (unsigned int)args);
	close(disp_dev);
}

int va_display_ui_enable(int en)
{
	unsigned int args[4]={0};
	int disp_dev = open(DISP_DEV_NAME, O_RDWR);
	int ret = 0;

	args[0] = 0;
	args[1] = UI_LAYER_NUM;
	if(en) {
		args[2] = 0;
		ret = ioctl(disp_dev, DISP_CMD_LAYER_ENABLE, args);
	} else {
		args[2] = 0;
		ret = ioctl(disp_dev, DISP_CMD_LAYER_DISABLE, args);
	}
	close(disp_dev);

	return ret;
}

#endif
