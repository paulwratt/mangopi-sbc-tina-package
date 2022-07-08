#ifndef APP_GENERIC_ON_OF_SERVER_H__
#define APP_GENERIC_ON_OF_SERVER_H__

#include "pts_app.h"
#include "generic_onoff_server.h"
#include "app_timer_mesh.h"

#if (RFU_USED == 0)
#define APP_ONOFF_SERVER_DEF(_name, _force_segmented, _mic_size, _set_cb, _get_cb)  \
    APP_TIMER_DEF(_name ## _timer); \
    static app_onoff_server_t _name =  \
    {  \
        .server.settings.force_segmented = _force_segmented,  \
        .server.settings.transmic_size = _mic_size,  \
        .p_timer_id = &_name ## _timer,  \
        .onoff_set_cb = _set_cb,  \
        .onoff_get_cb = _get_cb  \
    };
#endif

/** Internal structure to hold state and timing information. */
typedef struct
{
    /** Present value of the OnOff state */
    bool present_onoff;
    /** Target value of the OnOff state, as received from the model interface. */
    bool target_onoff;
    /** Remaining time to reach `target_onoff`. */
    uint32_t remaining_time_ms;
    /** Time to delay the processing of received SET message. */
    uint32_t delay_ms;
} app_onoff_state_t;

/* Forward declaration */
typedef struct __app_onoff_server_t app_onoff_server_t;

/** Application state set callback prototype.
 *
 * This callback is called by the this module whenever application is required to
 * be informed to reflect the desired OnOff value, as a result of the received SET message. Depending
 * on the received Target OnOff value and timing parameters, this callback may be triggered after the
 * delay+transition time is over or instantly after the delay if the Target OnOff value is `1`, as
 * required by the Mesh Model Specification v1.0.
 *
 * Note: Since the behavioral module encapsulates functionality required for the compliance with timing
 * behaviour, it is not possible to infer number of Generic OnOff Set messages received by the
 * node by counting the number of times this callback is triggered.
 *
 * @param[in]   p_server        Pointer to @ref __app_onoff_server_t [app_onoff_server_t] context
 * @param[in]   onoff           New onoff value to be used by the application
 */
typedef void (*app_onoff_set_cb_t)(const app_onoff_server_t * p_server, bool onoff);

/** Application state read callback prototype.
 * This callback is called by the app_model_behaviour.c whenever application onoff state is required
 * to be read.
 *
 * @param[in]  p_server          Pointer to @ref __app_onoff_server_t [app_onoff_server_t] context
 * @param[out] p_present_onoff   User application fills this value with the value retrived from
 *                               the hardware interface.
 */
typedef void (*app_onoff_get_cb_t)(const app_onoff_server_t * p_server, bool * p_present_onoff);
typedef void (*app_onoff_update_cb)(void *bound_context,bool onoff);

/** Application level structure holding the OnOff server model context and OnOff state representation */
struct __app_onoff_server_t
{
    /** OnOff server model interface context structure */
    generic_onoff_server_t server;

    /** APP timer instance pointer */
    app_timer_id_t  p_timer_id ;

    /** Callaback to be called for informing the user application to update the value*/
    app_onoff_set_cb_t  onoff_set_cb;
    /** Callback to be called for requesting current value from the user application */
    app_onoff_get_cb_t onoff_get_cb;
    app_onoff_update_cb onoff_update_cb;
    void *bound_context;
    /** Internal variable. Representation of the OnOff state related data and transition parameters
     *  required for behavioral implementation, and for communicating with the application */
    app_onoff_state_t state;
    /** Internal variable. It is used for acquiring RTC counter value. */
    uint32_t last_rtc_counter;
    struct timeval last_systime;
    /** Internal variable. To flag if the received message has been processed to update the present
     * OnOff value */
    bool value_updated;
};

/** Initiates value fetch from the user application by calling a get callback, updates internal state,
 * and publishes the Generic OnOff Status message.
 *
 * This API must always be called by an application when user initiated action (e.g. button press) results
 * in the local OnOff state change. Mesh Profile Specification v1.0 mandates that, every local state
 * change must be published if model publication state is configured. If model publication is not
 * configured this API call will not generate any error condition.
 *
 * @param[in] p_server              Pointer to @ref __app_onoff_server_t [app_onoff_server_t] context
 */
void app_onoff_status_publish(app_onoff_server_t * p_server);

/** Initializes the behavioral module for the generic OnOff model
 *
 * @param[in] p_server               Pointer to the application OnOff server struture array.
 * @param[in] element_index          Element index on which this server will be instantiated.
 *
 * @retval  NRF_ERROR_NULL           NULL pointer is supplied to the function or to the required
 *                                   member variable pointers.
 * @retval  NRF_ERROR_INVALID_PARAM  If value of the `server_count` is zero, or other parameters
 *                                   required by lower level APIs are not correct.
 * @returns Other return values returned by the lower layer APIs.
 *
*/

uint32_t pts_onoff_reg(pts_app_t *p_db, uint8_t element_index);

/** @} end of PTS_APP */
#endif /* PTS_APP_H__ */
