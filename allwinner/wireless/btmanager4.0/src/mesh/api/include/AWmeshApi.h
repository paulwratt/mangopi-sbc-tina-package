#ifndef __AW_MESH_API_H__
#define __AW_MESH_API_H__

#include "AWDefine.h"
#include "AWmeshDefine.h"
#include "AWmeshCfgMdl.h"
#include "AWmeshGatewayApi.h"
#include "AWmeshEvent.h"
#include "AWDbg.h"
#include "AwmeshSigMdl.h"

//API FOR MESH ARCH
int32_t aw_mesh_init(AwMeshEventCb_t cb);
int32_t aw_mesh_deinit(void);
int32_t aw_mesh_enable(AW_MESH_ROLE_T role, uint32_t feature, uint8_t *uuid);

//API FOR cfg client  model
int32_t aw_mesh_send_config_client_msg(AW_MESH_CONFIGURATION_MSG_TX_T* cc_msg);

//API FOR cfg server model
bool aw_is_opcode_of_model(const aw_access_opcode_handler_t *handler_list, UINT32 *hdl_index,AW_MESH_ACCESS_OPCODE_T opcode);

void aw_mesh_model_receive_call(uint16_t opcode, uint8_t ele_idx, uint16_t src, uint16_t dst, uint8_t *data,uint32_t len);
int32_t aw_mesh_send_sig_model_msg(AW_MESH_SIG_MDL_MSG_TX_T* mdl_msg);

// API FOR PROVISIONER
int32_t aw_mesh_prov_invite(uint8_t *uuid, int32_t size, int32_t attentionDuration);
int32_t aw_mesh_prov_cancel(int32_t reason);
/**
 *  aw_mesh_scan: mesh scan
 * @param start: TRUE:start scan; FALSE: stop scan
 * @param duration: scan timeout in millisecond
 * @return value: 0 success; else failed
 */
int32_t aw_mesh_scan(bool start, uint32_t duration);


//API FOR MESH NODE TO CONFIG LOCAL MESH NODE
int32_t aw_mesh_primary_addr_get(uint16_t *primary);
int32_t aw_mesh_add_element(uint16_t location,uint16_t* element_idx);
//int32_t aw_mesh_add_model(uint8_t ele_idx,uint32_t model_id, AwMeshAccessRxCb access_rx_cb);
int32_t aw_mesh_add_model(uint8_t ele_idx,uint32_t model_id, void *model_data);

int32_t aw_mesh_local_app_key_add(uint16_t net_idx,uint16_t app_idx,uint8_t *app_key);
int32_t aw_mesh_key_phase_set(uint16_t net_idx, uint8_t phase);

int32_t aw_mesh_send_packet(int32_t dst, int32_t dst_addr_type, uint8_t ele_idx,
			int32_t ttl, int32_t net_key_idx, int32_t app_key_idx,
			uint8_t *data, int32_t data_len);



//API FOR MESH NODE FEATURE CONTROL
int32_t aw_mesh_set_start_beacon(uint16_t dst, uint16_t net_id, uint16_t phase); //not realization
int32_t aw_mesh_send_beacon(uint16_t num, uint16_t interval);//not realization
int32_t aw_mesh_frnd_request_friend(uint8_t cache, uint8_t offer_delay, uint8_t delay, uint32_t timeout);//not realization
int32_t aw_mesh_send_mesh_msg(access_tx_msg_p pmsg);
int32_t aw_mesh_publish_mesh_msg(access_tx_publish_msg_p pmsg);
int32_t aw_mesh_publish_vendor_mesh_msg(access_tx_publish_msg_p pmsg);
int32_t aw_mesh_iv_test_mode(uint8_t test_mode);
int32_t aw_mesh_update_iv_info(uint32_t iv_index, uint8_t flags);
int32_t aw_mesh_get_iv_info();
int32_t aw_mesh_set_protocol_param(uint16_t crpl, uint16_t prov_data_interval, uint32_t feature);

int32_t aw_mesh_set_prov_data(uint16_t net_idx,uint16_t unicast);

int32_t aw_mesh_prov_set_manaual_choose(uint8_t enable);
int32_t aw_mesh_prov_set_auth_static_value(uint8_t *value);
int32_t aw_mesh_prov_set_auth_number(uint32_t number);
int32_t aw_mesh_prov_set_pub_key(uint8_t *pub_key);
int32_t aw_mesh_prov_set_start_choose_paramters(uint8_t pub_type, uint8_t auth, uint8_t auth_size, uint16_t auth_action);

int32_t aw_mesh_model_goo_send(uint16_t dst,uint16_t appkey_idx,uint16_t on_off);
//API FOR UTIL
char * getstr_hex2str(uint8_t *in, size_t in_len);
char * getstr_hex2str1(uint8_t *in, size_t in_len);
char * getstr_hex2str2(uint8_t *in, size_t in_len);

//API FOR DEBUG
int32_t aw_mesh_debug_config(AWDBG_T *cfg_list, uint8_t size,bool enable_syslog);
void mesh_log(AW_DBG_LEVEL_T level,AW_MODULE_T module,const char *fmt,...);
void mesh_test_log(const char *fmt,...);
//API FOR MESH DATABASE
void aw_mesh_show_prov_db(void);

#endif
