#include "media_spectrum.h"
#include "speex/speex_preprocess.h"

static spectrum_t mspectrum;

void media_spectrum_destroy(void)
{
	spectrum_t *sp = &mspectrum;

	if(sp->speex_spectrum){
		speex_preprocess_state_destroy(sp->speex_spectrum);
	}
	if(sp->ps_buffer){
		free(sp->ps_buffer);
	}
	if(sp->pdata){
		free(sp->pdata);
	}
	memset(sp, 0, sizeof(spectrum_t));
	pthread_mutex_destroy(&sp->spectrum_mutex);
}

void media_spectrum_init(AudioPcmData *audio_info)
{
	int frame_size;
	spectrum_t *sp = &mspectrum;

    if (sp->init_flag)
    {
        if (sp->audio_info.samplerate != audio_info->samplerate)
        {
            pthread_mutex_lock(&sp->spectrum_mutex);
            media_spectrum_destroy();
            pthread_mutex_unlock(&sp->spectrum_mutex);
            sp->init_flag = 0;
        }
    }

    if (!sp->init_flag)
    {
        pthread_mutex_init(&sp->spectrum_mutex, NULL);
    }

    pthread_mutex_lock(&sp->spectrum_mutex);

	memcpy(&sp->audio_info, audio_info, sizeof(AudioPcmData));

	if(sp->init_flag)
	{
		goto END;
	}
	if(!sp->audio_info.pData)
	{
		goto END;
	}
	frame_size = sp->audio_info.nSize/(sp->audio_info.channels*sizeof(short));
	sp->speex_spectrum = speex_preprocess_state_init(frame_size, sp->audio_info.samplerate);
	speex_preprocess_ctl(sp->speex_spectrum, SPEEX_PREPROCESS_GET_PSD_SIZE, &sp->ps_size);
	if(!sp->ps_buffer)
	{
		sp->ps_buffer = (int *)malloc(sp->ps_size * sizeof(int));
	}
	if(!sp->pdata)
	{
		sp->pdata = malloc(sp->audio_info.nSize);
	}
	sp->init_flag = 1;
END:
	if(sp->pdata)
	{
		memcpy(sp->pdata, sp->audio_info.pData, sp->audio_info.nSize);
	}

    pthread_mutex_unlock(&sp->spectrum_mutex);

	return;
}

int *media_get_spectrum(void)
{
	spectrum_t *sp = &mspectrum;

	if(!sp->init_flag)
	{
		return NULL;
	}

    pthread_mutex_lock(&sp->spectrum_mutex);
	speex_preprocess_estimate_update(sp->speex_spectrum, (spx_int16_t *)sp->pdata);
	speex_preprocess_ctl(sp->speex_spectrum, SPEEX_PREPROCESS_GET_PSD, sp->ps_buffer);
    pthread_mutex_unlock(&sp->spectrum_mutex);

#if 0
	FILE *fd;
	char buffer[32];
	fd = fopen("/tmp/spectrum.log", "w+");
	for(int i=0; i<sp->ps_size; i++){
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%u ", sp->ps_buffer[i]);
		fwrite(buffer,1, strlen(buffer),fd);
	}
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "\n");
	fwrite(buffer,1, strlen(buffer),fd);
	fclose(fd);
#endif

	return sp->ps_buffer;
}

int media_get_spectrum_size(void)
{
	spectrum_t *sp = &mspectrum;
	int ps_size;
    pthread_mutex_lock(&sp->spectrum_mutex);
    ps_size = sp->ps_size;
    pthread_mutex_unlock(&sp->spectrum_mutex);

	return ps_size;
}
