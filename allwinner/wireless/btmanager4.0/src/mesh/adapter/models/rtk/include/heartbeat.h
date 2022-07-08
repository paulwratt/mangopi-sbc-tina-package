/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     heartbeat.h
  * @brief    Head file for heartbeat.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2017-7-17
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _HEARTBEAT_H
#define _HEARTBEAT_H

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include "mesh_api.h"
#include "configuration.h"

/** @addtogroup HB HB
  * @brief HB related
  * @{
  */

/** @defgroup HB_Exported_Types HB Exported Types
  * @brief
  * @{
  */

typedef enum
{
    HB_TRIGGER_RELAY,
    HB_TRIGGER_PROXY,
    HB_TRIGGER_FN,
    HB_TRIGGER_LPN,
    HB_TRIGGER_ALL
} hb_trigger_type_t;

typedef enum
{
    HB_COUNT_LOG_NOT_PERIODICALLY = 0,
    HB_COUNT_LOG_PROHIBITED = 12,
    HB_COUNT_LOG_INDEFINITELY
} hb_count_log_t;

typedef struct
{
    uint16_t dst;
    uint32_t count; //!< 0x0000 not send, 0xFFFF send periodically
    uint32_t period; //!< in seconds
    uint8_t ttl; //!< 0x00 - 0x7F
    hb_pub_features_t features;
    uint16_t net_key_index; //!< global NetKey index, in case NetKey is delete later
    plt_timer_t timer;
} hb_pub_state_t, *hb_pub_state_p;

typedef struct
{
    uint16_t src;
    uint16_t dst;
    uint32_t count;
    uint32_t period;
    uint8_t min_hops;
    uint8_t max_hops;
    plt_timer_t timer;
} hb_sub_state_t, *hb_sub_state_p;

typedef struct
{
    hb_pub_state_t pub_state;
    hb_sub_state_t sub_state;
} hb_state_t, *hb_state_p;

/** @} */

extern hb_state_t hb_state;

/** @defgroup HB_Exported_Functions HB Exported Functions
  * @brief
  * @{
  */

/**
  * @brief start the heartbeat timer
  * @param[in] pub_flag: publish or subscribe
  * @return none
  */
void hb_timer_start(bool pub_flag);

/**
  * @brief stop the heartbeat timer
  * @param[in] pub_flag: publish or subscribe
  * @return none
  */
void hb_timer_stop(bool pub_flag);

/**
  * @brief detect if the message need be processed
  * @param[in] src: message source addr
  * @param[in] dst: message destination addr
  * @return operation result
  * @retval true: need process
  * @retval false: do not process
  */
bool hb_receive_filter(uint16_t src, uint16_t dst);

/**
  * @brief process the message
  * @param[in] pmesh_msg: mesh message
  * @return operation result
  * @retval true: processed
  * @retval false: not processed
  */
bool hb_handle_msg(mesh_msg_p pmesh_msg);

/**
  * @brief send hb message
  * @param[in] type: the trigger source type
  * @return none
  */
void hb_msg_send(hb_trigger_type_t type);

/**
  * @brief timeout handler
  * @param[in] pub_flag: publish or subscribe
  * @return none
  */
void hb_handle_timeout(bool pub_flag);

/** @} */

/** @} End of group HB */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif /* _HEARTBEAT_H */
