/*
* Copyright (c) 2020 Allwinner Technology Co., Ltd. ALL rights reserved.
* Author: laumy liumingyuan@allwinnertech.com
* Date: 2020.04.25
* Description:ring buffer.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bt_alarm.h"
#include "ring_buff.h"
#include "bt_log.h"

#define RING_INFO BTMG_INFO
#define RING_DEBUG BTMG_DEBUG
#define RING_ERROR BTMG_ERROR
#define RING_DUMP BTMG_DUMP
#define RING_WARN BTMG_WARNG
#define RING_EXCESSIVE BTMG_EXCESSIVE

int ring_buff_init(ring_buff_t *q, uint32_t size)
{
    RING_DEBUG("enter");

    q->buff = (uint8_t *)malloc(size);
    if (q->buff == NULL) {
        RING_ERROR("data queue malloc failed.\n");
        return -1;
    }
    memset(q->buff, 0, size);

    q->buff_size = size;
    q->signal = false;
    q->data_len = 0;
    q->p_wait = false;
    q->comsumer_ready = true;
    q->enable = false;
    q->write_pos = q->read_pos = q->buff;

    if (pthread_mutex_init(&q->lock, NULL) != 0) {
        goto failed1;
    }

    if (pthread_condattr_init(&q->cattr) != 0) {
        goto failed2;
    }

    pthread_condattr_setclock(&q->cattr, CLOCK_MONOTONIC);

    if (pthread_cond_init(&q->cond, &q->cattr) != 0) {
        goto failed3;
    }

    if (pthread_condattr_init(&q->pcattr) != 0) {
        goto failed4;
    }

    pthread_condattr_setclock(&q->pcattr, CLOCK_MONOTONIC);

    if (pthread_cond_init(&q->pcond, &q->pcattr) != 0) {
        goto failed4;
    }

    return 0;
failed4:
    pthread_condattr_destroy(&q->pcattr);
failed3:
    pthread_condattr_destroy(&q->cattr);
failed2:
    pthread_mutex_destroy(&q->lock);
failed1:
    if (q->buff) {
        free(q->buff);
    }

    return -1;
}

int ring_buff_deinit(ring_buff_t *q)
{
    RING_DEBUG("enter");

    if (q && q->buff) {
        pthread_condattr_destroy(&q->pcattr);
        pthread_cond_destroy(&q->pcond);

        pthread_mutex_destroy(&q->lock);
        pthread_condattr_destroy(&q->cattr);
        pthread_cond_destroy(&q->cond);
        free(q->buff);
    }
    return 0;
}

static int _pthread_cond_wait_timeout(pthread_mutex_t *lock, pthread_cond_t *cond, uint32_t ms)
{
    struct timespec outime;
    uint64_t us;

    clock_gettime(CLOCK_MONOTONIC, &outime);
    outime.tv_sec += ms / 1000;

    us = outime.tv_nsec / 1000 + 1000 * (ms % 1000);

    //us over 1 s
    outime.tv_sec += us / 1000000;

    us = us % 1000000;

    outime.tv_nsec = us * 1000;
    return pthread_cond_timedwait(cond, lock, &outime);
}

static void data_hex_dump(char *pref, int width, unsigned char *buf, int len)
{
    int i, n;
    for (i = 0, n = 1; i < len; i++, n++) {
        if (n == 1)
            printf("%s ", pref);
        printf("%2.2X ", buf[i]);
        if (n == width) {
            printf("\n");
            n = 0;
        }
    }
    if (i && n != 1)
        printf("\n");
}

#if RING_BUFF_DEBUG
#define ring_buff_writing_debug(q, writing, written)                                               \
    {                                                                                              \
        RING_INFO("RING BUFF DEBUG: buffer pos: %p, end  pos: %p", q->buff,                        \
                  q->buff + q->buff_size - 1);                                                     \
        RING_INFO("RING BUFF DEBUG: write  pos: %p, read pos: %p", q->write_pos, q->read_pos);     \
        RING_INFO("RING BUFF DEBUG: buffersize: %d, data len: %d", q->buff_size, q->data_len);     \
        RING_INFO("RING BUFF DEBUG: writing   : %d, written : %d", writing, written);              \
    }
#else
#define ring_buff_writing_debug(q, writing, written) ;
#endif

uint32_t ring_buff_data_len(ring_buff_t *q)
{
    RING_EXCESSIVE("enter");

    if (q == NULL) {
        RING_ERROR("ring_buff_t is null");
        return -1;
    }

    return q->data_len;
}
int ring_buff_signal(ring_buff_t *q, rb_signal_type_t signal)
{
    RING_DUMP("enter");
    pthread_mutex_lock(&q->lock);
    if (signal == RB_SG_QUIT)
        q->signal = true;
    pthread_mutex_unlock(&q->lock);
    pthread_cond_signal(&q->cond);

    return 0;
}

int ring_buff_start(ring_buff_t *q)
{
    RING_DEBUG("enter");

    pthread_mutex_lock(&q->lock);
    if (q->enable == true)
        goto end;

    memset(q->buff, 0, q->buff_size);
    q->read_pos = q->write_pos = q->buff;
    q->data_len = 0;
    q->signal = false;
    q->p_wait = false;
    q->comsumer_ready = true;
    q->enable = true;

end:
    pthread_mutex_unlock(&q->lock);
    return 0;
}

int ring_buff_stop(ring_buff_t *q)
{
    RING_DEBUG("enter");

    //pthread_mutex_lock(&q->lock);
    if (q->enable == false) {
        RING_DEBUG("ring buffer has stopped");
        goto end;
    }

    q->signal = true;
    q->enable = false;
end:
    //pthread_mutex_unlock(&q->lock);
    pthread_cond_signal(&q->cond);
    pthread_cond_signal(&q->pcond);

    return 0;
}

/**
 * brief write an amount of data to ring buffer.
 * return number of bytes written.
 */

int ring_buff_put(ring_buff_t *q, void *data, uint32_t data_size, int timeout_ms,
                  rb_signal_type_t signal)
{
    uint8_t *data_end = NULL;
    int len = -1;
    int ret = data_size;
    if (q == NULL)
        return -1;

    if (q->buff == NULL)
        return -1;

    if (data_size == 0) {
        return -1;
    }

    if (q->enable == false) {
        RING_ERROR("ring buffer is disable");
        return -1;
    }
    pthread_mutex_lock(&q->lock);

    ring_buff_writing_debug(q, data_size, 0);

    if (q->write_pos == q->read_pos && q->data_len == q->buff_size) {
        ret = 0;
        goto end;
    }
    while (timeout_ms != 0 && q->comsumer_ready == false && q->signal == false) {
        if (timeout_ms < 0) {
            pthread_cond_wait(&q->pcond, &q->lock);
        } else {
            _pthread_cond_wait_timeout(&q->lock, &q->pcond, timeout_ms);
            break;
        }
    }

    if (q->signal == true) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    }

    if (timeout_ms != 0) {
        q->p_wait = true;
        q->comsumer_ready = false;
    }

    data_end = q->buff + q->buff_size - 1;

    if (q->write_pos >= q->read_pos) {
        len = (data_end - q->write_pos + 1) + (q->read_pos - q->buff);

        if (len >= data_size) {
            if ((q->write_pos + data_size - 1) <= data_end) {
                memcpy(q->write_pos, data, data_size);
                q->write_pos = q->write_pos + data_size;
                //just eq data_end
                if (q->write_pos > data_end) {
                    q->write_pos = q->buff;
                }
            } else {
                len = data_end - q->write_pos + 1;
                memcpy(q->write_pos, data, len);
                memcpy(q->buff, data + len, data_size - len);
                q->write_pos = q->buff + (data_size - len);
            }
        } else {
            ret = 0;
        }
    } else {
        len = q->read_pos - q->write_pos;

        if (len >= data_size) {
            memcpy(q->write_pos, data, data_size);
            q->write_pos = q->write_pos + data_size;
            //just eq data_end
            if (q->write_pos > data_end) {
                q->write_pos = q->buff;
            }
        } else {
            ret = 0;
        }
    }
end:
    if (ret == 0) {
        RING_EXCESSIVE("ring buff is full.");
    }

    q->data_len += ret;

    ring_buff_writing_debug(q, data_size, ret);

    if (btmg_ex_debug_mask & EX_DBG_RING_BUFF_WRITE) {
        int time;
        time = btmg_interval_time((void *)ring_buff_put, 1000);
        if (time) {
            RING_EXCESSIVE("RING BUFF DEBUG time interval:%d", time);
            RING_EXCESSIVE("RING BUFF DEBUG timeout:%d", timeout_ms);
            RING_EXCESSIVE("RING BUFF DEBUG: buffer pos: %p, end  pos: %p", q->buff,
                           q->buff + q->buff_size - 1);
            RING_EXCESSIVE("RING BUFF DEBUG: write  pos: %p, read pos: %p", q->write_pos,
                           q->read_pos);
            RING_INFO("RING BUFF DEBUG: buffersize: %d, data len: %d", q->buff_size, q->data_len);
            RING_INFO("RING BUFF DEBUG: writing   : %d, written : %d\n", data_size, ret);
        }
    }

    if (ret == 0) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    }

    int cache = 0;
    switch (signal) {
    case RB_SG_NONE:
        pthread_mutex_unlock(&q->lock);
        break;
    case RB_SG_NOMAL:
        pthread_mutex_unlock(&q->lock);
        pthread_cond_signal(&q->cond);
        break;
    case RB_SG_1_4:
        cache = q->buff_size / 4;
        break;
    case RB_SG_1_2:
        cache = q->buff_size / 2;
        break;
    case RG_SG_2_3:
        cache = q->buff_size * 2 / 3;
        break;
    case RB_SG_QUIT:
        break;
    }

    if (signal >= RB_SG_1_4 && signal <= RG_SG_2_3) {
        if (q->data_len > cache) {
            pthread_mutex_unlock(&q->lock);
            pthread_cond_signal(&q->cond);
        } else {
            pthread_mutex_unlock(&q->lock);
        }
    }
    return ret;
}

#if RING_BUFF_DEBUG
#define ring_buff_reading_debug(q, reading, read)                                                  \
    {                                                                                              \
        RING_INFO("RING BUFF DEBUG: buffer pos: %p, end  pos: %p", q->buff,                        \
                  q->buff + q->buff_size - 1);                                                     \
        RING_INFO("RING BUFF DEBUG: write  pos: %p, read pos: %p", q->write_pos, q->read_pos);     \
        RING_INFO("RING BUFF DEBUG: buffersize: %d, data len: %d", q->buff_size, q->data_len);     \
        RING_INFO("RING BUFF DEBUG: reading   : %d, read    : %d", reading, read);                 \
    }
#else
#define ring_buff_reading_debug(q, reading, read) ;
#endif

uint32_t ring_buff_get_buff_size(ring_buff_t *q)
{
    if (q == NULL)
        return 0;
    return q->buff_size;
}

int ring_buff_get(ring_buff_t *q, void *data, uint32_t data_size, int timeout_ms, uint32_t cache)
{
    uint8_t *data_end = NULL;
    int len = 0;

    if (q == NULL) {
        RING_ERROR("ring buff handle is invaild.");
        return -1;
    }
    if (q->buff == NULL) {
        RING_ERROR("ring buffer is null.");
        return -1;
    }

    if (q->enable == false) {
        RING_ERROR("ring buffer is disable");
        return -1;
    }
    if (data_size == 0 || data == NULL) {
        RING_ERROR("ring buff read buff & len is invaild(%p,%d).", data, data_size);
        return -1;
    }
    pthread_mutex_lock(&q->lock);

    while (q->data_len == 0 && q->signal == false) {
        if (q->p_wait) {
            q->comsumer_ready = true;
            pthread_cond_signal(&q->pcond);
        }
        if (timeout_ms <= 0) {
            pthread_cond_wait(&q->cond, &q->lock);
        } else {
            _pthread_cond_wait_timeout(&q->lock, &q->cond, timeout_ms);
            if (q->data_len == 0) {
                pthread_mutex_unlock(&q->lock);
                return 0;
            } else
                break;
        }
    }

    //capture quit signal.
    if (q->signal == true) {
        if (q->p_wait) {
            q->comsumer_ready = true;
            pthread_cond_signal(&q->pcond);
        }
        pthread_mutex_unlock(&q->lock);
        return 0;
    }

    ring_buff_reading_debug(q, data_size, 0);

    data_end = q->buff + q->buff_size - 1;

    if (q->write_pos <= q->read_pos) {
        len = (data_end - q->read_pos + 1) + (q->write_pos - q->buff);
        if (len >= data_size)
            len = data_size;

        if ((q->read_pos + len - 1) <= data_end) {
            memcpy(data, q->read_pos, len);
            q->read_pos = q->read_pos + len;
            if (q->read_pos > data_end) {
                q->read_pos = q->buff;
            }
        } else {
            int t;
            t = data_end - q->read_pos + 1;

            memcpy(data, q->read_pos, t);
            memcpy(data + t, q->buff, len - t);
            q->read_pos = q->buff + (len - t);
        }
    } else {
        len = q->write_pos - q->read_pos;
        if (len >= data_size)
            len = data_size;

        memcpy(data, q->read_pos, len);

        q->read_pos = q->read_pos + len;

        if (q->read_pos > data_end) {
            q->read_pos = q->buff;
        }
    }
    q->data_len = q->data_len - len;

    if (btmg_ex_debug_mask & EX_DBG_RING_BUFF_READ) {
        int time = btmg_interval_time((void *)ring_buff_get, 1000);
        if (time) {
            RING_EXCESSIVE("RING BUFF DEBUG time interval:%d", time);
            RING_INFO("RING BUFF DEBUG cache:%d,timeout:%d", cache, timeout_ms);
            RING_EXCESSIVE("RING BUFF DEBUG: buffer pos: %p, end  pos: %p", q->buff,
                           q->buff + q->buff_size - 1);
            RING_EXCESSIVE("RING BUFF DEBUG: write  pos: %p, read pos: %p", q->write_pos,
                           q->read_pos);
            RING_INFO("RING BUFF DEBUG: buffersize: %d, data len: %d", q->buff_size, q->data_len);
            RING_INFO("RING BUFF DEBUG: reading   : %d, read    : %d\n", data_size, len);
        }
    }

    if (q->p_wait) {
        q->comsumer_ready = true;
        pthread_cond_signal(&q->pcond);
    }

    ring_buff_reading_debug(q, data_size, len);
    pthread_mutex_unlock(&q->lock);

    return len;
}
