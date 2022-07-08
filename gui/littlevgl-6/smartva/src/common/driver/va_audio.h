#ifndef _VA_AUDIO_H_
#define _VA_AUDIO_H_

#include "smt_config.h"

int va_audio_set_volume(int volume);

int va_audio_get_volume(void);

int va_audio_set_mic_AMP_gain_value(int val);

int va_audio_set_mic_mixer_value(int val);

int va_audio_get_mic_AMP_gain_value(int val);

int va_audio_get_mic_mixer_value(int val);

int va_audio_init(void);

int va_audio_keytone_play(void);
int va_audio_keytone_init(const char *partname);
int va_audio_keytone_exit(void);
int va_audio_play_wav_music(const char *partname);

int va_audio_record_init(void);
int va_audio_record_exit(void);
int va_audio_record_set_para(int fs, int bps, int chn);
int va_audio_record_get_para(int *fs, int *bps, int *chn);
int va_audio_record_set_source(unsigned int source);
int va_audio_record_get_source(void);
int va_audio_record_buf_flush(void);
int va_audio_record_start(void);
int va_audio_record_stop(void);
int va_audio_record_pause(void);
int va_audio_record_resume(void);
int va_audio_record_get_data(unsigned char *buf, int size);
int va_audio_record_get_buf_size(int mode);

int va_audio_play_init(void);
int va_audio_play_set_volume_map(unsigned int min, unsigned int max);
int va_audio_play_get_volume_map(unsigned int *min, unsigned int *max);
int va_audio_play_set_volume_value(unsigned int volume_L,
				   unsigned int volume_R);
int va_audio_play_get_volume_value(int *volume_L, int *volume_R);

#endif
