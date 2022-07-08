#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include "bt_log.h"
#include "bt_alsa.h"

#define LG_ALSA_DEBUG(fmt, arg...)                                                                 \
    btmg_print(MSG_DEBUG, "[%s:%u]:  " fmt "\n", __func__, __LINE__, ##arg)
#define LG_ALSA_INFO(fmt, arg...)                                                                  \
    btmg_print(MSG_INFO, "[%s:%u]:  " fmt "\n", __func__, __LINE__, ##arg)
#define LG_ALSA_WARNG(fmt, arg...)                                                                 \
    btmg_print(MSG_WARNING, "[%s:%u]:  " fmt "\n", __func__, __LINE__, ##arg)
#define LG_ALSA_ERROR(fmt, arg...)                                                                 \
    btmg_print(MSG_ERROR, "[%s:%u]:  " fmt "\n", __func__, __LINE__, ##arg)
#define LG_ALSA_DUMP(fmt, arg...)                                                                  \
    btmg_print(MSG_MSGDUMP, "[%s:%u]:  " fmt "\n", __func__, __LINE__, ##arg)

#define DEBUG_A2DP_UNDERRUN_FILE "/tmp/a2dp_underrun_file"
static snd_output_t *output = NULL;
static unsigned long underrun_cnt = 0;
static FILE *underrun_fd = NULL;

static int aw_pcm_set_hw_params(snd_pcm_t *pcm, int channels, int rate, unsigned int *buffer_time,
                                unsigned int *period_time, const snd_pcm_format_t format,
                                char **msg)
{
    const snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
    snd_pcm_hw_params_t *params;
    char buf[256];
    int dir;
    int err;

    snd_pcm_hw_params_alloca(&params);
    if ((err = snd_pcm_hw_params_any(pcm, params)) != 0) {
        snprintf(buf, sizeof(buf), "Set all possible ranges: %s", snd_strerror(err));
        goto fail;
    }
    if ((err = snd_pcm_hw_params_set_access(pcm, params, access)) != 0) {
        snprintf(buf, sizeof(buf), "Set assess type: %s: %s", snd_strerror(err),
                 snd_pcm_access_name(access));
        goto fail;
    }
    if ((err = snd_pcm_hw_params_set_format(pcm, params, format)) != 0) {
        snprintf(buf, sizeof(buf), "Set format: %s: %s", snd_strerror(err),
                 snd_pcm_format_name(format));
        goto fail;
    }
    if ((err = snd_pcm_hw_params_set_channels(pcm, params, channels)) != 0) {
        snprintf(buf, sizeof(buf), "Set channels: %s: %d", snd_strerror(err), channels);
        goto fail;
    }
    if ((err = snd_pcm_hw_params_set_rate(pcm, params, rate, 0)) != 0) {
        snprintf(buf, sizeof(buf), "Set sampling rate: %s: %d", snd_strerror(err), rate);
        goto fail;
    }
    dir = 0;
    if ((err = snd_pcm_hw_params_set_period_time_near(pcm, params, period_time, &dir)) != 0) {
        snprintf(buf, sizeof(buf), "Set period time: %s: %d", snd_strerror(err), *period_time);
        goto fail;
    }
    dir = 0;
    if ((err = snd_pcm_hw_params_set_buffer_time_near(pcm, params, buffer_time, &dir)) != 0) {
        snprintf(buf, sizeof(buf), "Set buffer time: %s: %u", snd_strerror(err), *buffer_time);
        goto fail;
    }

    if ((err = snd_pcm_hw_params(pcm, params)) != 0) {
        snprintf(buf, sizeof(buf), "%s", snd_strerror(err));
        goto fail;
    }

    return BT_OK;

fail:
    if (msg != NULL)
        *msg = strdup(buf);

    BTMG_ERROR("set hw param fail");

    return err;
}

static int aw_pcm_set_sw_params(snd_pcm_t *pcm, snd_pcm_stream_t stream,
                                snd_pcm_uframes_t buffer_size, snd_pcm_uframes_t period_size,
                                char **msg)
{
    int err;
    char buf[256];
    snd_pcm_sw_params_t *params;
    snd_pcm_uframes_t threshold;

    snd_pcm_sw_params_alloca(&params);
    if ((err = snd_pcm_sw_params_current(pcm, params)) != 0) {
        snprintf(buf, sizeof(buf), "Get current params: %s", snd_strerror(err));
        goto fail;
    }
    /* start the transfer when the buffer is full (or almost full) */
    if (stream == SND_PCM_STREAM_PLAYBACK)
        threshold = (buffer_size / period_size) * period_size;
    else
        threshold = 0;

    LG_ALSA_INFO("set threadshold :%d", (int)threshold);
    if ((err = snd_pcm_sw_params_set_start_threshold(pcm, params, threshold)) != 0) {
        snprintf(buf, sizeof(buf), "Set start threshold: %s: %lu", snd_strerror(err), threshold);
        goto fail;
    }

    /* allow the transfer when at least period_size samples can be processed */
    if ((err = snd_pcm_sw_params_set_avail_min(pcm, params, period_size)) != 0) {
        snprintf(buf, sizeof(buf), "Set avail min: %s: %lu", snd_strerror(err), period_size);
        goto fail;
    }

    if ((err = snd_pcm_sw_params(pcm, params)) != 0) {
        snprintf(buf, sizeof(buf), "%s", snd_strerror(err));
        goto fail;
    }

    return BT_OK;

fail:
    if (msg != NULL)
        *msg = strdup(buf);

    BTMG_ERROR("set sw params");
    return err;
}

static int pcm_open(struct pcm_config *p, char **msg)
{
    int err = -1;
    char buf[256];
    char *tmp = NULL;
    snd_pcm_t *_pcm = NULL;
    snd_pcm_uframes_t buffer_size, period_size;

    LG_ALSA_INFO("snd pcm open device : %s stream:%s", p->device,
                 p->stream ? "captrue" : "playback");

    if (btmg_get_debug_level() >= MSG_DEBUG) {
        err = snd_output_stdio_attach(&output, stdout, 0);
        if (err < 0) {
            LG_ALSA_ERROR("output failed :%s", snd_strerror(err));
            return -1;
        }
    }

    if ((err = snd_pcm_open(&_pcm, p->device, p->stream, 0)) != 0) {
        snprintf(buf, sizeof(buf), "Open PCM: %s", snd_strerror(err));
        goto fail;
    }
    p->pcm = _pcm;
    LG_ALSA_INFO("pcm open:%s", p->device);
    if ((err = aw_pcm_set_hw_params(_pcm, p->channels, p->sampling, &p->buffer_time,
                                    &p->period_time, p->format, &tmp)) != 0) {
        snprintf(buf, sizeof(buf), "Set HW params: %s", tmp);
        goto fail;
    }

    if ((err = snd_pcm_get_params(_pcm, &buffer_size, &period_size)) != 0) {
        snprintf(buf, sizeof(buf), "Get params: %s", snd_strerror(err));
        goto fail;
    }
    if ((err = aw_pcm_set_sw_params(_pcm, p->stream, buffer_size, period_size, &tmp)) != 0) {
        snprintf(buf, sizeof(buf), "Set SW params: %s", tmp);
        goto fail;
    }

    if ((err = snd_pcm_prepare(_pcm)) != 0) {
        snprintf(buf, sizeof(buf), "Prepare: %s", snd_strerror(err));
        goto fail;
    }

    if (btmg_get_debug_level() >= MSG_MSGDUMP) {
        snd_pcm_dump(_pcm, output);
    }

    return BT_OK;

fail:
    if (_pcm != NULL)
        snd_pcm_close(_pcm);
    if (msg != NULL)
        *msg = strdup(buf);
    if (tmp != NULL)
        free(tmp);

    return err;
}

void aw_pcm_free(struct pcm_config *pf)
{
    if (btmg_debug_level >= MSG_MSGDUMP && underrun_fd) {
        fclose(underrun_fd);
        underrun_fd = NULL;
    }

    BTMG_DEBUG("---->free pcm(%s),address:%p,stream:%s", pf->device, pf->pcm,
               pf->stream ? "captrue" : "playback");

    if (pf->pcm != NULL) {
        if (pf->drop)
            snd_pcm_drop(pf->pcm);
        else
            snd_pcm_drain(pf->pcm);
        snd_pcm_close(pf->pcm);
        pf->pcm = NULL;
    }
}

int aw_pcm_open(struct pcm_config *pf)
{
    int err = 0;

    if (pf->pcm == NULL) {
        snd_pcm_uframes_t buffer_size;
        snd_pcm_uframes_t period_size;
        char *tmp;
        if (btmg_debug_level >= MSG_DEBUG) {
            printf("DEBUG_A2DP_UNDERRUN_FILE = %s\n", DEBUG_A2DP_UNDERRUN_FILE);
            underrun_fd = fopen(DEBUG_A2DP_UNDERRUN_FILE, "w");
        }
        err = pcm_open(pf, &tmp);
        if (err < 0) {
            BTMG_ERROR("Couldn't open PCM:%s, error:%s", pf->device, tmp);
            free(tmp);
            return err;
        }

        snd_pcm_get_params(pf->pcm, &buffer_size, &period_size);
        LG_ALSA_DEBUG("              \n \
					PCM buffer time:%u us\n \
					PCM period time:%u us\n \
					PCM buffer size:%zu bytes\n \
					PCM period size:%zu bytes\n \
					Sampling rate: %u Hz\n \
					Channels:%u",
                      pf->buffer_time, pf->period_time,
                      snd_pcm_frames_to_bytes(pf->pcm, buffer_size),
                      snd_pcm_frames_to_bytes(pf->pcm, period_size), pf->sampling, pf->channels);
    }

    return 0;
}

int aw_pcm_get_params(struct pcm_config *pf, snd_pcm_uframes_t *buff_size,
                      snd_pcm_uframes_t *period_size)
{
    return snd_pcm_get_params(pf->pcm, buff_size, period_size);
}

int aw_pcm_read(snd_pcm_t *pcm, char *buffer, int frames)
{
    int actual_frams;

    if (NULL == pcm)
        return -1;
    actual_frams = snd_pcm_readi(pcm, buffer, frames);

    if (actual_frams == -EPIPE) {
        LG_ALSA_DEBUG("An overrun has occurred");
        snd_pcm_prepare(pcm);
        usleep(50000);
        actual_frams = 0;
    } else if (actual_frams < 0) {
        LG_ALSA_ERROR("snd_pcm_readi failed: %s", snd_strerror(actual_frams));
        snd_pcm_prepare(pcm);
        usleep(50000);
    }
    if (actual_frams != frames) {
        LG_ALSA_ERROR("Short read (expected: %d, actual: %d)", frames, actual_frams);
    }

    return actual_frams;
}

static void aw_pcm_suspend(snd_pcm_t *handle)
{
    int res = 0;

    while ((res = snd_pcm_resume(handle)) == -EAGAIN)
        sleep(1); /* wait until suspend flag is released */

    if (res < 0) {
        if ((res = snd_pcm_prepare(handle)) < 0) {
            LG_ALSA_ERROR("suspend: prepare error: %s", snd_strerror(res));
            return;
        }
    }
}

int aw_pcm_write(snd_pcm_t *pcm, char *buffer, int frames)
{
    if (pcm == NULL) {
        LG_ALSA_ERROR("pcm is null");
        return -1;
    }

    if ((frames = snd_pcm_writei(pcm, buffer, frames)) < 0)
        switch (-frames) {
        case EPIPE:
            LG_ALSA_ERROR("An underrun has occurred");
            if (btmg_debug_level >= MSG_MSGDUMP && underrun_fd) {
                underrun_cnt++;
                fseek(underrun_fd, 0, SEEK_SET);
                fprintf(underrun_fd, "%lu", underrun_cnt);
                fflush(underrun_fd);
            }
            snd_pcm_prepare(pcm);
            break;
        case ESTRPIPE:
            LG_ALSA_ERROR("snd_pcm_writei errer:ESTRPIPE");
            aw_pcm_suspend(pcm);
            break;
        case EBADFD:
            LG_ALSA_ERROR("snd_pcm_writei error:EBADFD");
            snd_pcm_prepare(pcm);
            break;
        default:
            LG_ALSA_ERROR("Couldn't write to PCM(%p):%s", pcm, snd_strerror(frames));
            frames = -1;
        }

    return frames;
}

int bt_alsa_dump_pcm(int *fd, const char *path, char *buff, uint32_t len, uint32_t max_len)
{
    if (*fd == -1) {
        if (access(path, F_OK) == 0) {
            char cmd[128];
            sprintf(cmd, "rm -rf %s", path);
            system(cmd);
        }
        *fd = open(path, O_WRONLY | O_CREAT, 0777);
        if (*fd == -1) {
            BTMG_ERROR("open %s error:%s", path, strerror(errno));
            return -1;
        }
        BTMG_DUMP("PCM dump start:%s", path);
    }

    struct stat f_info;

    if (*fd >= 0) {
        fstat(*fd, &f_info);
        if (f_info.st_size < max_len) { //8388608 = 8M 1M=1024k=1024*1024 B
            write(*fd, buff, len);
        } else {
            BTMG_INFO("PCM dump stop:%s", path);
            close(*fd);
            *fd = -1;
            return -1;
        }
    }

    return BT_OK;
}
