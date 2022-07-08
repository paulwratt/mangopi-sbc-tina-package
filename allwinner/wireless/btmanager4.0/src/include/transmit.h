/*
* Copyright (c) 2018-2020 Allwinner Technology Co., Ltd. ALL rights reserved.
* Author: laumy liumingyuan@allwinnertech.com
* Date: 2020.04.23
* Description:producer and comsumer data transmit.
*/

#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <semaphore.h>
#include "ring_buff.h"

#define TS_LOG_DEBUG BTMG_DEBUG
#define TS_LOG_INFO BTMG_INFO
#define TS_LOG_WARN BTMG_WARNG
#define TS_LOG_ERROR BTMG_ERROR
#define TS_LOG_DUMP BTMG_DUMP

typedef int (*transmit_func)(void *handle, char *buff, uint32_t *len);
typedef int (*ts_app_cb_init_t)(void *user_data);
typedef int (*ts_app_cb_t)(void *user_data, char *buff, uint32_t *len);
typedef int (*ts_app_cb_deinit_t)(void *user_data);

typedef struct {
    ts_app_cb_init_t app_cb_init;
    ts_app_cb_t app_tran_cb;
    ts_app_cb_deinit_t app_cb_deinit;
    void *user_data;
} ts_app_callback_t;

typedef pthread_mutex_t ts_mutex_t;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool wait_enable;
} ts_sem_t;

typedef enum {
    TS_COMSUMER,
    TS_PRODUCER,
} ts_type_t;

typedef enum {
    TS_SG_NONE = 0,
    TS_SG_NOMAL,
    TS_SG_1_4,
    TS_SG_1_2,
    TS_SG_2_3,
    TS_SG_QUIT,
} ts_comsumer_signal_type_t;

typedef struct {
    char name[6];
    ts_type_t type;
    bool thread_loop;
    bool thread_enable;
    pthread_t thread;
    ts_mutex_t mutex;
    ts_app_callback_t app_cb;
    char *data;
    uint32_t data_len;
    uint32_t max_data_len;
    ts_sem_t sem;
} ts_common_t;

typedef struct {
    ts_common_t com;
    int timeout;
    uint32_t cache;
    bool cache_enable;
    struct alarm_t *alarm;
    uint32_t cache_timeout;
} ts_comsumer_t;

typedef struct {
    ts_common_t com;
    ts_comsumer_signal_type_t sg_type;
    int timeout;
} ts_producer_t;

typedef struct {
    ts_producer_t producer;
    ts_comsumer_t comsumer;
    ring_buff_t rb;
} transmit_t;

static int _transmit_thread_start(transmit_t *t, ts_type_t type);
static int _transmit_thread_stop(transmit_t *t, ts_type_t type);
static int transmit_sem_init(ts_sem_t *s);
static void transmit_sem_deinit(ts_sem_t *s);
int transmit_sem_post(ts_sem_t *s, bool wait_enable);
int transmit_sem_wait(ts_sem_t *s);
static int transmit_sem_wait_enable(ts_sem_t *s);
int transmit_init(transmit_t *t, uint32_t buffer_len);
int transmit_deinit(transmit_t *t);
static int transmit_common_init(ts_common_t *co);
static void transmit_common_deinit(ts_common_t *co);
int transmit_comsumer_init(transmit_t *t);
int transmit_comsumer_deinit(transmit_t *t);
int transmit_producer_init(transmit_t *t);
int transmit_producer_deinit(transmit_t *t);
static int _transmit_com_start(ts_common_t *co, void *app_cb_data, uint32_t data_len);
static int _transmit_com_stop(ts_common_t *co);
static int _transmit_com_stop_immediately(ts_common_t *co);
static void transmit_cache_alarm_init(transmit_t *t);
static void transmit_cache_alarm_deinit(transmit_t *t);
static void transmit_cache_alarm_timeout(void *data);
static int transmit_cache_set_alarm(transmit_t *t);
int transmit_comsumer_start(transmit_t *t, void *app_cb_data, uint32_t data_len);
int transmit_comsumer_stop(transmit_t *t, bool immediately);
int transmit_producer_start(transmit_t *t, void *app_cb_data, uint32_t data_len);
int transmit_producer_stop(transmit_t *t);
int transmit_producer(transmit_t *t, char *buff, uint32_t len, ts_comsumer_signal_type_t signal);
static int _transmit_com_update_data_size(ts_common_t *c, uint32_t data_len);
static uint32_t _transmit_com_get_data_size(ts_common_t *c);
int transmit_comsumer_get_data_size(transmit_t *t);
int transmit_comsumer_update_data_size(transmit_t *t, uint32_t data_size);
int transmit_producer_update_data_size(transmit_t *t, uint32_t data_size);
int transmit_comsumer_signal(transmit_t *t);
int transmit_get_data_cache(transmit_t *t);
static void producer_thread_exit(void *arg);
static void *producer_thread(void *arg);
static void comsumer_thread_exit(void *arg);
static void *comsumer_thread(void *arg);
static int _transmit_thread_start(transmit_t *t, ts_type_t type);
static int _transmit_thread_stop(transmit_t *t, ts_type_t type);
#ifdef __cplusplus
}
#endif
#endif
