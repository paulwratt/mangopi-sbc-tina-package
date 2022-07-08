#include "generic_level_server.h"
#include "app_generic_level_server.h"

static pts_app_t *g_app_db;

#define E_IDLE 0
#define E_SET 1
#define E_DELTA_SET 2
#define E_MOVE_SET 3
#define E_TIMEOUT 4

#if (RFU_USED == 0)
#define REMAINING_TIME(pserver)             ((pserver)->state.transition_time_ms - MODEL_TIMER_PERIOD_MS_GET(model_timer_elapsed_ticks_get(&(pserver)->timer)))
#define TRANSITION_TIME_COMPLETE(pserver)   (MODEL_TIMER_PERIOD_MS_GET(model_timer_elapsed_ticks_get(&(pserver)->timer)) >= (pserver)->state.transition_time_ms)
#define ELAPSED_TIME(pserver)               (MODEL_TIMER_PERIOD_MS_GET(model_timer_elapsed_ticks_get(&(pserver)->timer)))
#endif
static void app_level_server_set_cb(const app_level_server_t * p_server, int16_t present_level);
static void app_level_server_get_cb(const app_level_server_t * p_server, int16_t * p_present_level);

/* Forward declaration */
static void generic_level_state_get_cb(const generic_level_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       generic_level_status_params_t * p_out);
static void generic_level_state_set_cb(const generic_level_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const generic_level_set_params_t * p_in,
                                       const model_transition_t * p_in_transition,
                                       generic_level_status_params_t * p_out);

static void generic_level_state_delta_set_cb(const generic_level_server_t * p_self,
                                             const access_message_rx_meta_t * p_meta,
                                             const generic_level_delta_set_params_t * p_in,
                                             const model_transition_t * p_in_transition,
                                             generic_level_status_params_t * p_out);

static void generic_level_state_move_set_cb(const generic_level_server_t * p_self,
                                            const access_message_rx_meta_t * p_meta,
                                            const generic_level_move_set_params_t * p_in,
                                            const model_transition_t * p_in_transition,
                                            generic_level_status_params_t * p_out);

static const generic_level_server_callbacks_t m_level_srv_cbs =
{
    .level_cbs.get_cb = generic_level_state_get_cb,
    .level_cbs.set_cb = generic_level_state_set_cb,
    .level_cbs.delta_set_cb = generic_level_state_delta_set_cb,
    .level_cbs.move_set_cb = generic_level_state_move_set_cb
};

app_level_server_t m_level_server_0 = {
        .level_set_cb = app_level_server_set_cb,
        .level_get_cb = app_level_server_get_cb,
        .value_updated = false,
        .p_dtt_ms = NULL,
        .state = {
            .present_level = 0,
            .target_level = 0,
            .transition_time_ms = 0,
            .delay_ms = 0,
            .transition_type = TRANSITION_NONE,
            .params = {
                .set = {
                            .required_delta = 0,
                            .initial_present_level = 0
                       }
            }
        }
};

#if (RFU_USED == 0)
#else
static void a_transition_complete(void * p_data)
{
    app_level_server_t * p_server = (app_level_server_t *) p_data;
    app_timer_stop(&p_server->p_timer_id);
    p_server->state.transition_time_ms = 0;

    p_server->state.present_level = p_server->state.target_level;

    generic_level_status_params_t status_params;
    status_params.present_level = p_server->state.present_level;
    status_params.target_level = p_server->state.target_level;
    status_params.remaining_time_ms = 0;
    (void) generic_level_server_status_publish(&p_server->server, &status_params);

    p_server->level_set_cb(p_server, p_server->state.present_level);
}

static void level_state_value_update(app_level_server_t * p_server)
{
    /* Requirement: If delay and transition time is zero, current state changes to the target state. */
    if ((p_server->state.delay_ms == 0 && p_server->state.transition_time_ms == 0))
    {
        a_transition_complete(p_server);
    }
}

static void level_state_process_timing(app_level_server_t * p_server)
{
    uint32_t status = NRF_SUCCESS;
    if(p_server->p_timer_id.p_timer_handler != NULL)
    {
        app_timer_stop(&p_server->p_timer_id);
    }
    /* Process timing requirements */
    if (p_server->state.delay_ms != 0)
    {
        status = app_timer_start(&p_server->p_timer_id, APP_TIMER_TICKS(p_server->state.delay_ms), p_server);
    }
    else if (p_server->state.transition_time_ms != 0)
    {
#if 1
        uint32_t app_timer_ticks = APP_TIMER_TICKS(p_server->state.transition_time_ms);
        if (app_timer_ticks > APP_TIMER_MAX_CNT_VAL)
        {
            status = app_timer_start(&p_server->p_timer_id, APP_TIMER_MAX_CNT_VAL, p_server);
        }
        else if (app_timer_ticks >= APP_TIMER_MIN_TIMEOUT_TICKS)
        {
            status = app_timer_start(&p_server->p_timer_id, APP_TIMER_TICKS(p_server->state.transition_time_ms), p_server);
        }
        else
        {
            status = app_timer_start(&p_server->p_timer_id, APP_TIMER_MIN_TIMEOUT_TICKS, p_server);
        }
#endif
    }
    if (status != NRF_SUCCESS)
    {
       l_info("State transition timer error\n");
    }

}

static void level_state_timer_cb(struct l_timeout *timeout, void * p_context)
{

    l_info("%s,time:%p,user_data:%p\n",__FUNCTION__,timeout,p_context);
    app_level_server_t * p_server = (app_level_server_t *) p_context;
    uint32_t delt_ms = 0;

    if (p_server->state.delay_ms != 0)
    {
        p_server->state.delay_ms = 0;
    }
    else if (p_server->state.transition_time_ms != 0)
    {
        delt_ms = app_timer_pass(&p_server->p_timer_id);
        p_server->state.total_time_pass_ms += delt_ms;
        l_info("%s,total time pass %d\n",__FUNCTION__,p_server->state.total_time_pass_ms);
        if(p_server->state.transition_time_ms > delt_ms)
        {
            p_server->state.transition_time_ms -= delt_ms;
        }
        else
        {
            p_server->state.transition_time_ms = 0;
            level_state_value_update(p_server);
        }
    }
    level_state_process_timing(p_server);
}

static void fsm_event_post(fsm_t *p_fsm, uint8_t event, app_level_server_t *p_server)
{
    level_state_value_update(p_server);
    level_state_process_timing(p_server);
}

#endif

/**************************************************************************************************/
/***** Generic Level model interface callbacks *****/

static void generic_level_state_get_cb(const generic_level_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       generic_level_status_params_t * p_out)
{
    uint32_t delta;

    app_level_server_t   * p_server = PARENT_BY_FIELD_GET(app_level_server_t, server, p_self);
    /* Requirement: Provide the current value of the Level state */
    p_server->level_get_cb(p_server, &p_server->state.present_level);
    p_out->present_level = p_server->state.present_level;
    p_out->target_level = p_server->state.target_level;
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg: GET %d\n",p_server->state.transition_type);

    /* Requirement: Report remaining time during processing of SET or DELTA SET,
     *              Report transition time during processing of MOVE */
    if (p_server->state.transition_type == TRANSITION_MOVE_SET)
    {
        p_out->remaining_time_ms = p_server->state.transition_time_ms;
        l_info("%s,remain time %d\n",__FUNCTION__,p_out->remaining_time_ms);
    }
    else
    {
#if (RFU_USED == 0)
        if (TRANSITION_TIME_COMPLETE(p_server))
        {
            p_out->remaining_time_ms = 0;
        }
        else
        {
            p_out->remaining_time_ms = REMAINING_TIME(p_server);
        }
#else
        /* Requirement: Always report remaining time */
        if (p_server->state.transition_time_ms > 0 && p_server->state.delay_ms == 0)
        {
            delta = app_timer_pass(&p_server->p_timer_id);
            p_server->state.total_time_pass_ms += delta;
            l_info("%s,total time pass %d,total %d,delta %d\n",__FUNCTION__,p_server->state.total_time_pass_ms,p_server->state.transition_time_ms,delta);
            p_out->present_level = (p_server->state.params.set.initial_present_level +  \
                p_server->state.params.set.required_delta*delta/p_server->state.transition_time_ms);

            l_info("present level %d,required delta %d, init level:%d ,target level %d\n",p_out->present_level,p_server->state.params.set.required_delta, \
                p_server->state.params.set.initial_present_level,p_out->target_level);

            if (p_server->state.transition_time_ms >= delta && delta > 0)
            {
                p_out->remaining_time_ms = p_server->state.transition_time_ms - delta;
            }
            else
            {
                p_out->remaining_time_ms = 0;
            }
        }
#endif
    }
}

static void generic_level_state_set_cb(const generic_level_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const generic_level_set_params_t * p_in,
                                       const model_transition_t * p_in_transition,
                                       generic_level_status_params_t * p_out)
{
    app_level_server_t   * p_server = PARENT_BY_FIELD_GET(app_level_server_t, server, p_self);

    /* Requirement: If transition time parameters are unavailable and default transition time state
    is not available, transition shall be instantaneous. */
    if (p_in_transition == NULL)
    {
        p_server->state.delay_ms = 0;
        if (p_server->p_dtt_ms == NULL)
        {
            p_server->state.transition_time_ms = \
                pts_transition_time_to_ms(g_app_db->db.glv_trans_time);

            //p_server->state.transition_time_ms = 0;
        }
        else
        {
            p_server->state.transition_time_ms = *p_server->p_dtt_ms;
        }
    }
    else
    {
        p_server->state.delay_ms = p_in_transition->delay_ms;
        p_server->state.transition_time_ms = p_in_transition->transition_time_ms;
    }

    /* Update internal representation of Level value, process timing. */
    p_server->state.target_level = p_in->level;
    p_server->state.transition_type = TRANSITION_SET;
    p_server->state.params.set.initial_present_level = p_server->state.present_level;
    p_server->state.params.set.required_delta = p_server->state.target_level - p_server->state.present_level;
    p_server->state.total_time_pass_ms = 0;
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "SET: Level: %d  delay: %d  tt: %d  req-delta: %d\n",
          p_server->state.target_level,  p_server->state.delay_ms, p_server->state.transition_time_ms,
          p_server->state.params.set.required_delta);

    fsm_event_post(&p_server->fsm, E_SET, p_server);

    /* Prepare response */
    if (p_out != NULL)
    {
        p_out->present_level = p_server->state.present_level;
        p_out->target_level = p_server->state.target_level;
        p_out->remaining_time_ms = p_server->state.transition_time_ms;
    }
    else
    {
        l_info("%s:set unack\n",__FUNCTION__);
    }
}

static void generic_level_state_delta_set_cb(const generic_level_server_t * p_self,
                                             const access_message_rx_meta_t * p_meta,
                                             const generic_level_delta_set_params_t * p_in,
                                             const model_transition_t * p_in_transition,
                                             generic_level_status_params_t * p_out)
{
    app_level_server_t   * p_server = PARENT_BY_FIELD_GET(app_level_server_t, server, p_self);

    /* Requirement: If transition time parameters are unavailable and default transition time state
    is not available, transition shall be instantaneous. */
    if (p_in_transition == NULL)
    {
        p_server->state.delay_ms = 0;
        if (p_server->p_dtt_ms == NULL)
        {
            p_server->state.transition_time_ms = \
                pts_transition_time_to_ms(g_app_db->db.glv_trans_time);

            //p_server->state.transition_time_ms = 0;
        }
        else
        {
            p_server->state.transition_time_ms = *p_server->p_dtt_ms;
        }
    }
    else
    {
        p_server->state.delay_ms = p_in_transition->delay_ms;
        p_server->state.transition_time_ms = p_in_transition->transition_time_ms;
    }

    p_server->state.total_time_pass_ms = 0;
    p_server->state.transition_type = TRANSITION_DELTA_SET;

    /* Update internal representation of Level value, process timing. */
    /* Requirement: If TID is same as previous TID for the same message, delta value is cumulative. */
    int32_t delta = p_in->delta_level % UINT16_MAX;
    if (!model_transaction_is_new(&p_server->server.tid_tracker))
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "tid: %d Same TID, assuming cumulative delta set.\n", p_in->tid);
    }
    else
    {
        p_server->state.params.set.initial_present_level = p_server->state.present_level;
    }
    l_info("%s,delta:%d,%d,%d\n",__FUNCTION__,delta,p_server->state.present_level,p_server->state.params.set.initial_present_level);
    p_server->state.target_level = p_server->state.params.set.initial_present_level + delta;
    p_server->state.params.set.required_delta = delta;

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Delta SET: delta: %d  delay: %d  tt: %d\n",
          p_in->delta_level, p_server->state.delay_ms, p_server->state.transition_time_ms);

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Delta SET: initial-level: %d  present-level: %d  target-level: %d\n",
          p_server->state.params.set.initial_present_level, p_server->state.present_level, p_server->state.target_level);

    fsm_event_post(&p_server->fsm, E_DELTA_SET, p_server);

    /* Prepare response */
    if (p_out != NULL)
    {
        p_out->present_level = p_server->state.present_level;
        p_out->target_level = p_server->state.target_level;
        p_out->remaining_time_ms = p_server->state.transition_time_ms;
    }
    else
    {
        l_info("%s:delta unack\n",__FUNCTION__);
    }

}

static void generic_level_state_move_set_cb(const generic_level_server_t * p_self,
                                            const access_message_rx_meta_t * p_meta,
                                            const generic_level_move_set_params_t * p_in,
                                            const model_transition_t * p_in_transition,
                                            generic_level_status_params_t * p_out)
{
    app_level_server_t   * p_server = PARENT_BY_FIELD_GET(app_level_server_t, server, p_self);
    l_info("%s@1,p_server->state.present_level = %d,p_server->state.target_level = %d\n",__FUNCTION__,p_server->state.present_level,p_server->state.target_level);
    /* Update internal representation of Level value, process timing. */
    if (p_in_transition == NULL)
    {
        //the transition time field is undefine ,the generic move set command will not initate any generic level state change. descript in model spec P47.
        p_server->state.delay_ms = 0;
        if (p_server->p_dtt_ms == NULL)
        {
            //p_server->state.transition_time_ms = 0;
            p_server->state.transition_time_ms = \
                pts_transition_time_to_ms(g_app_db->db.glv_trans_time);
        }
        else
        {
            p_server->state.transition_time_ms = *p_server->p_dtt_ms;
        }
        //l_info("%s,p_in_transition = %p,p_server->state.transition_time_ms=%d,present level =%d\n",__FUNCTION__,p_in_transition,p_server->state.transition_time_ms,p_server->state.present_level);
        p_server->state.target_level = p_server->state.present_level;
    }
    else
    {
        l_info("%s,%d,%d\n",__FUNCTION__,p_in_transition->delay_ms,p_in_transition->transition_time_ms);
        p_server->state.delay_ms = p_in_transition->delay_ms;
        /* Requirement: If transition time is out of range, transition cannot be started. However
        this must be stored to respond correctly to the get messages. */
        if (p_in_transition->transition_time_ms > TRANSITION_TIME_STEP_100MS_MAX)
        {
            p_server->state.transition_time_ms = MODEL_TRANSITION_TIME_INVALID;
        }
        else
        {
            p_server->state.transition_time_ms = p_in_transition->transition_time_ms;
        }
    }

    /* Requirement: For the status message: The target Generic Level state is the upper limit of
       the Generic Level state when the transition speed is positive, or the lower limit of the
       Generic Level state when the transition speed is negative. */
    if (p_in->move_level > 0)
    {
        p_server->state.target_level = INT16_MAX;
    }
    else
    {
        p_server->state.target_level = INT16_MIN;
    }
    l_info("%s@2,p_server->state.present_level = %d,p_server->state.target_level = %d,p_server->state.transition_time_ms =%d\n",__FUNCTION__,p_server->state.present_level,p_server->state.target_level,p_server->state.transition_time_ms);
    p_server->state.params.move.required_move = p_in->move_level;
    p_server->state.params.move.initial_present_level = p_server->state.present_level;
    p_server->state.transition_type = TRANSITION_MOVE_SET;
    p_server->state.total_time_pass_ms = 0;

#if (RFU_USED == 0)
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "MOVE SET: move-level: %d  delay: %d  tt: %d\n",
          p_in->move_level, p_server->state.delay_ms, p_server->state.transition_time_ms);
#else
    l_info("MOVE SET: move-level: %d  delay: %d  tt: %d\n",
          p_in->move_level, p_server->state.delay_ms, p_server->state.transition_time_ms);
#endif
    if((p_server->state.transition_time_ms != MODEL_TRANSITION_TIME_INVALID)&&(p_server->state.transition_time_ms != 0))
        fsm_event_post(&p_server->fsm, E_MOVE_SET, p_server);

    /* Prepare response */
    if (p_out != NULL)
    {
        p_out->present_level = p_server->state.present_level;
        p_out->target_level = p_server->state.target_level;
        p_out->remaining_time_ms = p_server->state.transition_time_ms;
        l_info("%s:move out %d,%d,%d\n",__FUNCTION__,p_out->present_level,p_out->target_level,p_out->remaining_time_ms);
    }
    else
    {
        l_info("%s:move unack\n",__FUNCTION__);
    }
}


/***** Interface functions *****/

uint32_t app_level_current_value_publish(app_level_server_t * p_server)
{
    p_server->level_get_cb(p_server, &p_server->state.present_level);
#if (RFU_USED == 0)
    model_timer_abort(&p_server->timer);
#else
    app_timer_stop(&p_server->p_timer_id);
#endif
    p_server->state.target_level = p_server->state.present_level;
    p_server->state.delay_ms = 0;
    p_server->state.transition_time_ms = 0;

    generic_level_status_params_t status = {
                .present_level = p_server->state.present_level,
                .target_level = p_server->state.target_level,
                .remaining_time_ms = p_server->state.transition_time_ms
            };
    return generic_level_server_status_publish(&p_server->server, &status);
}

/* Callback for updating the hardware state */
static void app_level_server_set_cb(const app_level_server_t * p_server, int16_t present_level)
{
    /* Resolve the server instance here if required, this example uses only 1 instance. */
    l_info("level set callback:new level :%d,current level :%d\n",present_level,g_app_db->db.present_level);
    if(g_app_db->db.present_level != present_level)
    {
        g_app_db->db.present_level = present_level;

        PTS_SEED_START(g_app_db->seed.glv);
        glv_update_plv(g_app_db->seed.glv,present_level);
        glv_update_lln(g_app_db->seed.glv,present_level);
        glv_update_lctl(g_app_db->seed.glv,present_level);
        glv_update_lhsl(g_app_db->seed.glv,present_level);
        PTS_SEED_END(g_app_db->seed.glv);
    }
}

/* Callback for reading the hardware state */
static void app_level_server_get_cb(const app_level_server_t * p_server, int16_t * p_present_level)
{
    /* Resolve the server instance here if required, this example uses only 1 instance. */
    *p_present_level = g_app_db->db.present_level;
}

uint32_t pts_level_reg(pts_app_t *pts_db, uint8_t element_index)
{
    uint32_t status = NRF_ERROR_INTERNAL;
    app_level_server_t * p_server = &m_level_server_0;
    g_app_db = pts_db;

    if (p_server == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if (p_server->level_set_cb == NULL || p_server->level_get_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }
    p_server->server.tid_tracker.old_tid = 0xFF;
    p_server->server.settings.p_callbacks = &m_level_srv_cbs;
    status = generic_level_server_init(&p_server->server, element_index);
#if (RFU_USED == 0)
    if (status == NRF_SUCCESS)
    {
        p_server->timer.p_context = p_server;
        p_server->timer.cb = level_state_timer_cb;

        fsm_init(&p_server->fsm, &m_level_behaviour_descriptor);

        status = model_timer_create(&p_server->timer);
    }
#else
    if(status == NRF_SUCCESS)
    {
        status = app_timer_create(&p_server->p_timer_id, APP_TIMER_MODE_SINGLE_SHOT,
                                  level_state_timer_cb);
    }
    else
    {
        l_info("%s,timer create fail\n",__FUNCTION__);
    }
#endif

    return status;
}

void gdtt_update_glv(uint8_t seed,generic_transition_time_t trans_time)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.glv,seed);
    g_app_db->db.glv_trans_time = trans_time;
}

void hsl_hue_value_update_glv(uint8_t seed,uint16_t hue)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.glv,seed);
    app_level_server_t * p_server = &m_level_server_0;
    int16_t present_level;
    present_level = (hue - 32768);
    g_app_db->db.present_level = present_level;
    p_server->level_set_cb(p_server,present_level);
    app_level_current_value_publish(p_server);
    l_info("%s,%d,%d\n",__FUNCTION__,hue,present_level);
}

void hsl_saturation_value_update_glv(uint8_t seed,uint16_t saturation)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.glv,seed);
    app_level_server_t * p_server = &m_level_server_0;
    int16_t present_level;
    present_level = (saturation - 32768);
    g_app_db->db.present_level = present_level;
    p_server->level_set_cb(p_server,present_level);
    app_level_current_value_publish(p_server);
    l_info("%s,%d,%d\n",__FUNCTION__,saturation,present_level);
}

void ctl_temperature_value_update_glv(uint8_t seed,uint16_t temperature)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.glv,seed);
    app_level_server_t * p_server = &m_level_server_0;
    int16_t present_level;
    present_level = app_light_ctl_temperature_to_generic_level(temperature);
    g_app_db->db.present_level = present_level;
    p_server->level_set_cb(p_server,present_level);
    l_info("%s,%d\n",__FUNCTION__,present_level);
    app_level_current_value_publish(p_server);
}

void lln_update_glv(uint8_t seed,uint16_t lightness)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.glv,seed);
    app_level_server_t * p_server = &m_level_server_0;
    int16_t present_level;
    present_level = (lightness - 32768);

    p_server->level_set_cb(p_server,present_level);
    app_level_current_value_publish(p_server);
    l_info("%s,%d\n",__FUNCTION__,present_level);
}

void gpl_update_glv(uint8_t seed,uint16_t power)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.glv,seed);
    app_level_server_t * p_server = &m_level_server_0;
    int16_t present_level;
    present_level = (power - 32768);

    p_server->level_set_cb(p_server,present_level);
    l_info("%s,%d\n",__FUNCTION__,present_level);
    app_level_current_value_publish(p_server);
}
