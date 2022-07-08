#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fcntl.h>
#include "common.h"
#include "ion_mem_alloc.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ARRAY_NUM(x) (sizeof(x) / sizeof(x[0]))
#define ALIGN_IN(x, order) ((x) & ~((order) - 1))
#define ALIGN_OUT(x, order) (((x) + ((order) - 1)) & ~((order) - 1))

#define IMAGE_FULL_SCREEN NULL
#define IMAGE_HOR_RES_MAX          (800)
#define IMAGE_VER_RES_MAX          (480)

typedef enum {
	IMAGE_ALPHA_PIXEL,
	IMAGE_ALPHA_GLOBAL,
} image_alpha_mode_t;

typedef struct {
	s32 x;
	s32 y;
	u32 w;
	u32 h;
} image_rect_t;

typedef struct {
	s32 x;
	s32 y;
} image_coor_t;

typedef enum {
	IMAGE_FORMAT_ARGB8888,
	IMAGE_FORMAT_ABGR8888,
	IMAGE_FORMAT_RGBA8888,
	IMAGE_FORMAT_BGRA8888,
	IMAGE_FORMAT_BGR888,
	IMAGE_FORMAT_YUV420P,
} image_data_format_t;

// typedef enum {
//	IMAGE_SHOW_SLIDE,
//	IMAGE_SHOW_STRETCH,
//	IMAGE_SHOW_ZOOM,
//	IMAGE_SHOW_FADE,
//	IMAGE_SHOW_PERSIANBLIND,
//	IMAGE_SHOW_MOSIAC,
//	IMAGE_SHOW_FILL,
//	IMAGE_SHOW_RANDOM,
// } image_effect_show_mode_t;

typedef enum {
	IMAGE_SHOW_NORMAL,
	IMAGE_SHOW_RANDOM,
	IMAGE_SHOW_FADE,
	IMAGE_SHOW_MOSIAC,
	IMAGE_SHOW_SLIDE_UP,
	IMAGE_SHOW_SLIDE_DOWN,
	IMAGE_SHOW_SLIDE_LEFT,
	IMAGE_SHOW_SLIDE_RIGHT,
	IMAGE_SHOW_STRETCH_UP,
	IMAGE_SHOW_STRETCH_DOWN,
	IMAGE_SHOW_STRETCH_LEFT,
	IMAGE_SHOW_STRETCH_RIGHT,
	IMAGE_SHOW_ZOOM_IN,
	IMAGE_SHOW_ZOOM_OUT,
	IMAGE_SHOW_PERSIANBLIND_H,
	IMAGE_SHOW_PERSIANBLIND_V,
	IMAGE_SHOW_MAX,
} image_effect_show_mode_t;

typedef enum {
	IMAGE_ROTATE_0,
	IMAGE_ROTATE_90,
	IMAGE_ROTATE_180,
	IMAGE_ROTATE_270,
} image_rotate_angle_t;

typedef enum {
	IMAGE_FULL_SCREEN_SCLAER,
	IMAGE_FULL_SCREEN_FILL,
	IMAGE_SHOW_MODE_NORMAL,
	IMAGE_SHOW_MODE_THUMB,
} image_show_mode_t;

typedef struct {
	void *fb_vir;
	void *fb_phy;
	image_rect_t disp_rect;
	image_rect_t src_crop_rect;
	unsigned int screen_width;
	unsigned int screen_height;
} image_disp_info_t;

typedef struct {
	unsigned int width_or;
	unsigned int height_or;
	unsigned int width;
	unsigned int height;
	unsigned int length;
	unsigned int comp;
	unsigned int scaler_ratito;
	image_rotate_angle_t rotate_angle;
	image_data_format_t fmt;
	unsigned int frame_cnt;
	int *delays;
	void *vir_addr;
	void *phy_addr;
	int fd;
} image_buffer_t;

typedef struct {
	image_buffer_t buf;            /*图片信息*/
	image_rect_t clip_rect;		   /*裁剪区域*/
	image_coor_t coor;             /*目标位置*/
	unsigned int alpha_value;      /*alpha值*/
	image_alpha_mode_t alpha_mode; /*alpha模式*/
} image_enh_t;

typedef struct {
	int (*init)(void);
	int (*exit)(void);
	int (*display)(image_buffer_t *src, image_rect_t *src_crop_rect);
	int (*rect_set)(image_rect_t *rect);
	int (*src_crop_rect_set)(image_rect_t *crop_rect);
	int (*disp_info_get)(image_disp_info_t *disp_info);
	int (*disp_cache_enable)(int enable);
	int (*disp_dequeue)(unsigned long phy_addr,unsigned long vir_addr,image_buffer_t *src, image_rect_t *crop_rect);
	int (*dequeue)(unsigned long *phy_addr,unsigned long *vir_addr);
} image_display_t;

typedef struct {
	int (*init)(void);
	int (*exit)(void);
	int (*scaler)(image_enh_t *src, image_enh_t *dst);
	int (*rotate)(image_buffer_t *src, image_buffer_t *dst, image_rotate_angle_t angle);
	int (*clip)(image_enh_t *src, image_enh_t *dst, int src_num);
	int (*conver)(image_enh_t *src, image_enh_t *dst, int src_num);
	int (*alpha_blend)(image_enh_t *src, image_enh_t *dst, int src_num);
	int (*fill)(image_enh_t *src,unsigned int color);
} image_process_t;

typedef struct {
	int (*init)(void);
	int (*deinit)(void);
	int (*zoom)(void);
	int (*stretch)(void);
	int (*slide)(void);
	int (*fade)(void);
	int (*mosiac)(void);
	int (*persianblind)(void);
} image_effect_show_t;

typedef struct {
	image_display_t *display;
	image_process_t *process;
	image_effect_show_t *effect_show;
	int (*decode)(char const *filename, image_buffer_t *buf);
	int (*show)(char const *filename, image_show_mode_t mode);
} image_opr_t;

typedef struct {
	image_buffer_t src;//标准化之后的数据
	image_buffer_t dst_next;//申请好内存的临时buffer，大小为屏幕大小
	image_buffer_t dst_previous;//申请好内存的临时buffer，大小为屏幕大小
	image_opr_t opr;//g2d和显示驱动的接口
} image_t;

int image_init(void);
int image_exit(void);
int image_disp_rect_set(image_rect_t *rect);
int image_show(char *file, image_show_mode_t mode);
int image_start(char *file);
int image_stop(void);
int image_rotate(image_rotate_angle_t rotate_angle);
int image_scaler(int *ratio);
int image_move(int x, int y);
int image_effect_show(char *filename, image_effect_show_mode_t mode);
int image_process_opr_register(image_process_t *process);
int image_display_opr_register(image_display_t *display);
void image_process_opr_init(void);
void image_display_opr_init(void);
int image_disp_cache_enable(int enable);
int image_show_normal(void);
int image_get_width(void);
int image_get_height(void);




#ifdef __cplusplus
}
#endif

#endif
