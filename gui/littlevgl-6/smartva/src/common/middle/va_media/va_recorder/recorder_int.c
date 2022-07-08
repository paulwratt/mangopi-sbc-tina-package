#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include "recorder_int.h"
#include "common.h"
#include "smt_config.h"

#define		LOCK
#ifdef LOCK
#define	MUTEX_LOCK(mtx)			if(pthread_mutex_lock(&mtx)){\
									printf("%s:%d mutex lock fail\n",__func__,__LINE__);\
								}
#define	MUTEX_UNLOCK(mtx)		if(pthread_mutex_unlock(&mtx)){\
									printf("%s:%d mutex unlock fail\n",__func__,__LINE__);\
								}
#else
#define MUTEX_LOCK(mtx)
#define MUTEX_UNLOCK(mtx)
#endif

#define	WM_POS_X_F		(180)
#define	WM_POS_Y_F		(156)
#define	WM_POS_X_B		(180)
#define	WM_POS_Y_B		(128)

int recorder_send_cmd(recorder_t *recorder, recorder_cmd_t cmd, int index, int param)
{
	CHECK_NULL_POINTER(recorder);
	rec_cmd_param_t *p = (rec_cmd_param_t *)malloc(sizeof(rec_cmd_param_t));
	p->cmd= cmd;
	p->index = index;
	p->param[0] = param;
	__db_list_put_tail(recorder->queue_head, p);
	MUTEX_LOCK(recorder->cond_mutex);
	pthread_cond_signal(&recorder->cond);
	MUTEX_UNLOCK(recorder->cond_mutex);
	com_info("media ui send event = %d ", p->cmd);
	return 0;
}

int get_recorder_path(recorder_t *recorder, int index, int flag)
{
	DIR *dir;
	char path[64] = {0};
	int status;

	if(recorder->mount_path == NULL){
		com_err("no disk!!!");
		return -1;
	}
	if(flag == 0){
		sprintf(path, "%s/photo%d", recorder->mount_path, index);
	}else if(flag == 1){
		sprintf(path, "%s/video%d", recorder->mount_path, index);
	}else{
		sprintf(path, "%s/audio%d", recorder->mount_path, index);
	}
	if((dir = opendir(path)) == NULL){
		com_warn("%s is not exist, it will be created.\n", path);
		status = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
		if(status < 0){
			com_err("mkdir failed %s.\n", path);
			return -1;
		}
	}else{
		closedir(dir);
	}

	time_t timep;
	struct tm *p;
	time(&timep);

	p = localtime(&timep);

	if(flag == 0){
		memset(recorder->photo_path, 0, sizeof(recorder->photo_path));
		sprintf(recorder->photo_path, "%s/AW_%d%02d%02d_%02d%02d%02dA.jpg", path,
			(1900+p->tm_year),(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	}else if(flag == 1){
		memset(recorder->video_path, 0, sizeof(recorder->video_path));
		sprintf(recorder->video_path, "%s/AW_%d%02d%02d_%02d%02d%02dA.mov", path,
			(1900+p->tm_year),(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	}else{
		memset(recorder->audio_path, 0, sizeof(recorder->audio_path));
		sprintf(recorder->audio_path, "%s/AW_%d%02d%02d_%02d%02d%02dA.aac", path,
			(1900+p->tm_year),(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	}
	return 0;
}

static void __dv_get_record_size(__dv_core_t *dv_core)
{
	switch(dv_core->rec_quality_mode){
	case RECORD_QUALITY_640_480:
		dv_core->rec_size.width = 640;
		dv_core->rec_size.height = 480;
		dv_core->video_bps = 1*1000*1000;
		dv_core->frame_rate = 10;
		break;
	case RECORD_QUALITY_1280_720:
		dv_core->rec_size.width = 1280;
		dv_core->rec_size.height = 720;
		dv_core->video_bps = 6*1000*1000;
		dv_core->frame_rate = 10;
		break;
	case RECORD_QUALITY_1920_1080:
		dv_core->rec_size.width = 1920;
		dv_core->rec_size.height = 1080;
		dv_core->video_bps = 8*1000*1000;
		dv_core->frame_rate = 10;
		break;
	default:
		dv_core->rec_size.width = 1280;
		dv_core->rec_size.height = 720;
		dv_core->video_bps = 3*1000*1000;
		dv_core->frame_rate = 10;
		break;
	}
}

static void __dv_get_camera_size(__dv_core_t *dv_core)
{
    switch(dv_core->cam_quality_mode){
    case CAMERA_QUALITY_100:
        dv_core->cam_size.width = 1280;
        dv_core->cam_size.height = 960;
        break;
    case CAMERA_QUALITY_200:
        dv_core->cam_size.width = 1600;
        dv_core->cam_size.height = 1200;
        break;
    case CAMERA_QUALITY_300:
        dv_core->cam_size.width = 2048;
        dv_core->cam_size.height = 1536;
        break;
    case CAMERA_QUALITY_500:
        dv_core->cam_size.width = 2560;
        dv_core->cam_size.height = 1920;
        break;
    case CAMERA_QUALITY_800:
        dv_core->cam_size.width = 3264;
        dv_core->cam_size.height = 2448;
        break;
    default:
        dv_core->cam_size.width = 1600;
        dv_core->cam_size.height = 1200;
        break;
    }
}

static void __dv_get_cycle_rec_time(__dv_core_t  *dv_core)
{
	switch(dv_core->cycle_rec_time){
	case CYCLE_REC_TIME_1_MIM:
		dv_core->rec_time_ms = 60*1000;
		break;
	case CYCLE_REC_TIME_2_MIM:
		dv_core->rec_time_ms = 2*60*1000;
		break;
	case CYCLE_REC_TIME_3_MIM:
		dv_core->rec_time_ms = 3*60*1000;
		break;
	case CYCLE_REC_TIME_5_MIM:
		dv_core->rec_time_ms = 5*60*1000;
		break;
	default:
		dv_core->rec_time_ms = 2*60*1000;
		break;
	}
}

int recorder_set_mount_path(recorder_t *recorder, char *path)
{
	CHECK_NULL_POINTER(recorder);
	recorder->mount_path = path;
	return 0;
}

int recorder_set_sensor_info(recorder_t *recorder, int index)
{
	__dv_core_t *info;

	CHECK_NULL_POINTER(recorder);

	info = &recorder->dv_core[index];
	info->show_rect.x = (800 - 640)/2;
	info->show_rect.y = 0;
	info->show_rect.width = 640;
	info->show_rect.height = 480;
	info->cam_quality_mode = CAMERA_QUALITY_100;
	info->cycle_rec_time = CYCLE_REC_TIME_1_MIM;
	info->mute_en = 0;
	info->pre_mode = PREVIEW_HOST;
	info->rec_quality_mode = RECORD_QUALITY_640_480;
	info->source_size.width = 640;
	info->source_size.height = 480;
	info->source_frate = 30;
	info->time_water_en = 0;
	__dv_get_camera_size(info);
	__dv_get_record_size(info);
	__dv_get_cycle_rec_time(info);
	return 0;
}

static int CallbackFromTRecorder0(void* pUserData, int msg, void* param)
{
//    __dv_core_t  *dv_core = ( __dv_core_t  *)pUserData;
	char parth[128];
	memset(parth, 0, sizeof(parth));
    switch(msg){
    case T_RECORD_ONE_FILE_COMPLETE:
        com_info("T_RECORD_ONE_FILE_COMPLETE 0\n");
		//TRchangeOutputPath(dv_core->mTrecorder[0],parth);
		break;
    default:
        com_warn("warning: unknown callback from trecorder\n");
        break;
	}
    return 0;
}

static void *recorder_pthread(void *arg)
{
	recorder_t *recorder = (recorder_t *)arg;
	__dv_core_t *dv_core;
	rec_cmd_param_t  *cmd_param;
	int index;
	int empty;

	while(1){
		empty = is_list_empty(recorder->queue_head);
		if(empty){
			MUTEX_LOCK(recorder->cond_mutex);
			pthread_cond_wait(&recorder->cond, &recorder->cond_mutex);
			MUTEX_UNLOCK(recorder->cond_mutex);
			continue;
		}
		cmd_param = (struct rec_cmd_param_t *)__db_list_pop(recorder->queue_head);
		com_info("cmd = %u\n", cmd_param->cmd);

		index = cmd_param->index;
		dv_core = &recorder->dv_core[index];
		switch(cmd_param->cmd){
			case CAMRERA_INIT_CMD:
				if(dv_core->mTrecorder){
					com_warn("Trecorder already init");
					break;
				}
				if(get_recorder_path(recorder, index, 1) < 0){
					break;
				}
				dv_core->mTrecorder = CreateTRecorder();
				if(dv_core->mTrecorder == NULL){
					com_err("CreateTRecorder err0\n");
					break;
				}
				TRreset(dv_core->mTrecorder);
				TRsetRecorderCallback(dv_core->mTrecorder, CallbackFromTRecorder0,(void*)dv_core);

				recorder_set_sensor_info(recorder, index);
				TRsetCamera(dv_core->mTrecorder, index == 0 ? T_CAMERA_FRONT : T_CAMERA_BACK);
				TRsetAudioSrc(dv_core->mTrecorder,T_AUDIO_MIC0);
				TRsetPreview(dv_core->mTrecorder, index == 0 ? T_DISP_LAYER0 : T_DISP_LAYER1);
				TRsetCameraEnableWM(dv_core->mTrecorder, WM_POS_X_F, WM_POS_Y_F, dv_core->time_water_en);

				TRsetOutput(dv_core->mTrecorder, recorder->video_path);
				TRsetOutputFormat(dv_core->mTrecorder,T_OUTPUT_MOV); // 设置封装格式
				//TRsetOutputFormat(dv_core->mTrecorder[index],T_OUTPUT_TS); // 设置封装格式
				TRsetVideoEncoderFormat(dv_core->mTrecorder,T_VIDEO_H264); //设置视频编码格式
				TRsetAudioEncoderFormat(dv_core->mTrecorder,T_AUDIO_AAC); // 设置音频编码格式

				TRsetMaxRecordTimeMs(dv_core->mTrecorder, dv_core->rec_time_ms);
				TRsetRecorderEnable(dv_core->mTrecorder,1);
				TRsetEncoderBitRate(dv_core->mTrecorder, dv_core->video_bps);
				TRsetEncodeFramerate(dv_core->mTrecorder, dv_core->frame_rate);
				TRsetVideoEncodeSize(dv_core->mTrecorder,dv_core->rec_size.width,dv_core->rec_size.height);

				TRsetCameraEnable(dv_core->mTrecorder,1);
				TRsetCameraInputFormat(dv_core->mTrecorder,T_CAMERA_YUV420SP);
				TRsetCameraFramerate(dv_core->mTrecorder,dv_core->source_frate);
				TRsetCameraCaptureSize(dv_core->mTrecorder,dv_core->source_size.width, dv_core->source_size.height);

				TRsetMICEnable(dv_core->mTrecorder, !dv_core->mute_en);
				TRsetMICInputFormat(dv_core->mTrecorder,T_MIC_PCM);
				TRsetMICSampleRate(dv_core->mTrecorder,8000);
				TRsetMICChannels(dv_core->mTrecorder,2);

				TRsetAudioMute(dv_core->mTrecorder, dv_core->mute_en);

				TRsetPreviewRoute(dv_core->mTrecorder,T_ROUTE_CAMERA);
				TRsetPreviewEnable(dv_core->mTrecorder,1);
				TRsetPreviewRotate(dv_core->mTrecorder,T_ROTATION_ANGLE_0);
				TRsetVEScaleDownRatio(dv_core->mTrecorder,0);

				TRsetPreviewRect(dv_core->mTrecorder,&dv_core->show_rect);
				if(dv_core->pre_mode == PREVIEW_HOST){
					TRsetPreviewZorder(dv_core->mTrecorder, T_PREVIEW_ZORDER_BOTTOM);
				}else{
					TRsetPreviewZorder(dv_core->mTrecorder, T_PREVIEW_ZORDER_MIDDLE);
				}
				TRprepare(dv_core->mTrecorder);
				dv_core->record_sta = RECORD_STOP;
				break;
			case CAMRERA_EXIT_CMD:
				if(!dv_core->mTrecorder){
					com_err("camera index %d is not init.\n", index);
					break;
				}
				TRsetPreviewEnable(dv_core->mTrecorder, 0);
				TRstop(dv_core->mTrecorder, T_ALL);
				TRrelease(dv_core->mTrecorder);
				dv_core->mTrecorder = NULL;
				dv_core->record_sta = RECORD_UNINIT;
				break;
			case AUDIO_START_CMD:
				if(dv_core->mTrecorder){
					com_warn("Trecorder already init");
					break;
				}
				if(get_recorder_path(recorder, index, 2) < 0){
					break;
				}
				dv_core->mTrecorder = CreateTRecorder();
				if(dv_core->mTrecorder == NULL){
					com_err("CreateTRecorder err0\n");
					break;
				}
				recorder_set_sensor_info(recorder, index);
				TRreset(dv_core->mTrecorder);
				TRsetRecorderCallback(dv_core->mTrecorder, CallbackFromTRecorder0,(void*)dv_core);
				TRsetAudioSrc(dv_core->mTrecorder, T_AUDIO_MIC0);
				TRsetOutput(dv_core->mTrecorder, recorder->audio_path);
				TRsetOutputFormat(dv_core->mTrecorder, T_OUTPUT_AAC); // 设置封装格式
				TRsetAudioEncoderFormat(dv_core->mTrecorder, T_AUDIO_AAC); // 设置音频编码格式
				TRsetMaxRecordTimeMs(dv_core->mTrecorder, dv_core->rec_time_ms);
				TRsetRecorderEnable(dv_core->mTrecorder, 1);
				TRsetMICEnable(dv_core->mTrecorder, 1);
				TRsetMICInputFormat(dv_core->mTrecorder, T_MIC_PCM);
				TRsetMICSampleRate(dv_core->mTrecorder, 44100);
				TRsetMICChannels(dv_core->mTrecorder, 2);
				TRsetAudioMute(dv_core->mTrecorder, 0);
				TRprepare(dv_core->mTrecorder);
				TRstart(dv_core->mTrecorder, T_RECORD);
				dv_core->record_sta = RECORD_START;
				break;
			case AUDIO_STOP_CMD:
				if(!dv_core->mTrecorder){
					com_err("preview start index %d is not init.\n", index);
					break;
				}
				TRstop(dv_core->mTrecorder, T_RECORD);
				TRrelease(dv_core->mTrecorder);
				dv_core->mTrecorder = NULL;
				dv_core->record_sta = RECORD_UNINIT;
				break;
			case PREVIEW_START_CMD:
				if(!dv_core->mTrecorder){
					com_err("preview start index %d is not init.\n", index);
					break;
				}
				TRstart(dv_core->mTrecorder, T_PREVIEW);
				break;
			case PREVIEW_STOP_CMD:
				if(!dv_core->mTrecorder){
					com_err("preview stop index %d is not init.\n", index);
					break;
				}
				TRstop(dv_core->mTrecorder, T_PREVIEW);
				break;
			case PREVIEW_DISPLAY_ENABLE_CMD:
				if(!dv_core->mTrecorder){
					com_err("record start index %d is not init.\n", index);
					break;
				}
				TRsetPreviewEnable(dv_core->mTrecorder[index], cmd_param->param[0]);
				break;
			case RECORDER_START_CMD:
				if(!dv_core->mTrecorder){
					com_err("record start index %d is not init.\n", index);
					break;
				}
				TRstart(dv_core->mTrecorder, T_RECORD);
				dv_core->record_sta = RECORD_START;
				break;
			case RECORDER_STOP_CMD:
				if(!dv_core->mTrecorder){
					com_err("record stop index %d is not init.\n", index);
					break;
				}
				TRstop(dv_core->mTrecorder, T_RECORD);
				dv_core->record_sta = RECORD_STOP;
				break;
			case TAKE_PICTURE_CMD:
				if(!dv_core->mTrecorder){
					com_err("take picture stop index %d is not init.\n", index);
					break;
				}
				if(get_recorder_path(recorder, index, 0) < 0){
					break;
				}
				TCaptureConfig config;

				memset(&config, 0, sizeof(TCaptureConfig));
				sprintf(config.capturePath, "%s", recorder->photo_path);
				config.captureFormat = T_CAPTURE_JPG;
				config.captureWidth = dv_core->cam_size.width;
				config.captureHeight = dv_core->cam_size.height;
				TRCaptureCurrent(dv_core->mTrecorder, &config);
				break;
			default:
				break;
		}

		free(cmd_param);
	}
    return NULL;
}

recorder_t *recorder_pthread_create(void)
{
	recorder_t *recorder = NULL;
	int result;
	int i;

	recorder = (recorder_t *)malloc(sizeof(recorder_t));
	if(recorder == NULL){
		com_err("tplayer failed");
		goto end;
	}
	memset(recorder, 0, sizeof(recorder_t));
	recorder->queue_head = db_list_create("record_list", 1);
#ifdef	LOCK
	result = pthread_mutex_init(&recorder->cond_mutex, NULL);
	if(result != 0){
			com_err("pthread mutex init failed!\r\n");
			goto end;
	}
	for(i = 0; i < SENSOR_NUM; i++){
		pthread_mutex_init (&recorder->dv_core[i].mutex0, NULL);
		pthread_mutex_init (&recorder->dv_core[i].mutex1, NULL);
	}
#endif
	pthread_cond_init(&recorder->cond, NULL);
	result = pthread_create(&recorder->id, NULL, recorder_pthread, (void*)recorder);
	if(result != 0){
		com_err("pthread create fail!\r\n");
		recorder = NULL;
		goto end;
	}

	return recorder;
end:
	if(recorder){
		free(recorder);
	}
	return NULL;
}

int recorder_pthread_destory(recorder_t *recorder)
{
	int i;

	CHECK_NULL_POINTER(recorder);

	for(i = 0; i < SENSOR_NUM; i++){
		if(recorder->dv_core[i].record_sta == RECORD_UNINIT){
			continue;
		}
		//TRsetPreviewEnable(recorder->dv_core[i].mTrecorder,0);
		TRstop(recorder->dv_core[i].mTrecorder, T_ALL);
		TRrelease(recorder->dv_core[i].mTrecorder);
	}
	pthread_cancel(recorder->id);
	pthread_join(recorder->id, NULL);
#ifdef	LOCK
	pthread_mutex_destroy(&recorder->cond_mutex);
	for(i = 0; i < SENSOR_NUM; i++){
		pthread_mutex_destroy(&recorder->dv_core[i].mutex0);
		pthread_mutex_destroy(&recorder->dv_core[i].mutex1);
	}
#endif
	pthread_cond_destroy(&recorder->cond);
	__db_list_destory(recorder->queue_head);
	if(recorder){
		free(recorder);
	}
	return 0;
}


__record_state_e recorder_get_status(recorder_t *recorder, int index)
{
	CHECK_NULL_POINTER(recorder);
	return recorder->dv_core[index].record_sta;
}
