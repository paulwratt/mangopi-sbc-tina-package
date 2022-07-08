#ifndef APP_TIMER_MESH_H__
#define APP_TIMER_MESH_H__
#include "nrf_error.h"
#include <sys/time.h>
#include "mesh_internal_api.h"
#define APP_TIMER_TICKS(ms) ms
#define APP_TIMER_MAX_CNT_VAL 0xFFFFFFFF
#define APP_TIMER_MIN_TIMEOUT_TICKS 1
#define APP_TIMER_TIMEOUT_100MS_TICKS 100
#define APP_TIMER_MODE_SINGLE_SHOT 0

enum
{
    APP_TIME_MS_TYPE = 0,
    APP_TIMER_SEC_TYPE,
};

typedef uint8_t timer_type;
typedef unsigned int app_timer_mode_t;
typedef unsigned int ret_code_t;
typedef void(*app_timer_timeout_handler_t)(struct l_timeout *timeout,void * p_context);
typedef struct
{
    bool                        is_running;                                 /**< True if timer is running, False otherwise. */
    timer_type                     type;
    struct l_timeout            *p_timer_handler;
    struct timeval              last_systime;
    app_timer_mode_t            mode;                                       /**< Timer mode. */
    app_timer_timeout_handler_t p_timeout_handler;                          /**< Pointer to function to be executed when the timer expires. */
    void *                      p_context;                                  /**< General purpose pointer. Will be passed to the timeout handler when the timer expires. */
    void *                      next;                                       /**< Pointer to the next node. */
} timer_node_t;
typedef timer_node_t app_timer_id_t;
ret_code_t app_timer_create(app_timer_id_t * p_timer_id, app_timer_mode_t mode, app_timer_timeout_handler_t timeout_handler);
ret_code_t app_timer_start(app_timer_id_t *p_timer_id, uint32_t timeout_ticks, void * p_context);
ret_code_t app_timer_stop(app_timer_id_t *p_timer_id);
uint32_t app_timer_pass(app_timer_id_t *p_time_id);
#endif