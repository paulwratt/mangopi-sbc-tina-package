
#include "generic_onoff_server.h"
#include "app_generic_on_off_server.h"

static pts_app_t *g_app_db = NULL;

static void app_onoff_server_set_cb(const app_onoff_server_t * p_server, bool onoff);
static void app_onoff_server_get_cb(const app_onoff_server_t * p_server, bool * p_present_onoff);
static void generic_onoff_state_get_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       generic_onoff_status_params_t * p_out);
static void generic_onoff_state_set_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const generic_onoff_set_params_t * p_in,
                                       const model_transition_t * p_in_transition,
                                       generic_onoff_status_params_t * p_out);

app_onoff_server_t m_onoff_server_0 = {
        .onoff_set_cb = app_onoff_server_set_cb,
        .onoff_get_cb = app_onoff_server_get_cb,
        .value_updated = false,
        .state = {
            .present_onoff = false,
            .target_onoff = false,
            .remaining_time_ms = 0,
            .delay_ms = 0
        }
    };

const generic_onoff_server_callbacks_t onoff_srv_cbs =
{
    .onoff_cbs.set_cb = generic_onoff_state_set_cb,
    .onoff_cbs.get_cb = generic_onoff_state_get_cb
};

/* Callback for updating the hardware state */
static void app_onoff_server_set_cb(const app_onoff_server_t * p_server, bool onoff)
{
#if (RFU_USED == 0)
    /* Resolve the server instance here if required, this example uses only 1 instance. */
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting GPIO value: %d\n", onoff)
    hal_led_pin_set(ONOFF_SERVER_0_LED, onoff);
#endif
    if(g_app_db->db.present_onoff != onoff)
    {
        g_app_db->db.present_onoff = onoff;

        PTS_SEED_START(g_app_db->seed.goo);
        goo_update_plv(g_app_db->seed.goo,onoff);
        goo_update_lln(g_app_db->seed.goo,onoff);
        PTS_SEED_END(g_app_db->seed.goo);
    }
}

/* Callback for reading the hardware state */
static void app_onoff_server_get_cb(const app_onoff_server_t * p_server, bool * p_present_onoff)
{
#if (RFU_USED == 0)
    /* Resolve the server instance here if required, this example uses only 1 instance. */
    *p_present_onoff = hal_led_pin_get(ONOFF_SERVER_0_LED);
#endif
    *p_present_onoff = g_app_db->db.present_onoff;
}

static void onoff_state_process_timing(app_onoff_server_t * p_server)
{
    uint32_t status = NRF_SUCCESS;

    app_timer_stop(&p_server->p_timer_id);

    /* Process timing requirements */
    if (p_server->state.delay_ms != 0)
    {
        status = app_timer_start(&p_server->p_timer_id, APP_TIMER_TICKS(p_server->state.delay_ms), p_server);
    }
    else if (p_server->state.remaining_time_ms != 0)
    {
        /* Note: We cannot use the full length of the app_timer, since RTC counter is 24 bit, and
        application needs to report the remaining time whenever GET message is received in the
        middle of the transition. Correctness of the reported value is limited to 100 ms at the
        highest resolution as defined in section 3.1.3 of Mesh Model Specification v1.0 */
        uint32_t app_timer_ticks = APP_TIMER_TICKS(p_server->state.remaining_time_ms);
        if (app_timer_ticks > APP_TIMER_MAX_CNT_VAL)
        {
            status = app_timer_start(&p_server->p_timer_id, APP_TIMER_MAX_CNT_VAL, p_server);
        }
        else if (app_timer_ticks >= APP_TIMER_MIN_TIMEOUT_TICKS)
        {
            status = app_timer_start(&p_server->p_timer_id, APP_TIMER_TICKS(p_server->state.remaining_time_ms), p_server);
        }
        else
        {
            status = app_timer_start(&p_server->p_timer_id, APP_TIMER_MIN_TIMEOUT_TICKS, p_server);
        }
    }

    if (status != NRF_SUCCESS)
    {
       l_info("State transition timer error\n");
    }

}

static void onoff_state_value_update(app_onoff_server_t * p_server)
{
    /* Requirement: If delay and transition time is zero, current state changes to the target state. */
    if ((p_server->state.delay_ms == 0 && p_server->state.remaining_time_ms == 0) ||
    /* Requirement: If current state is 0 (checked earlier) and target state is 1, current state value changes
     * to the target state value immediately after the delay.
     */
        (p_server->state.delay_ms == 0 && p_server->state.target_onoff == 1))
    {
        p_server->state.present_onoff = p_server->state.target_onoff;

        generic_onoff_status_params_t status_params;
        status_params.present_on_off = p_server->state.present_onoff;
        status_params.target_on_off = p_server->state.target_onoff;
        status_params.remaining_time_ms = p_server->state.remaining_time_ms;
        generic_onoff_server_status_publish(&p_server->server, &status_params);

        if (!p_server->value_updated)
        {
            p_server->onoff_set_cb(p_server, p_server->state.present_onoff);
            p_server->value_updated = true;
        }
    }
    l_info("pts_app##cur onoff: %d  target: %d  delay: %d ms  remaining time: %d ms\n",
          p_server->state.present_onoff, p_server->state.target_onoff, p_server->state.delay_ms, p_server->state.remaining_time_ms);

    ;//__LOG(LOG_SRC_APP, LOG_LEVEL_DBG1, "cur onoff: %d  target: %d  delay: %d ms  remaining time: %d ms\n",
          //p_server->state.present_onoff, p_server->state.target_onoff, p_server->state.delay_ms, p_server->state.remaining_time_ms);
}

static void onoff_state_timer_cb(struct l_timeout *timeout, void * p_context)
{

    l_info("%s,time:%p,user_data:%p\n",__FUNCTION__,timeout,p_context);

    app_onoff_server_t * p_server = (app_onoff_server_t *) p_context;
    uint32_t delt_ms = 0;
    /* Requirement: Process timing. Process the delay first (Non-zero delay will delay the required
     * state transition by the specified amount) and then the transition time.
     */
    if (p_server->state.delay_ms != 0)
    {
        p_server->state.delay_ms = 0;
        onoff_state_value_update(p_server);
    }
    else if (p_server->state.remaining_time_ms != 0)
    {
#if 0
        if (APP_TIMER_TICKS(p_server->state.remaining_time_ms) > APP_TIMER_MAX_CNT_VAL)
        {
            p_server->state.remaining_time_ms -= (APP_TIMER_MAX_CNT_VAL/APP_TIMER_CLOCK_FREQ);
        }
        else
        {
            p_server->state.remaining_time_ms = 0;
            onoff_state_value_update(p_server);
        }
#else
        delt_ms = app_timer_pass(&p_server->p_timer_id);
        if(p_server->state.remaining_time_ms > delt_ms)
        {
            p_server->state.remaining_time_ms -= delt_ms;
        }
        else
        {
            p_server->state.remaining_time_ms = 0;
            onoff_state_value_update(p_server);
        }
#endif
    }
    onoff_state_process_timing(p_server);

}


/***** Generic OnOff model interface callbacks *****/

static void generic_onoff_state_get_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       generic_onoff_status_params_t * p_out)
{
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg: GET\n");
    uint32_t delta = 0;
    app_onoff_server_t   * p_server = PARENT_BY_FIELD_GET(app_onoff_server_t, server, p_self);
    l_info("%s\tmsg: GET\n", __FUNCTION__);
    /* Requirement: Provide the current value of the OnOff state */
    p_server->onoff_get_cb(p_server, &p_server->state.present_onoff);
    p_out->present_on_off = p_server->state.present_onoff;
    p_out->target_on_off = p_server->state.target_onoff;
#if (RFU_USED == 0)
    /* Requirement: Always report remaining time */
    if (p_server->state.remaining_time_ms > 0 && p_server->state.delay_ms == 0)
    {
        uint32_t delta = (1000ul * app_timer_cnt_diff_compute(app_timer_cnt_get(), p_server->last_rtc_counter)) / APP_TIMER_CLOCK_FREQ;
        if (p_server->state.remaining_time_ms >= delta && delta > 0)
        {
            p_out->remaining_time_ms = p_server->state.remaining_time_ms - delta;
        }
        else
        {
            p_out->remaining_time_ms = 0;
        }
    }
    else
    {
        p_out->remaining_time_ms = p_server->state.remaining_time_ms;
    }
#else
    /* Requirement: Always report remaining time */
    if (p_server->state.remaining_time_ms > 0 && p_server->state.delay_ms == 0)
    {
        delta = app_timer_pass(&p_server->p_timer_id);
        if (p_server->state.remaining_time_ms >= delta && delta > 0)
        {
            p_out->remaining_time_ms = p_server->state.remaining_time_ms - delta;
        }
        else
        {
            p_out->remaining_time_ms = 0;
        }
    }
#endif
}

static void generic_onoff_state_set_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const generic_onoff_set_params_t * p_in,
                                       const model_transition_t * p_in_transition,
                                       generic_onoff_status_params_t * p_out)
{
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg: SET: %d\n", p_in->on_off);
    l_info("%s\tSET: %d\n", __FUNCTION__,p_in->on_off);
    app_onoff_server_t   * p_server = PARENT_BY_FIELD_GET(app_onoff_server_t, server, p_self);

    /* Update internal representation of OnOff value, process timing */
    p_server->value_updated = false;
    p_server->state.target_onoff = p_in->on_off;
    if (p_in_transition == NULL)
    {

        if(g_app_db->db.goo.trans_time.num_steps != 0)
        {
            p_server->state.remaining_time_ms = \
                pts_transition_time_to_ms(g_app_db->db.goo.trans_time);
            p_server->state.delay_ms = PTS_DEFAULT_DELAY_MS;
        }
        else
        {
            p_server->state.delay_ms = 0;
            p_server->state.remaining_time_ms = 0;
        }
    }
    else
    {
        p_server->state.delay_ms = p_in_transition->delay_ms;
        p_server->state.remaining_time_ms = p_in_transition->transition_time_ms;
    }

    onoff_state_value_update(p_server);
    onoff_state_process_timing(p_server);

    /* Prepare response */
    if (p_out != NULL)
    {
        p_out->present_on_off = p_server->state.present_onoff;
        p_out->target_on_off = p_server->state.target_onoff;
        p_out->remaining_time_ms = p_server->state.remaining_time_ms;
    }
}


/***** Interface functions *****/

void app_onoff_status_publish(app_onoff_server_t * p_server)
{
    p_server->onoff_get_cb(p_server, &p_server->state.present_onoff);

    p_server->state.target_onoff = p_server->state.present_onoff;
    p_server->state.delay_ms = 0;
    p_server->state.remaining_time_ms = 0;
    app_timer_stop(&p_server->p_timer_id);
    generic_onoff_status_params_t status = {
                .present_on_off = p_server->state.present_onoff,
                .target_on_off = p_server->state.target_onoff,
                .remaining_time_ms = p_server->state.remaining_time_ms
            };
    generic_onoff_server_status_publish(&p_server->server, &status);
}

uint32_t pts_goo_reg(pts_app_t *pts_db, uint8_t element_index)//(pts_app_t *p_db, app_onoff_server_t * p_server, uint8_t element_index)
{
    uint32_t status = NRF_ERROR_INTERNAL;
    app_onoff_server_t * p_server = &m_onoff_server_0;
    l_info("%s\telement_index %d\n",__FUNCTION__,element_index);
    g_app_db = pts_db;
    if (p_server == NULL)
    {
        return NRF_ERROR_NULL;
    }

    p_server->server.settings.p_callbacks = &onoff_srv_cbs;
    if (p_server->onoff_set_cb == NULL || p_server->onoff_get_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }

    status = generic_onoff_server_init(&p_server->server, element_index);
    if(status != NRF_SUCCESS)
    {
        l_info("%s\tgeneric onoff server init fail %d\n",__FUNCTION__,status);
    }

    if (status == NRF_SUCCESS)
    {
        status = app_timer_create(&p_server->p_timer_id, APP_TIMER_MODE_SINGLE_SHOT,
                                  onoff_state_timer_cb);
    }
    app_timer_start(&p_server->p_timer_id, 25, p_server);
    return status;
}

void power_rst_goo(uint8_t onpower_state)
{
    app_onoff_server_t *p_onoff_server = &m_onoff_server_0;
    switch(onpower_state)
    {
        case 0:
            g_app_db->db.present_onoff = 0;
            break;

        case 1:
            g_app_db->db.present_onoff = 1;
            break;

        case 2:
            g_app_db->db.present_onoff = p_onoff_server->state.target_onoff;
            break;

        default:
            break;
    }
    p_onoff_server->state.present_onoff = g_app_db->db.present_onoff;
    l_info("publish app onoff = %d\n",g_app_db->db.present_onoff);
    app_onoff_status_publish(p_onoff_server);
}

void gdtt_update_goo(uint8_t seed,generic_transition_time_t trans_time)
{
    PTS_SEED_CHECK(g_app_db,g_app_db->seed.goo,seed);
    g_app_db->db.goo.trans_time = trans_time;
}

void lln_update_goo(uint8_t seed,uint16_t lightness)
{
    app_onoff_server_t * p_server = &m_onoff_server_0;
    bool onoff ;
    if(lightness == 0)
    {
        onoff = false;
    }
    else
    {
        onoff = true;
    }
    if(onoff != g_app_db->db.present_onoff)
    {
        g_app_db->db.present_onoff = onoff;
        l_info("%s,%d\n",__FUNCTION__,onoff);
        //p_server->onoff_set_cb(p_server,onoff);
    }
    app_onoff_status_publish(p_server);
}

void gpl_update_goo(uint8_t seed,uint16_t power)
{
    app_onoff_server_t * p_server = &m_onoff_server_0;
    bool onoff ;
    if(power == 0)
    {
        onoff = false;
    }
    else
    {
        onoff = true;
    }
    g_app_db->db.present_onoff = onoff;
    l_info("%s,%d\n",__FUNCTION__,onoff);
    //p_server->onoff_set_cb(p_server,onoff);
    app_onoff_status_publish(p_server);
}
