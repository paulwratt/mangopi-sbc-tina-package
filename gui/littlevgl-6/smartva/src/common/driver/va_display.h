#ifndef _VA_DISPLAY_H_
#define _VA_DISPLAY_H_

#include "smt_config.h"
#ifdef __SUNXI_DISPLAY2__
#include "sunxi_display2.h"
#else//__SUNXI_DISPLAY__
#include "drv_display.h"
#endif


#ifndef __SUNXI_DISPLAY2__
struct disp_rectsz {
	__u32 width;
	__u32 height;
};
#else
typedef struct {
        int x;
        int y;
        unsigned int width;
        unsigned int height;
} disp_window;
#endif
#if 0
typedef enum {
	LCD_BRIGHTNESS_LEVEL0 = 0,
	LCD_BRIGHTNESS_LEVEL1,
	LCD_BRIGHTNESS_LEVEL2,
	LCD_BRIGHTNESS_LEVEL3,
	LCD_BRIGHTNESS_LEVEL4,
	LCD_BRIGHTNESS_LEVEL5,
	LCD_BRIGHTNESS_LEVEL6,
	LCD_BRIGHTNESS_LEVEL7,
	LCD_BRIGHTNESS_LEVEL8,
	LCD_BRIGHTNESS_LEVEL9,
	LCD_BRIGHTNESS_LEVEL10,
	LCD_BRIGHTNESS_NUM,
} lcd_brightness_t;
#endif

#define LCD_BLACK_OFF 0
#define LCD_BLACK_ON 1

int va_display_lcd_onoff(bool onoff);
int va_display_lcd_backlight_onoff(bool onoff);
bool va_display_lcd_backlight_status(void);
int va_display_lcd_set_backlight(int16_t val);
int va_display_lcd_get_backlight(void);
int va_display_get_lcd_rect(struct disp_rectsz *rect);
int va_display_ui_layer_onoff(bool onoff);
int va_display_set_ui_layer_screen(disp_window *screen_win);
int va_display_get_ui_layer_screen(disp_window *screen_win);
int va_display_set_ui_layer_src(disp_window *src);
int va_display_get_ui_layer_src(disp_window *src);
int va_display_set_enhance_mode(uint16_t val);
int va_display_get_enhance_mode(void);
int va_display_enhance_set_bright(int16_t val);
int va_display_enhance_get_bright(void);
int va_display_enhance_set_contrast(int16_t val);
int va_display_enhance_get_contrast(void);
int va_display_enhance_set_saturation(int16_t val);
int va_display_enhance_get_saturation(void);
int va_display_enhance_set_edge(int16_t val);
int va_display_enhance_get_edge(void);
int va_display_enhance_set_detail(int16_t val);
int va_display_enhance_get_detail(void);
#ifdef __SUNXI_DISPLAY__
int va_display_ui_pipe_set(unsigned char pipe);
int va_display_ui_enable(int en);
void va_display_ui_zorder_set(unsigned char zorder);
int va_display_ui_layer_mode_switch(disp_layer_mode mode);
#endif
#endif
