#ifndef __MODEL_ADAPTOR_H__
#define __MODEL_ADAPTOR_H__

#include "AWTypes.h"
#include "mesh_internal_api.h"

#include "mesh_access.h"
#include "mesh_internal_api.h"
/****others*************/
#include "mesh_config_app.h"
#include <stdio.h>
#include "mesh_node.h"

#include "src/bluez-5.54_shared/util.h"
#include "pts_client_app.h"
//ERROR CODE
#define MODEL_ADPATOR_SUCCESS 0
#define MODEL_ADPATOR_ERROR 1

typedef uint32_t (*mesh_adaptor_send_msg)(AW_MESH_SIG_MDL_MSG_TX_T *mdl_msg, mesh_model_info_p pmodel_info);

typedef struct _model_adaptor_message_t
{
    uint16_t  opcode;
    uint8_t  ele_idx;
    uint16_t src;
    uint16_t dst;
    uint8_t  *buf;
    uint32_t size;
    uint16_t appkey_index;
    uint16_t netkey_index;
    uint8_t rssi;
    uint8_t ttl;
}model_adaptor_message_t,*model_adaptor_message_p;

int32_t mesh_model_adaptor_reg(uint8_t element_index, mesh_model_info_p pmodel_info);
int32_t mesh_model_adaptor_receive_msg(model_adaptor_message_p pmdl_rx_msg);
uint32_t model_adaptor_goo_server_reg(uint8_t ele_idx,mesh_model_info_p pmodel_info);
uint32_t model_adaptor_goo_client_reg(uint8_t ele_idx,mesh_model_info_p pmodel_info);
bool generic_on_off_server_init(mesh_model_info_p pmodel_info);
bool generic_on_off_client_init(mesh_model_info_p pmodel_info);

#endif
