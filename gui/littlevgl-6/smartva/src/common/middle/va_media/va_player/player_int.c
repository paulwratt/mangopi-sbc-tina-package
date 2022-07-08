#include "player_int.h"
#include "smt_config.h"
#include "dbList.h"

#define ISNULL(x) if(!x){return -1;}
void tplayer_set_callback(player_t* tplayer, TPlayerNotifyCallback callback)
{
	tplayer->callback = callback;
}

//* a callback for tplayer.
static int CallbackForTPlayer(void* pUserData, int msg, int param0, void* param1)
{
	int videoFormat;
    player_t* pPlayer = (player_t*)pUserData;

    CEDARX_UNUSE(param1);

	if(pPlayer->callback){
		pPlayer->callback(pUserData, msg, param0, param1);
	}
    switch(msg){
	    case TPLAYER_NOTIFY_PREPARED:
	        printf("TPLAYER_NOTIFY_PREPARED,has prepared.\n");
			pPlayer->mpstatus = PREPARED_STATUS;
	        sem_post(&pPlayer->mPreparedSem);
	        break;
	    case TPLAYER_NOTIFY_PLAYBACK_COMPLETE:
	        printf("TPLAYER_NOTIFY_PLAYBACK_COMPLETE\n");
			pPlayer->mpstatus = COMPLETE_STATUS;
	        break;
	    case TPLAYER_NOTIFY_SEEK_COMPLETE:
	        printf("TPLAYER_NOTIFY_SEEK_COMPLETE>>>>info: seek ok.\n");
	        break;
	    case TPLAYER_NOTIFY_MEDIA_ERROR:
	        switch (param0)
	        {
	            case TPLAYER_MEDIA_ERROR_UNKNOWN:
	            {
	                printf("erro type:TPLAYER_MEDIA_ERROR_UNKNOWN\n");
	                break;
	            }
	            case TPLAYER_MEDIA_ERROR_UNSUPPORTED:
	            {
	                printf("erro type:TPLAYER_MEDIA_ERROR_UNSUPPORTED\n");
	                break;
	            }
	            case TPLAYER_MEDIA_ERROR_IO:
	            {
	                printf("erro type:TPLAYER_MEDIA_ERROR_IO\n");
	                break;
	            }
	        }
			pPlayer->mpstatus = ERROR_STATUS;
			pPlayer->mError = 1;
	        sem_post(&pPlayer->mPreparedSem);
	        printf("error: open media source fail.\n");
	        break;
	    case TPLAYER_NOTIFY_NOT_SEEKABLE:
	        pPlayer->mSeekable = 0;
	        printf("info: media source is unseekable.\n");
	        break;
		case TPLAYER_NOTIFY_BUFFER_START:
	        printf("have no enough data to play\n");
	        break;
		case TPLAYER_NOTIFY_BUFFER_END:
	        printf("have enough data to play again\n");
	        break;
	    case TPLAYER_NOTIFY_VIDEO_FRAME:
	        //printf("get the decoded video frame\n");
	        break;
	    case TPLAYER_NOTIFY_AUDIO_FRAME:
	        break;
	    case TPLAYER_NOTIFY_SUBTITLE_FRAME:
	        //printf("get the decoded subtitle frame\n");
	        break;
		case TPLAYER_NOTIFY_MEDIA_VIDEO_SIZE:
			printf ("@xkt player_int.c callback\n");
			break;
		case TPLAYER_NOTIFY_MEDIA_FORMAT:
			videoFormat = ((int *)param1)[0];
			if ((videoFormat == VIDEO_CODEC_FORMAT_H264) || (videoFormat == VIDEO_CODEC_FORMAT_H265))
				TPlayerSetScaleDownRatio(pPlayer->mTPlayer,TPLAYER_VIDEO_SCALE_DOWN_1, TPLAYER_VIDEO_SCALE_DOWN_1);
			else
				TPlayerSetScaleDownRatio(pPlayer->mTPlayer,TPLAYER_VIDEO_SCALE_DOWN_2, TPLAYER_VIDEO_SCALE_DOWN_2);
			break;
	    default:
	        printf("warning: unknown callback from Tinaplayer.\n");
	        break;
    }
    return 0;
}

static int tplayer_queue(player_t *tplayer, void *param)
{
    int video_resolution[2];
	struct player_param *p = (struct player_param *)param;
	playerCmd cmd = p->cmd;
	playerStatus old_mpstaus;
	int *DisplayRect;

	ISNULL(tplayer);
	if(cmd != INIT_CMD){
		ISNULL(tplayer->mTPlayer);
	}

	switch(cmd){
		case INIT_CMD:
			if(tplayer->mTPlayer != NULL)
			{
				com_warn("tplayer is already created\n");
				goto end;
			}

			if ((int)p->param[0] == CEDARX_PLAYER)
			{
			    tplayer->mTPlayer = TPlayerCreate(CEDARX_PLAYER);
			}
			else if ((int)p->param[0] == AUDIO_PLAYER)
			{
                tplayer->mTPlayer = TPlayerCreate(AUDIO_PLAYER);
			}

			if(tplayer->mTPlayer == NULL)
			{
				com_err("can not create tplayer, quit.\n");
				goto end;
			}
			tplayer->mError = 0;
			tplayer->mSeekable = 1;
			tplayer->mMediaInfo = NULL;
			tplayer->mpstatus = INIT_STATUS;
			TPlayerSetNotifyCallback(tplayer->mTPlayer,CallbackForTPlayer, (void*)tplayer);
			//TPlayerReset(tplayer->mTPlayer);
			TPlayerSetDebugFlag(tplayer->mTPlayer, 0);
			com_info("init finished\n");
			break;
		case EXIT_CMD:
			//TPlayerStop(tplayer->mTPlayer);
			//TPlayerReset(tplayer->mTPlayer);
			TPlayerDestroy(tplayer->mTPlayer);
			tplayer->mTPlayer = NULL;
			tplayer->mpstatus = EXIT_STATUS;
			com_info("exit finished\n");
			break;
		case STOP_CMD:
			TPlayerStop(tplayer->mTPlayer);
			tplayer->mpstatus = STOP_STATUS;
			com_info("stop finished\n");
			break;
		case PREPARE_CMD:
			//TPlayerStop(tplayer->mTPlayer);
			TPlayerReset(tplayer->mTPlayer);
			tplayer->mpstatus = PREPARING_STATUS;
			if(TPlayerSetDataSource(tplayer->mTPlayer, (const char *)p->param[0], (CdxKeyedVectorT *)p->param[1])){
				tplayer->mpstatus = ERROR_STATUS;
                sem_post(&tplayer->mPreparedSem);
				goto end;
			}else{
				com_warn("setDataSource end\n");
			}
            tplayer->mpstatus = PREPARED_STATUS;
            tplayer->mMediaInfo = TPlayerGetMediaInfo(tplayer->mTPlayer);
            if (tplayer->mMediaInfo->pVideoStreamInfo)
            {
                printf ("@xkt nWidth = %d nHeight = %d\n", tplayer->mMediaInfo->pVideoStreamInfo->nWidth,
                    tplayer->mMediaInfo->pVideoStreamInfo->nHeight);
                video_resolution[0] = tplayer->mMediaInfo->pVideoStreamInfo->nWidth;
                video_resolution[1] = tplayer->mMediaInfo->pVideoStreamInfo->nHeight;
                if (video_resolution[0] > 1280 && video_resolution[1] > 720)
                {
                    com_info("more than 720P\r\n");
                    TPlayerStop(tplayer->mTPlayer);
                    tplayer->mpstatus = STOP_STATUS;
                    sem_post(&tplayer->mPreparedSem);
                    if(tplayer->callback)
                    {
                        tplayer->callback(tplayer, TPLAYER_NOTIFY_MEDIA_VIDEO_SIZE, 0, (void *)video_resolution);
                    }
                    goto end;
                }
            }
            sem_post(&tplayer->mPreparedSem);
			break;
		case PAUSE_CMD:
			if(tplayer_get_status(tplayer) < PREPARED_STATUS){
				com_warn("not prepared!\n");
				goto end;
			}
			if(!TPlayerIsPlaying(tplayer->mTPlayer)){
				com_warn("not playing!\n");
				goto end;
			}
			TPlayerPause(tplayer->mTPlayer);
			tplayer->mpstatus = PAUSE_STATUS;
			com_info("pause finished\n");
			break;
		case PLAY_CMD:
			if(tplayer_get_status(tplayer) < PREPARED_STATUS){
				com_warn("not prepared!\n");
				goto end;
			}
			//TPlayerSetHoldLastPicture(tplayer->mTPlayer, 1);
			TPlayerStart(tplayer->mTPlayer);
			if(TPlayerIsPlaying(tplayer->mTPlayer)){
				com_info("play finished\n");
				tplayer->mpstatus = PLAY_STATUS;
			}
			break;
		case SEEK_TO:
			if(tplayer_get_status(tplayer) < PREPARED_STATUS){
				com_warn("not prepared!\n");
				goto end;
			}
			old_mpstaus = tplayer->mpstatus;
			tplayer->mpstatus = SEEKTO_STATUS;
			TPlayerSeekTo(tplayer->mTPlayer, (int)p->param[0] * 1000);
			tplayer->mpstatus = old_mpstaus;
			break;
		case SETTING:
			if(tplayer_get_status(tplayer) < PREPARED_STATUS){
				com_warn("not prepared = %d !\n", (int)p->param[0]);
				goto end;
			}
			old_mpstaus = tplayer->mpstatus;
			tplayer->mpstatus = SETTING_STATUS;
			switch((int)p->param[0]){
				case SET_LOOP:
					TPlayerSetLooping(tplayer->mTPlayer, (bool)p->param[1]);
					break;
				case SET_DISPLAY:
					DisplayRect = (int *)p->param[1];
					TPlayerSetDisplayRect(tplayer->mTPlayer, DisplayRect[0], DisplayRect[1],DisplayRect[2], DisplayRect[3]);
					break;
				case SET_ROTATE:
					TPlayerSetRotate(tplayer->mTPlayer, (TplayerVideoRotateType)p->param[1]);
					break;
				case SET_VOLUME:
					TPlayerSetVolume(tplayer->mTPlayer, (int)p->param[1]);
					break;
				case SET_SPEED:
					TPlayerSetSpeed(tplayer->mTPlayer, (TplayerPlaySpeedType)p->param[1]);
					break;
				case SET_AUDIO:
					TPlayerSwitchAudio(tplayer->mTPlayer, (int)p->param[1]);
					break;
				case SET_SOUND_CHANNEL:
					TPlayerSetSoundChannelMode(tplayer->mTPlayer, (int)p->param[1]);
					break;
				case SET_AUDIO_EQ:
					printf ("@xkt SET_AUDIO_EQ indx = %d\n", (int)p->param[1]);
					TPlayerSetAudioEQType(tplayer->mTPlayer, (int)p->param[1]);
					break;
				default:
					break;
			}
			tplayer->mpstatus = old_mpstaus;
			break;
		default:
			break;
	}
end:
	return 0;
}


static int tplayer_send_cmd(player_t *tplayer, playerCmd cmd, unsigned long int param0, unsigned long int param1)
{
	struct player_param *pparam;

	ISNULL(tplayer);

	pparam = (struct player_param *)malloc(sizeof(struct player_param));
	if (!pparam) {
		com_err("malloc failed!!!");
		return -1;
	}
	memset(pparam, 0x00, sizeof(struct player_param));
	pparam->cmd = cmd;
	pparam->param[0] = param0;
	pparam->param[1] = param1;
	__db_list_put_tail(tplayer->queue_head, (void *)pparam);
#if 0
	//if queue too many, it will wait.
	if(queue_size){}//delete warnning
	while(queue_size > QUEUE_MAX_SIZE){
		com_warn("queue too many, please wait...");
		usleep(10000);
		queue_size = param->tplayer->queue_head->limit_size;
	}
#endif
	return 0;
}

static void *tplayer_pthread(void *arg)
{
	player_t *tplayer = (player_t *)arg;
	struct player_param *queue;

	while(1) {
		queue = (struct player_param *)__db_list_pop(tplayer->queue_head);
		if (NULL == queue) {
			com_info("queue null");
			continue;
		}
		com_info("cmd = %u\n", queue->cmd);
		tplayer_queue(tplayer, queue);
		free(queue);
	}
    return NULL;
}

player_t *tplayer_pthread_create(void)
{
	player_t *tplayer;
	int result;
	pthread_attr_t attr;


	tplayer = (player_t *)malloc(sizeof(player_t));
	if(tplayer == NULL){
		com_err("tplayer failed");
		goto end;
	}
	memset(tplayer, 0, sizeof(player_t));
	tplayer->queue_head = db_list_create("tplayer_list", 1);
	sem_init(&tplayer->mPreparedSem, 0, 0);

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x4000);
	result = pthread_create(&tplayer->id, &attr, tplayer_pthread, (void*)tplayer);
	pthread_attr_destroy(&attr);
	if(result != 0){
		com_err("pthread create fail!\r\n");
		free(tplayer);
		tplayer = NULL;
		goto end;
	}
end:
	return tplayer;
}

void tplayer_pthread_destory(player_t *tplayer)
{
	if(!tplayer){
		return;
	}
	pthread_cancel(tplayer->id);
	pthread_join(tplayer->id, NULL);
	__db_list_destory(tplayer->queue_head);
	sem_destroy(&tplayer->mPreparedSem);
	free(tplayer);
}

CdxKeyedVectorT http_pHeaders;
char Headers_key[256] = { 0 };
char Headers_val[256] = { 0 };

int tplayer_init(player_t *tplayer,int player_type)
{
	tplayer_send_cmd(tplayer, INIT_CMD, player_type, 0);
	return 0;
}

int tplayer_exit(player_t *tplayer)
{
	tplayer_send_cmd(tplayer, EXIT_CMD, 0, 0);
	return 0;
}

int tplayer_play_url(player_t *tplayer, const char *path)
{
	if(tplayer_get_status(tplayer) == PREPARING_STATUS)
	{
		com_err("this is invaild, last player is preparing. please wait...");
		return -1;
	}

	tplayer_send_cmd(tplayer, PREPARE_CMD, (long int)path, 0);

    sem_wait(&tplayer->mPreparedSem);

	if(tplayer_get_status(tplayer) != PREPARED_STATUS)
	{
		com_err("this is invaild, prepare fail\n");
		return -1;
	}
	return 0;
}

int tplayer_play(player_t *tplayer)
{
	tplayer_send_cmd(tplayer, PLAY_CMD, 0, 0);
	return 0;
}

int tplayer_pause(player_t *tplayer)
{
	tplayer_send_cmd(tplayer, PAUSE_CMD, 0, 0);
	return 0;
}

int tplayer_seekto(player_t *tplayer, int nSeekTimeSec)
{
	tplayer_send_cmd(tplayer, SEEK_TO, nSeekTimeSec, 0);
	return 0;
}

int tplayer_stop(player_t *tplayer)
{
	tplayer_send_cmd(tplayer, STOP_CMD, 0, 0);
	return 0;
}

int tplayer_volume(player_t *tplayer, int volume)
{
	tplayer_send_cmd(tplayer, SETTING, SET_VOLUME, volume);
	return 0;
}

int tplayer_switch_audio(player_t *tplayer, int audio)
{
	tplayer_send_cmd(tplayer, SETTING, SET_AUDIO, audio);
	return 0;
}

int tplayer_switch_soundchannel(player_t *tplayer, int soundchannelmode)
{
	tplayer_send_cmd(tplayer, SETTING, SET_SOUND_CHANNEL, soundchannelmode);
	return 0;
}

int tplayer_set_looping(player_t *tplayer, bool bLoop)
{
	tplayer_send_cmd(tplayer, SETTING, SET_LOOP, bLoop);
	return 0;
}

int display_rect[4];
int tplayer_set_displayrect(player_t *tplayer, int x, int y, unsigned int width, unsigned int height)
{
	display_rect[0] = x;
	display_rect[1] = y;
	display_rect[2] = width;
	display_rect[3] = height;
	tplayer_send_cmd(tplayer, SETTING, SET_DISPLAY, (unsigned long int)display_rect);
	return 0;
}

int tplayer_set_speed(player_t *tplayer, TplayerPlaySpeedType nSpeed)
{
	tplayer_send_cmd(tplayer, SETTING, SET_SPEED, nSpeed);
	return 0;
}

int tplayer_set_rotate(player_t *tplayer, TplayerVideoRotateType rotateDegree)
{
	tplayer_send_cmd(tplayer, SETTING, SET_ROTATE, rotateDegree);
	return 0;
}

int tplayer_set_audio_eq(player_t *tplayer, AudioEqType type)
{
	tplayer_send_cmd(tplayer, SETTING, SET_AUDIO_EQ, type);
	return 0;
}

int tplayer_get_duration(player_t *tplayer, int* sec)
{
	ISNULL(tplayer);
	ISNULL(tplayer->mTPlayer);
	int msec;
	int tmp_sec;

	if(tplayer_get_status(tplayer) < PREPARED_STATUS)
	{
		com_info("not prepared!\n");
		return -1;
	}
    TPlayerGetDuration(tplayer->mTPlayer, &msec);
    tmp_sec = msec / 1000;
    *sec = tmp_sec;
	return 0;
}

int tplayer_get_current_pos(player_t *tplayer, int* sec)
{
	ISNULL(tplayer);
	ISNULL(tplayer->mTPlayer);
	int msec;
	int tmp_sec;

	if(tplayer_get_status(tplayer) < PREPARED_STATUS)
	{
		com_info("not prepared!\n");
		return -1;
	}
	TPlayerGetCurrentPosition(tplayer->mTPlayer, &msec);
    tmp_sec = msec / 1000;
    *sec = tmp_sec;
	return 0;
}

MediaInfo* tplayer_get_media_Info(player_t *tplayer)
{
	if(!tplayer){
		return NULL;
	}
	return tplayer->mMediaInfo;
}

playerStatus tplayer_get_status(player_t *tplayer)
{
	if(!tplayer){
		return EXIT_STATUS;
	}
	return tplayer->mpstatus;
}
