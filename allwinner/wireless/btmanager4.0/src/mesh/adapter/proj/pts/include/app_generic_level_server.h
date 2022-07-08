#ifndef APP_GENERIC_LEVEL_SERVER_H__
#define APP_GENERIC_LEVEL_SERVER_H__

#include "pts_app.h"
//#include <stdint.h>
#include "generic_level_server.h"

#if (RFU_USED == 1)
typedef uint32_t fsm_t;
#endif

#define APP_LEVEL_SERVER_DEF(_name, _force_segmented, _mic_size, _p_dtt, _set_cb, _get_cb)  \
    APP_TIMER_DEF(_name ## _timer); \
    static app_level_server_t _name =  \
    {  \
        .server.settings.force_segmented = _force_segmented,  \
        .server.settings.transmic_size = _mic_size,  \
        .timer.p_timer_id = &_name ## _timer,  \
        .p_dtt_ms = _p_dtt, \
        .level_set_cb = _set_cb,  \
        .level_get_cb = _get_cb  \
    };

/** Transition types */
typedef enum
{
    /** indicating SET message */
    TRANSITION_SET,
    /** indicating DELTA SET message */
    TRANSITION_DELTA_SET,
    /** indicating MOVE SET message */
    TRANSITION_MOVE_SET,
    /** indicating no transition */
    TRANSITION_NONE
} app_level_transition_type_t;

/** Internal structure for holding Set/Delta Set transition related variables */
typedef struct
{
    /** For storing actual required amount of level change. */
    int32_t required_delta;
    /** Initial present level required for handling Set/Delta Set message. */
    int16_t initial_present_level;
} set_transition_t;

/** Internal structure for holding Move transition related variables */
typedef struct
{
    /** Scaled representation of the Level value. */
    int16_t required_move;
    /** Initial present level required for handling Set/Delta Set message. */
    int16_t initial_present_level;
} move_transition_t;

/** Internal structure to hold state and timing information. */
typedef struct
{
    /** Present value of the Level state */
    int16_t present_level;
    /** Target value of the Level state, as received from the model interface. */
    int16_t target_level;
    /** Remaining time to reach `target_level`. */
    uint32_t transition_time_ms;
    /** Time to delay the processing of received SET message. */
    uint32_t delay_ms;
    uint32_t total_time_pass_ms;
    /** Transition Type */
    app_level_transition_type_t transition_type;
    union {
        /* Parameters for Set Transition */
        set_transition_t set;
        /* Parameters for Move Transition */
        move_transition_t move;
    } params;
} app_level_state_t;

/* Forward declaration */
typedef struct __app_level_server_t app_level_server_t;

/** Application state set callback prototype.
 *
 * This callback is called by the this module whenever application is required to
 * be informed to reflect the desired Level value, as a result of the received SET/DELTA SET/MOVE SET
 * message, depending on the received Target Level value and timing parameters.
 *
 * Note: Since the behavioral module encapsulates functionality required for the compliance with timing
 * behaviour, it is not possible to infer number of Level messages received by the
 * node by counting the number of times this callback is triggered. If such counting is required,
 * it should be done in the `app_level.c` module.
 *
 * @param[in]   p_server        Pointer to @ref __app_level_server_t [app_level_server_t] context
 * @param[in]   present_level   Instantaneous new level value to be used by the application
 */
typedef void (*app_level_set_cb_t)(const app_level_server_t * p_server, int16_t present_level);

/** Application state read callback prototype.
 * This callback is called by the app_level.c whenever application level state is required
 * to be read.
 *
 * @param[in]  p_server          Pointer to @ref __app_level_server_t [app_level_server_t] context
 * @param[out] p_present_level   User application fills this value with the value retrived from
 *                               the hardware interface.
 */
typedef void (*app_level_get_cb_t)(const app_level_server_t * p_server, int16_t * p_present_level);
typedef void (*app_level_update_cb)(void *bound_context,int16_t present_level);

/** Application level structure holding the Level server model context and Level state representation */
struct __app_level_server_t
{
    /** Level server model interface context structure */
    generic_level_server_t server;
    /** Timer instance pointer */
    app_timer_id_t  p_timer_id ;
    /** Callaback to be called for informing the user application to update the value*/
    app_level_set_cb_t  level_set_cb;
    /** Callback to be called for requesting current value from the user application */
    app_level_get_cb_t level_get_cb;
    /**Callback to be called for requesting  level value update from the user application */
    app_level_update_cb level_update_cb;
    void *bound_context;
    /** Pointer to the default transition time value (in milliseconds) if present */
    const uint32_t * p_dtt_ms;
    /** Internal variable. Representation of the Level state related data and transition parameters
     *  required for behavioral implementation, and for communicating with the application */
    app_level_state_t state;

    bool value_updated;

    /** Internal. */
    fsm_t fsm;
};

/** Initiates value fetch from the user application by calling a get callback, updates internal state,
 * and publishes the Generic Level Status message.
 *
 * This API must always be called by an application when user initiated action (e.g. button press) results
 * in the local Level state change. Mesh Profile Specification v1.0 mandates that, every local state
 * change must be published if model publication state is configured. If model publication is not
 * configured this API call will not generate any assertion.
 *
 * @param[in] p_server              Pointer to @ref __app_level_server_t [app_level_server_t] context
 *
 * @retval NRF_SUCCESS              If status message is succesfully published.
 * @returns Other return values returned by the lower layer APIs.
 */
uint32_t app_level_current_value_publish(app_level_server_t * p_server);

/** Initializes the behavioral module for the Generic Level model
 *
 * @param[in] p_server               Pointer to the application Level server struture array.
 * @param[in] element_index          Element index on which this server will be instantiated.
 *
 * @retval  NRF_ERROR_NULL           NULL pointer is supplied to the function or to the required
 *                                   member variable pointers.
 * @retval  NRF_ERROR_INVALID_PARAM  If value of the `server_count` is zero, or other parameters
 *                                   required by lower level APIs are not correct.
 * @returns Other return values returned by the lower layer APIs.
 *
*/
uint32_t app_level_init(app_level_server_t * p_server, uint8_t element_index);
uint32_t pts_level_reg(pts_app_t *p_db, uint8_t element_index);
#endif //endof APP_GENERIC_LEVEL_SERVER_H__
