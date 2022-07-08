#ifndef __AW_MESH_SIG_MDL_H__
#define __AW_MESH_SIG_MDL_H__

#define APP_DEFAULT_ELEMENT_0   0

#define APP_NONE_ERROR_CODE 0
#define APP_REG_ALREADY_ERROR_CODE 1

typedef enum
{
    AW_MESH_MODEL_SEND = 0,
    AW_MESH_MODEL_PUB
}AW_MESH_MODEL_TX_TYPE_T;

// structs for trans time states
typedef struct
{
    uint8_t num_steps;
    uint8_t step_resolution;
} AW_MESH_MDL_TRANS_TIME_T;

// structs for generic on off models states
typedef enum
{
    AW_MESH_GENERIC_OFF = 0,
    AW_MESH_GENERIC_ON
} AW_MESH_GOO_T;

//structs for model status
typedef struct
{
    BOOL b_target_present;
    AW_MESH_GOO_T present_onoff;
    AW_MESH_GOO_T target_onoff;
    AW_MESH_MDL_TRANS_TIME_T remaining_time;
}AW_MESH_GOO_STATUS_T;

typedef struct
{
    AW_MESH_GOO_T present_onoff;
}AW_MESH_GOO_GET_T;

typedef struct
{
    AW_MESH_MDL_TRANS_TIME_T trans_time;
}AW_MESH_GTT_GET_T;

typedef struct
{
    BOOL b_trans_present;
    BOOL b_ack;
    AW_MESH_GOO_T target_onoff;
    UINT8 tid;
    AW_MESH_MDL_TRANS_TIME_T remaining_time;
    AW_MESH_MDL_TRANS_TIME_T total_time;
    UINT8 delay;
}AW_MESH_GOO_SET_T;

//structures for sig models messages
typedef struct {
    AW_MESH_MODEL_TX_TYPE_T tx_type;
    UINT16      src_addr;
    UINT16      dst_addr;
    UINT8       ttl;
    UINT8       rssi;
    UINT32      model_id;
    UINT8       src_element_index;
    UINT16      msg_netkey_index;
    UINT16      msg_appkey_index;
} AW_MESH_SIG_MDL_META_T;

typedef struct {
    UINT16                            opcode;
    AW_MESH_SIG_MDL_META_T             meta;
    union {
        AW_MESH_GOO_GET_T             goo_get;
        AW_MESH_GOO_SET_T             goo_set;
        AW_MESH_GOO_STATUS_T          goo_status;
        AW_MESH_GTT_GET_T             gdtt_status;
    } data;
} AW_MESH_SIG_MDL_MSG_TX_T,*AW_MESH_SIG_MSG_TX_P,AW_MESH_SIG_MDL_MSG_RX_T;

typedef struct {
    UINT32  model_id;
    UINT8   ele_idx;
    void *app_private;
}AW_MESH_SIG_MDL_REG_T,*AW_MESH_SIG_MDL_REG_P;



int32_t aw_mesh_send_sig_model_msg(AW_MESH_SIG_MDL_MSG_TX_T* mdl_msg);
int32_t aw_mesh_register_sig_model(AW_MESH_SIG_MDL_REG_T *mdl_reg);

typedef void (*AwmeshMdlMsgRxCb_t)(void *msg);
typedef void (*AwmeshMdlRxCb_t)(void *msg);

typedef struct
{
    AwmeshMdlMsgRxCb_t msg_rx;
    AwmeshMdlRxCb_t    modle_rx;
    void *private;
}AW_MESH_MODEL_MSG_CB_T;

#endif
