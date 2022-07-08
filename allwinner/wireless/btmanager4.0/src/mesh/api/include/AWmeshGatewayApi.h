#ifndef __AW_MESH_GATEWAY_API_H__
#define __AW_MESH_GATEWAY_API_H__
#include "AWmeshGateway.h"

/**
 *@brief    enable mesh stack from mesh gatewap .
 *         Report Mesh Stack Init Done Status Evt.
 *         Report event:  AW_MESH_EVENT_STACK_INIT_DONE
 *@param    [in] info: mesh gate info to import into mesh stack.
 *@param    [in] uuid: mesh device uuid
 *@return   0: success, negetive value: failure
 */
 //API FOR GATEWAY
int32_t aw_mesh_gateway_enable(AW_MESH_GATEWAY_INFO_T * info, uint8_t *uuid);
int32_t aw_mesh_gateway_cfg_node(AW_MESH_GATEWAY_OP_T ops,AW_MESH_GATEWAY_NODE_INFO_T *device);
#endif
