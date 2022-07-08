#ifndef PTS_CLIENT_APP_H__
#define PTS_CLIENT_APP_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ell/ell.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "pts_app.h"
#include "mesh_internal_api.h"
#include "model_common.h"
#include "app_timer_mesh.h"

//RTK COMMON
#include "pdu.h"
#include "configuration.h"
#include "mesh_node.h"
//RTK MODELS
#include "generic_on_off.h"
#include "generic_level.h"
#include "generic_default_transition_time.h"
#include "generic_power_on_off.h"
#include "generic_power_level.h"
#include "generic_battery.h"
#include "light_lightness.h"
#include "light_ctl.h"
#include "light_hsl.h"
#include "health.h"
//platform api
#include "semaphore.h"

#define goo_log(FMT, ...) l_info(FMT, ##__VA_ARGS__)
#define glv_log(FMT, ...) l_info(FMT, ##__VA_ARGS__)
#define gdtt_log(FMT, ...) l_info(FMT, ##__VA_ARGS__)
#define gponoff_log(FMT, ...) l_info(FMT, ##__VA_ARGS__)
#define ts_log(FMT, ...) l_info(FMT, ##__VA_ARGS__)

#define app_ts_log(TAG,FMT, ...) l_info("app_ts_%s_log:\t"FMT,#TAG,##__VA_ARGS__)

#define data_uart_debug(FMT, ...) l_info(FMT, ##__VA_ARGS__)

#define ENABLE_GENERIC_ONOFF_CLIENT_APP 1
#define ENABLE_GENERIC_LEVEL_CLIENT_APP 1
#define ENABLE_GENERIC_TRANSITION_TIME_CLIENT_APP 1
#define ENABLE_GENERIC_POWER_ONOFF_CLIENT_APP 1
#define ENABLE_GENERIC_POWER_LEVEL_CLIENT_APP 1
#define ENABLE_GENERIC_BATTERY_CLIENT_APP 1

#define ENABLE_LIGHT_LIGHTNESS_CLIENT_APP 1
#define ENABLE_LIGHT_CTL_CLIENT_APP 1
#define ENABLE_LIGHT_HSL_CLIENT_APP 1
#define ENABLE_LIGHT_HM_CLIENT_APP 1

#define STR_PTS_RESULT_SUCCESS "PTS PASS"
#define STR_PTS_RESULT_FAIL "PTS FAIL"
#define STR_PTS_RESULT_RX_INVAL "PTS INVAL RX"
#define STR_PTS_RUN_END "PTS RUN END"

#define STR_CL_ACK_RX(TESTCASE) "MMDL/CL/"#TESTCASE"/ACK"

#define STR_TESTCASE_01 "bv-01-c" //cas 5, 6,7
#define STR_TESTCASE_02 "bv-02-c"
#define STR_TESTCASE_03 "bv-03-c"
#define STR_TESTCASE_04 "bv-04-c"
#define STR_TESTCASE_05 "bv-05-c"
#define STR_TESTCASE_06 "bv-06-c"
#define STR_TESTCASE_07 "bv-07-c"
#define STR_TESTCASE_08 "bv-08-c"
#define STR_TESTCASE_09 "bv-09-c"
#define STR_TESTCASE_10 "bv-10-c"
#define STR_TESTCASE_11 "bv-11-c"
#define STR_TESTCASE_12 "bv-12-c"
#define STR_TESTCASE_13 "bv-13-c"
#define STR_TESTCASE_14 "bv-14-c"
#define STR_TESTCASE_15 "bv-15-c"
//test case cmdline
#define STR_TESTCASE_BV_NB(NB)"bv-"#NB"-c"
#define STR_TESTCASE_NAME(NAME,NB) "MMDL/CL/"#NAME"/BV-"#NB"-C"
#define func_testcase(NAME,NB)   NAME##_bv_##NB##_c
#define PTS_SET_ACK 1
#define PTS_SET_UNACK 0
#define PTS_TID_0 0

//ERROR CODE
#define PTS_SUCCESS 0
#define PTS_ERROR 1

typedef struct
{
    uint8_t tid;
    int32_t dst;
    int32_t app_key_idx;
}pts_client_tx_info;

typedef struct
{
    bool present_onoff;
}pts_client_goo_db;

typedef struct
{
    pts_client_goo_db goo;
}pts_client_database_t;

typedef struct
{
    uint16_t element_index;
    pts_database_t db;
}pts_client_app_t;

#define TS_RETRY_MAX_CNT 2
//sem api
#define TS_STATE_0 0
#define TS_STATE_1 1
#define TS_STATE_2 2
#define TS_STATE_3 3
#define TS_STATE_4 4

#define TS_STATE_STOP 0xFF
//ts cb
typedef struct _ts_mgs_t *ts_mgs_p;
typedef void (*ts_release_cb)(ts_mgs_p p_msg);
typedef void (*ts_tx_cb)(ts_mgs_p p_msg);
typedef void (*ts_rx_cb)(ts_mgs_p p_msg,void *p_data, uint32_t len);

typedef struct _ts_mgs_t
{
    uint32_t timeout_ms;
    uint32_t run_delay_ms;
    uint8_t  ts_state;
    uint8_t  msg_state;
    uint8_t cmd_id;
    bool     msg_rx;
    bool     msg_tx;
    sem_t    *p_sem;
    struct timespec ts;
    void     *reply;
    void     *pargs;
    ts_tx_cb    tx_cb;
    ts_rx_cb    rx_cb;
    ts_release_cb    release_cb;
}ts_msg;
#if 1
extern  generic_transition_time_t pts_trans_time_ei;
extern  generic_transition_time_t pts_trans_time_i;
extern  generic_transition_time_t pts_trans_time_c;
extern  uint8_t pts_delay_i;
extern  uint8_t pts_delay_c;
#else
extern const generic_transition_time_t pts_trans_time_ei;
extern const generic_transition_time_t pts_trans_time_i;
extern const generic_transition_time_t pts_trans_time_c;
extern const uint8_t pts_delay_i;
extern const uint8_t pts_delay_c;
#endif
void ts_rx(void *p_data, uint32_t len);
void ts_run(ts_mgs_p p_msg);

typedef void (*app_client_cmd_cb)(ts_mgs_p p_msg);
uint32_t pts_generic_onoff_reg(uint8_t element_index);
void run_hm_client_ts(int argc, char *args[]);
void run_lhsl_client_ts(int argc, char *args[]);
void run_lctl_client_ts(int argc, char *args[]);
void run_lln_client_ts(int argc, char *args[]);
void run_gbat_client_ts(int argc, char *args[]);
void run_gpl_client_ts(int argc, char *args[]);
void run_gpoo_client_ts(int argc, char *args[]);
void run_gdtt_client_ts(int argc, char *args[]);
void run_goo_client_ts(int argc, char *args[]);
void run_glv_client_ts(int argc, char *args[]);
void run_client_ts(int argc, char *args[]);
bool ts_get_param(uint16_t *p_value, uint8_t cnt, char *c_pattern);
uint16_t pts_get_element_idx();
#endif /* PTS_CLIENT_APP_H__ */
