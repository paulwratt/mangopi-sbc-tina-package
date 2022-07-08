/*
* Copyright (c) 2018-2020 Allwinner Technology Co., Ltd. ALL rights reserved.
* Author: laumy liumingyuan@allwinnertech.com
* Date: 2018.11.26
* Description:Create A2DP Source function.
*/

#include <alsa/asoundlib.h>
#include <unistd.h>
#include "bt_log.h"
#include "common.h"
#include "bt_a2dp_source.h"
#include "bt_alsa.h"
#include "bt_alarm.h"
#include "transmit.h"

static bool a2dp_stream_enable = false;
static int a2dp_write_data_len = 4096;
static transmit_t *a2dp_src_ts = NULL;
struct pcm_config *a2dp_src_pcm_pf = NULL;

static int _a2dp_src_pcm_write(void *handle, char *buff, uint32_t *len)
{
    int fd = -1;
    int ret = BT_OK;
    int ex_frames = -1;
    int ac_frames = -1;
    snd_pcm_uframes_t residue;
    snd_pcm_uframes_t total;
    static bool pcm_dump_hw = false;
    struct pcm_config *cf = (struct pcm_config *)handle;

    if (cf == NULL || buff == NULL || len == NULL) {
        BTMG_ERROR("parameter error");
        return BT_ERROR_INVALID_ARGS;
    }

    if (a2dp_src_pcm_pf->pcm == NULL) {
        ret = aw_pcm_open(a2dp_src_pcm_pf);
        ms_sleep(100);
        if (ret < 0) {
            BTMG_ERROR("a2dp source open pcm error:%s", snd_strerror(ret));
            return BT_ERROR_A2DP_PCM_OPEN_FAIL;
        }
    }

    ex_frames = *len / (2 * cf->channels);
    residue = ex_frames;
    total = ex_frames;

    while (residue > 0) {
        if (_is_a2dp_device_connected() == false) {
            BTMG_ERROR("device is disconnected!");
            break;
        }
        void *buf = buff + snd_pcm_frames_to_bytes(cf->pcm, total - residue);
        ac_frames = aw_pcm_write(cf->pcm, buf, residue);
        if (ac_frames >= 0) {
            residue -= ac_frames;
            continue;
        } else {
            BTMG_ERROR("pcm write fail");
            break;
        }
    }

    if (btmg_ex_debug_mask & EX_DBG_A2DP_SOURCE_DUMP_LOW) {
        pcm_dump_hw = true;
        btmg_ex_debug_mask &= ~EX_DBG_A2DP_SOURCE_DUMP_LOW;
    }
    if (pcm_dump_hw == true) {
        if (bt_alsa_dump_pcm(&fd, "/tmp/a2dp_bluealsa.raw", buff, *len, (1024 * 1024 * 8)) == -1) {
            pcm_dump_hw = false;
        }
    }

    if (btmg_ex_debug_mask & EX_DBG_A2DP_SOURCE_LOW_RATE) {
        static int data_count = 0;
        int speed = 0;
        int time_ms;

        data_count += *len;

        time_ms = btmg_interval_time((void *)_a2dp_src_pcm_write, 1500);
        if (time_ms) {
            speed = data_count * 1000 / time_ms;
            BTMG_INFO("time_ms[%d] tot[%d] len[%d] ex_frames[%d] ac_frames[%d] speed[%d]", time_ms,
                      data_count, *len, ex_frames, ac_frames, speed);
            data_count = 0;
        }
    }

    return ac_frames;
}

static int _a2dp_src_pcm_deinit(void *usr_data)
{
    BTMG_DEBUG("enter");

    if (a2dp_src_pcm_pf != NULL) {
        aw_pcm_free(a2dp_src_pcm_pf);
        free(a2dp_src_pcm_pf);
        a2dp_src_pcm_pf = NULL;
        BTMG_DEBUG("free a2dp_src_pcm_pf");
    }

    return BT_OK;
}

int bt_a2dp_src_stream_send(char *data, int len)
{
    int ret = BT_OK;
    int byte_s = -1, us = -1;
    static int fd = -1;
    static bool pcm_dump_hw = false;

    if (a2dp_stream_enable == false) {
        BTMG_ERROR("a2dp src stream has not started, send fail");
        return BT_ERROR_A2DP_SRC_STREAM_NOT_START;
    }

    if (btmg_ex_debug_mask & EX_DBG_A2DP_SOURCE_UP_RATE) {
        static int data_count = 0;
        int speed = 0;
        int time_ms;

        data_count += len;

        time_ms = btmg_interval_time((void *)bt_a2dp_src_stream_send, 500);
        if (time_ms) {
            speed = data_count * 1000 / time_ms;
            BTMG_INFO("time_ms[%d] tot[%d] len[%d] speed[%d]", time_ms, data_count, len, speed);
            data_count = 0;
        }
    }

    if (btmg_ex_debug_mask & EX_DBG_A2DP_SOURCE_DUMP_UP) {
        pcm_dump_hw = true;
        btmg_ex_debug_mask &= ~EX_DBG_A2DP_SOURCE_DUMP_UP;
    }
    if (pcm_dump_hw == true) {
        if (bt_alsa_dump_pcm(&fd, "/tmp/a2dp_src_stream.raw", data, len, (1024 * 1024 * 8)) == -1) {
            pcm_dump_hw = false;
        }
    }

    ret = transmit_producer(a2dp_src_ts, data, len, TS_SG_NONE);

    if (transmit_get_data_cache(a2dp_src_ts) < (int)(a2dp_src_ts->comsumer.cache / 4)) {
         BTMG_DEBUG("data cache is below threshold..");
         return ret;
    }

    byte_s = a2dp_src_pcm_pf->sampling * a2dp_src_pcm_pf->channels * 16 / 8;
    us = (len * 1000 / byte_s) * 1000;
    us = us - 150;
    if (us <= 0) {
        us = 60;
    }
    usleep(us);

    return ret;
}

int bt_a2dp_src_stream_start(uint32_t len)
{
    BTMG_DEBUG("enter");

    if (a2dp_src_ts == NULL || a2dp_src_pcm_pf == NULL) {
        BTMG_ERROR("a2dp src has not been initialized");
        return BT_ERROR_A2DP_SRC_NOT_INIT;
    }
    if (transmit_comsumer_start(a2dp_src_ts, a2dp_src_pcm_pf, len) < 0) {
        BTMG_ERROR("comsumer start fail");
        return BT_ERROR;
    }
    a2dp_write_data_len = len;
    a2dp_stream_enable = true;

    return BT_OK;
}

int bt_a2dp_src_stream_stop(bool drop)
{
    int ret = BT_OK;

    BTMG_DEBUG("enter");

    if (a2dp_src_ts == NULL || a2dp_src_pcm_pf == NULL) {
        BTMG_WARNG("a2dp src stream has stopped");
        return BT_OK;
    }

    a2dp_stream_enable = false;
    a2dp_src_pcm_pf->drop = drop;
    if (!drop) {
        while ((ret = transmit_get_data_cache(a2dp_src_ts)) > 0) {
            BTMG_DEBUG("waiting transmit end...(%d), comsumer.cache:%d, comsumer.timeout:%d", ret,
                       a2dp_src_ts->comsumer.cache, a2dp_src_ts->comsumer.timeout);
            transmit_sem_post(&a2dp_src_ts->comsumer.com.sem, false);
            usleep(1000 * 100);
        }
    }

    if (transmit_comsumer_stop(a2dp_src_ts, drop) < 0) {
        BTMG_ERROR("comsumer stop fail");
        return BT_ERROR;
    }

    BTMG_DEBUG("a2dp source stream stop successful!");

    return BT_OK;
}

bool bt_a2dp_src_is_stream_start(void)
{
    return a2dp_stream_enable;
}

int bt_a2dp_src_init(uint16_t channels, uint16_t sampling)
{
    int byte_s;
    char mac[18] = { 0 };
    char pcm_dev[32] = { 0 };
    dev_node_t *dev_node = NULL;

    BTMG_DEBUG("enter");
    dev_node = connected_devices->head;
    while (dev_node != NULL) {
        if (dev_node->profile & BTMG_REMOTE_DEVICE_A2DP) {
            memcpy(mac, dev_node->dev_addr, sizeof(mac));
            break;
        }
        dev_node = dev_node->next;
    }
    if (mac[0] == '\0') {
        BTMG_ERROR("no device connected, please try again after connected");
        return BT_ERROR_A2DP_DEVICE_NOT_CONNECTED;
    }

    //bluealsa:DEV=XX:XX:XX:XX:XX:XX
    snprintf(pcm_dev, 31, "bluealsa:DEV=%s", mac);
    BTMG_DEBUG("a2dp source pcm dev:%s", pcm_dev);
    if (a2dp_src_pcm_pf != NULL) {
        BTMG_DEBUG("a2dp source pcm already open.");
        return BT_OK;
    }

    a2dp_src_pcm_pf = (struct pcm_config *)malloc(sizeof(struct pcm_config));
    if (a2dp_src_pcm_pf == NULL) {
        BTMG_ERROR("a2dp source pcm config malloc failed.");
        goto fail;
    }

    memset(a2dp_src_pcm_pf, 0, sizeof(struct pcm_config));
    a2dp_src_pcm_pf->buffer_time = 400000;
    a2dp_src_pcm_pf->period_time = 100000;
    a2dp_src_pcm_pf->format = SND_PCM_FORMAT_S16_LE;
    a2dp_src_pcm_pf->stream = SND_PCM_STREAM_PLAYBACK;
    a2dp_src_pcm_pf->channels = channels;
    a2dp_src_pcm_pf->sampling = sampling;
    strncpy(a2dp_src_pcm_pf->device, pcm_dev, 32);

    a2dp_src_ts = (transmit_t *)malloc(sizeof(transmit_t));
    if (a2dp_src_ts == NULL) {
        BTMG_ERROR("a2dp source transmit_t malloc failed.");
        goto fail;
    }

    byte_s = a2dp_src_pcm_pf->sampling * a2dp_src_pcm_pf->channels * 16 / 8;
    if (transmit_init(a2dp_src_ts, byte_s) == 0) {
        strncpy(a2dp_src_ts->comsumer.com.name, "a2src", 5);
        a2dp_src_ts->comsumer.cache_timeout = 300;
        a2dp_src_ts->comsumer.cache = byte_s * a2dp_src_ts->comsumer.cache_timeout / 1000;
        a2dp_src_ts->comsumer.com.app_cb.app_cb_init = NULL;
        a2dp_src_ts->comsumer.com.app_cb.app_cb_deinit = _a2dp_src_pcm_deinit;
        a2dp_src_ts->comsumer.com.app_cb.app_tran_cb = _a2dp_src_pcm_write;
        a2dp_src_ts->comsumer.com.data_len = a2dp_write_data_len;
        a2dp_src_ts->comsumer.com.max_data_len = byte_s;
        a2dp_src_ts->comsumer.timeout = (a2dp_write_data_len * 1000) / byte_s;
        BTMG_DEBUG("comsumer timeout :%d", a2dp_src_ts->comsumer.timeout);
        BTMG_DEBUG("comsumer cache :%d", a2dp_src_ts->comsumer.cache);
    } else {
        BTMG_ERROR("transmit init fail");
        goto fail;
    }
    if (transmit_comsumer_init(a2dp_src_ts) < 0) {
        BTMG_ERROR("comsumer init fail");
        goto fail;
    }

    BTMG_DEBUG("a2dp source init successful");

    return BT_OK;
fail:

    if (a2dp_src_ts) {
        free(a2dp_src_ts);
        a2dp_src_ts = NULL;
    }

    if (a2dp_src_pcm_pf) {
        free(a2dp_src_pcm_pf);
        a2dp_src_pcm_pf = NULL;
    }

    return BT_ERROR;
}

int bt_a2dp_src_deinit(void)
{
    BTMG_DEBUG("enter");

    if (a2dp_src_ts) {
        transmit_comsumer_deinit(a2dp_src_ts);
        transmit_deinit(a2dp_src_ts);
        free(a2dp_src_ts);
        a2dp_src_ts = NULL;
        BTMG_DEBUG("free a2dp_src_ts");
    }

    BTMG_DEBUG("a2dp source deinit successful!");

    return BT_OK;
}
