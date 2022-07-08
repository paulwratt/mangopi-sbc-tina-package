#ifndef __AW_MESH_EVENT_H__
#define __AW_MESH_EVENT_H__
#include "AWDefine.h"
#include "AWmeshDefine.h"
#include "AWmeshfoundationModel.h"
#include "AwmeshSigMdl.h"
#include "AWmeshProv.h"

typedef struct
{
    AW_MESH_BLE_ADDR_T    peer_addr;
    INT32                 rssi;
    AW_MESH_REPORT_TYPE   type;
    UINT8                 dlen;
    UINT8                 data[AW_MESH_ADV_DATA_SIZE];
} AW_MESH_EVT_ADV_REPORT_T;

// mesh stack reply report
typedef enum {
    AW_MESH_UNKNOW_REQ =0,
    AW_MESH_INIT_REQ,
    AW_MESH_DEINIT_REQ,
    AW_MESH_ENABLE_REQ,
    AW_MESH_SEND_CFG_CLINET_MSG_REQ,
    AW_MESH_PROV_INVITE_REQ,
    AW_MESH_PROV_CANCEL_REQ,
    AW_MESH_SET_PROCOTOL_PARAM_REQ,
    AW_MESH_UNPROV_SCAN_START,
    AW_MESH_UNPROV_SCAN_STOP,
    AW_MESH_NODE_CFG_REQ,
}AW_MESH_APP_REQUEST_TYPE_T;

// mesh stack reply report
typedef enum {
    AW_MESH_STACK_REPLY_SUCCESS = 0,
    AW_MESH_STACK_REPLY_FAILED
}AW_MESH_STACK_STATUS_T;

typedef struct
{
    AW_MESH_APP_REQUEST_TYPE_T type;
    //AwMeshEventCb_t event_cb;
    //struct mesh_application *app;
}AW_MESH_REQ_T;

typedef struct
{
	uint16_t company_id;
	uint16_t product_id;
	uint16_t version_id;
    uint32_t feature;
}AW_MESH_CFG_INFO_REQ_T;

typedef struct
{
    AW_MESH_STACK_STATUS_T status;
    INT32   stack_errcode;
    AW_MESH_REQ_T req;
    const char *readme;
}AW_MESH_STACK_REPLY_T;

//struct for iv update
typedef enum {
    AW_MESH_IV_UPDATE_STATE_NORMAL = 0,        /**< Indicates IV update is in normal operation. */
    AW_MESH_IV_UPDATE_STATE_IN_PROGRESS = 1,   /**< Indicates IV update is in progress. */
}AW_MESH_IV_UPDATE_STATE_T;

enum _iv_upd_phase {
	/* Allows acceptance of any iv_index secure net beacon */
	AW_IV_UPD_INIT,
	/* Normal, can transition, accept current or old */
	AW_IV_UPD_NORMAL,
	/* Updating proc running, we use old, accept old or new */
	AW_IV_UPD_UPDATING,
	/* Normal, can *not* transition, accept current or old iv_index */
	AW_IV_UPD_NORMAL_HOLD,
};

typedef struct {
    UINT8 state;
}AW_MESH_EVT_IV_TEST_MODE_T;

typedef struct {
    UINT32 iv_index;  /**< The IV index currently used for sending messages. */
    UINT8 iv_phase;
    AW_MESH_IV_UPDATE_STATE_T state; /**< Current IV update state.*/
}AW_MESH_EVT_IV_UPDATE_T;

typedef struct {
    UINT16 src;
    UINT16 dst;
    UINT16 feature;
    UINT8 ttl;
    UINT8 hops;
    INT8  rssi;
}AW_MESH_EVT_HEARTBEAT_T;

typedef enum {
    AW_MESH_FRIENDSHIP_TERMINATED = 0,              /**< The friendship is terminated. */
    AW_MESH_FRIENDSHIP_ESTABLISHED = 1,             /**< The friendship is successfully established. */
    AW_MESH_FRIENDSHIP_ESTABLISH_FAILED = 2,        /**< The friendship is not established. */
    AW_MESH_FRIENDSHIP_REQUEST_FRIEND_TIMEOUT = 3,  /**< Request friend procedure timeout. The status is only received when low power feature in use. */
    AW_MESH_FRIENDSHIP_SELECT_FRIEND_TIMEOUT = 4,   /**< Select friend procedure timeout. The status is only received when low power feature in use. */
} AG_MESH_FRIDSHIP_STATUS_T;

typedef enum {
    AW_MESH_FRND_NO_ERROR = 0,
    AW_MESH_FRND_DISABLE,
    AW_MESH_FRND_POLLTIMEOUT,
    AW_MESH_FRND_CLEAR,
    AW_MESH_FRND_NET_FREE
}AW_MESH_FRIDSHIP_REASON_T;

typedef struct {
	UINT8 number_of_elements;	/**< Number of elements supported by the device */
	UINT16 algorithms;			/**< Supported algorithms and other capabilities */
	UINT8 public_key_type;		/**< Supported public key types */
	UINT8 static_oob_type;		/**< Supported static OOB Types */
	UINT8 output_oob_size;		/**< Maximum size of Output OOB supported */
	UINT16 output_oob_action;	/**< Supported Output OOB Actions */
	UINT8 input_oob_size;		/**< Maximum size in octets of Input OOB supported */
	UINT16 input_oob_action;	/**< Supported Input OOB Actions */
} AW_MESH_PROV_CAPABILITIES_T;

typedef struct {
	AW_MESH_PROV_CAPABILITIES_T cap;	 /**< The capabilities detail value. */
} AW_MESH_EVT_PROV_CAPABILITIES_T;

typedef struct {
	UINT8 method;	/**< Authentication Method used */
	UINT8 action;	/**< Selected Output OOB Action or Input OOB Action or 0x00 */
	UINT8 size;	/**< Size of the Output OOB used or size of the Input OOB used or 0x00 */
} AW_MESH_EVT_PROV_REQUEST_AUTH_T;

typedef struct {
    BOOL prov_set;
    UINT16 net_idx;
    UINT16 unicast;
    UINT16 num_ele;
}AW_MESH_EVT_PROV_DATA_REQUEST_T;

typedef struct {
	UINT8 *pk;	  /**< The public key received. */
} AW_MESH_EVT_PROV_SHOW_PK_T;

typedef struct {
	UINT8 auth[AW_MESH_AUTHENTICATION_SIZE];  /**< The authentication value received. */
} AW_MESH_EVT_PROV_SHOW_AUTH_T;

typedef enum {
    AW_MESH_PROV_FACTOR_CONFIRMATION_KEY,
    AW_MESH_PROV_FACTOR_RANDOM_PROVISIONER,
    AW_MESH_PROV_FACTOR_RANDOM_DEVICE,
    AW_MESH_PROV_FACTOR_CONFIRMATION_PROVISIONER,
    AW_MESH_PROV_FACTOR_CONFIRMATION_DEVICE,
    AW_MESH_PROV_FACTOR_PUB_KEY,
    AW_MESH_PROV_FACTOR_AUTHEN_VALUE,
    AW_MESH_PROV_FACTOR_AUTHEN_RESULT,
} AW_MESH_PROV_FACTOR_TYPE_T;

typedef struct {
	uint32_t auth_num;
} AW_MESH_EVT_PROV_SHOW_AUTH_NUM_T;

typedef struct {
    AW_MESH_PROV_FACTOR_TYPE_T type;
    UINT8 *buf;
    UINT16 buf_len;
} AW_MESH_PROV_FACTOR_T;

typedef struct {
    UINT16 lpn_addr;
    UINT8 status;
    UINT8 reason;
}AW_MESH_EVT_FRIENDSHIP_STATUS_T;

//struct for architecture
typedef union {
    AW_MESH_ACCESS_MESSAGE_RX_T     access_rx_msg;
    AW_MESH_SIG_MDL_MSG_RX_T        model_rx_msg;
    AW_MESH_CONFIG_MODEL_STATUS_T   config_mdl_status;
    AW_MESH_HLTH_MODEL_STATUS_T     health_mdl_status;
    AW_MESH_EVT_IV_TEST_MODE_T      iv_test_mode;
    AW_MESH_EVT_IV_UPDATE_T         iv_update;
	AW_MESH_EVT_FRIENDSHIP_STATUS_T	friendship_status;
    //struct for prov
    AW_MESH_EVT_PROV_CAPABILITIES_T	 prov_cap;			 /**<  parameter of mesh event @ref AW_MESH_EVT_PROV_CAPABILITIES */
	AW_MESH_EVT_PROV_REQUEST_AUTH_T		 prov_request_auth;  /**<  parameter of mesh event @ref AW_MESH_EVT_PROV_REQUEST_OOB_AUTH_VALUE */
    AW_MESH_EVT_PROV_DATA_REQUEST_T      prov_request_data;
	AW_MESH_EVT_PROV_SHOW_PK_T			 prov_show_pk;		 /**<  parameter of mesh event @ref AW_MESH_EVT_PROV_SHOW_OOB_PUBLIC_KEY */
	AW_MESH_EVT_PROV_SHOW_AUTH_T		 prov_show_auth;	 /**<  parameter of mesh event @ref AW_MESH_EVT_PROV_SHOW_OOB_AUTH_VALUE */
	AW_MESH_EVT_PROV_SHOW_AUTH_NUM_T	 prov_show_auth_num;
	AW_MESH_PROV_FACTOR_T				 prov_factor;
	AW_MESH_PROV_SCAN_UD_T          prov_scan_ud;
    AW_MESH_PROV_DONE_T             prov_done;
    AW_MESH_EVT_ADV_REPORT_T        adv_report;
    AW_MESH_EVT_HEARTBEAT_T         heartbeat;
    //struct for mesh configure info from customer made
    AW_MESH_CFG_INFO_REQ_T          config_info;
    AW_MESH_STACK_REPLY_T           stack_reply;
}AW_MESH_EVENT_PARAMS_T;

typedef struct {
    UINT32  evt_code;
    AW_MESH_EVENT_PARAMS_T param;
}AW_MESH_EVENT_T;

typedef enum {
    AW_MESH_EVENT_STACK_INIT_DONE = 0, // init mesh stack node.
    AW_MESH_EVENT_LOCAL_NODE_ATTACH_NODE,// load local node in stack done.
    AW_MESH_EVENT_ACCESS_MSG,
    AW_MESH_EVENT_CFG_MDL_MSG,
    AW_MESH_EVENT_HLTH_MDL_MSG,
    AW_MESH_EVENT_UD,
    //mesh procedure
    AW_MESH_EVENT_IV_TEST_MODE,
    AW_MESH_EVENT_IV_UPDATE, //Done
    AW_MESH_EVENT_KEY_REFRESH,
    AW_MESH_EVENT_HEARTBEAT,
    //mesh prov
    AW_MESH_EVENT_PROV_SCAN_UD_RESULT,
    AW_MESH_EVENT_PROV_CAPABILITIES,
    AW_MESH_EVENT_PROV_REQUEST_OOB_PUBLIC_KEY,
    AW_MESH_EVENT_PROV_REQUEST_OOB_AUTH_VALUE,
    AW_MESH_EVENT_PROV_REQUEST_OOB_AUTH_NUM,
    AW_MESH_EVENT_PROV_REQUEST_DATA,
    AW_MESH_EVENT_PROV_SHOW_OOB_AUTH_VALUE,
    AW_MESH_EVENT_PROV_SHOW_OOB_AUTH_VALUE_NUM,
    AW_MESH_EVENT_PROV_DONE,
    AW_MESH_EVENT_PROV_FAILED,
    //mesh friend
    AW_MESH_EVENT_FRIEND_STATUS,
    //mesh stack reply result
    AW_MESH_STACK_REPLY_STATUS, //Done
    AW_MESH_EVENT_LPN_FRIEND_OFFER,
    AW_MESH_EVENT_LPN_FRIEND_SUBSCRIPTION_LIST_CONFRIM,
    //mesh status
    AW_MESH_SEQ_CHANGE,
    //ble evt
    AW_MESH_ADV_REPORT,
    //mesh iv update event Done
    //AW_MESH_EVENT_IV_UPDATE,
    //mesh access rx cb
    AW_MESH_EVENT_ACCESS_RX_CB,
    AW_MESH_EVENT_MODEL_RX_CB,
    //mesh config req
    AW_MESH_CONFIG_INFO_REQ,
    AW_MESH_EVENT_MAX,
}AW_MESH_EVT_CODE_T;

//AW_MESH_EVENT_ID  AW_MESH_EVENT_T
//mible_mesh_iv_t

//mesh event cb func_t
typedef VOID (*AwMeshEventCb_t)(AW_MESH_EVENT_T *event , void *user_data);
typedef VOID (*AwMeshAccessRxCb)();

#endif
