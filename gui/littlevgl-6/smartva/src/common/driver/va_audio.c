#include <stdlib.h>
#include "va_audio.h"
#include "smt_config.h"
#include "common.h"

static unsigned int vol;

struct amixer_control {
	char *name;
	unsigned int value;
};

struct amixer_control controls[] = {
#ifdef CONFIG_C200S
	{ "head phone power", 1 },       { "ADC MIC Boost AMP en", 1 },
	{ "ADC mixer mute for mic", 1 },
#endif

#ifdef CONFIG_T113
	{ "Headphone Switch", 1 },
#endif

#ifdef CONFIG_F133
	{ "Headphone Switch", 1 },
	{ "Headphone volume", 7 },
#endif
};

typedef struct __WAVE_HEADER1 {
	unsigned int uRiffFcc; // four character code, "RIFF"
	unsigned int uFileLen; // file total length, don't care it

	unsigned int uWaveFcc; // four character code, "WAVE"

	unsigned int uFmtFcc; // four character code, "fmt "
	unsigned int uFmtDataLen; // Length of the fmt data (=16)
	unsigned short uWavEncodeTag; // WAVE File Encoding Tag
	unsigned short uChannels; // Channels: 1 = mono, 2 = stereo
	unsigned int uSampleRate; // Samples per second: e.g., 44100
	unsigned int uBytesPerSec; // sample rate * block align
	unsigned short uBlockAlign; // channels * bits/sample / 8
	unsigned short uBitsPerSample; // 8 or 16

	unsigned int uDataFcc; // four character code "data"
	unsigned int uSampDataSize; // Sample data size(n)

} __attribute__((packed)) wave_header_t;
#define BUF_LEN 1024

static wave_header_t keytone_head;
static char *keytone_buff = NULL;
static unsigned int keytone_len;

static int audio_mixer_cset(char *mixer_name, int value)
{
	char cmd[128] = { 0 };
#ifdef CONFIG_F133
	sprintf(cmd, "amixer cset name='%s' %d:%d", mixer_name, value, value);
#else
	sprintf(cmd, "amixer cset name='%s' %d", mixer_name, value);
#endif
	system(cmd);
	return 0;
}

int audio_mixer_get(const char *shell, const char *name)
{
	int bytes;
	char buf[10] = { 0 };
	char cmd[500] = { 0 };
	sprintf(cmd, shell, name);
	FILE *stream;
	printf("%s\n", cmd);
	stream = popen(cmd, "r");
	if (!stream)
		return -1;
	bytes = fread(buf, sizeof(char), sizeof(buf), stream);
	pclose(stream);
	if (bytes > 0) {
		return atoi(buf);
	} else {
		printf("%s --> failed\n", cmd);
		return -1;
	}
}

/*0~31*/
int va_audio_set_volume(int volume)
{
#ifdef CONFIG_C200S
	char *name = "head phone volume";
#endif

#ifdef CONFIG_T113
	char *name = "head volume";
#endif

#ifdef CONFIG_F133
	char *name = "DAC volume";
#endif

	audio_mixer_cset(name, volume);
	vol = volume;
	return 0;
}

int va_audio_get_volume(void)
{
	return vol;
}
/*0~7 */
int va_audio_set_mic_AMP_gain_value(int val)
{
#ifdef CONFIG_C200S
	char *name = "MICIN GAIN control";
#endif

#ifdef CONFIG_T113
	char *name = "MIC1 gain volume";
#endif

#ifdef CONFIG_F133
	char *name = "MIC1 gain volume";
#endif

	audio_mixer_cset(name, val);
	return 0;
}
/*0~7*/
int va_audio_set_mic_mixer_value(int val)
{
	char *name = "MIC1_G boost stage output mixer control";
	audio_mixer_cset(name, val);
	return 0;
}

int va_audio_get_mic_AMP_gain_value(int val)
{
#ifdef CONFIG_C200S
	char *name = "MICIN GAIN control";
#endif

#ifdef CONFIG_T113
	char *name = "MIC1 gain volume";
#endif

#ifdef CONFIG_F133
	char *name = "MIC1 gain volume";
#endif

	const char *shell =
		"volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
	return audio_mixer_get(shell, name);
}

int va_audio_get_mic_mixer_value(int val)
{
	char *name = "MIC1_G boost stage output mixer control";
	const char *shell =
		"volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
	return audio_mixer_get(shell, name);
}

int va_audio_init(void)
{
	int i = 0;
	int num = sizeof(controls) / sizeof(controls[0]);
	for (i = 0; i < num; i++) {
		audio_mixer_cset(controls[i].name, controls[i].value);
	}

	return 0;
}

int va_audio_play_wav_music(const char *partname)
{
	int err;
	wave_header_t wav;
	int headwavcntp;
	snd_pcm_t *playback_handle;
	snd_pcm_hw_params_t *hw_params;
	FILE *fp = NULL;
	snd_pcm_format_t pcm_fmt;
	char buf[BUF_LEN];

	fprintf(stderr, "open file : %s\n", partname);
	fp = fopen(partname, "r");
	if (fp == NULL) {
		fprintf(stderr, "open test pcm file err\n");
		return -1;
	}

	headwavcntp = fread(&wav, 1, sizeof(wave_header_t), fp);
	if (headwavcntp != sizeof(wave_header_t)) {
		printf("read wav file head error!\n");
		fclose(fp);
		return -1;
	}

	printf("read wav file head success \n");
	//printf("bps = %d\n", wav.uBitsPerSample);
	//printf("chn = %d\n", wav.uChannels);
	//printf("fs = %d\n", wav.uSampleRate);

	if (wav.uBitsPerSample == 8) {
		pcm_fmt = SND_PCM_FORMAT_S8;
	} else if (wav.uBitsPerSample == 16) {
		pcm_fmt = SND_PCM_FORMAT_S16_LE;
	} else {
		printf("uBitsPerSample not support!\n");
		fclose(fp);
		return -1;
	}
	if ((err = snd_pcm_open(&playback_handle, "default",
				SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "cannot open audio device record.pcm (%s)\n",
			snd_strerror(err));
		fclose(fp);
		return -1;
	}
	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr,
			"cannot allocate hardware parameter structure (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0) {
		fprintf(stderr,
			"cannot initialize hardware parameter structure (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params_set_access(
		     playback_handle, hw_params,
		     SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr,
			"cannot allocate hardware parameter structure (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params,
						pcm_fmt)) < 0) {
		fprintf(stderr,
			"cannot allocate hardware parameter structure (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params_set_rate(playback_handle, hw_params,
					      wav.uSampleRate, 0)) < 0) {
		fprintf(stderr, "cannot set sample rate (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params,
						  wav.uChannels)) < 0) {
		fprintf(stderr, "cannot set channel count (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
		fprintf(stderr, "cannot set parameters (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	snd_pcm_hw_params_free(hw_params);

	while (!feof(fp)) {
		err = fread(buf, 1, BUF_LEN, fp);
		if (err < 0) {
			fprintf(stderr, "read pcm from file err\n");
			goto play_wav_out;
		}
		err = snd_pcm_writei(playback_handle, buf, BUF_LEN / 4);
		if (err < 0) {
			fprintf(stderr,
				"write to audio interface failed (%s)\n",
				snd_strerror(err));
			goto play_wav_out;
		}
	}

play_wav_out:
	fprintf(stderr, "close file\n");
	fclose(fp);
	fprintf(stderr, "close dev\n");
	snd_pcm_close(playback_handle);
	fprintf(stderr, "ok\n");
	return 0;
}

int va_audio_keytone_play(void)
{
	int err;
	snd_pcm_t *playback_handle;
	snd_pcm_hw_params_t *hw_params = NULL;
	snd_pcm_format_t pcm_fmt;
	if (keytone_head.uBitsPerSample == 8) {
		pcm_fmt = SND_PCM_FORMAT_S8;
	} else if (keytone_head.uBitsPerSample == 16) {
		pcm_fmt = SND_PCM_FORMAT_S16_LE;
	} else {
		printf("uBitsPerSample not support!\n");
		return -1;
	}
	if ((err = snd_pcm_open(&playback_handle, "default",
				SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "cannot open audio device record.pcm (%s)\n",
			snd_strerror(err));
		return -1;
	}
	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr,
			"cannot allocate hardware parameter structure (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0) {
		fprintf(stderr,
			"cannot initialize hardware parameter structure (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params_set_access(
		     playback_handle, hw_params,
		     SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr,
			"cannot allocate hardware parameter structure (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params,
						pcm_fmt)) < 0) {
		fprintf(stderr,
			"cannot allocate hardware parameter structure (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params_set_rate(playback_handle, hw_params,
					      keytone_head.uSampleRate, 0)) <
	    0) {
		fprintf(stderr, "cannot set sample rate (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params_set_channels(
		     playback_handle, hw_params, keytone_head.uChannels)) < 0) {
		fprintf(stderr, "cannot set channel count (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
		fprintf(stderr, "cannot set parameters (%s)\n",
			snd_strerror(err));
		goto play_wav_out;
	}
	snd_pcm_hw_params_free(hw_params);
	snd_pcm_writei(playback_handle, keytone_buff, keytone_len);
play_wav_out:
	snd_pcm_close(playback_handle);
	return 0;
}

int va_audio_keytone_init(const char *partname)
{
	FILE *fp = NULL;
	int r_size;
	int f_size;
	fprintf(stderr, "open file : %s\n", partname);
	fp = fopen(partname, "r");
	if (fp == NULL) {
		fprintf(stderr, "open test pcm file err\n");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	f_size = ftell(fp);
	keytone_buff = malloc(f_size);
	if (!keytone_buff) {
		printf("keytone_buff malloc failed!\n");
		fclose(fp);
		return -1;
	}
	fseek(fp, 0, SEEK_SET);
	r_size = fread(&keytone_head, 1, sizeof(wave_header_t), fp);
	if (r_size != sizeof(wave_header_t)) {
		printf("read 111 wav file head error!\n");
		fclose(fp);
		free(keytone_buff);
		keytone_buff = NULL;
		return -1;
	}
	printf("read wav file head success \n");
	keytone_len = fread(keytone_buff, 1, f_size, fp);
	if (keytone_len <= 0) {
		printf("read 222 wav file head error!\n");
		fclose(fp);
		free(keytone_buff);
		keytone_buff = NULL;
		return -1;
	}
	fclose(fp);
	return 0;
}

int va_audio_keytone_exit(void)
{
	if (keytone_buff) {
		free(keytone_buff);
		keytone_buff = NULL;
	}
	return 0;
}

/*****************************************************************/
#include "circle_buf.h"

typedef struct AUDIO_CONFIG_MGR {
	snd_pcm_t *handle;
	char cardName[16];

	snd_pcm_format_t format;
	unsigned int sampleRate;
	unsigned int channels;
	snd_pcm_uframes_t periodSize;
	snd_pcm_uframes_t bufferSize;

	unsigned long frameBytes;
	unsigned long chunkBytes;

	unsigned in_aborting;
	unsigned int capture_duration;
	unsigned int can_paused;
} audio_config_mgr_t;

typedef enum {
	AUDIO_REC_FM_L = (1 << 0), //请勿同时使能FM和LINEIN
	AUDIO_REC_FM_R = (1 << 1),
	AUDIO_REC_LINEIN_L = (1 << 2),
	AUDIO_REC_LINEIN_R = (1 << 3),
	AUDIO_REC_MIC = (1 << 4),
} __audio_record_channel_e;

typedef enum AUDIO_DEV_STAT {
	AUDIO_DEV_STAT_IDLE = 0,
	AUDIO_DEV_STAT_RUN,
	AUDIO_DEV_STAT_PAUS,
	AUDIO_DEV_STAT_EXIT,
	AUDIO_DEV_STAT_
} audio_dev_stat_e;

#define SOUND_CARD_AUDIOCODEC "hw:audiocodec"
#define AUDIO_DAC_VOLUME_MIN (120)
#define AUDIO_DAC_VOLUME_MAX (160)
#define AUDIO_REC_USR_BUF_SIZE 1024 * 64
#define AUDIO_REC_TMP_BUF_SIZE 1024 * 4

typedef struct {
	audio_config_mgr_t *mgr;
	__audio_dev_buf_manager_t circle_buf;
	unsigned char *alsa_buf;
	unsigned int alsa_buf_size;
	audio_dev_stat_e status;
	unsigned int source;
	pthread_t id;
	unsigned int is_busy;
} audio_rec_ctrl_t;

typedef struct {
	audio_config_mgr_t *mgr;
	audio_dev_stat_e status;
	unsigned int output;
	unsigned int vol_min;
	unsigned int vol_max;
} audio_play_ctrl_t;

static audio_rec_ctrl_t *g_audio_rec_ctrl;
static audio_play_ctrl_t *g_audio_play_ctrl;
static pthread_cond_t record_wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t record_wait_mutex;

audio_config_mgr_t *audio_playback_config_mgr_create(void)
{
	audio_config_mgr_t *audio_conf_mgr = NULL;

	audio_conf_mgr = malloc(sizeof(audio_config_mgr_t));
	if (!audio_conf_mgr) {
		printf("no memory for create audio config manager\n");
		return NULL;
	}
	memset(audio_conf_mgr, 0, sizeof(audio_config_mgr_t));
	audio_conf_mgr->format = SND_PCM_FORMAT_S16_LE;
	audio_conf_mgr->sampleRate = 16000;
	audio_conf_mgr->channels = 2;
	audio_conf_mgr->periodSize = 1024; //1024;
	audio_conf_mgr->bufferSize = 4096; //(4*1024)
	audio_conf_mgr->can_paused = 1;

	return audio_conf_mgr;
}

audio_config_mgr_t *audio_record_config_mgr_create(void)
{
	audio_config_mgr_t *audio_conf_mgr = NULL;

	audio_conf_mgr = malloc(sizeof(audio_config_mgr_t));
	if (!audio_conf_mgr) {
		printf("no memory for create audio config manager\n");
		return NULL;
	}
	memset(audio_conf_mgr, 0, sizeof(audio_config_mgr_t));
	audio_conf_mgr->format = SND_PCM_FORMAT_S16_LE;
	audio_conf_mgr->sampleRate = 16000;
	audio_conf_mgr->channels = 2;
	audio_conf_mgr->periodSize = 1024; //1024;
	audio_conf_mgr->bufferSize = 16384; //(16*1024)
	audio_conf_mgr->can_paused = 1;

	return audio_conf_mgr;
}

void audio_config_mgr_release(audio_config_mgr_t *mgr)
{
	if (!mgr) {
		printf("%s: mgr null !\n", __func__);
		return;
	}
	free(mgr);
}

int alsa_amixer_set(char *name, int *values, int num)
{
	char cmd[128];
	int i;
	int len;
	sprintf(cmd, "amixer cset name='%s' ", name);
	for (i = 0; i < num; i++) {
		len = strlen(cmd);
		sprintf(cmd + len, "%d,", values[i]);
	}
	system(cmd);
	return 0;
}

int alsa_amixer_get(char *name, int *values, int num)
{
	char cmd[128];
	char *p_dot = NULL;
	char *p_num = NULL;
	FILE *stream;
	char *buf;
	int len, i, bytes;
	sprintf(cmd,
		"volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume",
		name);
	stream = popen(cmd, "r");
	if (!stream) {
		printf("stream err!\n");
		return -1;
	}

	//fseek(stream,0,SEEK_END);//管道无法获取size
	//len = ftell(stream);
	//fseek(stream,0,SEEK_SET);
	len = 50;
	buf = malloc(len);
	if (!buf) {
		printf("buf err!\n");
		return -1;
	}
	memset(buf, 0, len);

	bytes = fread(buf, 1, len, stream);
	pclose(stream);
	p_num = buf;
	if (bytes) {
		for (i = 0; i < num; i++) {
			p_dot = strchr(p_num, ',');
			if (p_dot == NULL) {
				if ((num - 1) > i) {
					printf("amixer get value num err! %d\n",
					       __LINE__);
					free(buf);
					return -1;
				}
				values[i] = atoi(p_num);
			} else {
				*p_dot = '\0';
				values[i] = atoi(p_num);
				p_num = p_dot + 1;
			}
		}
	} else {
		printf("amixer get value err!");
		free(buf);
		return -1;
	}
	free(buf);
	return 0;
}

int alsa_set_pcm_params(audio_config_mgr_t *pcmMgr)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *sw_params;
	int err = -1;
	int dir = 0;

	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!");
		return err;
	}

	//snd_pcm_prepare(pcmMgr->handle);
	//printf("[]pcm status:%d\n",snd_pcm_state(pcmMgr->handle));

	/* HW params */
	snd_pcm_hw_params_alloca(&params);
	err = snd_pcm_hw_params_any(pcmMgr->handle, params);
	if (err < 0) {
		printf("Broken configuration for this PCM: no configurations available!");
		return err;
	}

	err = snd_pcm_hw_params_set_access(pcmMgr->handle, params,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		printf("Access type not available");
		return err;
	}

	err = snd_pcm_hw_params_set_format(pcmMgr->handle, params,
					   pcmMgr->format);
	if (err < 0) {
		printf("Sample format not available");
		return err;
	}

	err = snd_pcm_hw_params_set_channels(pcmMgr->handle, params,
					     pcmMgr->channels);
	if (err < 0) {
		printf("Channels count not available");
		return err;
	}
	printf("set fs = %d\n", pcmMgr->sampleRate);
	err = snd_pcm_hw_params_set_rate_near(pcmMgr->handle, params,
					      &pcmMgr->sampleRate, &dir);
	if (err < 0) {
		printf("set_rate error!");
		return err;
	}

	err = snd_pcm_hw_params_set_period_size_near(pcmMgr->handle, params,
						     &pcmMgr->periodSize, &dir);
	//err = snd_pcm_hw_params_set_period_size(pcmMgr->handle, params, pcmMgr->periodSize, &dir);
	if (err < 0) {
		printf("set_period_size error!");
		return err;
	}

	// double 1024-sample capacity -> 4
	snd_pcm_uframes_t bufferSize =
		pcmMgr->bufferSize; //pcmMgr->periodSize * (pcmMgr->channels * snd_pcm_format_physical_width(pcmMgr->format)/8) * 4;
	err = snd_pcm_hw_params_set_buffer_size_near(pcmMgr->handle, params,
						     &bufferSize);
	if (err < 0) {
		printf("set_buffer_size error!");
		return err;
	}

	err = snd_pcm_hw_params(pcmMgr->handle, params);
	if (err < 0) {
		printf("Unable to install hw params");
		return err;
	}

	snd_pcm_hw_params_get_period_size(params, &pcmMgr->periodSize, 0);
	snd_pcm_hw_params_get_buffer_size(params, &pcmMgr->bufferSize);
	if (pcmMgr->periodSize == pcmMgr->bufferSize) {
		printf("Can't use period equal to buffer size (%lu == %lu)",
		       pcmMgr->periodSize, pcmMgr->bufferSize);
		return err;
	}

	/* SW params */
	snd_pcm_sw_params_alloca(&sw_params);
	snd_pcm_sw_params_current(pcmMgr->handle, sw_params);

	if (snd_pcm_stream(pcmMgr->handle) == SND_PCM_STREAM_CAPTURE) {
		snd_pcm_sw_params_set_start_threshold(pcmMgr->handle, sw_params,
						      1);
	} else {
		snd_pcm_uframes_t boundary = 0;
		snd_pcm_sw_params_get_boundary(sw_params, &boundary);
		snd_pcm_sw_params_set_start_threshold(pcmMgr->handle, sw_params,
						      pcmMgr->bufferSize / 2);
		/* set silence size, in order to fill silence data into ringbuffer */
		snd_pcm_sw_params_set_silence_size(pcmMgr->handle, sw_params,
						   boundary);
	}

	snd_pcm_sw_params_set_stop_threshold(pcmMgr->handle, sw_params,
					     pcmMgr->bufferSize);
	snd_pcm_sw_params_set_avail_min(pcmMgr->handle, sw_params,
					pcmMgr->periodSize);
	err = snd_pcm_sw_params(pcmMgr->handle, sw_params);
	if (err < 0) {
		printf("Unable to install sw prams!\n");
		return err;
	}

	pcmMgr->frameBytes = snd_pcm_frames_to_bytes(pcmMgr->handle, 1);
	pcmMgr->chunkBytes =
		snd_pcm_frames_to_bytes(pcmMgr->handle, pcmMgr->periodSize);
	pcmMgr->can_paused = snd_pcm_hw_params_can_pause(params);
	//pcmMgr->bitsPerSample = snd_pcm_format_physical_width(pcmMgr->format);
	//pcmMgr->significantBitsPerSample = snd_pcm_format_width(pcmMgr->format);
	//pcmMgr->bitsPerFrame = pcmMgr->bitsPerSample * pcmMgr->chnCnt;
	//pcmMgr->chunkBytes = pcmMgr->chunkSize * pcmMgr->bitsPerFrame / 8;

	// printf("--------can_paused:%d-------------\n",pcmMgr->can_paused);
	// printf(">>periodSize: %ld, BufferSize: %ld, frameBytes: %ld\n",  pcmMgr->periodSize, pcmMgr->bufferSize,pcmMgr->frameBytes);

	return 0;
}

int alsa_open_pcm(audio_config_mgr_t *pcmMgr, const char *card, int pcmFlag)
{
	int err;
	int open_mode = 0;
	snd_pcm_stream_t stream;

	if (pcmMgr->handle != NULL) {
		printf("PCM is opened already!");
		return 0;
	}
	printf("open pcm! card:[%s], pcmFlag:[%d](0-cap;1-play)\n", card,
	       pcmFlag);

	// 0-cap; 1-play
	stream = (pcmFlag == 0) ? SND_PCM_STREAM_CAPTURE :
				  SND_PCM_STREAM_PLAYBACK;
	memset(pcmMgr->cardName, 0, sizeof(pcmMgr->cardName));
	strncpy(pcmMgr->cardName, card, sizeof(pcmMgr->cardName));

	//open_mode |= 0x00010000;  // not to used the auto resample

	err = snd_pcm_open(&pcmMgr->handle, card, stream, open_mode);
	if (err < 0) {
		printf("PCM open error: %d", err);
		return err;
	}

	return 0;
}

void alsa_close_pcm(audio_config_mgr_t *pcmMgr)
{
	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!");
		return;
	}
	printf("[]pcm status:%d\n", snd_pcm_state(pcmMgr->handle));
	snd_pcm_close(pcmMgr->handle);
	pcmMgr->handle = NULL;
}

void alsa_prepare_pcm(audio_config_mgr_t *pcmMgr)
{
	printf("[]pcm status:%d\n", snd_pcm_state(pcmMgr->handle));

	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!");
		return;
	}
	snd_pcm_prepare(pcmMgr->handle);
	printf("[]pcm status:%d\n", snd_pcm_state(pcmMgr->handle));
}

ssize_t alsa_read_pcm(audio_config_mgr_t *pcmMgr, void *data, size_t rcount)
{
	ssize_t ret;
	ssize_t result = 0;
	// int err = 0;
	// char cardName[sizeof(pcmMgr->cardName)] = {0};

	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!\n");
		return 0;
	}
	//if (rcount != pcmMgr->periodSize)/*do not change the parameter [rcount]*/
	//    rcount = pcmMgr->periodSize;

	if (pcmMgr == NULL || data == NULL) {
		printf("invalid input parameter(pcmMgr=%p, data=%p)!\n", pcmMgr,
		       data);
		return -1;
	}

	while (rcount > 0) {
		ret = snd_pcm_readi(pcmMgr->handle, data, rcount);
		if (ret == -EAGAIN || (ret >= 0 && (size_t)ret < rcount)) {
			snd_pcm_wait(pcmMgr->handle, 100);
		} else if (ret == -EPIPE) {
			snd_pcm_prepare(pcmMgr->handle);
			//if(pcmMgr->read_pcm_aec)    // for aec condition,need to return directly and re-trigger cap dma again
			//{
			//    printf("aec_rtn_drtly");
			//    return ret;
			//}
		} else if (ret == -ESTRPIPE) {
			printf("need recover(%s)!", strerror(errno));
			snd_pcm_recover(pcmMgr->handle, ret, 0);
		} else if (ret < 0) {
			printf("read error: %d", (int)(ret));
			return ret;
		}

		if (ret > 0) {
			result += ret;
			rcount -= ret;
			data += ret * pcmMgr->frameBytes;
		}
	}

	return result;
}

ssize_t alsa_write_pcm(audio_config_mgr_t *pcmMgr, void *data, size_t wcount)
{
	ssize_t ret;
	ssize_t result = 0;
	int err = 0;
	char cardName[sizeof(pcmMgr->cardName)] = { 0 };

	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!\n");
		return 0;
	}

	if (snd_pcm_state(pcmMgr->handle) == SND_PCM_STATE_SUSPENDED) {
		while ((err = snd_pcm_resume(pcmMgr->handle)) == -EAGAIN) {
			printf("snd_pcm_resume again!\n");
			sleep(1);
		}
		switch (snd_pcm_state(pcmMgr->handle)) {
		case SND_PCM_STATE_XRUN: {
			snd_pcm_drop(pcmMgr->handle);
			break;
		}
		case SND_PCM_STATE_SETUP:
			break;
		default: {
			printf("pcm_lib_state:%s\n",
			       snd_pcm_state_name(
				       snd_pcm_state(pcmMgr->handle)));
			snd_pcm_prepare(pcmMgr->handle);
			break;
		}
		}
		alsa_set_pcm_params(pcmMgr);
	}

	while (wcount > 0) {
		if (snd_pcm_state(pcmMgr->handle) == SND_PCM_STATE_SETUP) {
			snd_pcm_prepare(pcmMgr->handle);
		}
		ret = snd_pcm_writei(pcmMgr->handle, data, wcount);
		if (ret == -EAGAIN || (ret >= 0 && (size_t)ret < wcount)) {
			snd_pcm_wait(pcmMgr->handle, 100);
		} else if (ret == -EPIPE) {
			//printf("xrun!");
			snd_pcm_prepare(pcmMgr->handle);
		} else if (ret == -EBADFD) {
			//printf("careful! current pcm state: %d", snd_pcm_state(pcmMgr->handle));
			snd_pcm_prepare(pcmMgr->handle);
		} else if (ret == -ESTRPIPE) {
			printf("need recover!");
			snd_pcm_recover(pcmMgr->handle, ret, 0);
		} else if (ret < 0) {
			printf("write error! ret:%d", (int)ret);
			//0-cap; 1-play
			alsa_close_pcm(pcmMgr);
			//FIXME: reopen
			printf("cardName:[%s], pcmFlag:[play]\n",
			       pcmMgr->cardName);
			strncpy(cardName, pcmMgr->cardName, sizeof(cardName));
			ret = alsa_open_pcm(pcmMgr, cardName, 1);
			if (ret < 0) {
				printf("alsa_open_pcm failed!");
				return ret;
			}
			ret = alsa_set_pcm_params(pcmMgr);
			if (ret < 0) {
				printf("alsa_set_pcm_params failed!");
				return ret;
			}
			if (pcmMgr->handle != NULL) {
				//snd_pcm_reset(pcmMgr->handle);
				snd_pcm_prepare(pcmMgr->handle);
				snd_pcm_start(pcmMgr->handle);
			}
			printf("set pcm prepare finished!");
			return ret;
		}

		if (ret > 0) {
			result += ret;
			wcount -= ret;
			data += ret * pcmMgr->frameBytes;
		}
	}

	return result;
}

int audio_cmd_start(audio_config_mgr_t *pcmMgr)
{
	int ret = 0;

	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!");
		return 0;
	}

	ret = snd_pcm_start(pcmMgr->handle);
	if (ret < 0) {
		printf("unable to start PCM stream, return %d\n", ret);
		return ret;
	}
	printf("[]pcm status:%d\n", snd_pcm_state(pcmMgr->handle));

	return 0;
}

int audio_cmd_stop(audio_config_mgr_t *pcmMgr)
{
	int ret = 0;

	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!");
		return 0;
	}

#if 0
    ret = snd_pcm_drain(pcmMgr->handle);
	if (ret < 0)
	{
		printf("stop failed!, return %d\n", ret);
		return ret;
	}
#endif
	ret = snd_pcm_pause(pcmMgr->handle, 1);
	if (ret < 0) {
		printf("pause failed!, return %d\n", ret);
		return ret;
	}
	printf("[]pcm status:%d\n", snd_pcm_state(pcmMgr->handle));

	printf("[audio_cmd_stop]\n");
	return 0;
}

int audio_cmd_pause(audio_config_mgr_t *pcmMgr)
{
	int ret = 0;
	printf("[]pcm status:%d\n", snd_pcm_state(pcmMgr->handle));

	if (pcmMgr->can_paused) {
		ret = snd_pcm_pause(pcmMgr->handle, 1);
	} else {
		ret = snd_pcm_drop(pcmMgr->handle);
	}

	if (ret < 0) {
		printf("pause failed!, return %d\n", ret);
		return ret;
	}
	return 0;
}

int audio_cmd_drop(audio_config_mgr_t *pcmMgr)
{
	int ret = 0;

	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!");
		return 0;
	}

	ret = snd_pcm_drop(pcmMgr->handle);

	if (ret < 0) {
		printf("pause failed!, return %d\n", ret);
		return ret;
	}
	return 0;
}

int audio_cmd_drain(audio_config_mgr_t *pcmMgr)
{
	int ret = 0;

	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!");
		return 0;
	}

	ret = snd_pcm_drain(pcmMgr->handle);

	if (ret < 0) {
		printf("drain failed!, return %d\n", ret);
		return ret;
	}
	return 0;
}

int audio_cmd_resume(audio_config_mgr_t *pcmMgr)
{
	int ret = 0;

	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!");
		return 0;
	}

	if (pcmMgr->can_paused) {
		ret = snd_pcm_pause(pcmMgr->handle, 0);
	} else {
		ret = snd_pcm_prepare(pcmMgr->handle);
	}
	printf("[]pcm status:%d\n", snd_pcm_state(pcmMgr->handle));

	if (ret < 0) {
		printf("resume failed!, return %d\n", ret);
		return ret;
	}
	return 0;
}
/* Get the available(writeable) space for playback  byte*/
int audio_get_playback_avail(audio_config_mgr_t *pcmMgr)
{
	snd_pcm_sframes_t size = 0;
	if (pcmMgr->handle == NULL) {
		printf("PCM is not open yet!");
		return 0;
	}
	snd_pcm_delay(pcmMgr->handle, &size);
	return snd_pcm_frames_to_bytes(pcmMgr->handle, size);
}

/*************************************************************/

static void record_down(void)
{
	printf("down!\n");
	pthread_mutex_lock(&record_wait_mutex);
	pthread_cond_wait(&record_wait_cond, &record_wait_mutex);
	pthread_mutex_unlock(&record_wait_mutex);
}
static void record_up(void)
{
	printf("up!\n");
	pthread_mutex_lock(&record_wait_mutex);
	pthread_cond_signal(&record_wait_cond);
	pthread_mutex_unlock(&record_wait_mutex);
}

static void *audio_record_task(void *arg)
{
	int frames, ret, tmpSampDataSize;
	while (1) {
		switch (g_audio_rec_ctrl->status) {
		case AUDIO_DEV_STAT_EXIT:
			g_audio_rec_ctrl->is_busy = 0;
			pthread_exit(0);
			break;
		case AUDIO_DEV_STAT_PAUS:
			g_audio_rec_ctrl->is_busy = 0;
			record_down();
			break;
		case AUDIO_DEV_STAT_IDLE:
			g_audio_rec_ctrl->is_busy = 0;
			record_down();
			break;
		case AUDIO_DEV_STAT_RUN:
			g_audio_rec_ctrl->is_busy = 1;
			frames = g_audio_rec_ctrl->alsa_buf_size /
				 g_audio_rec_ctrl->mgr->frameBytes;
			ret = alsa_read_pcm(g_audio_rec_ctrl->mgr,
					    g_audio_rec_ctrl->alsa_buf, frames);
			if (ret <= 0) {
				printf("read pcm data error!\n");
				if (g_audio_rec_ctrl->status !=
				    AUDIO_DEV_STAT_RUN) {
					break;
				}
				usleep(100);
				continue;
			}

			tmpSampDataSize =
				ret * g_audio_rec_ctrl->mgr->frameBytes;

			//check if user buffer is full, if so, we need not save current frame
			if (CircleBufQuerySize(&g_audio_rec_ctrl->circle_buf,
					       AUDIO_DEV_QUERY_BUF_SIZE_FREE) >=
			    tmpSampDataSize) {
				//write audio data to user cache from dma buffer
				CircleBufWrite(&g_audio_rec_ctrl->circle_buf,
					       g_audio_rec_ctrl->alsa_buf,
					       tmpSampDataSize);
			} else {
				//printf("audio record buffer overflow!\n");
			}
			break;
		default:
			break;
		}
	}
}

int va_audio_record_init(void)
{
	int ret;
	if (g_audio_rec_ctrl != NULL) {
		printf("do not init record again!\n");
		return 0;
	}
	usleep(100);
	g_audio_rec_ctrl = malloc(sizeof(audio_rec_ctrl_t));
	if (g_audio_rec_ctrl == NULL) {
		goto err0;
	}
	memset(g_audio_rec_ctrl, 0, sizeof(audio_rec_ctrl_t));

	pthread_mutex_init(&record_wait_mutex, NULL);
	pthread_cond_init(&record_wait_cond, NULL);

	g_audio_rec_ctrl->alsa_buf_size = AUDIO_REC_TMP_BUF_SIZE;
	g_audio_rec_ctrl->alsa_buf =
		(unsigned char *)malloc(g_audio_rec_ctrl->alsa_buf_size);
	if (g_audio_rec_ctrl->alsa_buf == NULL) {
		goto err1;
	}

	if (CircleBufCreate(&g_audio_rec_ctrl->circle_buf,
			    AUDIO_REC_USR_BUF_SIZE) < 0) {
		printf("Create circle buffer for audio rec device failed!\n");
		goto err2;
	}

	ret = pthread_create(&g_audio_rec_ctrl->id, NULL, audio_record_task,
			     NULL);
	if (ret < 0) {
		goto err3;
	}

	g_audio_rec_ctrl->mgr = audio_record_config_mgr_create();
	ret = alsa_open_pcm(g_audio_rec_ctrl->mgr, SOUND_CARD_AUDIOCODEC, 0);
	if (ret < 0) {
		printf("alsa open err!\n");
		goto err4;
	}
	printf("init rec ok!\n");
	g_audio_rec_ctrl->status = AUDIO_DEV_STAT_IDLE;
	return 0;
err4:
	g_audio_rec_ctrl->status = AUDIO_DEV_STAT_EXIT;
	pthread_join(g_audio_rec_ctrl->id, NULL);
err3:
	CircleBufDestroy(&g_audio_rec_ctrl->circle_buf);
err2:
	free(g_audio_rec_ctrl->alsa_buf);
err1:
	free(g_audio_rec_ctrl);
	g_audio_rec_ctrl = NULL;
err0:
	return -1; /*g_audio_rec_ctrl*/
}

int va_audio_record_exit(void)
{
	g_audio_rec_ctrl->status = AUDIO_DEV_STAT_EXIT;
	while (1) {
		if (g_audio_rec_ctrl->is_busy == 0) {
			printf("we get busy!\n");
			break;
		} else {
			printf("pp %d\n", g_audio_rec_ctrl->is_busy);
		}
		sleep(1);
	}
	record_up();
	pthread_join(g_audio_rec_ctrl->id, NULL);
	free(g_audio_rec_ctrl->alsa_buf);
	CircleBufDestroy(&g_audio_rec_ctrl->circle_buf);
	alsa_close_pcm(g_audio_rec_ctrl->mgr);
	audio_config_mgr_release(g_audio_rec_ctrl->mgr);
	free(g_audio_rec_ctrl);
	g_audio_rec_ctrl = NULL;
	return 0;
}

int va_audio_record_set_para(int fs, int bps, int chn)
{
	int ret;
	switch (bps) {
	case 16:
		g_audio_rec_ctrl->mgr->format = SND_PCM_FORMAT_S16_LE;
		break;
	case 24:
		g_audio_rec_ctrl->mgr->format = SND_PCM_FORMAT_S24_LE;
		break;
	default:
		printf("%d bits not supprot\n", bps);
		g_audio_rec_ctrl->mgr->format = SND_PCM_FORMAT_S16_LE;
		return -1;
	}
	g_audio_rec_ctrl->mgr->channels = chn;
	g_audio_rec_ctrl->mgr->sampleRate = fs;
	ret = alsa_set_pcm_params(g_audio_rec_ctrl->mgr);
	if (ret < 0) {
		printf("audio set pcm param error:%d\n", ret);
		return ret;
	}
	return 0;
}

int va_audio_record_get_para(int *fs, int *bps, int *chn)
{
	switch (g_audio_rec_ctrl->mgr->format) {
	case SND_PCM_FORMAT_S16_LE:
		*bps = 16;
		break;
	case SND_PCM_FORMAT_S24_LE:
		*bps = 24;
		break;
	default:
		printf("%d format are not known\n",
		       g_audio_rec_ctrl->mgr->format);
		return -1;
	}

	*chn = g_audio_rec_ctrl->mgr->channels;
	*fs = g_audio_rec_ctrl->mgr->sampleRate;
	return 0;
}

/*__audio_record_channel_t;*/

int va_audio_record_set_source(unsigned int source)
{
	if (g_audio_rec_ctrl == NULL) {
		printf("please init audio rec first!");
		return -1;
	}
	if (source & AUDIO_REC_FM_L) {
		system("amixer set \"ADC1 Input FMINL\" on");
		audio_mixer_cset("FMINL gain volume", 1);
	}
	if (source & AUDIO_REC_FM_R) {
		system("amixer set \"ADC2 Input FMINR\" on");
		audio_mixer_cset("FMINR gain volume", 1);
	}
	if (source & AUDIO_REC_LINEIN_L) {
		system("amixer set \"ADC1 Input LINEINL\" on");
		audio_mixer_cset("LINEINL gain volume", 1);
	}
	if (source & AUDIO_REC_LINEIN_R) {
		system("amixer set \"ADC2 Input LINEINR\" on");
		audio_mixer_cset("LINEINR gain volume", 1);
	}
	if (source & AUDIO_REC_MIC) {
		system("amixer set \"ADC3 Input MIC3 Boost\" on");
		audio_mixer_cset("MIC3 gain volume", 31);
	}
	g_audio_rec_ctrl->source = source;
	return 0;
}

int va_audio_record_get_source(void)
{
	if (g_audio_rec_ctrl) {
		return g_audio_rec_ctrl->source;
	}
	return -1;
}

int va_audio_record_start(void)
{
	audio_cmd_start(g_audio_rec_ctrl->mgr);
	g_audio_rec_ctrl->status = AUDIO_DEV_STAT_RUN;
	record_up();
	return 0;
}

int va_audio_record_stop(void)
{
	g_audio_rec_ctrl->status = AUDIO_DEV_STAT_IDLE;
	return audio_cmd_stop(g_audio_rec_ctrl->mgr);
}

int va_audio_record_pause(void)
{
	g_audio_rec_ctrl->status = AUDIO_DEV_STAT_PAUS;
	while (1) {
		usleep(100);
		if (g_audio_rec_ctrl->is_busy == 0) {
			printf("pause get busy!");
			break;
		}
	}
	audio_cmd_pause(g_audio_rec_ctrl->mgr);
	return 0;
}
int va_audio_record_resume(void)
{
	g_audio_rec_ctrl->status = AUDIO_DEV_STAT_RUN;
	record_up();
	return audio_cmd_resume(g_audio_rec_ctrl->mgr);
}

int va_audio_record_buf_flush(void)
{
	CircleBufFlush(&g_audio_rec_ctrl->circle_buf);
	return 0;
}

int va_audio_record_get_data(unsigned char *buf, int size)
{
	int tmpRdSize = CircleBufRead(&g_audio_rec_ctrl->circle_buf, buf, size);
	return tmpRdSize;
}

int va_audio_record_get_buf_size(int mode)
{
	return CircleBufQuerySize(&g_audio_rec_ctrl->circle_buf, mode);
}

int va_audio_play_init(void)
{
	if (g_audio_play_ctrl != NULL) {
		printf("do not init play again!\n");
		return 0;
	}
	g_audio_play_ctrl = malloc(sizeof(audio_play_ctrl_t));
	if (g_audio_play_ctrl == NULL) {
		return -1;
	}
	memset(g_audio_play_ctrl, 0, sizeof(audio_play_ctrl_t));

	g_audio_play_ctrl->vol_min = 0;
	g_audio_play_ctrl->vol_max = 100;
	g_audio_play_ctrl->status = AUDIO_DEV_STAT_IDLE;
	printf("init rec ok!\n");
	return 0;
}

int va_audio_play_set_para(int fs, int bps, int chn)
{
	int ret;
	switch (bps) {
	case 16:
		g_audio_play_ctrl->mgr->format = SND_PCM_FORMAT_S16_LE;
		break;
	case 24:
		g_audio_play_ctrl->mgr->format = SND_PCM_FORMAT_S24_LE;
		break;
	default:
		printf("%d bits not supprot\n", bps);
		g_audio_play_ctrl->mgr->format = SND_PCM_FORMAT_S16_LE;
		return -1;
	}
	g_audio_play_ctrl->mgr->channels = chn;
	g_audio_play_ctrl->mgr->sampleRate = fs;
	ret = alsa_set_pcm_params(g_audio_play_ctrl->mgr);
	if (ret < 0) {
		printf("audio set pcm param error:%d\n", ret);
		return ret;
	}
	return 0;
}
int va_audio_play_get_para(int *fs, int *bps, int *chn)
{
	switch (g_audio_play_ctrl->mgr->format) {
	case SND_PCM_FORMAT_S16_LE:
		*bps = 16;
		break;
	case SND_PCM_FORMAT_S24_LE:
		*bps = 24;
		break;
	default:
		printf("%d format are not known\n",
		       g_audio_rec_ctrl->mgr->format);
		return -1;
	}

	*chn = g_audio_play_ctrl->mgr->channels;
	*fs = g_audio_play_ctrl->mgr->sampleRate;
	return 0;
}

int va_audio_play_start(void)
{
	return 0;
}

int va_audio_play_stop(void)
{
	return 0;
}

//DAC
int va_audio_play_set_volume_map(unsigned int min, unsigned int max)
{
	if (max <= min || g_audio_play_ctrl == NULL) {
		return -1;
	}
	g_audio_play_ctrl->vol_min = min;
	g_audio_play_ctrl->vol_max = max;
	return 0;
}
int va_audio_play_get_volume_map(unsigned int *min, unsigned int *max)
{
	if (g_audio_play_ctrl == NULL) {
		return -1;
	}
	*min = g_audio_play_ctrl->vol_min;
	*max = g_audio_play_ctrl->vol_max;
	return 0;
}
//DAC rang 0 ~ 255
int va_audio_play_set_volume_value(unsigned int volume_L, unsigned int volume_R)
{
	int temp_vol_L;
	int temp_vol_R;
	int arg[2];
	if (g_audio_play_ctrl == NULL) {
		return -1;
	}
	if (volume_L < g_audio_play_ctrl->vol_min ||
	    volume_L > g_audio_play_ctrl->vol_max) {
	    printf("%s %d volume_L:%d param error\n", __FILE__, __LINE__, volume_L);
	    printf("%s %d g_audio_play_ctrl->vol_max:%d\n", __FILE__, __LINE__, g_audio_play_ctrl->vol_max);
	    printf("%s %d g_audio_play_ctrl->vol_min:%d\n", __FILE__, __LINE__, g_audio_play_ctrl->vol_min);
		return -1;
	}
	if (volume_L == 0) {
		temp_vol_L = 0;
	} else {
		temp_vol_L = (volume_L - g_audio_play_ctrl->vol_min) * 100 /
			     (g_audio_play_ctrl->vol_max - g_audio_play_ctrl->vol_min) *
			     (AUDIO_DAC_VOLUME_MAX - AUDIO_DAC_VOLUME_MIN) / 100 + AUDIO_DAC_VOLUME_MIN;
		if (temp_vol_L < AUDIO_DAC_VOLUME_MIN ||
		    temp_vol_L > AUDIO_DAC_VOLUME_MAX) {
			printf("set volume L err:temp_vol_L:%d!\n", temp_vol_L);
			printf("%s %d AUDIO_DAC_VOLUME_MIN:%d AUDIO_DAC_VOLUME_MAX:%d\n", __FILE__, __LINE__, AUDIO_DAC_VOLUME_MIN, AUDIO_DAC_VOLUME_MAX);
			return -1;
		}
	}

	if (volume_R < g_audio_play_ctrl->vol_min ||
	    volume_R > g_audio_play_ctrl->vol_max) {
	    printf("%s %d volume_R:%d param error\n", __FILE__, __LINE__, volume_R);
	    printf("%s %d g_audio_play_ctrl->vol_max:%d\n", __FILE__, __LINE__, g_audio_play_ctrl->vol_max);
	    printf("%s %d g_audio_play_ctrl->vol_min:%d\n", __FILE__, __LINE__, g_audio_play_ctrl->vol_min);
		return -1;
	}

	if (volume_R == 0) {
		temp_vol_R = 0;
	} else {
		temp_vol_R = (volume_R - g_audio_play_ctrl->vol_min) * 100 /
			     (g_audio_play_ctrl->vol_max - g_audio_play_ctrl->vol_min) *
			     (AUDIO_DAC_VOLUME_MAX - AUDIO_DAC_VOLUME_MIN) / 100 + AUDIO_DAC_VOLUME_MIN;
		if (temp_vol_R < AUDIO_DAC_VOLUME_MIN ||
		    temp_vol_R > AUDIO_DAC_VOLUME_MAX) {
			printf("set volume R err:temp_vol_L:%d!\n", temp_vol_R);
			printf("%s %d AUDIO_DAC_VOLUME_MIN:%d AUDIO_DAC_VOLUME_MAX:%d\n", __FILE__, __LINE__, AUDIO_DAC_VOLUME_MIN, AUDIO_DAC_VOLUME_MAX);
			return -1;
		}
	}
	arg[0] = temp_vol_L;
	arg[1] = temp_vol_R;
	alsa_amixer_set("DAC volume", arg, 2);

	return 0;
}
int va_audio_play_get_volume_value(int *volume_L, int *volume_R)
{
	int arg[2];

	arg[0] = 0;
	arg[1] = 0;
	alsa_amixer_get("DAC volume", arg, 2);

	*volume_L = arg[0];
	*volume_R = arg[1];

	return 0;
}
