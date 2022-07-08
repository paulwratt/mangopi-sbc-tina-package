#ifndef _RECORDER_INT_H_
#define _RECORDER_INT_H_

#include "Trecorder.h"
#include "dbList.h"

#define CHECK_NULL_POINTER(e)                                            \
		do {														\
			if (!(e))												\
			{														\
				printf("check (%s) failed.", #e);		   \
				return -1;											\
			}														\
		} while (0)


//#define PARTH_A  "/mnt/SDCARD/DCIMA"
//#define PARTH_B  "/mnt/SDCARD/DCIMB"
#define SENSOR_NUM 1

#define FILE_NAME_PREFIX  "AW_"
typedef struct {
	unsigned int width;
	unsigned int height;
}R_SIZE;

/*
   0: 画中画开，前摄像头主显
   1: 画中画开，后摄像头主显
   2:  画中画关，显示前摄像头
   3: 画中画关, 显示后摄像头
*/
typedef enum tag_PREVIEW_MODE_E{
	PREVIEW_HOST,
	PREVIEW_PIP,
	PREVIEW_
}__preview_mode_e;

typedef enum tag_RECORD_STATE{
	RECORD_UNINIT,
    RECORD_STOP,
	RECORD_START,
	RECORD_HALT,
}__record_state_e;

typedef enum tag_CAMERA_QUALITY{
    CAMERA_QUALITY_100,
    CAMERA_QUALITY_200,
    CAMERA_QUALITY_300,
    CAMERA_QUALITY_500,
    CAMERA_QUALITY_800,
}__camera_quality_e;

typedef enum tag_RECORD_QUALITY{
    RECORD_QUALITY_640_480,
    RECORD_QUALITY_1280_720,
    RECORD_QUALITY_1920_1080,
}__record_quality_e;

typedef enum tag_CYCLE_REC_TIME_E{
    CYCLE_REC_TIME_1_MIM,
    CYCLE_REC_TIME_2_MIM,
    CYCLE_REC_TIME_3_MIM,
    CYCLE_REC_TIME_5_MIM,
    CYCLE_REC_TIME_
}__cycle_rec_time_e;

typedef enum __RECORD_VID_WIN_RATIO_MODE
{
    RECORD_VID_WIN_BESTSHOW = 0x00,        /* 以图片本身的比例缩放至满窗口显示，图片不变形 */
    RECORD_VID_WIN_ORIGINAL,               /* 以图片原始大小在窗口内显示，不能溢出窗口     */
    RECORD_VID_WIN_FULLSCN,                /* 以窗口的比例缩放图片至满窗口显示，可能会变形 */
    RECORD_VID_WIN_CUTEDGE,                /* 裁边模式，在srcFrame区域再裁掉上下黑边，裁边后，以bestshow模式显示         */
    RECORD_VID_WIN_NOTCARE,                /* 不关心图片显示比例，以当前设置的比例         */
    RECORD_VID_WIN_ORIG_CUTEDGE_FULLSCN,    /* 以图片本身的比例缩放至满窗口显示，图片不变,图片超出部分裁剪掉     */
    RECORD_VID_WIN_UNKNOWN
}record_vid_win_ratio_mode_t;

typedef struct tag_DV_CORE{
	//内部赋值
	TrecorderHandle*       mTrecorder;			//摄像头句柄
	__record_state_e	   record_sta;			// 录像的状态
	R_SIZE				   cam_size;			//拍照分辨率
	R_SIZE                 rec_size;			//录像分辨率
	unsigned int           rec_time_ms;			//录像最大文件时间,单位ms
	pthread_mutex_t			mutex0;      //保护碰撞变量
	pthread_mutex_t			mutex1;      //保护录像器状态
	//外部赋值
    TdispRect              show_rect;					// 显示区域
    __camera_quality_e     cam_quality_mode;	//拍照质量
	__record_quality_e     rec_quality_mode;	//录像质量
	R_SIZE				   source_size;			//视频源分辨率
	unsigned int           source_frate;		//视频源帧率
	__cycle_rec_time_e     cycle_rec_time;      //循环录像的时间
	unsigned int           frame_rate;			//帧率
	unsigned int		   video_bps;			//码率
	unsigned int           mute_en;		    // 1 静音 0 不静音
	__preview_mode_e       pre_mode;
	int                    time_water_en;	// 时间水印开关
	TCaptureConfig			phtoto_config;
}__dv_core_t;

typedef struct REC_MEDIA_INFO_T{
    TdispRect              show_rect;					// 主显示区域
    record_vid_win_ratio_mode_t ratio_mode;					//显示模式
    __camera_quality_e     cam_quality_mode;	//拍照质量
	__record_quality_e     rec_quality_mode;	//录像质量
	R_SIZE				   source_size;			//视频源分辨率
	unsigned int		   source_frate;		//视频源帧率
	__cycle_rec_time_e     cycle_rec_time;      //循环录像的时间
	unsigned int           mute_en;			// 1 静音 0 不静音
	__preview_mode_e       pre_mode;
	int                    time_water_en;	// 时间水印开关
}rec_media_info_t;

typedef struct
{
	__cycle_rec_time_e     cycle_rec_time;      //循环录像的时间
	unsigned int           mute_en;			// 1 静音 0 不静音
	int                    time_water_en;	// 时间水印开关
}rec_media_part_info_t;


typedef enum recorder_cmd_t{
	CAMRERA_INIT_CMD,
	CAMRERA_EXIT_CMD,
	AUDIO_START_CMD,
	AUDIO_STOP_CMD,
	PREVIEW_START_CMD,
	PREVIEW_STOP_CMD,
	PREVIEW_DISPLAY_ENABLE_CMD,
	RECORDER_START_CMD,
	RECORDER_STOP_CMD,
	TAKE_PICTURE_CMD,
}recorder_cmd_t;

typedef struct rec_cmd_param_t{
	recorder_cmd_t cmd;
	int index;
	int param[2];
}rec_cmd_param_t;


typedef struct recorder_t
{
	db_list_t*			queue_head;
	pthread_t			id;
	__dv_core_t			dv_core[SENSOR_NUM];
	char				*mount_path;
	char				photo_path[128];
	char				video_path[128];
	char				audio_path[128];

	pthread_mutex_t   cond_mutex;
	pthread_cond_t	  cond;

	int				wait_flag;
}recorder_t;

recorder_t *recorder_pthread_create(void);
int recorder_pthread_destory(recorder_t *recorder);
int recorder_send_cmd(recorder_t *recorder, recorder_cmd_t cmd, int index, int param);
int recorder_set_sensor_info(recorder_t *recorder, int index);
int recorder_set_mount_path(recorder_t *recorder, char *path);
int get_recorder_path(recorder_t *recorder, int index, int flag);
__record_state_e recorder_get_status(recorder_t *recorder, int index);

#endif
