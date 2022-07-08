/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     mesh_access.h
  * @brief    Head file for mesh access layer.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2015-8-27
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _MESH_ACCESS_H
#define _MESH_ACCESS_H

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include "platform_misc.h"
#include "access.h"
#include "mesh_node.h"
/** @addtogroup Mesh_Access Mesh Access
  * @brief the access layer related
  * @{
  */

/** @defgroup Mesh_Access_Exported_Macros Mesh Access Exported Macros
  * @brief
  * @{
  */

/** @brief the length includes the length of the access opcode
  * @{
  */
#define ACCESS_PAYLOAD_UNSEG_MAX_SIZE           11
#define ACCESS_PAYLOAD_SEG_ONE_MAX_SIZE         8
#define ACCESS_PAYLOAD_SEG_TWO_MAX_SIZE         20
#define ACCESS_PAYLOAD_SEG_THREE_MAX_SIZE       32
#define ACCESS_PAYLOAD_MAX_SIZE                 380 //!< segmented
/** @} */

/** @defgroup Access_Opcode Access Opcode
  * @brief
  * @{
  */
/** #pragma diag_suppress 175 */
#define ACCESS_OPCODE_SIZE(opcode)              ((opcode) >= 0xc00000 ? 3 : ((opcode) >= 0x8000 ? 2 : 1))
#define ACCESS_OPCODE_BYTE(pbuffer, opcode)\
    do\
    {\
        if(ACCESS_OPCODE_SIZE(opcode) == 1)\
        {\
            *((uint8_t *)(pbuffer)) = (opcode) & 0xff;\
        }\
        else if(ACCESS_OPCODE_SIZE(opcode) == 2)\
        {\
            *((uint8_t *)(pbuffer)) = ((opcode) >> 8) & 0xff;\
            *((uint8_t *)(pbuffer) + 1) = (opcode) & 0xff;\
        }\
        else\
        {\
            *((uint8_t *)(pbuffer)) = ((opcode) >> 16) & 0xff;\
            *((uint8_t *)(pbuffer) + 1) = ((opcode) >> 8) & 0xff;\
            *((uint8_t *)(pbuffer) + 2) = (opcode) & 0xff;\
        }\
    } while(0)
/** @} End of Access_Opcode */

/** @} */

/** @defgroup Mesh_Access_Exported_Types Mesh Access Exported Types
  * @brief
  * @{
  */

/** @brief synchronized return value when send a mesh message */
typedef enum
{
    MESH_MSG_SEND_CAUSE_SUCCESS,
    MESH_MSG_SEND_CAUSE_INVALID_ACCESS_OPCODE,
    MESH_MSG_SEND_CAUSE_INVALID_ELEMENT,
    MESH_MSG_SEND_CAUSE_INVALID_MODEL,
    MESH_MSG_SEND_CAUSE_NODE_UNPROVISIONED,
    MESH_MSG_SEND_CAUSE_INVALID_SRC,
    MESH_MSG_SEND_CAUSE_INVALID_DST,
    MESH_MSG_SEND_CAUSE_PAYLOAD_SIZE_EXCEED,
    MESH_MSG_SEND_CAUSE_PAYLOAD_SIZE_EXCEED1, //!< 8
    MESH_MSG_SEND_CAUSE_INVALID_VIR_ADDR,
    MESH_MSG_SEND_CAUSE_INVALID_APP_KEY_INDEX,
    MESH_MSG_SEND_CAUSE_INVALID_APP_KEY_STATE,
    MESH_MSG_SEND_CAUSE_APP_KEY_NOT_BOUND_TO_MODEL,
    MESH_MSG_SEND_CAUSE_INVALID_NET_KEY_INDEX,
    MESH_MSG_SEND_CAUSE_INVALID_NET_KEY_STATE,
    MESH_MSG_SEND_CAUSE_NO_BUFFER_AVAILABLE,
    MESH_MSG_SEND_CAUSE_NO_MEMORY, //!< 16
    MESH_MSG_SEND_CAUSE_TRANS_TX_BUSY
} mesh_msg_send_cause_t;

/** @brief asynchronized callback value after sending a mesh message */
typedef enum _mesh_msg_send_stat_t
{
    MESH_MSG_SEND_STAT_FAIL,
    MESH_MSG_SEND_STAT_SENT, //!< unseg access msg
    MESH_MSG_SEND_STAT_ACKED, //!< seg access msg
    MESH_MSG_SEND_STAT_ACKED_CANCEL,
    MESH_MSG_SEND_STAT_ACKED_OBO, //!< seg access msg acked by fn
    MESH_MSG_SEND_STAT_ACKED_OBO_CANCEL,
    MESH_MSG_SEND_STAT_TIMEOUT //!< seg access msg
} mesh_msg_send_stat_t;

/** @brief the access layer pdu */
typedef struct
{
    /** variable length access message, composed of Operation Code and Parameters */
    uint8_t payload[ACCESS_PAYLOAD_MAX_SIZE];
} access_msg_t;

typedef struct _mesh_msg_t *mesh_msg_p;

typedef void (*opcode_handler_cb_t)(uint16_t handle,
                                           const access_message_rx_t * p_message,
                                           void * p_args);
typedef bool(*is_opcode_of_model_cb)(uint16_t opcode);
typedef struct __attribute((packed))
{
    /** Company ID. Bluetooth SIG models shall set this to @ref ACCESS_COMPANY_ID_NONE. */
    uint16_t company_id;
    /** Model ID. */
    uint16_t model_id;
}model_id_t;

typedef struct
{
    /** The Model and Company ID of this model. */
    model_id_t model_id;
    /** Element that owns this model. */
    uint16_t element_index;
} model_state_data_t;

typedef struct
{
    /** 14-bit or 7-bit Bluetooth SIG defined opcode or 6-bit vendor specific opcode. */
    uint16_t opcode;
    /** Company ID. Set to @ref ACCESS_COMPANY_ID_NONE if it is a Bluetooth SIG defined opcode. */
    uint16_t company_id;
} opcode_t;

typedef struct
{
    /** The model opcode. */
    opcode_t opcode;
    /** The opcode handler callback for the given opcode. */
    opcode_handler_cb_t handler;
} opcode_handler_t;

typedef struct
{
    /** Data pertaining to a specific model instance, which is crucial for maintaining the model's configuration in a network. */
    //model_state_data_t model_info;
    mesh_model_info_p p_model_info;
    /** Model publication state. */
    //access_model_publication_state_t publication_state;
    /** Pointer to the list of opcodes with the corresponding callback functions. */
    const opcode_handler_t * p_opcode_handlers;
    /** Number of opcodes in list @ref p_opcode_handlers. */
    uint16_t opcode_count;
    /** Generic pointer used to give context in the model callbacks. */
    void  * p_args;
    /** Used for tracking a model instance: see the defines ACCESS_INTERNAL_STATE_**. */
    uint8_t internal_state;
} access_rtk_t;


/** @} */

/** @defgroup Mesh_Access_Exported_Functions Mesh Access Exported Functions
  * @brief
  * @{
  */

/**
  * @brief cfg the mesh message use the default or the publishing parameters
  *
  * The caller may change any parameters later if she isn't satisfied with the default values.
  * @param[in] pmesh_msg: the mesh message
  * @return operation result
  */
mesh_msg_send_cause_t access_cfg(mesh_msg_p pmesh_msg);

/**
  * @brief send the mesh message to the access layer
  *
  * @param[in] pmesh_msg: the mesh message
  * @return operation result
  */
mesh_msg_send_cause_t access_send(mesh_msg_p pmesh_msg);

mesh_msg_send_cause_t access_publish(mesh_msg_p pmesh_msg);

/**
  * @brief receive the mesh message at the access layer
  *
  * @param[in] pmesh_msg: the mesh message
  * @return none
  */
void access_receive(mesh_msg_p pmesh_msg);

/**
  * @brief dispatch the mesh message at the access layer
  *
  * @param[in] pmesh_msg: the mesh message
  * @return none
  */
void access_dispatch(mesh_msg_p pmesh_msg);

void rtk_access_incoming_handle(const access_message_rx_t * p_message);

/** @} */

/** @} End of group Mesh_Access */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MESH_ACCESS_H */
