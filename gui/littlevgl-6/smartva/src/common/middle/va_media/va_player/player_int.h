#ifndef _PLAYER_INT_H_
#define _PLAYER_INT_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>
#include <allwinner/tplayer.h>
#include "dbList.h"

typedef enum
{
	DEFAULAT_CMD = 0,
	INIT_CMD = 1,
	EXIT_CMD = 2,
	PREPARE_CMD = 3,
	PLAY_CMD = 4,
	PAUSE_CMD = 5,
	STOP_CMD = 6,
	SEEK_TO = 7,
	SETTING,
}playerCmd;

enum playerSetParam
{
	SET_VOLUME = 0,
	SET_LOOP,
	SET_SCALE,
	SET_DISPLAY,
	SET_SPEED,
	SET_ROTATE,
	SET_AUDIO,
	SET_SOUND_CHANNEL,
	SET_AUDIO_EQ,
};

typedef enum
{
	EXIT_STATUS = 0,
	INIT_STATUS,
	STOP_STATUS,
	ERROR_STATUS,
	PREPARING_STATUS,
	PREPARED_STATUS,
	PAUSE_STATUS,
	PLAY_STATUS,
	SEEKTO_STATUS,
	SETTING_STATUS,
	COMPLETE_STATUS,
}playerStatus;

#define QUEUE_MAX_SIZE    10

typedef struct PLAYER_T
{
    TPlayer*          mTPlayer;
    int               mSeekable;
    int               mError;
    int               mVideoFrameNum;
    MediaInfo*		  mMediaInfo;
	playerStatus	  mpstatus;
	pthread_t		  id;
    sem_t             mPreparedSem;
	TPlayerNotifyCallback callback;

	db_list_t*		  queue_head;
}player_t;


struct player_param{
	playerCmd cmd;
	long int param[2];
};

int tplayer_init(player_t *tplayer,int player_type);
int tplayer_exit(player_t *tplayer);
int tplayer_play_url(player_t *tplayer, const char *path);
int tplayer_play(player_t *tplayer);
int tplayer_pause(player_t *tplayer);
int tplayer_seekto(player_t *tplayer, int nSeekTimeSec);
int tplayer_stop(player_t *tplayer);
int tplayer_volume(player_t *tplayer, int volume);
int tplayer_switch_audio(player_t *tplayer, int audio);
int tplayer_set_speed(player_t *tplayer, TplayerPlaySpeedType nSpeed);
int tplayer_set_looping(player_t *tplayer, bool bLoop);
int tplayer_set_rotate(player_t *tplayer, TplayerVideoRotateType rotateDegree);
int tplayer_set_audio_eq(player_t *tplayer, AudioEqType type);
int tplayer_get_duration(player_t *tplayer, int* sec);
int tplayer_get_current_pos(player_t *tplayer, int* sec);
int tplayer_set_displayrect(player_t *tplayer, int x, int y, unsigned int width, unsigned int height);
player_t *tplayer_pthread_create(void);
void tplayer_pthread_destory(player_t *tplayer);
void tplayer_set_callback(player_t *tplayer, TPlayerNotifyCallback callback);
MediaInfo* tplayer_get_media_Info(player_t *tplayer);
playerStatus tplayer_get_status(player_t *tplayer);
int tplayer_switch_soundchannel(player_t *tplayer, int soundchannelmode);


#endif
