#include <errno.h>
#include <getopt.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <dbus/dbus.h>
#include <sys/prctl.h>

#include "bt_log.h"
#include "bt_alsa.h"
#include "bt_alarm.h"
#include "transmit.h"
#include "bt_a2dp_sink.h"
#include "bt_config_json.h"
#include "bt_bluez_signals.h"
#include "common.h"

#include "dbus-client.h"
#include "defs.h"
#include "ffb.h"

#define CACHE_TIMEOUT 500
#define A2DP_SINK_COMSUMER_DATA_LEN 1764
static pthread_t a2dp_sink_thread;
size_t pcm_max_read_len;

static struct pcm_config a2dp_sink_pcm = {
    .device = "default",
    .pcm = NULL,
    .stream = SND_PCM_STREAM_PLAYBACK,
    .channels = 2,
    .sampling = 44100,
    .buffer_time = 400000,
    .period_time = 100000,
    .format = SND_PCM_FORMAT_S16_LE,
};

struct pcm_worker {
    pthread_t thread;
    /*whether the sink pcm thread is running */
    bool thread_enable;
    /* used BlueALSA PCM device */
    struct ba_pcm ba_pcm;
    /* file descriptor of PCM FIFO */
    int ba_pcm_fd;
    /* file descriptor of PCM control */
    int ba_pcm_ctrl_fd;
    /* if true, playback is active */
    bool active;
    /* human-readable BT address */
    char addr[18];
};

static bool ba_profile_a2dp = true;
static bool ba_addr_any = true;
static bdaddr_t *ba_addrs;
static size_t ba_addrs_count = 0;
static struct ba_dbus_ctx dbus_ctx;
static char dbus_ba_service[32] = BLUEALSA_SERVICE;
static struct ba_pcm *ba_pcms = NULL;
static size_t ba_pcms_count = 0;
static pthread_rwlock_t workers_lock = PTHREAD_RWLOCK_INITIALIZER;
static struct pcm_worker *workers = NULL;
static size_t workers_count = 0;
static size_t workers_size = 0;
static bool main_loop_on = true;
static bool media_status = false;
static bool a2dp_sink_stream_cb = false;
static transmit_t *a2dp_sink_ts = NULL;
static struct pcm_config *a2dp_sink_pcm_pf = NULL;

static int a2dp_sink_push_data(char *data, uint32_t len);
static int a2dp_sink_stream_stop(void);
static int a2dp_sink_stream_start(uint16_t channels, uint16_t sampling);

static void media_change_callback(bt_media_state_t state)
{
    if (state == BT_MEDIA_STOP || state == BT_MEDIA_PAUSE) {
        BTMG_DEBUG("a2dp sink stream stop.");
        media_status = false;
    } else if (state == BT_MEDIA_PLAYING) {
        BTMG_DEBUG("a2dp sink stream start.");
        media_status = true;
    }
}

static snd_pcm_format_t get_snd_pcm_format(const struct ba_pcm *pcm)
{
    switch (pcm->format) {
    case 0x0108:
        return SND_PCM_FORMAT_U8;
    case 0x8210:
        return SND_PCM_FORMAT_S16_LE;
    case 0x8318:
        return SND_PCM_FORMAT_S24_3LE;
    case 0x8418:
        return SND_PCM_FORMAT_S24_LE;
    case 0x8420:
        return SND_PCM_FORMAT_S32_LE;
    default:
        BTMG_ERROR("Unknown PCM format: %#x", pcm->format);
        return SND_PCM_FORMAT_UNKNOWN;
    }
}

static struct ba_pcm *get_ba_pcm(const char *path)
{
    size_t i;

    for (i = 0; i < ba_pcms_count; i++)
        if (strcmp(ba_pcms[i].pcm_path, path) == 0)
            return &ba_pcms[i];

    return NULL;
}

static struct pcm_worker *get_active_worker(void)
{
    size_t i;
    struct pcm_worker *w = NULL;

    pthread_rwlock_rdlock(&workers_lock);

    for (i = 0; i < workers_count; i++)
        if (workers[i].active) {
            w = &workers[i];
            break;
        }

    pthread_rwlock_unlock(&workers_lock);

    return w;
}

static void pcm_worker_routine_exit(struct pcm_worker *worker)
{
    if (worker->ba_pcm_fd != -1) {
        close(worker->ba_pcm_fd);
        worker->ba_pcm_fd = -1;
    }
    if (worker->ba_pcm_ctrl_fd != -1) {
        close(worker->ba_pcm_ctrl_fd);
        worker->ba_pcm_ctrl_fd = -1;
    }

    BTMG_INFO("Exiting PCM worker %s", worker->addr);
}

static void *pcm_worker_routine(struct pcm_worker *w)
{
    bool a2dp_stream_enable = false;
    bool pcm_dump = false;
    int pcm_dump_fd = -1;
    ffb_t buffer = { 0 };
    snd_pcm_format_t pcm_format = get_snd_pcm_format(&w->ba_pcm);
    ssize_t pcm_format_size = snd_pcm_format_size(pcm_format, 1);
    size_t pcm_1s_samples = w->ba_pcm.sampling * w->ba_pcm.channels;

    BTMG_DEBUG("enter");

    if (prctl(PR_SET_NAME, (unsigned long)"sink_pcm") == -1) {
        BTMG_ERROR("unable to set sink pcm thread name: %s", strerror(errno));
    }

    /* Cancellation should be possible only in the carefully selected place
	 * in order to prevent memory leaks and resources not being released. */
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_cleanup_push(PTHREAD_CLEANUP(pcm_worker_routine_exit), w);
    pthread_cleanup_push(PTHREAD_CLEANUP(ffb_free), &buffer);

    /* create buffer big enough to hold 100 ms of PCM data */
    if (ffb_init(&buffer, pcm_1s_samples / 10, pcm_format_size) == -1) {
        BTMG_ERROR("Couldn't create PCM buffer: %s", strerror(errno));
        goto fail;
    }

    DBusError err = DBUS_ERROR_INIT;
    if (!bluealsa_dbus_pcm_open(&dbus_ctx, w->ba_pcm.pcm_path, &w->ba_pcm_fd, &w->ba_pcm_ctrl_fd,
                                &err)) {
        BTMG_ERROR("Couldn't open PCM: %s", err.message);
        dbus_error_free(&err);
        goto fail;
    }

    /* Initialize the max read length to 10 ms. Later, when the PCM device
	 * will be opened, this value will be adjusted to one period size. */
    size_t pcm_max_read_len_init = pcm_1s_samples / 100 * pcm_format_size;
    pcm_max_read_len = pcm_max_read_len_init;

    struct pollfd pfds[] = { { w->ba_pcm_fd, POLLIN, 0 } };
    int timeout = -1;

    BTMG_INFO("Starting PCM loop");
    w->thread_enable = true;
    while (main_loop_on) {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

        ssize_t ret;
        /* Reading from the FIFO won't block unless there is an open connection
		 * on the writing side. However, the server does not open PCM FIFO until
		 * a transport is created. With the A2DP, the transport is created when
		 * some clients (BT device) requests audio transfer. */
        switch (poll(pfds, ARRAYSIZE(pfds), timeout)) {
        case -1:
            if (errno == EINTR)
                continue;
            BTMG_ERROR("PCM FIFO poll error: %s", strerror(errno));
            goto fail;
        case 0:
            BTMG_DEBUG("Device marked as inactive: %s", w->addr);
            if (a2dp_sink_stream_cb == false && media_status == false) {
                a2dp_sink_stream_stop();
                a2dp_stream_enable = false;
            }
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
            pcm_max_read_len = pcm_max_read_len_init;
            ffb_rewind(&buffer);
            w->active = false;
            timeout = -1;
            continue;
        }

        /* FIFO has been terminated on the writing side */
        if (pfds[0].revents & POLLHUP)
            break;

#define MIN_S(a, b) a < b ? a : b
        size_t _in = MIN_S(pcm_max_read_len, ffb_blen_in(&buffer));
        if ((ret = read(w->ba_pcm_fd, buffer.tail, _in)) == -1) {
            if (errno == EINTR)
                continue;
            BTMG_ERROR("PCM FIFO read error: %s", strerror(errno));
            goto fail;
        }

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        /* mark device as active and set timeout to 700ms */
        w->active = true;
        timeout = 700;

        ffb_seek(&buffer, ret / pcm_format_size);
        int write_len = ffb_len_out(&buffer) * 2;

        if (btmg_ex_debug_mask & EX_DBG_A2DP_SINK_DUMP_RB) {
            pcm_dump = true;
            btmg_ex_debug_mask &= ~EX_DBG_A2DP_SINK_DUMP_RB;
        }
        if (pcm_dump == true) {
            if (bt_alsa_dump_pcm(&pcm_dump_fd, "/tmp/a2dp_bluealsa.raw", buffer.data, write_len,
                                 (1024 * 1024 * 8)) == -1) {
                pcm_dump = false;
            }
        }
        if (a2dp_sink_stream_cb == true) {
            if (a2dp_stream_enable == true) {
                a2dp_sink_stream_stop();
                a2dp_stream_enable = false;
                ms_sleep(100);
            }
            if (a2dp_sink_pcm_pf && a2dp_sink_pcm_pf->pcm != NULL) {
                aw_pcm_free(a2dp_sink_pcm_pf);
            }
            if (btmg_cb_p && btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_stream_cb) {
                btmg_cb_p->btmg_a2dp_sink_cb.a2dp_sink_stream_cb(
                        NULL, w->ba_pcm.channels, w->ba_pcm.sampling, buffer.data, write_len);
            }
        } else {
            if (a2dp_stream_enable == false) {
                a2dp_sink_stream_start(w->ba_pcm.channels, w->ba_pcm.sampling);
                a2dp_stream_enable = true;
                ms_sleep(100);
            }
            ret = a2dp_sink_push_data(buffer.data, write_len);
            if (ret == 0) {
                write_len = 0;
            }
        }
        ffb_shift(&buffer, write_len / 2);
    }

fail:
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    return NULL;
}

static int a2dp_sink_pcm_worker_start(struct ba_pcm *ba_pcm)
{
    size_t i;

    BTMG_DEBUG("enter");
    for (i = 0; i < workers_count; i++) {
        BTMG_DEBUG("w-pcm_path:%s,b-pcm_path:%s", workers[i].ba_pcm.pcm_path, ba_pcm->pcm_path);
        if (strcmp(workers[i].ba_pcm.pcm_path, ba_pcm->pcm_path) == 0) {
            BTMG_WARNG("%s already started", ba_pcm->pcm_path);
            return BT_OK;
        }
    }
    pthread_rwlock_wrlock(&workers_lock);

    workers_count++;
    if (workers_size < workers_count) {
        struct pcm_worker *tmp = workers;
        workers_size += 4; /* coarse-grained realloc */
        if ((workers = realloc(workers, sizeof(*workers) * workers_size)) == NULL) {
            BTMG_ERROR("Couldn't (re)allocate memory for PCM workers: %s", strerror(ENOMEM));
            workers = tmp;
            pthread_rwlock_unlock(&workers_lock);
            return BT_ERROR_NO_MEMORY;
        }
    }

    struct pcm_worker *worker = &workers[workers_count - 1];
    memcpy(&worker->ba_pcm, ba_pcm, sizeof(worker->ba_pcm));
    ba2str(&worker->ba_pcm.addr, worker->addr);
    worker->active = false;
    worker->ba_pcm_fd = -1;
    worker->ba_pcm_ctrl_fd = -1;

    pthread_rwlock_unlock(&workers_lock);

    BTMG_DEBUG("Creating PCM worker %s", worker->addr);

    if ((errno = pthread_create(&worker->thread, NULL, PTHREAD_ROUTINE(pcm_worker_routine),
                                worker)) != 0) {
        BTMG_ERROR("Couldn't create PCM worker %s: %s", worker->addr, strerror(errno));
        workers_count--;
        return BT_ERROR;
    }

    return BT_OK;
}

static int a2dp_sink_pcm_worker_stop(struct ba_pcm *ba_pcm)
{
    size_t i;

    BTMG_INFO("enter");

    for (i = 0; i < workers_count; i++)
        if (strcmp(workers[i].ba_pcm.pcm_path, ba_pcm->pcm_path) == 0) {
            pthread_rwlock_wrlock(&workers_lock);
            pthread_cancel(workers[i].thread);
            pthread_join(workers[i].thread, NULL);
            workers[i].thread_enable = false;
            memcpy(&workers[i], &workers[--workers_count], sizeof(workers[i]));
            pthread_rwlock_unlock(&workers_lock);
        }

    return BT_OK;
}

static int a2dp_sink_pcm_worker(struct ba_pcm *ba_pcm)
{
    BTMG_DEBUG("enter");

    if (ba_pcm == NULL)
        return BT_ERROR_INVALID_ARGS;

    if (!(ba_pcm->mode & BA_PCM_MODE_SOURCE))
        goto stop;
    if ((ba_profile_a2dp && !(ba_pcm->transport & BA_PCM_TRANSPORT_MASK_A2DP)) ||
        (!ba_profile_a2dp && !(ba_pcm->transport & BA_PCM_TRANSPORT_MASK_SCO)))
        goto stop;
    /* check whether SCO has selected codec */
    if (ba_pcm->transport & BA_PCM_TRANSPORT_MASK_SCO && ba_pcm->sampling == 0) {
        BTMG_DEBUG("Skipping SCO with codec not selected");
        goto stop;
    }

    if (ba_addr_any)
        goto start;

    size_t i;
    for (i = 0; i < ba_addrs_count; i++)
        if (bacmp(&ba_addrs[i], &ba_pcm->addr) == 0)
            goto start;

stop:
    return a2dp_sink_pcm_worker_stop(ba_pcm);
start:
    return a2dp_sink_pcm_worker_start(ba_pcm);
}

static DBusHandlerResult dbus_signal_handler(DBusConnection *conn, DBusMessage *message, void *data)
{
    (void)conn;
    (void)data;

    if (dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_SIGNAL)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    const char *path = dbus_message_get_path(message);
    const char *interface = dbus_message_get_interface(message);
    const char *signal = dbus_message_get_member(message);

    DBusMessageIter iter;
    struct pcm_worker *worker;

    if (strcmp(interface, DBUS_INTERFACE_OBJECT_MANAGER) == 0) {
        if (strcmp(signal, "InterfacesAdded") == 0) {
            if (!dbus_message_iter_init(message, &iter))
                goto fail;
            struct ba_pcm pcm;
            DBusError err = DBUS_ERROR_INIT;
            if (!bluealsa_dbus_message_iter_get_pcm(&iter, &err, &pcm)) {
                BTMG_ERROR("Couldn't add new PCM: %s", err.message);
                dbus_error_free(&err);
                goto fail;
            }
            if (pcm.transport == BA_PCM_TRANSPORT_NONE)
                goto fail;
            struct ba_pcm *tmp = ba_pcms;
            if ((ba_pcms = realloc(ba_pcms, (ba_pcms_count + 1) * sizeof(*ba_pcms))) == NULL) {
                BTMG_ERROR("Couldn't add new PCM: %s", strerror(ENOMEM));
                ba_pcms = tmp;
                goto fail;
            }
            memcpy(&ba_pcms[ba_pcms_count++], &pcm, sizeof(*ba_pcms));
            a2dp_sink_pcm_worker(&pcm);
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        if (strcmp(signal, "InterfacesRemoved") == 0) {
            if (!dbus_message_iter_init(message, &iter) ||
                dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_OBJECT_PATH) {
                BTMG_ERROR("Couldn't remove PCM: %s", "Invalid signal signature");
                goto fail;
            }
            dbus_message_iter_get_basic(&iter, &path);
            struct ba_pcm *pcm;
            if ((pcm = get_ba_pcm(path)) == NULL)
                goto fail;
            a2dp_sink_pcm_worker_stop(pcm);
            return DBUS_HANDLER_RESULT_HANDLED;
        }
    }

    if (strcmp(interface, DBUS_INTERFACE_PROPERTIES) == 0) {
        struct ba_pcm *pcm;
        if ((pcm = get_ba_pcm(path)) == NULL)
            goto fail;
        if (!dbus_message_iter_init(message, &iter) ||
            dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING) {
            BTMG_ERROR("Couldn't update PCM: %s", "Invalid signal signature");
            goto fail;
        }
        dbus_message_iter_get_basic(&iter, &interface);
        dbus_message_iter_next(&iter);
        if (!bluealsa_dbus_message_iter_get_pcm_props(&iter, NULL, pcm))
            goto fail;

        return DBUS_HANDLER_RESULT_HANDLED;
    }

fail:
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void *a2dp_sink_thread_process(void *arg)
{
    BTMG_DEBUG("enter");

    if (prctl(PR_SET_NAME, (unsigned long)"a2dp_sink_thread") == -1) {
        BTMG_ERROR("unable to set a2dp sink thread name: %s", strerror(errno));
    }
    dbus_threads_init_default();
    DBusError err = DBUS_ERROR_INIT;
    if (!bluealsa_dbus_connection_ctx_init(&dbus_ctx, dbus_ba_service, &err)) {
        BTMG_ERROR("Couldn't initialize D-Bus context: %s", err.message);
        return NULL;
    }
    bluealsa_dbus_connection_signal_match_add(&dbus_ctx, dbus_ba_service, NULL,
                                              DBUS_INTERFACE_OBJECT_MANAGER, "InterfacesAdded",
                                              "path_namespace='/org/bluealsa'");
    bluealsa_dbus_connection_signal_match_add(&dbus_ctx, dbus_ba_service, NULL,
                                              DBUS_INTERFACE_OBJECT_MANAGER, "InterfacesRemoved",
                                              "path_namespace='/org/bluealsa'");
    bluealsa_dbus_connection_signal_match_add(&dbus_ctx, dbus_ba_service, NULL,
                                              DBUS_INTERFACE_PROPERTIES, "PropertiesChanged",
                                              "arg0='" BLUEALSA_INTERFACE_PCM "'");

    if (!dbus_connection_add_filter(dbus_ctx.conn, dbus_signal_handler, NULL, NULL)) {
        BTMG_ERROR("Couldn't add D-Bus filter: %s", err.message);
        return NULL;
    }
    if (!bluealsa_dbus_get_pcms(&dbus_ctx, &ba_pcms, &ba_pcms_count, &err))
        BTMG_WARNG("Couldn't get BlueALSA PCM list: %s", err.message);

    size_t i;
    for (i = 0; i < ba_pcms_count; i++)
        a2dp_sink_pcm_worker(&ba_pcms[i]);

    BTMG_DEBUG("Starting main loop");
    while (main_loop_on) {
        struct pollfd pfds[10];
        nfds_t pfds_len = ARRAYSIZE(pfds);

        if (!bluealsa_dbus_connection_poll_fds(&dbus_ctx, pfds, &pfds_len)) {
            BTMG_ERROR("Couldn't get D-Bus connection file descriptors");
            return NULL;
        }

        if (poll(pfds, pfds_len, -1) == -1 && errno == EINTR)
            continue;

        if (bluealsa_dbus_connection_poll_dispatch(&dbus_ctx, pfds, pfds_len))
            while (dbus_connection_dispatch(dbus_ctx.conn) == DBUS_DISPATCH_DATA_REMAINS)
                continue;
    }
    BTMG_DEBUG("main loop quit");

    return NULL;
}

static int a2dp_sink_pcm_write(void *handle, char *buff, uint32_t *len)
{
    struct pcm_config *pf = (struct pcm_config *)handle;
    snd_pcm_uframes_t residue;
    snd_pcm_uframes_t total;
    int ret = BT_OK;
    int ex_frames = -1;
    int ac_frames = -1;
    static int fd = -1;
    static bool pcm_dump_hw = false;

    if (pf == NULL || buff == NULL || len == NULL) {
        BTMG_ERROR("a2dp sink send data error.");
        return BT_ERROR_NULL_VARIABLE;
    }
    if (pf->pcm == NULL) {
        snd_pcm_uframes_t buffer_size;
        snd_pcm_uframes_t period_size;
        ret = aw_pcm_open(pf);
        ms_sleep(100);
        if (ret < 0) {
            BTMG_ERROR("a2dp sink open pcm error:%s", snd_strerror(ret));
            return BT_ERROR_A2DP_PCM_OPEN_FAIL;
        }
        snd_pcm_get_params(pf->pcm, &buffer_size, &period_size);
        pcm_max_read_len = period_size * pf->channels * pf->format;
    }
    ex_frames = *len / (2 * pf->channels);
    residue = ex_frames;
    total = ex_frames;
    while (residue > 0) {
        void *buf = buff + snd_pcm_frames_to_bytes(pf->pcm, total - residue);
        ac_frames = aw_pcm_write(pf->pcm, buf, residue);
        if (ac_frames >= 0) {
            residue -= ac_frames;
            continue;
        }
    }

    if (btmg_ex_debug_mask & EX_DBG_A2DP_SINK_RATE) {
        int data_count = 0;
        int speed = 0;
        int time_ms;

        data_count += *len;
        time_ms = btmg_interval_time((void *)a2dp_sink_pcm_write, 2000);
        if (time_ms) {
            speed = data_count * 1000 / time_ms;
            BTMG_INFO("time_ms[%d] tot[%d] len[%d] ex_frames[%d] ac_frames[%d] speed[%d]", time_ms,
                      data_count, *len, ex_frames, ac_frames, speed);
            data_count = 0;
        }
    }

    if (btmg_ex_debug_mask & EX_DBG_A2DP_SINK_DUMP_HW) {
        pcm_dump_hw = true;
        btmg_ex_debug_mask &= ~EX_DBG_A2DP_SINK_DUMP_HW;
    }
    if (pcm_dump_hw == true) {
        if (bt_alsa_dump_pcm(&fd, "/tmp/a2dp_sink_hw.raw", buff, *len, (1024 * 1024 * 8)) == -1) {
            pcm_dump_hw = false;
        }
    }

    return (ac_frames * (2 * pf->channels));
}

static int a2dp_sink_pcm_deinit(void *usr_data)
{
    BTMG_DEBUG("a2dp sink deinit.");

    if (a2dp_sink_pcm_pf) {
        aw_pcm_free(a2dp_sink_pcm_pf);
        a2dp_sink_pcm_pf = NULL;
    }

    return BT_OK;
}

static int a2dp_sink_stream_start(uint16_t channels, uint16_t sampling)
{
    int len = 0;

    BTMG_DEBUG("enter");
    if (a2dp_sink_ts == NULL || a2dp_sink_pcm_pf == NULL) {
        BTMG_ERROR("a2dp sink has not been initialized");
        return BT_ERROR_A2DP_SINK_NOT_INIT;
    }

    a2dp_sink_pcm_pf->channels = channels;
    a2dp_sink_pcm_pf->sampling = sampling;

    len = A2DP_SINK_COMSUMER_DATA_LEN;
    int byte_s = channels * sampling * 16 / 8;
    a2dp_sink_ts->comsumer.timeout = (len * 1000) / byte_s;
    a2dp_sink_ts->comsumer.cache_timeout = CACHE_TIMEOUT;
    a2dp_sink_ts->comsumer.cache = byte_s * a2dp_sink_ts->comsumer.cache_timeout / 1000;

    BTMG_DEBUG("a2dp sink transmit data len:%d", len);
    if (transmit_comsumer_start(a2dp_sink_ts, a2dp_sink_pcm_pf, len) < 0) {
        BTMG_ERROR("comsumer start fail");
        return BT_ERROR;
    }

    return BT_OK;
}

static int a2dp_sink_stream_stop(void)
{
    BTMG_INFO("a2dp sink stop transmit.");

    if (a2dp_sink_ts == NULL || a2dp_sink_pcm_pf == NULL) {
        BTMG_WARNG("a2dp sink stream has stopped");
        return BT_OK;
    }

    return transmit_comsumer_stop(a2dp_sink_ts, true);
}

static int a2dp_sink_push_data(char *data, uint32_t len)
{
    if (a2dp_sink_ts == NULL) {
        BTMG_ERROR("a2dp sink is not init");
        return BT_ERROR_A2DP_SINK_NOT_INIT;
    }

    return transmit_producer(a2dp_sink_ts, data, len, TS_SG_NONE);
}

int bt_a2dp_sink_init(void)
{
    int ret = -1;
    struct bt_a2dp_sink_cf sink_cf;

    BTMG_DEBUG("a2dp sink init");
    bluez_register_media_change_cb(media_change_callback);
    if (bt_config_read_a2dp_sink(&sink_cf) == -1) {
        BTMG_ERROR("read a2dp sink config failed.");
        return BT_ERROR;
    }

    a2dp_sink_pcm_pf = &a2dp_sink_pcm;
    strcpy(a2dp_sink_pcm_pf->device, sink_cf.device);
    a2dp_sink_pcm_pf->buffer_time = sink_cf.buffer_time;
    a2dp_sink_pcm_pf->period_time = sink_cf.period_time;

    a2dp_sink_ts = (transmit_t *)malloc(sizeof(transmit_t));
    if (a2dp_sink_ts == NULL) {
        BTMG_ERROR("a2dp sink transmit_t malloc failed.");
        return BT_ERROR_NO_MEMORY;
    }

    main_loop_on = true;
    if ((ret = pthread_create(&a2dp_sink_thread, NULL, a2dp_sink_thread_process, NULL)) != 0) {
        BTMG_ERROR("Couldn't create a2dp sink process thread.");
        goto fail;
    }

    int byte_s = a2dp_sink_pcm_pf->sampling * a2dp_sink_pcm_pf->channels * 16 / 8;
    if (transmit_init(a2dp_sink_ts, byte_s) == 0) {
        strcpy(a2dp_sink_ts->comsumer.com.name, "a2sin");
        a2dp_sink_ts->comsumer.cache_timeout = CACHE_TIMEOUT;
        a2dp_sink_ts->comsumer.timeout = (A2DP_SINK_COMSUMER_DATA_LEN * 1000) / byte_s;
        a2dp_sink_ts->comsumer.cache = byte_s * a2dp_sink_ts->comsumer.cache_timeout / 1000;
        a2dp_sink_ts->comsumer.com.app_cb.app_cb_init = NULL;
        a2dp_sink_ts->comsumer.com.app_cb.app_cb_deinit = a2dp_sink_pcm_deinit;
        a2dp_sink_ts->comsumer.com.app_cb.app_tran_cb = a2dp_sink_pcm_write;
        a2dp_sink_ts->comsumer.com.data_len = A2DP_SINK_COMSUMER_DATA_LEN;
        a2dp_sink_ts->comsumer.com.max_data_len = byte_s;

    } else {
        BTMG_ERROR("transmit init fail");
        goto fail;
    }

    if (transmit_comsumer_init(a2dp_sink_ts) < 0) {
        BTMG_ERROR("comsumer init fail");
        goto fail;
    }

    return BT_OK;
fail:

    if (a2dp_sink_ts) {
        free(a2dp_sink_ts);
        a2dp_sink_ts = NULL;
    }

    return BT_ERROR;
}

int bt_a2dp_sink_deinit(void)
{
    main_loop_on = false;

    BTMG_DEBUG("stop A2dp Sink thread.");
    for (int i = 0; i < workers_count; i++) {
        a2dp_sink_pcm_worker_stop(&workers[i].ba_pcm);
    }

    while (1) {
        bool test_worker_quit = false;
        for (int i = 0; i < workers_count; i++) {
            if (workers[i].thread_enable == true) {
                test_worker_quit = true;
            }
        }
        if (test_worker_quit == false)
            break;
        BTMG_INFO("waiting a2dp sink thread process quit.");
        usleep(1000 * 50);
    }
    pthread_cancel(a2dp_sink_thread);
    pthread_join(a2dp_sink_thread, NULL);

    if (a2dp_sink_ts) {
        if (transmit_comsumer_deinit(a2dp_sink_ts) < 0) {
            BTMG_ERROR("comsumer deinit fail");
            return -1;
        }
        if (transmit_deinit(a2dp_sink_ts) < 0) {
            BTMG_ERROR("transmit deinit fail");
            return -1;
        }
    }
    a2dp_sink_ts = NULL;
    a2dp_sink_pcm_pf = NULL;

    return BT_OK;
}

void bt_a2dp_sink_stream_cb_enable(bool enable)
{
    a2dp_sink_stream_cb = enable;
}
