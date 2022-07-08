#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/prctl.h>
#include "transmit.h"
#include "bt_log.h"
#include "bt_alarm.h"

static int transmit_sem_init(ts_sem_t *s)
{
    if (s == NULL) {
        TS_LOG_ERROR("ts_sem_t is null");
        return -1;
    }
    s->wait_enable = true;
    if (pthread_mutex_init(&s->mutex, NULL) != 0) {
        TS_LOG_ERROR("mutex init failed");
        goto failed1;
    }
    if (pthread_cond_init(&s->cond, NULL) == 0) {
        return 0;
    }
    pthread_mutex_destroy(&s->mutex);
    TS_LOG_ERROR("cond init failed");

failed1:
    if (s) {
        free(s);
    }

    return -1;
}

static void transmit_sem_deinit(ts_sem_t *s)
{
    s->wait_enable = false;
    if (s) {
        pthread_mutex_destroy(&s->mutex);
        pthread_cond_destroy(&s->cond);
    }
}

int transmit_sem_post(ts_sem_t *s, bool wait_enable)
{
    int ret = 0;

    pthread_mutex_lock(&s->mutex);
    s->wait_enable = wait_enable;
    pthread_mutex_unlock(&s->mutex);
    if (s->wait_enable == false)
        ret = pthread_cond_signal(&s->cond);

    return ret;
}

int transmit_sem_wait(ts_sem_t *s)
{
    int ret = 0;

    pthread_mutex_lock(&s->mutex);
    while (s->wait_enable == true) {
        ret = pthread_cond_wait(&s->cond, &s->mutex);
    }
    pthread_mutex_unlock(&s->mutex);

    return ret;
}

static int transmit_sem_wait_enable(ts_sem_t *s)
{
    pthread_mutex_lock(&s->mutex);
    s->wait_enable = true;
    pthread_mutex_unlock(&s->mutex);
}

int transmit_init(transmit_t *t, uint32_t buffer_len)
{
    TS_LOG_DEBUG("enter");

    memset(t, 0, sizeof(transmit_t));
    t->comsumer.timeout = 0;
    t->producer.timeout = 0;
    if (ring_buff_init(&t->rb, buffer_len) == -1) {
        TS_LOG_ERROR("ring buff init failed");
        return -1;
    }

    return 0;
}

int transmit_deinit(transmit_t *t)
{
    TS_LOG_DEBUG("enter");

    if (t == NULL) {
        TS_LOG_ERROR("transmit_t is null");
        return -1;
    }

    return ring_buff_deinit(&t->rb);
}

static int transmit_common_init(ts_common_t *co)
{
    TS_LOG_DEBUG("enter");

    if (co->max_data_len == 0 || co->name == NULL) {
        TS_LOG_ERROR("transmit common init fail");
        return -1;
    }
    if (pthread_mutex_init(&co->mutex, NULL) != 0) {
        TS_LOG_ERROR("transmit common mutex init failed.");
        return -1;
    }
    if (co->app_cb.app_tran_cb) {
        co->thread_enable = true;
        return transmit_sem_init(&co->sem);
    }

    return -1;
}

static void transmit_common_deinit(ts_common_t *co)
{
    TS_LOG_DEBUG("enter");

    transmit_sem_deinit(&co->sem);
    pthread_mutex_destroy(&co->mutex);
}

int transmit_comsumer_init(transmit_t *t)
{
    TS_LOG_DEBUG("enter, comsumer:%s", t->comsumer.com.name);

    if (transmit_common_init(&t->comsumer.com) == -1) {
        TS_LOG_ERROR("transmit common init fail");
        return -1;
    }
    t->comsumer.com.type = TS_COMSUMER;

    return _transmit_thread_start(t, TS_COMSUMER);
}

int transmit_comsumer_deinit(transmit_t *t)
{
    TS_LOG_DEBUG("enter, comsumer:%s", t->comsumer.com.name);

    if (ring_buff_stop(&t->rb) == -1) {
        TS_LOG_ERROR("ring buff stop fail");
        return -1;
    }
    if (_transmit_thread_stop(t, TS_COMSUMER) == -1) {
        TS_LOG_ERROR("transmit thread stop fail");
        return -1;
    }
    transmit_common_deinit(&t->comsumer.com);

    return 0;
}

int transmit_producer_init(transmit_t *t)
{
    TS_LOG_DEBUG("enter, producer:%s", t->producer.com.name);

    if (transmit_common_init(&t->producer.com) == -1) {
        TS_LOG_ERROR("transmit common init fail");
        return -1;
    }
    t->producer.com.type = TS_PRODUCER;

    return _transmit_thread_start(t, TS_PRODUCER);
}

int transmit_producer_deinit(transmit_t *t)
{
    TS_LOG_DEBUG("enter, producer:%s", t->producer.com.name);

    if (ring_buff_stop(&t->rb) == -1) {
        TS_LOG_ERROR("ring buff stop fail");
        return -1;
    }
    if (_transmit_thread_stop(t, TS_PRODUCER) == -1) {
        TS_LOG_ERROR("transmit thread stop fail");
        return -1;
    }
    transmit_common_deinit(&t->producer.com);

    return 0;
}

static int _transmit_com_start(ts_common_t *co, void *app_cb_data, uint32_t data_len)
{
    int ret = -1;

    TS_LOG_DEBUG("enter");
    pthread_mutex_lock(&co->mutex);
    co->app_cb.user_data = app_cb_data;
    if (co->max_data_len > data_len) {
        co->data_len = data_len;
    } else {
        TS_LOG_WARN("data_len is max_data_len");
        co->data_len = co->max_data_len;
    }

    if (co->thread_enable)
        ret = transmit_sem_post(&co->sem, false);
    pthread_mutex_unlock(&co->mutex);

    return ret;
}

static int _transmit_com_stop(ts_common_t *co)
{
    int ret = -1;

    TS_LOG_DEBUG("enter");
    pthread_mutex_lock(&co->mutex);
    ret = transmit_sem_post(&co->sem, true);
    co->app_cb.user_data = NULL;
    pthread_mutex_unlock(&co->mutex);

    TS_LOG_DEBUG("quit");
    return ret;
}

static int _transmit_com_stop_immediately(ts_common_t *co)
{
    int ret = -1;

    TS_LOG_DEBUG("enter");
    ret = transmit_sem_post(&co->sem, true);
    pthread_mutex_lock(&co->mutex);
    co->app_cb.user_data = NULL;
    pthread_mutex_unlock(&co->mutex);
    TS_LOG_DEBUG("quit");

    return ret;
}

static void transmit_cache_alarm_init(transmit_t *t)
{
    TS_LOG_DEBUG("enter");

    if (t->comsumer.cache_enable && t->comsumer.cache_timeout) {
        t->comsumer.alarm = btmg_alarm_new();
    }
}

static void transmit_cache_alarm_deinit(transmit_t *t)
{
    TS_LOG_DEBUG("enter");

    if (t->comsumer.cache_timeout && t->comsumer.alarm) {
        btmg_alarm_free(t->comsumer.alarm);
        t->comsumer.alarm = NULL;
    }
}

static void transmit_cache_alarm_timeout(void *data)
{
    transmit_t *t = (transmit_t *)data;

    TS_LOG_DEBUG("cache time is up");
    if (t->comsumer.cache_enable == true) {
        t->comsumer.cache_enable = false;
        transmit_sem_post(&t->comsumer.com.sem, false);
    }
}

static int transmit_cache_set_alarm(transmit_t *t)
{
    TS_LOG_DEBUG("enter");

    if (t->comsumer.cache_timeout) {
        if (t->comsumer.alarm == NULL) {
            TS_LOG_ERROR("comsumer alarm is null.");
            return -1;
        } else {
            btmg_alarm_set(t->comsumer.alarm, t->comsumer.cache_timeout,
                           transmit_cache_alarm_timeout, t);
        }
        return 0;
    }

    return -1;
}

int transmit_comsumer_start(transmit_t *t, void *app_cb_data, uint32_t data_len)
{
    TS_LOG_DEBUG("enter, comsumer:%s", t->comsumer.com.name);

    if (ring_buff_start(&t->rb) == -1) {
        TS_LOG_ERROR("ring buff start fail");
        return -1;
    }
    if (t->comsumer.cache_timeout) {
        t->comsumer.cache_enable = true;
    }
    transmit_cache_alarm_init(t);
    if (transmit_cache_set_alarm(t) == -1) {
        TS_LOG_ERROR("cache alarm set fail");
        return -1;
    }
    if (_transmit_com_start(&t->comsumer.com, app_cb_data, data_len) == -1) {
        TS_LOG_ERROR("transmit com start fail");
        return -1;
    }

    return 0;
}

int transmit_comsumer_stop(transmit_t *t, bool immediately)
{
    int ret = 0;

    TS_LOG_DEBUG("enter, comsumer:%s", t->comsumer.com.name);
    if (immediately)
        ret = _transmit_com_stop_immediately(&t->comsumer.com);
    else
        ret = _transmit_com_stop(&t->comsumer.com);
    if (ret == -1) {
        TS_LOG_ERROR("transmit com stop fail");
        return ret;
    }

    if (ring_buff_stop(&t->rb) == -1) {
        TS_LOG_ERROR("ring buff stop fail");
        return -1;
    }
    transmit_cache_alarm_deinit(t);

    return 0;
}

int transmit_producer_start(transmit_t *t, void *app_cb_data, uint32_t data_len)
{
    TS_LOG_DEBUG("enter, producer:%s", t->producer.com.name);

    if (ring_buff_start(&t->rb) == -1) {
        TS_LOG_DEBUG("ring buff start fail");
        return -1;
    }
    if (_transmit_com_start(&t->producer.com, app_cb_data, data_len) == -1) {
        TS_LOG_DEBUG("transmit com start fail");
        return -1;
    }

    return 0;
}

int transmit_producer_stop(transmit_t *t)
{
    TS_LOG_DEBUG("enter, producer:%s", t->producer.com.name);

    if (_transmit_com_stop(&t->producer.com) == -1) {
        TS_LOG_ERROR("transmit com stop fail");
        return -1;
    }
    if (ring_buff_stop(&t->rb) == -1) {
        TS_LOG_ERROR("ring buff stop fail");
        return -1;
    }

    return 0;
}

int transmit_producer(transmit_t *t, char *buff, uint32_t len, ts_comsumer_signal_type_t signal)
{
    int ret = -1;

    if (t == NULL) {
        TS_LOG_ERROR("transmit_t is null");
        return -1;
    }
    BTMG_EXCESSIVE("len:%d,ring buff data len:%d", len, ring_buff_data_len(&t->rb));
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    ret = ring_buff_put(&t->rb, (void *)buff, len, t->producer.timeout, signal);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (ret == -1) {
        TS_LOG_ERROR("ring buff put fail");
        return -1;
    }
    if (transmit_sem_post(&t->comsumer.com.sem, false) == -1) {
        TS_LOG_ERROR("transmit sem post fail");
        return -1;
    }

    return ret;
}

static int _transmit_com_update_data_size(ts_common_t *c, uint32_t data_len)
{
    if (c->data_len == data_len) {
        TS_LOG_ERROR("data_len has not changed");
        return 0;
    }
    TS_LOG_DUMP("%s: update data len from %d to %d", c->name, c->data_len, data_len);
    if (c->max_data_len >= data_len)
        c->data_len = data_len;
    else
        c->data_len = c->max_data_len;

    return 0;
}

static uint32_t _transmit_com_get_data_size(ts_common_t *c)
{
    return c->data_len;
}

int transmit_comsumer_get_data_size(transmit_t *t)
{
    return _transmit_com_get_data_size(&t->comsumer.com);
}

int transmit_comsumer_update_data_size(transmit_t *t, uint32_t data_size)
{
    return _transmit_com_update_data_size(&t->comsumer.com, data_size);
}

int transmit_producer_update_data_size(transmit_t *t, uint32_t data_size)
{
    return _transmit_com_update_data_size(&t->producer.com, data_size);
}

int transmit_comsumer_signal(transmit_t *t)
{
    int ret = -1;

    if (t == NULL) {
        TS_LOG_ERROR("transmit_t is null");
        return -1;
    }
    ret = ring_buff_data_len(&t->rb);
    if (ret > 0)
        return ring_buff_signal(&t->rb, RB_SG_NOMAL);
    else
        return -1;
}

int transmit_get_data_cache(transmit_t *t)
{
    if (t == NULL) {
        TS_LOG_ERROR("transmit_t is null");
        return -1;
    }

    return ring_buff_data_len(&t->rb);
}

static void producer_thread_exit(void *arg)
{
    int ret;
    ts_producer_t *p = (ts_producer_t *)arg;
    ts_common_t *c = &p->com;
    if (c->data != NULL) {
        free(c->data);
    }
    if (c->app_cb.app_cb_deinit) {
        ret = c->app_cb.app_cb_deinit(c->app_cb.user_data);
        if (ret < 0) {
            TS_LOG_ERROR("app callback deinit failed");
        }
    }
    TS_LOG_DEBUG("producer thread quit:%s", c->name);
}

static void *producer_thread(void *arg)
{
    transmit_t *t = (transmit_t *)arg;
    ts_producer_t *p = &t->producer;
    ts_common_t *co = &p->com;
    int ret;
    int read_len;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    co->data = (char *)malloc(co->max_data_len);
    if (co->data == NULL) {
        TS_LOG_ERROR("producer malloc failed:%s", co->name);
        return NULL;
    }

    pthread_cleanup_push(producer_thread_exit, p);

    if (co->app_cb.app_cb_init) {
        ret = co->app_cb.app_cb_init(co->app_cb.user_data);
        if (ret < 0) {
            TS_LOG_ERROR("app callback init failed");
            goto end;
        }
    }
    co->name[5] = '\0';
    co->thread_loop = true;

    TS_LOG_DEBUG("start producer thread loop(%s):%d", co->name, co->data_len);

    while (co->thread_loop) {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        transmit_sem_wait(&co->sem);

        if (co->app_cb.app_tran_cb) {
            ret = co->app_cb.app_tran_cb(co->app_cb.user_data, co->data, &co->data_len);
            pthread_mutex_unlock(&co->mutex);
        } else {
            pthread_mutex_unlock(&co->mutex);
            continue;
        }
        pthread_mutex_lock(&co->mutex);
        if (co->app_cb.app_tran_cb) {
            read_len = co->app_cb.app_tran_cb(co->app_cb.user_data, co->data, &co->data_len);

            pthread_mutex_unlock(&co->mutex);

            if (read_len <= 0) {
                usleep(1000);
                continue;
            }
        } else {
            pthread_mutex_unlock(&co->mutex);
            usleep(1000);
            continue;
        }

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    again:
        ret = ring_buff_put(&t->rb, co->data, read_len, p->timeout, p->sg_type);
        if (ret == -1) {
            TS_LOG_ERROR("ring buffer is disable");
            continue;
        }
        if (ret == 0 && co->thread_loop && co->app_cb.app_tran_cb) {
            TS_LOG_WARN("fifo is full:%s", co->name);
            usleep(1000);
            goto again;
        }
    }
end:
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_cleanup_pop(1);
}

static void comsumer_thread_exit(void *arg)
{
    int ret = 0;

    ts_comsumer_t *c = (ts_comsumer_t *)arg;
    ts_common_t *co = &c->com;

    TS_LOG_DEBUG("enter, comsumer: %s", co->name);

    if (co->data != NULL) {
        free(co->data);
    }
    if (co->app_cb.app_cb_deinit) {
        ret = co->app_cb.app_cb_deinit(co->app_cb.user_data);
        if (ret < 0) {
            TS_LOG_ERROR("app callback deinit failed");
        }
    }
}

#define EMPTY_COUNT_MAX 5
static void *comsumer_thread(void *arg)
{
    int ret = -1;
    uint32_t len;
    transmit_t *t = (transmit_t *)arg;
    ts_comsumer_t *c = &t->comsumer;
    ts_common_t *co = &c->com;
    static int empty_count = 0;

    if (prctl(PR_SET_NAME, (unsigned long)co->name) == -1) {
        BTMG_ERROR("unable to set comsumer thread name: %s", strerror(errno));
    }

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    co->data = (char *)malloc(co->data_len);
    if (co->data == NULL) {
        BTMG_ERROR("malloc data fail");
        return NULL;
    }

    pthread_cleanup_push(comsumer_thread_exit, c);
    if (co->app_cb.app_cb_init) {
        ret = co->app_cb.app_cb_init(co->app_cb.user_data);
        if (ret < 0) {
            TS_LOG_ERROR("app callback init failed");
            goto end;
        }
    }

    TS_LOG_DEBUG("start comsumer[%s]thread loop", co->name);
    co->thread_loop = true;
    while (co->thread_loop) {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        transmit_sem_wait(&co->sem);
        /*
		 waiting cache
		 */
        if (c->cache_enable) {
            ret = ring_buff_data_len(&t->rb);
            if (ret < c->cache) {
                transmit_sem_wait_enable(&co->sem);
                BTMG_DEBUG("comsumer cache:(%d-%d)", ret, c->cache);
                if (empty_count > EMPTY_COUNT_MAX) {
                    TS_LOG_WARN("trigger cache timeout:%s,%d", co->name, empty_count);
                    transmit_cache_set_alarm(t);
                    empty_count = 0;
                }
                continue;
            }
            c->cache_enable = false;
        }
        len = ring_buff_get(&t->rb, (void *)co->data, co->data_len, c->timeout, c->cache);
        if (len == -1) {
            TS_LOG_ERROR("ring buffer get fail");
            continue;
        }
        if (len == 0) {
            transmit_sem_wait_enable(&co->sem);
            empty_count++;
            TS_LOG_WARN("fifo is empty:%s(%d,%d)", co->name, ring_buff_data_len(&t->rb),
                        empty_count);
            if (empty_count) {
                uint64_t time_ms;
                time_ms = btmg_interval_time((void *)co->name, 1000);
                if (time_ms) {
                    TS_LOG_WARN("time_ms[%llu] empty_count:%d", time_ms, empty_count);
                    if (empty_count > EMPTY_COUNT_MAX)
                        c->cache_enable = true;
                    else
                        empty_count = 0;
                }
            }
            continue;
        }
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        pthread_mutex_lock(&co->mutex);
        if (co->app_cb.user_data && co->app_cb.app_tran_cb) {
            ret = co->app_cb.app_tran_cb(co->app_cb.user_data, co->data, &len);
            pthread_mutex_unlock(&co->mutex);
        } else {
            pthread_mutex_unlock(&co->mutex);
            continue;
        }
    }
end:
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_cleanup_pop(1);
}

static int _transmit_thread_start(transmit_t *t, ts_type_t type)
{
    int ret = -1;
    ts_common_t *co = NULL;

    TS_LOG_DEBUG("enter");

    if (t == NULL) {
        TS_LOG_ERROR("transmit_t is null");
        return -1;
    }
    if (type == TS_PRODUCER)
        co = &t->producer.com;
    else if (type == TS_COMSUMER)
        co = &t->comsumer.com;

    if (co->thread_enable) {
        if (co->type == TS_COMSUMER) {
            ret = pthread_create(&co->thread, NULL, comsumer_thread, t);
        } else if (co->type == TS_PRODUCER) {
            ret = pthread_create(&co->thread, NULL, producer_thread, t);
        }
        TS_LOG_DEBUG("Create thread: %s",
                     (type == TS_COMSUMER) ? "comsumer_thread" : "producer_thread");
        if (ret != 0) {
            TS_LOG_ERROR("Create thread failed:%s",
                         (co->type == TS_COMSUMER) ? "comsumer_thread" : "producer_thread");
            return -1;
        }
    }

    return 0;
}

static int _transmit_thread_stop(transmit_t *t, ts_type_t type)
{
    int err;
    ts_common_t *co = NULL;

    TS_LOG_DEBUG("enter");

    if (type == TS_PRODUCER)
        co = &t->producer.com;
    else if (type == TS_COMSUMER)
        co = &t->comsumer.com;

    if (co->thread_enable) {
        co->thread_loop = false;
        TS_LOG_DEBUG("cancel thread: %s",
                     (type == TS_COMSUMER) ? "comsumer_thread" : "producer_thread");

        if (pthread_cancel(co->thread) != 0) {
            TS_LOG_ERROR("stop thread failed:%s, %s",
                         (type == TS_COMSUMER) ? "comsumer_thread" : "producer_thread",
                         strerror(errno));
            return -1;
        }

        err = pthread_join(co->thread, NULL);
        if (err != 0) {
            TS_LOG_ERROR("stop thread failed:%s, %s",
                         (type == TS_COMSUMER) ? "comsumer_thread" : "producer_thread",
                         strerror(errno));
            return -1;
        }
    }
    TS_LOG_DEBUG("transmit thread stop end.");

    return 0;
}
