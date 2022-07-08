#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <signal.h>
#include <time.h>
#include <linux/fb.h>
#include <linux/kernel.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "g2d_driver_enh.h"
#include "ion_mem_alloc.h"
#include "image.h"

#define G2D_BLD_SRC_NUM 4

static int g2d_fd;
static struct SunxiMemOpsS *ionHdle;

static g2d_fmt_enh image_format_to_g2d(image_data_format_t fmt)
{
	g2d_fmt_enh g2d_fmt;

	switch (fmt) {
		case IMAGE_FORMAT_ABGR8888:
			g2d_fmt = G2D_FORMAT_ABGR8888;
			break;
		case IMAGE_FORMAT_ARGB8888:
			g2d_fmt = G2D_FORMAT_ARGB8888;
			break;
		case IMAGE_FORMAT_RGBA8888:
			g2d_fmt = G2D_FORMAT_RGBA8888;
			break;
		case IMAGE_FORMAT_BGRA8888:
			g2d_fmt = G2D_FORMAT_BGRA8888;
			break;
		case IMAGE_FORMAT_YUV420P:
			g2d_fmt = G2D_FORMAT_YUV420_PLANAR;
			break;
		case IMAGE_FORMAT_BGR888:
			g2d_fmt = G2D_FORMAT_BGR888;
			break;

		default:
			g2d_fmt = G2D_FORMAT_ABGR8888;
			break;
	}

	return g2d_fmt;
}

static g2d_alpha_mode_enh image_alpha_mode_to_g2d(image_alpha_mode_t mode)
{
	g2d_blt_flags_h g2d_alpha_mode;

	switch (mode) {
		case IMAGE_ALPHA_PIXEL:
			g2d_alpha_mode = G2D_PIXEL_ALPHA;
			break;
		case IMAGE_ALPHA_GLOBAL:
			g2d_alpha_mode = G2D_GLOBAL_ALPHA;
			break;
		default:
			g2d_alpha_mode = G2D_PIXEL_ALPHA;
			break;
	}

	return g2d_alpha_mode;
}

static g2d_blt_flags_h image_rotate_angle_to_g2d(image_rotate_angle_t rotate_angle)
{
	g2d_blt_flags_h g2d_ratate;

	switch (rotate_angle) {
		case IMAGE_ROTATE_0:
			g2d_ratate = G2D_ROT_0;
			break;
		case IMAGE_ROTATE_90:
			g2d_ratate = G2D_ROT_90;
			break;
		case IMAGE_ROTATE_180:
			g2d_ratate = G2D_ROT_180;
			break;
		case IMAGE_ROTATE_270:
			g2d_ratate = G2D_ROT_270;
			break;
		default:
			g2d_ratate = G2D_ROT_0;
			break;
	}

	return g2d_ratate;
}
static int get_time(void)
{
	struct timeval tv2;
	gettimeofday(&tv2, NULL);
	return (int)((tv2.tv_sec)*1000 + tv2.tv_usec/1000);
}
static int g2d_blend(image_enh_t *src, image_enh_t *dst, int src_num, g2d_bld_cmd_flag g2d_bld_cmd)
{
	g2d_bld info;
	int i, ret;

	memset(&info, 0, sizeof(info));
	info.bld_cmd = g2d_bld_cmd;
	for (i = 0; i < src_num; i++) {
		info.src_image[i].laddr[0] = (unsigned long)src[i].buf.phy_addr;
		info.src_image[i].use_phy_addr = 1;
		info.src_image[i].format = image_format_to_g2d(src[i].buf.fmt);
		info.src_image[i].width = src[i].buf.width;
		info.src_image[i].height = src[i].buf.height;
		info.src_image[i].clip_rect.x = src[i].clip_rect.x;
		info.src_image[i].clip_rect.y = src[i].clip_rect.y;
		info.src_image[i].clip_rect.w = src[i].clip_rect.w;
		info.src_image[i].clip_rect.h = src[i].clip_rect.h;
		info.src_image[i].coor.x = src[i].coor.x;
		info.src_image[i].coor.y = src[i].coor.y;
		info.src_image[i].mode = image_alpha_mode_to_g2d(src[i].alpha_mode);
		info.src_image[i].alpha = src[i].alpha_value;
	}

	info.dst_image.format = image_format_to_g2d(dst->buf.fmt);
	info.dst_image.width = dst->buf.width;
	info.dst_image.height = dst->buf.height;
	info.dst_image.clip_rect.x = dst->clip_rect.x;
	info.dst_image.clip_rect.y = dst->clip_rect.y;
	info.dst_image.clip_rect.w = dst->clip_rect.w;
	info.dst_image.clip_rect.h = dst->clip_rect.h;
	info.dst_image.mode = image_alpha_mode_to_g2d(dst->alpha_mode);
	info.dst_image.alpha = dst->alpha_value;
	info.dst_image.laddr[0] = (unsigned long)dst->buf.phy_addr;
	info.dst_image.use_phy_addr = 1;

	info.src_image[0].align[0] = 0;
	info.src_image[0].align[1] = 0;
	info.src_image[0].align[2] = 0;
	info.src_image[1].align[0] = 0;
	info.src_image[1].align[1] = 0;
	info.src_image[1].align[2] = 0;
	info.dst_image.align[0] = 0;
	info.dst_image.align[1] = 0;
	info.dst_image.align[2] = 0;

	ionHdle->flush_cache(src->buf.vir_addr, src->buf.length);
	static int cnt = 0;
	//printf("before\n");
	int time1 = get_time();

	ret = ioctl(g2d_fd, G2D_CMD_BLD_H, (unsigned long)(&info));
	//printf("after ret = %d time = %d cnt = %d\n",ret,get_time()-time1,cnt);
	int diff = get_time()-time1;
	if((diff)>3000)
	{
		cnt ++;
		printf("err cnt = %d diff = %d\n",cnt,diff);
	}
	ionHdle->flush_cache(dst->buf.vir_addr, dst->buf.length);

	return ret;
}

static int g2d_blend_clip(image_enh_t *src, image_enh_t *dst, int src_num)
{
	if (src_num > G2D_BLD_SRC_NUM)
		return -1;

	return g2d_blend(src, dst, src_num, G2D_BLD_COPY);
}

static int g2d_blend_alpha(image_enh_t *src, image_enh_t *dst, int src_num)
{
	if (src_num > G2D_BLD_SRC_NUM)
		return -1;

	return g2d_blend(src, dst, src_num, G2D_BLD_SRCOVER);
}
static int g2d_format_conver(image_enh_t *src, image_enh_t *dst, int src_num)
{
	int ret = 0;
	unsigned long arg[3];

	struct mixer_para para;
	memset(&para, 0, sizeof(para));
	para.flag_h  = G2D_BLT_NONE_H;
	para.op_flag = OP_BITBLT;

	para.src_image_h.use_phy_addr = 1;
	para.src_image_h.laddr[0] = (unsigned long)src->buf.phy_addr;

	para.src_image_h.format = image_format_to_g2d(src->buf.fmt);
	para.src_image_h.width = src->buf.width;
	para.src_image_h.height = src->buf.height;
	para.src_image_h.clip_rect.x = src->clip_rect.x;
	para.src_image_h.clip_rect.y = src->clip_rect.y;
	para.src_image_h.clip_rect.w = src->clip_rect.w;
	para.src_image_h.clip_rect.h = src->clip_rect.h;

	para.dst_image_h.use_phy_addr = 1;
	para.dst_image_h.laddr[0] = (unsigned long)dst->buf.phy_addr;
	para.dst_image_h.format = image_format_to_g2d(dst->buf.fmt);
	para.dst_image_h.width = dst->buf.width;
	para.dst_image_h.height = dst->buf.height;
	para.dst_image_h.clip_rect.x = dst->clip_rect.x;
	para.dst_image_h.clip_rect.y = dst->clip_rect.y;
	para.dst_image_h.clip_rect.w = dst->clip_rect.w;
	para.dst_image_h.clip_rect.h = dst->clip_rect.h;


	arg[0] = (unsigned long)(&para);
	arg[1] = 1;
	ionHdle->flush_cache(src->buf.vir_addr, src->buf.length);
	ret = ioctl(g2d_fd, G2D_CMD_MIXER_TASK,(void *)arg);
	if (ret != EPDK_OK)
	{
		printf("[%d][%s][%s]G2D_CMD_MIXER_TASK failure!\n", __LINE__, __FILE__, __FUNCTION__);
		ret = -1;
		goto exit;
	}
	ionHdle->flush_cache(dst->buf.vir_addr, dst->buf.length);
exit:
	return ret;
}

static int g2d_scaler(image_enh_t *src, image_enh_t *dst)
{
	int ret = 0;
	g2d_lbc_rot info;

	memset(&info, 0, sizeof(info));
	info.blt.flag_h = G2D_BLT_NONE_H;
//	info.lbc_cmp_ratio = 400;
//	info.enc_is_lossy = 1;
//	info.dec_is_lossy = 1;
	info.blt.src_image_h.align[0] = 0;
	info.blt.src_image_h.align[1] = 0;
	info.blt.src_image_h.align[2] = 0;
	info.blt.dst_image_h.align[0] = 0;
	info.blt.dst_image_h.align[1] = 0;
	info.blt.dst_image_h.align[2] = 0;

	info.blt.src_image_h.use_phy_addr = 1;
	info.blt.src_image_h.laddr[0] = (unsigned long)src->buf.phy_addr;
	info.blt.src_image_h.format = image_format_to_g2d(src->buf.fmt);//G2D_FORMAT_ARGB8888;
	info.blt.src_image_h.width = src->buf.width;
	info.blt.src_image_h.height = src->buf.height;
	info.blt.src_image_h.clip_rect.x = src->clip_rect.x;
	info.blt.src_image_h.clip_rect.y = src->clip_rect.y;
	info.blt.src_image_h.clip_rect.w = src->clip_rect.w;
	info.blt.src_image_h.clip_rect.h = src->clip_rect.h;

	info.blt.dst_image_h.use_phy_addr = 1;
	info.blt.dst_image_h.laddr[0] = (unsigned long)dst->buf.phy_addr;
	info.blt.dst_image_h.format = image_format_to_g2d(dst->buf.fmt);
	info.blt.dst_image_h.width = dst->buf.width;
	info.blt.dst_image_h.height = dst->buf.height;
	info.blt.dst_image_h.clip_rect.x = dst->clip_rect.x;
	info.blt.dst_image_h.clip_rect.y = dst->clip_rect.y;
	info.blt.dst_image_h.clip_rect.w = dst->clip_rect.w;
	info.blt.dst_image_h.clip_rect.h = dst->clip_rect.h;

	ionHdle->flush_cache(src->buf.vir_addr, src->buf.length);
	if (ioctl(g2d_fd, G2D_CMD_BITBLT_H, (unsigned long)(&info)) < 0) {
		printf("[%d][%s][%s]G2D_CMD_BITBLT_H failure!\n", __LINE__, __FILE__, __FUNCTION__);
		ret = -1;
		goto exit;
	}
	ionHdle->flush_cache(dst->buf.vir_addr, dst->buf.length);

exit:
	return ret;
}

static int g2d_rotate(image_buffer_t *src, image_buffer_t *dst, image_rotate_angle_t rotate_angle)
{
	int ret;
	g2d_blt_h blt;

	memset(&blt, 0, sizeof(blt));
	blt.dst_image_h.align[0] = 8;
	blt.dst_image_h.align[1] = 8;
	blt.dst_image_h.align[2] = 8;
	if ((rotate_angle == IMAGE_ROTATE_90) || (rotate_angle == IMAGE_ROTATE_270)) {
		blt.dst_image_h.width = ALIGN_IN(src->height, blt.dst_image_h.align[0]);
		blt.dst_image_h.height = src->width;
	} else {
		blt.dst_image_h.width = ALIGN_IN(src->width, blt.dst_image_h.align[0]);
		blt.dst_image_h.height = src->height;
	}
	blt.dst_image_h.clip_rect.w = blt.dst_image_h.width;
	blt.dst_image_h.clip_rect.h = blt.dst_image_h.height;
	blt.dst_image_h.clip_rect.x = 0;
	blt.dst_image_h.clip_rect.y = 0;

	/* NOTE: g2d cannot support rotate & format convert simultaneously */
	blt.dst_image_h.format = image_format_to_g2d(src->fmt);
	dst->fmt = image_format_to_g2d(src->fmt);
	blt.dst_image_h.laddr[0] = (unsigned long)dst->phy_addr;
	blt.dst_image_h.use_phy_addr = 1;

	blt.src_image_h.use_phy_addr = 1;
	blt.src_image_h.laddr[0] = (unsigned long)src->phy_addr;
	blt.src_image_h.align[0] = 0;
	blt.src_image_h.align[1] = 0;
	blt.src_image_h.align[2] = 0;
	blt.src_image_h.format = image_format_to_g2d(src->fmt);;
	blt.src_image_h.width = src->width;
	blt.src_image_h.height = src->height;
	blt.src_image_h.clip_rect.x = 0;
	blt.src_image_h.clip_rect.y = 0;
	blt.src_image_h.clip_rect.w = blt.src_image_h.width;
	blt.src_image_h.clip_rect.h = blt.src_image_h.height;
	blt.flag_h = image_rotate_angle_to_g2d(rotate_angle);

	ionHdle->flush_cache(src->vir_addr, src->length);
	ret = ioctl(g2d_fd , G2D_CMD_BITBLT_H ,(unsigned long)(&blt));
	if(ret != 0){
		printf("[%d][%s][%s]G2D_CMD_BITBLT_H failure!\n",__LINE__, __FILE__,__FUNCTION__);
		goto err;
	}
	ionHdle->flush_cache(dst->vir_addr, dst->length);
	dst->width = blt.dst_image_h.clip_rect.w;
	dst->height = blt.dst_image_h.clip_rect.h;
	dst->comp = 4;
	dst->length = dst->width * dst->height * dst->comp;

err:
	return ret;
}
/*
*YUV基本颜色
*黑 0x008080
*白 0xFF8080
*红 0x4C55FF
*黄 0xE20095
*蓝 0x1DFF6B
*青 0xB3AB00
*颜色填充
*/
int g2d_fill(image_enh_t *src,unsigned int color)
{
    g2d_fillrect_h info;
    int ret;

    memset(&info, 0, sizeof(g2d_fillrect_h));

    info.dst_image_h.bbuff  = 1;
    info.dst_image_h.format = image_format_to_g2d(src->buf.fmt);
    info.dst_image_h.color  = color;
    info.dst_image_h.width  = src->buf.width;
    info.dst_image_h.height = src->buf.height;
    info.dst_image_h.clip_rect.x = src->clip_rect.x;
    info.dst_image_h.clip_rect.y = src->clip_rect.y;
    info.dst_image_h.clip_rect.w = src->clip_rect.w;
    info.dst_image_h.clip_rect.h = src->clip_rect.h;
    info.dst_image_h.alpha = 0xff;
    info.dst_image_h.mode  = G2D_GLOBAL_ALPHA;
    info.dst_image_h.use_phy_addr = 1;
    info.dst_image_h.laddr[0] = (unsigned long)src->buf.phy_addr;
    info.dst_image_h.laddr[1] = 0;/*当前参数暂时只能支持argb，此参数设置0*/
    info.dst_image_h.laddr[2] = 0;/*当前参数暂时只能支持argb，此参数设置0*/

	ionHdle->flush_cache(src->buf.vir_addr, src->buf.length);
	ret = ioctl(g2d_fd , G2D_CMD_FILLRECT_H ,(unsigned long)(&info));
	if(ret != 0){
		printf("[%d][%s][%s]G2D_CMD_FILLRECT_H failure!\n",__LINE__, __FILE__,__FUNCTION__);
		goto err;
	}
	ionHdle->flush_cache(src->buf.vir_addr, src->buf.length);
err:
	return ret;

}
static int g2d_init(void)
{
	int ret;

	g2d_fd = open("/dev/g2d", O_RDWR);
	if (g2d_fd == -1) {
		printf("open g2d device fail!\n");
		ret = -1;
		goto open_err;
	}
	ionHdle = GetMemAdapterOpsS();
	ret = ionHdle->open();
	if (ret != 0) {
		printf("open ION error: %d\n", ret);
		goto ion_err;
	}

	return 0;
ion_err:
	close(g2d_fd);
open_err:
	return ret;
}

static int g2d_exit(void)
{
	int ret = 0;

	ionHdle->close();
	ret = close(g2d_fd);
	if (ret == -1) {
		printf("close g2d device fail!\n");
	}

	return ret;
}

void g2d_test(void)
{
	g2d_init();
}

static image_process_t image_process_opr = {
	.init		 = g2d_init,
	.exit		 = g2d_exit,
	.scaler		 = g2d_scaler,
	.rotate		 = g2d_rotate,
	.clip		 = g2d_blend_clip,
	.conver		 = g2d_format_conver,
	.alpha_blend = g2d_blend_alpha,
	.fill        = g2d_fill,
};

void image_process_opr_init(void)
{
	image_process_opr_register(&image_process_opr);
}
