#ifndef _SAMPLE_UVC_H_
#define _SAMPLE_UVC_H_

#include <semaphore.h>
#include <pthread.h>
#include "video.h"


#define MAX_FILE_PATH_SIZE (256)

typedef struct SampleUVCFormat {
    unsigned int iWidth;
    unsigned int iHeight;
    unsigned int iFormat;
    unsigned int iInterval; // units:100ns
} SampleUVCFormat;

typedef struct SampleUVCFrame {
    void *pVirAddr;
    void *pPhyAddr;
    unsigned int iBufLen;
} SampleUVCFrame;

typedef struct SampleUVCDevice {
    int iDev; // must be set before open video device

    int iFd;

    int bIsStreaming;

    int iCtrlSetCur;
    struct uvc_streaming_control stProbe;
    struct uvc_streaming_control stCommit;

    unsigned int iWidth;
    unsigned int iHeight;
    unsigned int iFormat;
    unsigned int iInterval; // units:100ns

    SampleUVCFrame *pstFrames;
	int iCurrBufIndex;
	int iCurrBufBytesUsed;
    int iBufsNum;

    void *pPrivite;
    void *pLocalCameraHdl;
	void *pLocalEncoderHdl;
	sem_t waitUVCsem;
	sem_t localFillSem;
	pthread_t localCameraPthread;
} SampleUVCDevice;

typedef struct SampleUVCCmdLineParam
{
    char mConfigFilePath[MAX_FILE_PATH_SIZE];

} SampleUVCCmdLineParam;


typedef struct SampleUVCInfo {
    pthread_t tEncTrd;
    pthread_t tFaceTrd;

    int iVippDev;
    int iVippChn;
    int iVencChn;
    int iIspDev;

    pthread_mutex_t mFrmLock;
} SampleUVCInfo;

typedef struct SampleUVCContext
{
    int bUseEve;

    int iUVCDev;
    int iCapDev;
    int iCapWidth;
    int iCapHeight;
    int iCapFrameRate;
    unsigned int eCapFormat;

    unsigned int eEncoderType;
    int iEncWidth;
    int iEncHeight;
    int iEncBitRate;
    int iEncFrameRate;
    int iEncQuality;

    SampleUVCCmdLineParam mCmdLinePara;

    SampleUVCInfo mUVCInfo;
    SampleUVCDevice stUVCDev;
} SampleUVCContext;

int initSampleUVCContext();
int destroySampleUVCContext();

#endif  /* _SAMPLE_UVC_H_ */
