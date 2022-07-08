#define LOG_TAG "sample_uvcout"

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <linux/usb/ch9.h>

#include "linux/videodev2.h"
#include "video.h"
#include "uvc.h"
#include "confparser.h"
#include "sample_uvcout.h"
#include "sample_uvcout_config.h"
#include "camerav4l2.h"

#include "awencoder.h"


static int g_bWaitVencDone = 0;
static int g_bSampleExit   = 0;

#define ARRAY_SIZE(a)   ((sizeof(a) / sizeof(a[0])))

static int OpenUVCDevice(SampleUVCDevice *pstUVCDev)
{
    struct v4l2_capability stCap;
    int iRet;
    int iFd;
    char pcDevName[256];

    sprintf(pcDevName, "/dev/video%d", pstUVCDev->iDev);
    iFd = open(pcDevName, O_RDWR);
    if (iFd < 0) {
        printf("open video device failed: device[%s]\n", pcDevName);
        goto open_err;
    }
    iRet = ioctl(iFd, VIDIOC_QUERYCAP, &stCap);
    if (iRet < 0) {
        printf("unable to query device\n");
        goto query_cap_err;
    }
    printf("device is %s on bus %s\n", stCap.card, stCap.bus_info);

    pstUVCDev->iFd = iFd;

    return 0;

query_cap_err:
    close(iFd);
open_err:
    return -1;
}

static void CloseUVCDevice(SampleUVCDevice *pUVCDev)
{
    close(pUVCDev->iFd);
}

typedef int (*ConverFunc)(void *rgbData,void *yuvData,int width,int height);

int NV12ToRGB24(void *RGB24,void *NV12,int width,int height)
{
    unsigned char *src_y = (unsigned char *)NV12;
    unsigned char *src_v = (unsigned char *)NV12 + width * height + 1;
    unsigned char *src_u = (unsigned char *)NV12 + width * height;

    unsigned char *dst_RGB = (unsigned char *)RGB24;

    int temp[3];

    if(RGB24 == NULL || NV12 == NULL || width <= 0 || height <= 0)
    {
        printf(" NV12ToRGB24 incorrect input parameter!\n");
        return -1;
    }

    for(int y = 0;y < height;y ++)
    {
        for(int x = 0;x < width;x ++)
        {
            int Y = y*width + x;
            int U = ( (y >> 1)*(width >>1) + (x >> 1) ) * 2;
            int V = U;

            temp[0] = src_y[Y] + ((7289 * src_u[U])>>12) - 228;  //b
            temp[1] = src_y[Y] - ((1415 * src_u[U])>>12) - ((2936 * src_v[V])>>12) + 136;  //g
            temp[2] = src_y[Y] + ((5765 * src_v[V])>>12) - 180;  //r

            dst_RGB[3*Y] = (temp[0]<0? 0: temp[0]>255? 255: temp[0]);
            dst_RGB[3*Y+1] = (temp[1]<0? 0: temp[1]>255? 255: temp[1]);
            dst_RGB[3*Y+2] = (temp[2]<0? 0: temp[2]>255? 255: temp[2]);
        }
    }

    return 0;

}

static int NV21ToRGB24(void *RGB24,void *NV21,int width,int height)
{
    unsigned char *src_y = (unsigned char *)NV21;
    unsigned char *src_v = (unsigned char *)NV21 + width * height;
    unsigned char *src_u = (unsigned char *)NV21 + width * height + 1;

    unsigned char *dst_RGB = (unsigned char *)RGB24;

    int temp[3];

    if(RGB24 == NULL || NV21 == NULL || width <= 0 || height <= 0){
        printf(" NV21ToRGB24 incorrect input parameter\n");
        return -1;
    }

    for(int y = 0;y < height;y ++){
        for(int x = 0;x < width;x ++){
            int Y = y*width + x;
            int U = ( (y >> 1)*(width >>1) + (x >> 1) )<<1;
            int V = U;

            temp[0] = src_y[Y] + ((7289 * src_u[U])>>12) - 228;  //b
            temp[1] = src_y[Y] - ((1415 * src_u[U])>>12) - ((2936 * src_v[V])>>12) + 136;  //g
            temp[2] = src_y[Y] + ((5765 * src_v[V])>>12) - 180;  //r

            dst_RGB[3*Y] = (temp[0]<0? 0: temp[0]>255? 255: temp[0]);
            dst_RGB[3*Y+1] = (temp[1]<0? 0: temp[1]>255? 255: temp[1]);
            dst_RGB[3*Y+2] = (temp[2]<0? 0: temp[2]>255? 255: temp[2]);
        }
    }

    return 0;
}

int YUYVToRGB24(void *RGB24,void *YUYV,int width,int height)
{
    unsigned char *src_y = (unsigned char *)YUYV;
    unsigned char *src_u = (unsigned char *)YUYV + 1;
    unsigned char *src_v = (unsigned char *)YUYV + 3;

    unsigned char *dst_RGB = (unsigned char *)RGB24;

    int temp[3];

    if(RGB24 == NULL || YUYV == NULL || width <= 0 || height <= 0)
    {
        printf(" YUYVToRGB24 incorrect input parameter!\n");
        return -1;
    }

    for(int i = 0;i < width*height;i ++)
    {
            int Y = 2 * i;
            int U = (i >> 1) * 4;
            int V = U;

            temp[0] = src_y[Y] + ((7289 * src_u[U])>>12) - 228;  //b
            temp[1] = src_y[Y] - ((1415 * src_u[U])>>12) - ((2936 * src_v[V])>>12) + 136;  //g
            temp[2] = src_y[Y] + ((5765 * src_v[V])>>12) - 180;  //r

            dst_RGB[3*i] = (temp[0]<0? 0: temp[0]>255? 255: temp[0]);
            dst_RGB[3*i+1] = (temp[1]<0? 0: temp[1]>255? 255: temp[1]);
            dst_RGB[3*i+2] = (temp[2]<0? 0: temp[2]>255? 255: temp[2]);

    }
    return 0;

}

static int YUVToBMP(const char *bmp_path,char *yuv_data,ConverFunc func,int width,int height)
{
    unsigned char *rgb_24 = NULL;
    FILE *fp = NULL;

#define WORD  unsigned short
#define DWORD unsigned int
#define LONG  unsigned int

/* Bitmap header */
typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
}__attribute__((packed)) BITMAPFILEHEADER;

/* Bitmap info header */
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
}__attribute__((packed)) BITMAPINFOHEADER;

    BITMAPFILEHEADER BmpFileHeader;
    BITMAPINFOHEADER BmpInfoHeader;

    if(bmp_path == NULL || yuv_data == NULL || func == NULL || width <= 0 || height <= 0){
        printf(" YUVToBMP incorrect input parameter!\n");
        return -1;
    }

   /* Fill header information */
   BmpFileHeader.bfType = 0x4d42;
   BmpFileHeader.bfSize = width*height*3 + sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
   BmpFileHeader.bfReserved1 = 0;
   BmpFileHeader.bfReserved2 = 0;
   BmpFileHeader.bfOffBits = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);

   BmpInfoHeader.biSize = sizeof(BmpInfoHeader);
   BmpInfoHeader.biWidth = width;
   BmpInfoHeader.biHeight = height;
   BmpInfoHeader.biPlanes = 0x01;
   BmpInfoHeader.biBitCount = 24;
   BmpInfoHeader.biCompression = 0;
   BmpInfoHeader.biSizeImage = 0;
   //BmpInfoHeader.biXPelsPerMeter = 0;
   //BmpInfoHeader.biYPelsPerMeter = 0;
   BmpInfoHeader.biClrUsed = 0;
   BmpInfoHeader.biClrImportant = 0;

    rgb_24 = (unsigned char *)malloc(width*height*3);
    if(rgb_24 == NULL){
       printf(" YUVToBMP alloc failed!\n");
       return -1;
    }

    func(rgb_24,yuv_data,width,height);

    /* Create bmp file */
    fp = fopen(bmp_path,"wb+");
    if(!fp){
        printf(" Create bmp file:%s faled!\n", bmp_path);
        free(rgb_24);
        return -1;
    }

    fwrite(&BmpFileHeader,sizeof(BmpFileHeader),1,fp);

    fwrite(&BmpInfoHeader,sizeof(BmpInfoHeader),1,fp);

    fwrite(rgb_24,width*height*3,1,fp);

    free(rgb_24);

    fclose(fp);

    return 0;
}

static int saveFrameToFile(void *pfilename, void *start, int length)
{
	FILE *fp = NULL;

	fp = fopen(pfilename, "wb+");
	if (!fp) {
		printf(" Open %s error\n", (char *)pfilename);

		return -1;
	}

	if (fwrite(start, length, 1, fp)) {
		fclose(fp);

		return 0;
	} else {
		printf(" Write file fail (%s)\n", strerror(errno));
		fclose(fp);

		return -1;
	}

	return 0;
}

int onVideoDataEnc(void *app,CdxMuxerPacketT *buff)
{
	SampleUVCDevice *pstUVCDev = (SampleUVCDevice *)app;
	struct timespec ts;
	int ret = 0;

	if(!pstUVCDev || !buff)
		return -1;

	/* wait uvc signal */
#if 0
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_nsec += 33 * 1000 * 1000;
	ret = sem_timedwait(&pstUVCDev->waitUVCsem, &ts);
	if (ret == -1) {
		if (errno == ETIMEDOUT)
			printf("sem_timedwait() timed out\n");
		else
			perror("sem_timedwait");
	}
#else
	sem_wait(&pstUVCDev->waitUVCsem);
#endif
    memcpy(pstUVCDev->pstFrames[pstUVCDev->iCurrBufIndex].pVirAddr, buff->buf, buff->buflen);
	pstUVCDev->iCurrBufBytesUsed = buff->buflen;

#if 0
        {
            static int index = 0;

            index++;
            if((index < 100) && (index%10) == 0){
                char jpg_path[64];

                memset(jpg_path, 0, sizeof(jpg_path));
                sprintf(jpg_path, "/tmp/disp0Test%d.jpg", index);

                saveFrameToFile(jpg_path,
					(char *)pstUVCDev->pstFrames[pstUVCDev->iCurrBufIndex].pVirAddr, pstUVCDev->iCurrBufBytesUsed);
            }
        }
#endif

	/* post uvc buffer fill sem */
	sem_post(&pstUVCDev->localFillSem);

	return 0;
}

static void NotifyCallbackForAwEncorder(void* pUserData, int msg, void* param)
{
#if 0
	TinaRecorder *hdlTina = (TinaRecorder *)pUserData;
	if(!hdlTina)
		return;

    int ret = 0;
    int bufId = 0;
    struct modulePacket *mPacket = NULL;
    struct modulePacket *OutputPacket = NULL;
    struct MediaPacket *outputbuf = NULL;
    VencThumbInfo thumbAddr;
    VencThumbInfo *pthumbAddr;
    DemoRecoderContext* demoRecoder = (DemoRecoderContext*)hdlTina->EncoderContext;
	renderBuf rBuf;
	int scaleWidth,scaleHeight;
    int index = 0;
    int errorCount = 0;

    switch(msg)
    {
        case AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_BUFFER:
        {
            bufId = *((int*)param);

callback_next:
            pthread_mutex_lock(&hdlTina->mEncoderPacketLock);
            for(index = 0; index < ENCODER_SAVE_NUM; index++){
                mPacket = hdlTina->encoderpacket[index];
                if(mPacket){
                    outputbuf = (struct MediaPacket *)(mPacket->buf);
                    if(outputbuf && (bufId == outputbuf->buf_index))
                        break;
                }
            }

            if(index < ENCODER_SAVE_NUM){
                packetDestroy(hdlTina->encoderpacket[index]);
                hdlTina->encoderpacket[index] = NULL;
                pthread_mutex_unlock(&hdlTina->mEncoderPacketLock);
            }else{
                pthread_mutex_unlock(&hdlTina->mEncoderPacketLock);
                TRerr("[%s] encoder %d unknow id %d, sleep wait...\n",
                                                __func__, hdlTina->vport.mCameraIndex, bufId);
                errorCount++;
                if(errorCount > MAX_ERR_NUM){
                    TRerr("[%s] encoder %d unknow id %d error count more than 3,encoder error exit\n",
                                                __func__, hdlTina->vport.mCameraIndex, bufId);
                    modulePort_SetEnable(&hdlTina->encoderPort, 0);
                    demoRecoder->encoderEnable = 0;
                    break;
                }

                usleep(500);
                goto callback_next;
            }
            break;
        }
		case AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_THUMBNAIL:
            if(hdlTina->dispport.enable == 0)
                break;

            pthread_mutex_lock(&hdlTina->mScaleCallbackLock);

			pthumbAddr = (VencThumbInfo *)param;

            if(pthumbAddr->pThumbBuf != (unsigned char *)hdlTina->rBuf.vir_addr){
                TRerr("[%s] encoder %d return thumbnail vir addr error\n",
                                                    __func__, hdlTina->vport.mCameraIndex);

                pthread_mutex_unlock(&hdlTina->mScaleCallbackLock);
                return;
            }

            scaleWidth = ((hdlTina->encodeWidth + 15)/16*16)/hdlTina->scaleDownRatio;
            scaleHeight = ((hdlTina->encodeHeight + 15)/16*16)/hdlTina->scaleDownRatio;

            /* create new packet */
            OutputPacket = (struct modulePacket *)packetCreate(sizeof(struct MediaPacket));
            if(!OutputPacket){
                TRerr("[%s] create packet error\n", __func__);

                pthread_mutex_unlock(&hdlTina->mScaleCallbackLock);
                return ;
            }
            /* fill new packet buf */
            outputbuf = (struct MediaPacket *)OutputPacket->buf;

            /* only push display, ignore buf_index and nPts */
            outputbuf->Vir_Y_addr = (unsigned char *)pthumbAddr->pThumbBuf;
            outputbuf->Vir_C_addr = (unsigned char *)pthumbAddr->pThumbBuf + scaleWidth*scaleHeight;
            outputbuf->Phy_Y_addr = (unsigned char *)hdlTina->rBuf.phy_addr;
            outputbuf->Phy_C_addr = (unsigned char *)hdlTina->rBuf.phy_addr + scaleWidth*scaleHeight;
            outputbuf->buf_index = 0;
            outputbuf->width = scaleWidth;
            outputbuf->height = scaleHeight;
            outputbuf->nPts = 0;
            outputbuf->data_len = scaleWidth*scaleHeight*3/2;
            outputbuf->bytes_used = scaleWidth*scaleHeight*3/2;
            outputbuf->format = TR_PIXEL_YUV420SP;

			/* push new packet to next queue */
            OutputPacket->OnlyMemFlag = 1;
			OutputPacket->packet_type = SCALE_YUV;
			ret = module_push(&hdlTina->encoderPort, OutputPacket);
            if(ret <= 0){
                TRerr("[%s] scale push data error\n", __func__);
                packetDestroy(OutputPacket);
            }
            OutputPacket = NULL;

            pthread_mutex_unlock(&hdlTina->mScaleCallbackLock);

			break;
		case AWENCODER_VIDEO_ENCODER_NOTIFY_THUMBNAIL_GETBUFFER:
            if(hdlTina->dispport.enable == 0)
                break;

            pthread_mutex_lock(&hdlTina->mScaleCallbackLock);

			scaleWidth = ((hdlTina->encodeWidth + 15)/16*16)/hdlTina->scaleDownRatio;
            scaleHeight = ((hdlTina->encodeHeight + 15)/16*16)/hdlTina->scaleDownRatio;

            memset(&rBuf, 0, sizeof(renderBuf));
            ret = hdlTina->dispport.dequeue(&hdlTina->dispport,&rBuf);
            if(ret < 0){
                TRerr("[%s] %d dispport dequeue return error\n",
                                                __func__, hdlTina->vport.mCameraIndex);

                pthread_mutex_unlock(&hdlTina->mScaleCallbackLock);
                return;
            }
            /* save disp physical address */
            memcpy(&hdlTina->rBuf, &rBuf, sizeof(renderBuf));

            thumbAddr.pThumbBuf = (unsigned char *)rBuf.vir_addr;
            thumbAddr.nThumbSize = scaleWidth*scaleHeight*3/2;

			AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetThumbNailAddr,&thumbAddr);

            pthread_mutex_unlock(&hdlTina->mScaleCallbackLock);
			break;
		case AWENCODER_VIDEO_ENCODER_NOTIFY_ERROR:
            TRerr("[%s] error callback from AwRecorder %d.\n",
                                                __func__, hdlTina->vport.mCameraIndex);
			break;
		default:
			TRerr("[%s] unknown callback from AwRecorder %d.\n",
                                                __func__, hdlTina->vport.mCameraIndex);
			break;
	}
#endif
	return ;
}

static int LocalEncoderInit(SampleUVCDevice *pstUVCDev)
{
	AwEncoder *mAwEncoder = NULL;
	VideoEncodeConfig videoConfig;
	EncDataCallBackOps *mEncDataCallBackOps = NULL;
    unsigned char *extractDataBuff = NULL;
    unsigned int extractDataLength;

	/* create AW encoder */
	mAwEncoder = AwEncoderCreate(pstUVCDev);
	if(!mAwEncoder) {
		printf("[%s] can not create AwRecorder, quit.\n", __func__);
		return -1;
	}

	memset(&videoConfig, 0, sizeof(videoConfig));
	videoConfig.nType = VIDEO_ENCODE_JPEG;
	videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV420_NV21;
	videoConfig.nSrcWidth = pstUVCDev->iWidth;
	videoConfig.nSrcHeight = pstUVCDev->iHeight;
	videoConfig.nOutWidth = pstUVCDev->iWidth;
	videoConfig.nOutHeight = pstUVCDev->iHeight;
	videoConfig.nBitRate = 16000000;
	videoConfig.nFrameRate = 30;
	videoConfig.bUsePhyBuf = 1;
	videoConfig.ratio = VENC_ISP_SCALER_0;

	mEncDataCallBackOps = (EncDataCallBackOps *)calloc(1, sizeof(EncDataCallBackOps)); ;
	mEncDataCallBackOps->onVideoDataEnc = onVideoDataEnc;

	/* set callback to recoder,if the encoder has used the buf ,it will callback to app */
	AwEncoderSetNotifyCallback(mAwEncoder, NotifyCallbackForAwEncorder, pstUVCDev);

	AwEncoderInit(mAwEncoder, &videoConfig, NULL, mEncDataCallBackOps);

	AwEncoderStart(mAwEncoder);
	AwEncoderGetExtradata(mAwEncoder, &extractDataBuff, &extractDataLength);

	pstUVCDev->pLocalEncoderHdl = mAwEncoder;

	return 0;
}

static int LocalEncoderDeInit(SampleUVCDevice *pstUVCDev)
{
	int ret = 0;
	AwEncoder *mAwEncoder = NULL;

	if (!pstUVCDev->pLocalEncoderHdl) {
		printf("%s: local AwRecoder handle is NULL,return\n", __func__);
		return -1;
	}

	mAwEncoder = (AwEncoder *)pstUVCDev->pLocalEncoderHdl;

	AwEncoderStop(mAwEncoder);
	AwEncoderDestory(mAwEncoder);

	return 0;
}

static int LocalCameraInit(SampleUVCDevice *pstUVCDev)
{
	int ret = 0;
	int buf_index;
	camera_hal *camera = NULL;

	camera = (camera_hal *)calloc(1, sizeof(camera_hal));
	if (!camera){
		printf("%s: calloc camera_hal failed\n", __func__);
		return -1;
	}

	camera->video_index = 0;
	camera->format = V4L2_PIX_FMT_NV21;
	camera->fps = 30;//pConfig->iCapFrameRate;
	camera->width = pstUVCDev->iWidth;
	camera->height = pstUVCDev->iHeight;

	ret = camerainit(camera);
	if(ret < 0){
		printf("camera init fail\n");
		free(camera);
		return -1;
	}
	camera->state = CAM_OPEN;

	ret = setformat(camera);
	if(ret < 0){
		printf("camera set format fail\n");
			releasecamera(camera);
			free(camera);
			return -1;
	}

	ret = requestbuf(camera);
	if(ret < 0){
			printf("camera request buf fail\n");
			releasecamera(camera);
			free(camera);
			return -1;
	}
	camera->state = CAM_REQUESTBUF;

	ret = streamon(camera);
	if(ret < 0){
			printf("camera stream on fail\n");
			releasebuf(camera);
			releasecamera(camera);
			free(camera);
//			camera->state = CAM_CLOSE;
			return -1;
	}
	camera->state = CAM_STREAMON;
	pstUVCDev->pLocalCameraHdl = camera;

	return ret;
}

static int LocalCameraDeInit(SampleUVCDevice *pstUVCDev)
{
	int ret = 0;
	camera_hal *camera = NULL;

	if (!pstUVCDev->pLocalCameraHdl) {
		printf("%s: local camera handle is NULL,return\n", __func__);
		return -1;
	}

	camera = (camera_hal *)pstUVCDev->pLocalCameraHdl;

	if(camera->state == CAM_STREAMON){
		ret = streamoff(camera);
		if(ret < 0)
				printf("camera stream off fail\n");
		camera->state = CAM_REQUESTBUF;
	}
	if(camera->state == CAM_REQUESTBUF){
		ret = releasebuf(camera);
		if(ret < 0)
				printf("camera release buf fail\n");
		camera->state = CAM_OPEN;
	}
	if(camera->state == CAM_OPEN){
		ret = releasecamera(camera);
		if(ret < 0)
				printf("release camera fail\n");
		camera->state = CAM_CLOSE;
	}

	camera->state = CAM_CLOSE;
	free(camera);
	pstUVCDev->pLocalCameraHdl = NULL;

	return ret;
}

static void *CaptureEncoderThread(void *Ptr)
{
	SampleUVCDevice *pstUVCDev = (SampleUVCDevice *)Ptr;
	camera_hal *camera = (camera_hal *)pstUVCDev->pLocalCameraHdl;
	AwEncoder *mAwEncoder = (AwEncoder *)pstUVCDev->pLocalEncoderHdl;
	int ret = 0;
	int bufIndex = -1;
    VideoInputBuffer videoInputBuffer;

	while (!g_bSampleExit) {
		if (waitingbuf(camera) != 0)
			continue;

		bufIndex = dqbuf(camera);
		if (bufIndex < 0){
			printf("dqbuf fail\n");
            continue;
        }

		/* fill encoder buffer */
		memset(&videoInputBuffer, 0, sizeof(videoInputBuffer));
		videoInputBuffer.nID = bufIndex;
		videoInputBuffer.nPts = camera->buffers[bufIndex].image_timestamp;
		videoInputBuffer.pData = (unsigned char *)camera->buffers[bufIndex].start[0];;
		videoInputBuffer.pAddrPhyY = (unsigned char *)camera->buffers[bufIndex].phy_addr;
		videoInputBuffer.pAddrPhyC = (unsigned char *)camera->buffers[bufIndex].phy_addr + pstUVCDev->iWidth * pstUVCDev->iHeight;
		videoInputBuffer.nLen = camera->buffers[bufIndex].bytes_used;

#if 0
        {
            static int index = 0;

            index++;
            if((index < 5) && (index%1) == 0){
                char bmp_path[64];
                ConverFunc ToRGB24;

                memset(bmp_path, 0, sizeof(bmp_path));
                sprintf(bmp_path, "/tmp/disp0Test%d.bmp", index);
                    ToRGB24 = NV21ToRGB24;

                YUVToBMP(bmp_path, (char *)videoInputBuffer.pData, ToRGB24,
                                                1920, 1080);
            }
        }
#endif

		/* process packet */
		ret = AwEncoderWriteYUVdata(mAwEncoder, &videoInputBuffer);
		if (ret < 0) {
			/**/
		}
		/* TO DO: encoder callback qbuf */
		qbuf(camera, bufIndex);
	}
	pthread_exit(NULL);
}

static inline int DoUVCVideoBufProcess(SampleUVCDevice *pstUVCDev)
{
    SampleUVCContext *pConfig = (SampleUVCContext *)pstUVCDev->pPrivite;
    SampleUVCInfo *pUVCInfo = &pConfig->mUVCInfo;
    struct v4l2_buffer stBuf;

    int iRet = 0;


    memset(&stBuf, 0, sizeof(struct v4l2_buffer));
    stBuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    stBuf.memory = V4L2_MEMORY_MMAP;
    iRet = ioctl(pstUVCDev->iFd, VIDIOC_DQBUF, &stBuf);
    if (iRet < 0) {
        printf("Unable to dequeue buffer, iRet = %d, errno(%d)\n", iRet, errno);
        goto dqbuf_err;
    }

	pstUVCDev->iCurrBufIndex = stBuf.index;
	/* post local camera wait */
	sem_post(&pstUVCDev->waitUVCsem);
	/* wait local fill ok sem */
	sem_wait(&pstUVCDev->localFillSem);

    stBuf.bytesused = pstUVCDev->iCurrBufBytesUsed;

    iRet = ioctl(pstUVCDev->iFd, VIDIOC_QBUF, &stBuf);
    if (iRet < 0) {
        printf("Unable to requeue buffer\n");
        goto qbuf_err;
    }

qbuf_err:
dqbuf_err:


    return iRet;
}

static SampleUVCFormat g_pstFormat[] = {
    {
        .iWidth  = 1920,
        .iHeight = 1080,
        .iFormat = V4L2_PIX_FMT_MJPEG,
        .iInterval = 333333, //units: 100ns
    },
};

static int UVCVideoSetFormat(SampleUVCDevice *pstUVCDev)
{
    SampleUVCContext *pstConfig = (SampleUVCContext*)pstUVCDev->pPrivite;
    int iRet;
    printf("iWidth=[%d],iHeight=[%d],iFormat=[0x%08x]\n",
        pstUVCDev->iWidth, pstUVCDev->iHeight, pstUVCDev->iFormat);
    struct v4l2_format stFormat;
    memset(&stFormat, 0, sizeof(struct v4l2_format));
    stFormat.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    stFormat.fmt.pix.pixelformat = pstUVCDev->iFormat;
    stFormat.fmt.pix.width       = pstUVCDev->iWidth;
    stFormat.fmt.pix.height      = pstUVCDev->iHeight;
    stFormat.fmt.pix.field       = V4L2_FIELD_NONE;
    stFormat.fmt.pix.sizeimage   = pstUVCDev->iWidth * pstUVCDev->iHeight * 1.5;
    iRet = ioctl(pstUVCDev->iFd, VIDIOC_S_FMT, &stFormat);
    if (iRet < 0) {
        printf("VIDIOC_S_FMT failed!!.\n");
    }

    iRet = ioctl(pstUVCDev->iFd, VIDIOC_G_FMT, &stFormat);
    printf("width=[%d],height=[%d],sizeimage=[%d]\n",
        stFormat.fmt.pix.width, stFormat.fmt.pix.height, stFormat.fmt.pix.sizeimage);

    return iRet;
}

static void UVCFillStreamingControl(SampleUVCDevice *pstUVCDev, struct uvc_streaming_control *pstCtrl, int iFmtIndex, int iFrmIndex)
{
    SampleUVCContext *pConfig = (SampleUVCContext *)pstUVCDev->pPrivite;

    /* 0: interval fixed
     * 1: keyframe rate fixed
     * 2: Pframe rate fixed
     */
    pstCtrl->bmHint = 0;
    pstCtrl->bFormatIndex    = iFmtIndex + 1;
    pstCtrl->bFrameIndex     = iFrmIndex + 1;
    pstCtrl->dwFrameInterval = g_pstFormat[iFmtIndex].iInterval;
    pstCtrl->wDelay = 0;
    pstCtrl->dwMaxVideoFrameSize = g_pstFormat[iFmtIndex].iWidth * g_pstFormat[iFmtIndex].iHeight;
    pstCtrl->dwMaxPayloadTransferSize = g_pstFormat[iFmtIndex].iWidth * g_pstFormat[iFmtIndex].iHeight;
    pstCtrl->bmFramingInfo = 3; //ignore in JPEG or MJPEG format
    pstCtrl->bPreferedVersion = 1;
    pstCtrl->bMinVersion = 1;
    pstCtrl->bMaxVersion = 1;
}

static void SubscribeUVCEvent(struct SampleUVCDevice *pstUVCDev)
{
    struct v4l2_event_subscription stSub;
#if 1
    UVCFillStreamingControl(pstUVCDev, &pstUVCDev->stProbe, 0, 0);
    UVCFillStreamingControl(pstUVCDev, &pstUVCDev->stCommit, 0, 0);
#endif
    /* subscribe events, for debug, subscribe all events */
    memset(&stSub, 0, sizeof stSub);
    stSub.type = UVC_EVENT_FIRST;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_CONNECT;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_DISCONNECT;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_STREAMON;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_STREAMOFF;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_SETUP;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_DATA;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_LAST;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
}

static void UnSubscribeUVCEvent(struct SampleUVCDevice *pstUVCDev)
{
    struct v4l2_event_subscription stSub;
#if 0
    uvc_fill_streaming_control(pstUVCDev, &pstUVCDev->probe, 0, 0);
    uvc_fill_streaming_control(pstUVCDev, &pstUVCDev->commit, 0, 0);
#endif
    /* subscribe events, for debug, subscribe all events */
    memset(&stSub, 0, sizeof stSub);
    stSub.type = UVC_EVENT_FIRST;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_CONNECT;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_DISCONNECT;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_STREAMON;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_STREAMOFF;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_SETUP;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_DATA;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_LAST;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
}

static int DoUVCEventSetupClassStreaming(SampleUVCDevice *pstUVCDev, struct uvc_event *pstEvent, struct uvc_request_data *pstReq)
{
    struct uvc_streaming_control *pstCtrl;
    uint8_t ucReq = pstEvent->req.bRequest;
    uint8_t ucCtrlSet = pstEvent->req.wValue >> 8;


    if (ucCtrlSet != UVC_VS_PROBE_CONTROL && ucCtrlSet != UVC_VS_COMMIT_CONTROL)
        return 0;

    pstCtrl = (struct uvc_streaming_control *)&pstReq->data[0];
    pstReq->length = sizeof(struct uvc_streaming_control);

    switch (ucReq) {
    case UVC_SET_CUR:
        pstUVCDev->iCtrlSetCur = ucCtrlSet;
        pstReq->length = 34;
        break;

    case UVC_GET_CUR:
	if (ucCtrlSet == UVC_VS_PROBE_CONTROL)
            memcpy(pstCtrl, &pstUVCDev->stProbe, sizeof(struct uvc_streaming_control));
        else
            memcpy(pstCtrl, &pstUVCDev->stCommit, sizeof(struct uvc_streaming_control));
        break;

    case UVC_GET_MIN:
    case UVC_GET_MAX:
    case UVC_GET_DEF:
        UVCFillStreamingControl(pstUVCDev, pstCtrl, 0, 0);
        break;

    case UVC_GET_RES:
        memset(pstCtrl, 0, sizeof(struct uvc_streaming_control));
        break;

    case UVC_GET_LEN:
        pstReq->data[0] = 0x00;
        pstReq->data[1] = 0x22;
        pstReq->length = 2;
        break;

    case UVC_GET_INFO:
        pstReq->data[0] = 0x03;
        pstReq->length = 1;
        break;
    }

    return 0;
}

static int DoUVCEventSetupClass(SampleUVCDevice *pstUVCDev, struct uvc_event *pstEvent, struct uvc_request_data *pstReq)
{
    if ((pstEvent->req.bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
        return 0;

    switch (pstEvent->req.wIndex & 0xff) {
    case UVC_INTF_CONTROL:
        break;

    case UVC_INTF_STREAMING:
        DoUVCEventSetupClassStreaming(pstUVCDev, pstEvent, pstReq);
        break;

    default:
        break;
    }

    return 0;
}

static int DoUVCEventSetup(SampleUVCDevice *pstUVCDev, struct uvc_event *pstEvent, struct uvc_request_data *pstReq)
{
    switch(pstEvent->req.bRequestType & USB_TYPE_MASK) {
        /* USB_TYPE_STANDARD: kernel driver will process it */
        case USB_TYPE_STANDARD:
        case USB_TYPE_VENDOR:
//            printf("do not care\n");
            break;
        case USB_TYPE_CLASS:
            DoUVCEventSetupClass(pstUVCDev, pstEvent, pstReq);
            break;

        default: break;
    }

    return 0;
}

static int DoUVCEventData(SampleUVCDevice *pstUVCDev, struct uvc_request_data *pstReq)
{
    struct uvc_streaming_control *pstTarget;

    switch(pstUVCDev->iCtrlSetCur) {
        case UVC_VS_PROBE_CONTROL:
            printf("setting probe control, length = %d\n", pstReq->length);
            pstTarget = &pstUVCDev->stProbe;
            break;

        case UVC_VS_COMMIT_CONTROL:
            printf("setting commit control, length = %d\n", pstReq->length);
            pstTarget = &pstUVCDev->stCommit;
            break;

        default:
            printf("setting unknown control, length = %d\n", pstReq->length);
            return 0;
    }

    struct uvc_streaming_control *pstCtrl;
    pstCtrl = (struct uvc_streaming_control*)&pstReq->data[0];

    memcpy(pstTarget, pstCtrl, sizeof(struct uvc_streaming_control));

    if (pstUVCDev->iCtrlSetCur == UVC_VS_COMMIT_CONTROL) {
        pstUVCDev->iWidth  = g_pstFormat[pstCtrl->bFormatIndex - 1].iWidth;
        pstUVCDev->iHeight = g_pstFormat[pstCtrl->bFormatIndex - 1].iHeight;
        pstUVCDev->iFormat = g_pstFormat[pstCtrl->bFormatIndex - 1].iFormat;
        UVCVideoSetFormat(pstUVCDev);
    }

    return 0;
}

static int DoUVDReqReleaseBufs(SampleUVCDevice *pstUVCDev,  int iBufsNum)
{
    int iRet;

    if (iBufsNum > 0) {


        struct v4l2_requestbuffers stReqBufs;
        memset(&stReqBufs, 0, sizeof(struct v4l2_requestbuffers));
        stReqBufs.count  = iBufsNum;
        stReqBufs.memory = V4L2_MEMORY_MMAP;
        stReqBufs.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        iRet = ioctl(pstUVCDev->iFd, VIDIOC_REQBUFS, &stReqBufs);
        if (iRet < 0) {
            printf("VIDIOC_REQBUFS failed!!\n");
            goto reqbufs_err;
        }

        pstUVCDev->pstFrames = malloc(iBufsNum * sizeof(SampleUVCFrame));

        for (int i = 0; i < iBufsNum; i++) {
            struct v4l2_buffer stBuffer;
            memset(&stBuffer, 0, sizeof(struct v4l2_buffer));

            stBuffer.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
            stBuffer.memory = V4L2_MEMORY_MMAP;
            stBuffer.index  = i;

            iRet = ioctl(pstUVCDev->iFd, VIDIOC_QUERYBUF, &stBuffer);
            if (iRet < 0) {
                printf("VIDIOC_QUERYBUF failed!!\n");
            }

            pstUVCDev->pstFrames[i].pVirAddr = mmap(0, stBuffer.length,
                PROT_READ | PROT_WRITE, MAP_SHARED, pstUVCDev->iFd, stBuffer.m.offset);
            pstUVCDev->pstFrames[i].iBufLen  = stBuffer.length;

            ioctl(pstUVCDev->iFd, VIDIOC_QBUF, &stBuffer);

        }

        pstUVCDev->iBufsNum = stReqBufs.count;
        printf("request [%d] buffers success\n", pstUVCDev->iBufsNum);
    } else {
        for (int i = 0; i < pstUVCDev->iBufsNum; i++) {
            munmap(pstUVCDev->pstFrames[i].pVirAddr, pstUVCDev->pstFrames[i].iBufLen);
        }

        free(pstUVCDev->pstFrames);
    }
reqbufs_err:
    return iRet;
}

static int DoUVCStreamOnOff(SampleUVCDevice *pstUVCDev, int iOn)
{
    int iRet;
    enum v4l2_buf_type stBufType;
    stBufType = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    if (iOn) {
        iRet = ioctl(pstUVCDev->iFd, VIDIOC_STREAMON, &stBufType);
        if (iRet == 0) {
            printf("begin to streaming\n");
            pstUVCDev->bIsStreaming = 1;
        }
    } else {
        iRet = ioctl(pstUVCDev->iFd, VIDIOC_STREAMOFF, &stBufType);
        if (iRet == 0) {
            printf("stop to streaming\n");
            pstUVCDev->bIsStreaming = 0;
        }
    }

    return iRet;
}

static int DoUVCEventProcess(SampleUVCDevice *pstUVCDev)
{
    int iRet;
    struct v4l2_event stEvent;
    struct uvc_event *pstUVCEvent = (struct uvc_event*)&stEvent.u.data[0];
    struct uvc_request_data stUVCReq;
    memset(&stUVCReq, 0, sizeof(struct uvc_request_data));
    stUVCReq.length = -EL2HLT;

    iRet = ioctl(pstUVCDev->iFd, VIDIOC_DQEVENT, &stEvent);
    if (iRet < 0) {
	printf("%s VIDIOC_DQEVENT fail\n", __func__);
        goto qevent_err;
    }

    printf("event is 0x%x\n", stEvent.type);
    switch (stEvent.type) {
        case UVC_EVENT_CONNECT:
            printf("uvc event first.\n");
            break;
        case UVC_EVENT_DISCONNECT:
            printf("uvc event disconnect.\n");
            break;
        case UVC_EVENT_STREAMON:
            printf("uvc event stream on.\n");
            DoUVDReqReleaseBufs(pstUVCDev, 1);
			sem_init(&pstUVCDev->waitUVCsem, 0, 0);
			sem_init(&pstUVCDev->localFillSem, 0, 0);
			LocalEncoderInit(pstUVCDev);
			LocalCameraInit(pstUVCDev);
            DoUVCStreamOnOff(pstUVCDev, 1);
			pthread_create(&pstUVCDev->localCameraPthread, NULL, CaptureEncoderThread, pstUVCDev);
            goto qevent_err;
        case UVC_EVENT_STREAMOFF:
            printf("uvc event stream off.\n");
			g_bSampleExit = 1;
			LocalCameraDeInit(pstUVCDev);
			LocalEncoderDeInit(pstUVCDev);
//			LocalCameraDeInit(pstUVCDev);
			pthread_join(pstUVCDev->localCameraPthread, NULL);
			pthread_cancel(pstUVCDev->localCameraPthread);
			sem_destroy(&pstUVCDev->waitUVCsem);
			sem_destroy(&pstUVCDev->localFillSem);
            DoUVDReqReleaseBufs(pstUVCDev, 0);
            DoUVCStreamOnOff(pstUVCDev, 0);
            goto qevent_err;
        case UVC_EVENT_SETUP:
            printf("uvc event setup.\n");
            DoUVCEventSetup(pstUVCDev, pstUVCEvent, &stUVCReq);
            break;
        case UVC_EVENT_DATA:
            DoUVCEventData(pstUVCDev, &pstUVCEvent->data);
            printf("uvc event data.\n");
            break;

        default: break;
    }

    ioctl(pstUVCDev->iFd, UVCIOC_SEND_RESPONSE, &stUVCReq);
qevent_err:
    return iRet;
}

static void usage(const char *argv0)
{
    printf(
        "\033[33m"
        "exec [-h|--help] [-p|--path]\n"
        "   <-h|--help>: print the help information\n"
        "   <-p|--path>       <args>: point to the configuration file path\n"
        "   <-x|--width>      <args>: set video picture width\n"
        "   <-y|--height>     <args>: set video picture height\n"
        "   <-f|--framerate>  <args>: set the video frame rate\n"
        "   <-b|--bulk>       <args>: Use bulk mode or not[0|1]\n"
        "   <-d|--device>     <args>: uvc video device number[0-3]\n"
        "   <-i|--image>      <args>: MJPEG image\n"
        "\033[0m\n");
}

static int LoadSampleUVCConfig(SampleUVCContext *pConfig, const char *conf_path)
{
    int iRet;

    CONFPARSER_S stConfParser;
    iRet = createConfParser(conf_path, &stConfParser);
    if(iRet < 0)
    {
        pConfig->iUVCDev = 2;
        pConfig->iCapDev = 1;
        pConfig->iCapWidth  = 1920;
        pConfig->iCapHeight = 1080;
        pConfig->iCapFrameRate = 30;
        pConfig->eCapFormat = V4L2_PIX_FMT_NV21;
        pConfig->eEncoderType = VIDEO_ENCODE_JPEG;
        pConfig->iEncBitRate  = 4194304;
        pConfig->iEncWidth  = 1920;
        pConfig->iEncHeight = 1080;
        pConfig->iEncFrameRate = 30;
        pConfig->iEncQuality = 99;
        goto use_default_conf;
    }

    if (pConfig->iUVCDev == -1) {
        pConfig->iUVCDev = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_UVC_DEVICE, 0);
    }
    if (pConfig->iCapDev == -1) {
        pConfig->iCapDev = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_VIN_DEVICE, 0);
    }
    if (pConfig->iCapWidth == -1) {
        pConfig->iCapWidth = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_CAP_WIDTH, 0);
    }
    if (pConfig->iCapHeight == -1) {
        pConfig->iCapHeight = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_CAP_HEIGHT, 0);
    }
    if (pConfig->iCapFrameRate == -1) {
        pConfig->iCapFrameRate = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_CAP_FRAMERATE, 0);
    }
    if (pConfig->iEncBitRate == -1) {
        pConfig->iEncBitRate = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_ENC_BITRATE, 0);
    }
    if (pConfig->iEncQuality == -1) {
        pConfig->iEncQuality = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_ENC_QUALITY, 0);
    }
    if (pConfig->iEncWidth == -1) {
        pConfig->iEncWidth = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_ENC_WIDTH, 0);
    }
    if (pConfig->iEncHeight == -1) {
        pConfig->iEncHeight = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_ENC_HEIGHT, 0);
    }
    if (pConfig->iEncFrameRate == -1) {
        pConfig->iEncFrameRate = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_ENC_FRAMERATE, 0);
    }

    char *pcTmpPtr;
    pcTmpPtr = (char *)GetConfParaString(&stConfParser, SAMPLE_UVC_KEY_CAP_FMT, NULL);
    if (NULL != pcTmpPtr && pConfig->eCapFormat == -1) {
        if (!strcmp(pcTmpPtr, "nv21"))
        {
            pConfig->eCapFormat = V4L2_PIX_FMT_NV21;
        }
        else
        {
            pConfig->eCapFormat = V4L2_PIX_FMT_NV21;
        }
    }

    pcTmpPtr = (char *)GetConfParaString(&stConfParser, SAMPLE_UVC_KEY_ENCODEER_TYPE, NULL);
    if (NULL != pcTmpPtr && pConfig->eEncoderType == -1) {
        if (!strcmp(pcTmpPtr, "mjpeg")) {
            pConfig->eEncoderType = VIDEO_ENCODE_JPEG;
        } else {
            pConfig->eEncoderType = VIDEO_ENCODE_JPEG;
        }
    }

use_default_conf:
    destroyConfParser(&stConfParser);
    return 0;
}

static struct option pstLongOptions[] = {
   {"help",        no_argument,       0, 'h'},
   {"bulk",        required_argument, 0, 'b'},
   {"path",        required_argument, 0, 'p'},
   {"width",       required_argument, 0, 'x'},
   {"height",      required_argument, 0, 'y'},
   {"framerate",   required_argument, 0, 'f'},
   {"device",      required_argument, 0, 'd'},
   {0,             0,                 0,  0 }
};

static int ParseCmdLine(int argc, char **argv, SampleUVCContext *pCmdLinePara)
{
    int mRet;
    int iOptIndex = 0;

    memset(pCmdLinePara, -1, sizeof(SampleUVCContext));
    pCmdLinePara->mCmdLinePara.mConfigFilePath[0] = 0;
    while (1) {
        mRet = getopt_long(argc, argv, ":p:b:x:y:f:d:h", pstLongOptions, &iOptIndex);
        if (mRet == -1) {
            break;
        }

        switch (mRet) {
            /* let the "sampleXXX -path sampleXXX.conf" command to be compatible with
             * "sampleXXX -p sampleXXX.conf"
             */
            case 'p':
                if (strcmp("ath", optarg) == 0) {
                    if (NULL == argv[optind]) {
                        usage(argv[0]);
                        goto opt_need_arg;
                    }
                    printf("path is [%s]\n", argv[optind]);
                    strncpy(pCmdLinePara->mCmdLinePara.mConfigFilePath, argv[optind], sizeof(pCmdLinePara->mCmdLinePara.mConfigFilePath));
                } else {
                    printf("path is [%s]\n", optarg);
                    strncpy(pCmdLinePara->mCmdLinePara.mConfigFilePath, optarg, sizeof(pCmdLinePara->mCmdLinePara.mConfigFilePath));
                }
                break;
            case 'x':
                pCmdLinePara->iCapWidth = atoi(optarg);
                break;
            case 'y':
                pCmdLinePara->iCapHeight = atoi(optarg);
                break;
            case 'f':
                pCmdLinePara->iCapFrameRate = atoi(optarg);
                break;
            case 'b':
                // fix
                break;
            case 'd':
                pCmdLinePara->iUVCDev = atoi(optarg);
                break;
            case 'h':
                usage(argv[0]);
                goto print_help_exit;
                break;
            case ':':
                goto opt_need_arg;
                break;
            case '?':
                if (optind > 2) {
                    break;
                }
                usage(argv[0]);
                goto unknow_option;
                break;
            default:
                printf("?? why getopt_long returned character code 0%o ??\n", mRet);
                break;
        }
    }

    return 0;
opt_need_arg:
unknow_option:
print_help_exit:
    return -1;
}

void SignalHandle(int iArg)
{
    g_bSampleExit = 1;
}

int main(int argc, char *argv[])
{
    int iRet;
    camera_hal *camera = (camera_hal *)calloc(1, sizeof(camera_hal));
    SampleUVCContext stContext;
    memset(&stContext, 0, sizeof(SampleUVCContext));

    iRet = ParseCmdLine(argc, argv, &stContext);
    if (iRet < 0) {
        printf("parse cmdline error.\n");
	free(camera);
        return -1;
    }

    /* parse config file. */
    if(LoadSampleUVCConfig(&stContext , stContext.mCmdLinePara.mConfigFilePath) != 0)
    {
        iRet = -1;
        goto load_conf_err;
    }

    SampleUVCDevice *pstUVCDev = &stContext.stUVCDev;
    pstUVCDev->iDev = stContext.iUVCDev;
    pstUVCDev->pPrivite = (void *)&stContext;
    iRet = OpenUVCDevice(pstUVCDev);
    if (iRet < 0) {
        printf("open uvc video device failed!!\n");
        iRet = -1;
        goto uvc_open_err;
    }
    pstUVCDev->bIsStreaming = 0;
    SubscribeUVCEvent(pstUVCDev);

    signal(SIGINT, SignalHandle);
    fd_set stFdSet;
    FD_ZERO(&stFdSet);
    FD_SET(pstUVCDev->iFd, &stFdSet);

    while (1) {
        fd_set stErSet = stFdSet;
        fd_set stWrSet = stFdSet;
        struct timeval stTimeVal;
        /* Wait up to five seconds. */
        stTimeVal.tv_sec = 0;
        stTimeVal.tv_usec = 10*1000;

        if (g_bSampleExit) {
            printf("uvcout g_bSampleExit\n");
            break;
        }

        iRet = select(pstUVCDev->iFd + 1, NULL, &stWrSet, &stErSet, &stTimeVal);
        if (FD_ISSET(pstUVCDev->iFd, &stErSet))
            DoUVCEventProcess(pstUVCDev);
        else if (FD_ISSET(pstUVCDev->iFd, &stWrSet) && pstUVCDev->bIsStreaming) {
            DoUVCVideoBufProcess(pstUVCDev);
        }
    }
    iRet = 0;

init_uvc_err:
start_processing_err:
init_vi2evefave_err:
init_vi2venc_err:
init_frm_err:
    UnSubscribeUVCEvent(pstUVCDev);
    CloseUVCDevice(pstUVCDev);
uvc_open_err:
load_conf_err:
    if (0 == iRet) {
        printf("sample_uvcout exit!\n");
    }
    return iRet;
}
