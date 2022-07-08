#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <poll.h>
#include <pthread.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <poll.h>
#include <pthread.h>
#include <bt_alsa.h>
#include <bt_log.h>
#include "bt_hfp_alsa.h"
#include "bt_config_json.h"

#define HFP_PCM_BUFF_TIME 200000
#define HFP_PCM_PERIOD_TIME 50000

static pthread_mutex_t pcm_open_mutex;

/*Mobile phone audio recording*/
static struct pcm_config phone_to_dev_cap = {
    .device = "hw:snddaudio1",
    .pcm = NULL,
    .stream = SND_PCM_STREAM_CAPTURE,
    .channels = 1,
    .sampling = 8000,
    .buffer_time = HFP_PCM_BUFF_TIME,
    .period_time = HFP_PCM_PERIOD_TIME,
    .format = SND_PCM_FORMAT_S16_LE,
};

/*Mobile phone audio play*/
static struct pcm_config phone_to_dev_play = {
    .device = "hw:audiocodec",
    .pcm = NULL,
    .stream = SND_PCM_STREAM_PLAYBACK,
    .channels = 1,
    .sampling = 8000,
    .buffer_time = HFP_PCM_BUFF_TIME,
    .period_time = HFP_PCM_PERIOD_TIME,
    .format = SND_PCM_FORMAT_S16_LE,
};

/*device recording*/
static struct pcm_config dev_to_phone_cap = {
    .device = "hw:audiocodec",
    .pcm = NULL,
    .stream = SND_PCM_STREAM_CAPTURE,
    .channels = 1,
    .sampling = 8000,
    .buffer_time = HFP_PCM_BUFF_TIME,
    .period_time = HFP_PCM_PERIOD_TIME,
    .format = SND_PCM_FORMAT_S16_LE,
};

/*device source write to mobile phone*/
static struct pcm_config dev_to_phone_play = {
    .device = "hw:snddaudio1",
    .pcm = NULL,
    .stream = SND_PCM_STREAM_PLAYBACK,
    .channels = 1,
    .sampling = 8000,
    .buffer_time = HFP_PCM_BUFF_TIME,
    .period_time = HFP_PCM_PERIOD_TIME,
    .format = SND_PCM_FORMAT_S16_LE,
};

static struct hfp_hf_worker PhoneStreamPlay = {
    .enable = false,
    .thread_flag = PTHONE_STREAM_PLAY,
    .thread_loop = false,
    .read_pcm = &phone_to_dev_cap,
    .write_pcm = &phone_to_dev_play,
    .read_write_frams = 128,
};

static struct hfp_hf_worker DevStreamRecord = {
    .enable = false,
    .thread_flag = DEV_STREAM_RECORD,
    .thread_loop = false,
    .read_pcm = &dev_to_phone_cap,
    .write_pcm = &dev_to_phone_play,
    .read_write_frams = 128,
};

static void _ms_sleep(unsigned long ms)
{
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    int err;
    do {
        err = select(0, NULL, NULL, NULL, &tv);
    } while (err < 0 && errno == EINTR);
}

static void aw_pcm_tansport_routine_data_free(void *arg)
{
    BTMG_DEBUG("enter");

    if (arg != NULL)
        free(arg);
}

static void aw_pcm_tansport_routine_device_free(void *arg)
{
    struct hfp_hf_worker *worker = (struct hfp_hf_worker *)arg;

    BTMG_DEBUG("enter,id:%d", worker->thread_flag);

    if (worker->read_pcm->pcm) {
        aw_pcm_free(worker->read_pcm);
        worker->read_pcm->pcm = NULL;
    }

    if (worker->write_pcm->pcm) {
        aw_pcm_free(worker->write_pcm);
        worker->write_pcm->pcm = NULL;
    }

    worker->enable = false;

    BTMG_DEBUG("quit,id:%d", worker->thread_flag);
}

static void *aw_pcm_transport_runtine(void *arg)
{
    char *read_buffer_ptr;
    int frams;
    int ac_frams = -1;
    static bool pcm_dump = false;
    struct hfp_hf_worker *worker = (struct hfp_hf_worker *)arg;
    static int fd = -1;

    BTMG_DEBUG("enter,id:%d", worker->thread_flag);

    worker->enable = true;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    read_buffer_ptr = (char *)malloc(worker->read_write_frams * 2 * worker->read_pcm->channels);
    if (read_buffer_ptr == NULL) {
        BTMG_ERROR("malloc failed:%s", strerror(errno));
        return NULL;
    }

    pthread_cleanup_push(aw_pcm_tansport_routine_data_free, read_buffer_ptr);
    pthread_cleanup_push(aw_pcm_tansport_routine_device_free, worker);

    BTMG_INFO("opening read pcm device:%s", worker->read_pcm->device);
    if (aw_pcm_open(worker->read_pcm) < 0) {
        BTMG_ERROR("open read pcm failed");
        goto failed;
    }
    BTMG_INFO("opening write pcm device:%s", worker->write_pcm->device);
    if (aw_pcm_open(worker->write_pcm) < 0) {
        BTMG_ERROR("open write pcm failed");
        goto failed;
    }

    while (worker->thread_loop) {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        frams = aw_pcm_read(worker->read_pcm->pcm, read_buffer_ptr, worker->read_write_frams);
        if (frams < 0) {
            BTMG_ERROR("read pcm data null");
            _ms_sleep(100);
            continue;
        }
        if (btmg_ex_debug_mask & EX_DBG_HFP_HF_DUMP_Capture) {
            pcm_dump = true;
            btmg_ex_debug_mask &= ~EX_DBG_HFP_HF_DUMP_Capture;
        }
        if (pcm_dump == true) {
            if (bt_alsa_dump_pcm(&fd, "/tmp/hfp_cap.raw", read_buffer_ptr,
                                 frams * worker->read_pcm->channels * 2, (1024 * 1024 * 8)) == -1) {
                pcm_dump = false;
            }
        }

        ac_frams = aw_pcm_write(worker->write_pcm->pcm, read_buffer_ptr, frams);
        if (ac_frams == -EPIPE)
            usleep(50000);
    }

failed:
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    return NULL;
}

int aw_pcm_tansport_start(struct hfp_hf_worker *worker)
{
    BTMG_DEBUG("enter,id:%d", worker->thread_flag);

    if (worker->enable)
        return BT_OK;

    BTMG_INFO("device:%s,%s;chanels:%d,sampling:%d", worker->read_pcm->device,
              worker->write_pcm->device, worker->read_pcm->channels, worker->read_pcm->sampling);

    BTMG_DEBUG("hfp worker address:%p", worker);

    worker->thread_loop = true;

    if (pthread_create(&worker->thread, NULL, aw_pcm_transport_runtine, worker) != 0) {
        BTMG_WARNG("Couldn't create PCM worker %s: ", strerror(errno));
    }

    return BT_OK;
}

int aw_pcm_transport_stop(struct hfp_hf_worker *worker)
{
    void *tret;
    int err;

    if (!worker->enable)
        return BT_OK;

    BTMG_DEBUG("hfp worker address:%p", worker);

    worker->thread_loop = false;

    if (pthread_cancel(worker->thread) != 0) {
        BTMG_ERROR("hfp worker rountine thread exit failed:%s", strerror(errno));
        return BT_ERROR;
    }

    err = pthread_join(worker->thread, &tret);

    if (err != 0) {
        BTMG_DEBUG("Can't join with thread ");
    }
    return BT_OK;
}

int bt_hfp_alsa_init(void)
{
    struct hfp_pcm pcm_config;

    if (bt_config_read_hfp(&pcm_config) != 0)
        return BT_ERROR;

    phone_to_dev_cap.sampling = pcm_config.rate;
    phone_to_dev_play.sampling = pcm_config.rate;
    dev_to_phone_cap.sampling = pcm_config.rate;
    dev_to_phone_play.sampling = pcm_config.rate;

    memset(phone_to_dev_cap.device, 0, sizeof(phone_to_dev_cap.device));
    strncpy(phone_to_dev_cap.device, pcm_config.phone_to_dev_cap,
            strlen(pcm_config.phone_to_dev_cap));

    memset(phone_to_dev_play.device, 0, sizeof(phone_to_dev_play.device));
    strncpy(phone_to_dev_play.device, pcm_config.phone_to_dev_play,
            strlen(pcm_config.phone_to_dev_play));

    memset(dev_to_phone_cap.device, 0, sizeof(dev_to_phone_cap.device));
    strncpy(dev_to_phone_cap.device, pcm_config.dev_to_phone_cap,
            strlen(pcm_config.dev_to_phone_cap));

    memset(dev_to_phone_play.device, 0, sizeof(dev_to_phone_play.device));
    strncpy(dev_to_phone_play.device, pcm_config.dev_to_phone_play,
            strlen(pcm_config.dev_to_phone_play));

    if (pthread_mutex_init(&pcm_open_mutex, NULL) != 0) {
        BTMG_ERROR("hfp alsa mutex init failed.");
        return BT_ERROR;
    }

    return BT_OK;
}

void bt_hfp_alsa_deinit(void)
{
    pthread_mutex_destroy(&pcm_open_mutex);
}

static bool hfp_hf_pcm_start = false;

void bt_hfp_hf_pcm_start(void)
{
    BTMG_DEBUG("enter");

    pthread_mutex_lock(&pcm_open_mutex);
    if (hfp_hf_pcm_start == true) {
        pthread_mutex_unlock(&pcm_open_mutex);
        BTMG_WARNG("hfp pcm already open");
        return;
    }
    if (!PhoneStreamPlay.enable)
        aw_pcm_tansport_start(&PhoneStreamPlay);

    if (!DevStreamRecord.enable)
        aw_pcm_tansport_start(&DevStreamRecord);

    hfp_hf_pcm_start = true;
    pthread_mutex_unlock(&pcm_open_mutex);
}
void bt_hfp_hf_pcm_stop(void)
{
    BTMG_DEBUG("enter");

    pthread_mutex_lock(&pcm_open_mutex);
    if (hfp_hf_pcm_start == false) {
        pthread_mutex_unlock(&pcm_open_mutex);
        BTMG_WARNG("hfp pcm already stop");
        return;
    }

    if (PhoneStreamPlay.enable)
        aw_pcm_transport_stop(&PhoneStreamPlay);

    if (DevStreamRecord.enable)
        aw_pcm_transport_stop(&DevStreamRecord);

    hfp_hf_pcm_start = false;
    pthread_mutex_unlock(&pcm_open_mutex);
}
