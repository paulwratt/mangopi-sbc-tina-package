#include "videoOutPort.h"
#include "image.h"

typedef enum {
	DISP_STOP,
	DISP_START
} disp_state_t;

typedef struct {
	dispOutPort *disp;
	image_disp_info_t info;
	disp_state_t state;
//	unsigned int index;
} disp_t;

#ifdef __SUNXI_DISPLAY2__
#define disp_layer_info_t struct disp_layer_config2
#else
typedef struct
{
    struct disp_layer_info info;
} disp_layer_info_t;
#endif

static disp_t disp_manager;
static struct SunxiMemOpsS *ionHdle;

static dispOutPort *disp_get(void)
{
	return disp_manager.disp;
}

static VideoPixelFormat image_format_to_display(image_data_format_t fmt)
{
	VideoPixelFormat disp_fmt;

	switch (fmt) {
		case IMAGE_FORMAT_ABGR8888:
			disp_fmt = VIDEO_PIXEL_FORMAT_ABGR;
			break;
		case IMAGE_FORMAT_ARGB8888:
			disp_fmt = VIDEO_PIXEL_FORMAT_ARGB;
			break;
		case IMAGE_FORMAT_RGBA8888:
			disp_fmt = VIDEO_PIXEL_FORMAT_RGBA;
			break;
		case IMAGE_FORMAT_BGRA8888:
			disp_fmt = VIDEO_PIXEL_FORMAT_BGRA;
			break;
		default:
			disp_fmt = VIDEO_PIXEL_FORMAT_ARGB;
			break;
	}
	return disp_fmt;
}

static int disp_info_get(image_disp_info_t *info)
{
	int ret;
	disp_layer_info_t layer_para;
	dispOutPort *disp = disp_get();

#ifdef __SUNXI_DISPLAY2__
	ret = disp->setIoctl(disp, DISP_LAYER_GET_CONFIG2, (unsigned long)&layer_para);
#else
	ret = disp->setIoctl(disp, DISP_CMD_LAYER_GET_INFO, (unsigned long)&layer_para.info);
#endif
	if(ret != 0)
		printf("%s: ioctl failed!\n", __func__);

	// disp_manager.info.fb_vir = ionHdle->cpu_get_viraddr((void *)(long)layer_para.info.fb.addr[0]);
	// disp_manager.info.fb_phy = (void *)(long)layer_para.info.fb.addr[0];
	int channel;
	int index = 0;
	#define HD2CHN(hlay) (hlay/4)
	channel = HD2CHN(disp->hlayer);
	if (disp->interBufSet[channel])
	{
		index = (disp->bufindex[channel] == 1) ? 0 : 1;
		disp_manager.info.fb_vir = (char*) disp->renderbuf[channel][index].vir_addr;
		disp_manager.info.fb_phy = (char*) disp->renderbuf[channel][index].phy_addr;
	}
	disp_manager.info.src_crop_rect.x = (__s32)(layer_para.info.fb.crop.x >> 32);
	disp_manager.info.src_crop_rect.y = (__s32)(layer_para.info.fb.crop.y >> 32);
	disp_manager.info.src_crop_rect.w = (__u32)(layer_para.info.fb.crop.width >> 32);
	disp_manager.info.src_crop_rect.h = (__u32)(layer_para.info.fb.crop.height >> 32);
	*info = disp_manager.info;

	return 0;
}

// static int disp_info_set(disp_layer_info_t *layer_para)
// {
//	dispOutPort *disPort = disp_get();

// #ifdef __SUNXI_DISPLAY2__
//	return disPort->setIoctl(disPort, DISP_LAYER_SET_CONFIG, (unsigned long)layer_para);
// #else
//	return disPort->setIoctl(disPort, DISP_CMD_LAYER_SET_INFO, (unsigned long)&layer_para->info);
// #endif
// }

static int display(image_buffer_t *src, image_rect_t *crop_rect)
{
	videoParam paramDisp;
	dispOutPort *disp = disp_get();

	paramDisp.isPhy = 0;
	paramDisp.srcInfo.w = src->width;
	paramDisp.srcInfo.h = src->height;
	paramDisp.srcInfo.crop_x = crop_rect->x;
	paramDisp.srcInfo.crop_y = crop_rect->y;
	paramDisp.srcInfo.crop_w = crop_rect->w;
	paramDisp.srcInfo.crop_h = crop_rect->h;
	paramDisp.srcInfo.format = image_format_to_display(src->fmt);
	paramDisp.srcInfo.color_space = 0;
	disp->writeData(disp, src->vir_addr, src->length, &paramDisp);
	disp->setEnable(disp, 1);

	return 0;

#if 0
	videoParam paramDisp;
	renderBuf rBuf;
	dispOutPort *disp = disp_get();

	//memset(&paramDisp,0,sizeof(videoParam));
	//memset(&rBuf,0,sizeof(renderBuf));

	rBuf.isExtPhy = VIDEO_USE_EXTERN_ION_BUF;
	rBuf.phy_addr = src->phy_addr;
	rBuf.vir_addr = src->vir_addr;
	rBuf.y_phaddr = src->phy_addr;
	rBuf.u_phaddr = 0;//src->vir_addr;
	rBuf.v_phaddr = 0;//src->vir_addr;
	rBuf.fd = SunxiMemGetBufferFd(disp->pMemops, (void*)src->vir_addr);

	paramDisp.isPhy = 0;
	paramDisp.srcInfo.w = src->width;
	paramDisp.srcInfo.h = src->height;
	paramDisp.srcInfo.crop_x = crop_rect->x;
	paramDisp.srcInfo.crop_y = crop_rect->y;
	paramDisp.srcInfo.crop_w = crop_rect->w;
	paramDisp.srcInfo.crop_h = crop_rect->h;
	paramDisp.srcInfo.format = VIDEO_PIXEL_FORMAT_ARGB;//image_format_to_display(src->fmt);
	paramDisp.srcInfo.color_space = 0;

	disp->queueToDisplay(disp, src->width*src->height*4 , &paramDisp,&rBuf);
	disp->SetZorder(disp, VIDEO_ZORDER_MIDDLE);
	disp->setEnable(disp, 1);
	return 0;
	#endif
}

static int dequeue(unsigned long *phy_addr,unsigned long *vir_addr)
{
	renderBuf rBuf;
	dispOutPort *disp = disp_get();
	disp->dequeue(disp, &rBuf);
	*phy_addr = rBuf.phy_addr;
	*vir_addr = rBuf.vir_addr;

	return 0;
}
static int disp_dequeue(unsigned long phy_addr,unsigned long vir_addr,image_buffer_t *src, image_rect_t *crop_rect)
{
	videoParam paramDisp;
	renderBuf rBuf;
	int size = 800*480*4;//src->length;
	//printf("length = %d\n",size);
	//usleep(10);
	dispOutPort *disp = disp_get();

	//memset(&rBuf,0,sizeof(renderBuf));
	rBuf.isExtPhy = VIDEO_USE_EXTERN_ION_BUF;
	rBuf.phy_addr = phy_addr;
	rBuf.vir_addr = vir_addr;

	paramDisp.isPhy = 0;
	paramDisp.srcInfo.w = src->width;
	paramDisp.srcInfo.h = src->height;
	paramDisp.srcInfo.crop_x = crop_rect->x;
	paramDisp.srcInfo.crop_y = crop_rect->y;
	paramDisp.srcInfo.crop_w = crop_rect->w;
	paramDisp.srcInfo.crop_h = crop_rect->h;
	paramDisp.srcInfo.format = image_format_to_display(src->fmt);
	paramDisp.srcInfo.color_space = 0;

	disp->queueToDisplay(disp, size , &paramDisp,&rBuf);
	disp->setEnable(disp, 1);

	return 0;
}

static int src_crop_rect_set(image_rect_t *crop_rect)
{
	disp_layer_info_t layer_para;

	disp_manager.disp->setIoctl(disp_manager.disp, DISP_LAYER_GET_CONFIG2, (unsigned long)&layer_para);
	disp_manager.info.src_crop_rect = *crop_rect;
	layer_para.info.fb.crop.x = (long long)crop_rect->x << 32;
	layer_para.info.fb.crop.y = (long long)crop_rect->y << 32;
	layer_para.info.fb.crop.width = (long long)crop_rect->w << 32;
	layer_para.info.fb.crop.height = (long long)crop_rect->h << 32;
	disp_manager.disp->setIoctl(disp_manager.disp, DISP_LAYER_SET_CONFIG2, (unsigned long)&layer_para);

	return 0;
}

static int disp_rect_set(image_rect_t *disp_rect)
{
	disp_manager.info.disp_rect = *disp_rect;
	return disp_manager.disp->setRect(disp_manager.disp, (VoutRect *)&disp_manager.info.disp_rect);
}

static int disp_dev_init(void)
{
	dispOutPort *disp;
	videoParam param;
	int ret;

	if (disp_manager.state == DISP_STOP) {
		ionHdle = GetMemAdapterOpsS();
		ret = ionHdle->open();
		if (ret != 0) {
			printf("open ION error: %d\n", ret);
			goto ion_err;
		}
		disp = CreateVideoOutport(0);

		// disp_manager.info.disp_rect.x = 0;
		// disp_manager.info.disp_rect.y = 0;
		// disp_manager.info.disp_rect.w = 800;
		// disp_manager.info.disp_rect.h = 480;

		ret = disp->init(disp, 0, ROTATION_ANGLE_0, (VoutRect *)&disp_manager.info.disp_rect);
		if (ret != 0) {
			printf("disp init failed\n");
			goto init_err;
		}
		disp_manager.info.disp_rect.x = 0;
		disp_manager.info.disp_rect.y = 0;
		disp_manager.info.disp_rect.w = disp->getScreenWidth(disp);
		disp_manager.info.disp_rect.h = disp->getScreenHeight(disp);

		disp->setRoute(disp, VIDEO_SRC_FROM_FILE);
		disp->setRect(disp, (VoutRect *)&disp_manager.info.disp_rect);
		param.srcInfo.format = VIDEO_PIXEL_FORMAT_ARGB;
		param.srcInfo.w = disp_manager.info.disp_rect.w;
		param.srcInfo.h = disp_manager.info.disp_rect.h;
		ret = disp->allocateVideoMem(disp, &param);
		disp_manager.info.screen_width  = disp_manager.info.disp_rect.w;
		disp_manager.info.screen_height = disp_manager.info.disp_rect.h;
		disp_manager.state = DISP_START;
		disp_manager.disp = disp;
	}

	return 0;
init_err:
	ionHdle->close();
ion_err:
    return ret;
}
static int disp_cache_enable(int enable)
{
	//printf("disp_cache_enable %d\n",enable);
	disp_manager.disp->setCache(disp_manager.disp, enable);

	//  unsigned long para[4];
	//  para[0] = 0;
	//  para[1] = enable;
	//  para[2] = 0;
	//  para[3] = 0;
	//  disp_manager.disp->setIoctl(disp_manager.disp, DISP_SHADOW_PROTECT,para);

	return 0;
}
static int disp_dev_exit(void)
{
	dispOutPort *disp;

	if (disp_manager.state == DISP_START) {
		disp = disp_manager.disp;
	    disp->enable = 0;
		disp->setEnable(disp,0);
	    disp->freeVideoMem(disp);
	    disp->deinit(disp);
	    DestroyVideoOutport(disp);
		disp_manager.state = DISP_STOP;
		disp_manager.info.disp_rect.w = 0;
		disp_manager.info.disp_rect.h = 0;
		ionHdle->close();
	}

	return 0;
}


static image_display_t image_disp_opr = {
	.init			   = disp_dev_init,
	.exit			   = disp_dev_exit,
	.display		   = display,
	.rect_set		   = disp_rect_set,
	.src_crop_rect_set = src_crop_rect_set,
	.disp_info_get	   = disp_info_get,
	.disp_cache_enable = disp_cache_enable,
	.dequeue		   = dequeue,
	.disp_dequeue	   = disp_dequeue,
};

void image_display_opr_init(void)
{
	image_display_opr_register(&image_disp_opr);
}
