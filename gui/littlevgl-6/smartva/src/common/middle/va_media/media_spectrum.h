#ifndef _MEDIA_SPECTRUM_H_
#define _MEDIA_SPECTRUM_H_
#include "player_int.h"
#include "speex/speex_preprocess.h"

typedef struct spectrum_t{
	SpeexPreprocessState *speex_spectrum;
	AudioPcmData audio_info;
	unsigned char *pdata;
	int ps_size;
	int *ps_buffer;
	int init_flag;

	pthread_mutex_t	spectrum_mutex;
}spectrum_t;

void media_spectrum_destroy(void);
void media_spectrum_init(AudioPcmData *audio_info);
int  *media_get_spectrum(void);
int media_get_spectrum_size(void);

#endif
