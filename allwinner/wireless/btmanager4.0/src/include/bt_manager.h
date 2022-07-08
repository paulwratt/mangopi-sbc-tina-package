#ifndef __BT_MANAGER_H
#define __BT_MANAGER_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#if __cplusplus
extern "C" {
#endif

#define BTMGVERSION "Version:4.0.2.202201141520"

/*Debug Control*/
typedef enum {
    BTMG_LOG_LEVEL_NONE = 0,
    BTMG_LOG_LEVEL_ERROR,
    BTMG_LOG_LEVEL_WARNG,
    BTMG_LOG_LEVEL_INFO,
    BTMG_LOG_LEVEL_DEBUG,
} btmg_log_level_t;

int bt_manager_set_loglevel(btmg_log_level_t log_level);
btmg_log_level_t bt_manager_get_loglevel(void);
int bt_manager_set_ex_debug_mask(int mask);
int bt_manager_get_ex_debug_mask(void);
const char *bt_manager_get_error_info(int error);

#define BTMG_A2DP_SINK_ENABLE (1 << 0)
#define BTMG_A2DP_SOUCE_ENABLE (1 << 1)
#define BTMG_AVRCP_ENABLE (1 << 2)
#define BTMG_HFP_HF_ENABLE (1 << 3)
#define BTMG_GATT_SERVER_ENABLE (1 << 4)
#define BTMG_GATT_CLIENT_ENABLE (1 << 5)
#define BTMG_SPP_ENABLE (1 << 6)

typedef struct {
    bool is_bt_enable_default;
    bool is_a2dp_sink_enabled;
    bool is_avrcp_enabled;
    bool is_a2dp_source_enabled;
    bool is_hfp_hf_enabled;
    bool is_gatts_enabled;
    bool is_gattc_enabled;
    bool is_sppc_enabled;
    bool is_spps_enabled;
} btmg_profile_info_t;
typedef enum {
    BTMG_SCAN_AUTO,
    BTMG_SCAN_BR_EDR,
    BTMG_SCAN_LE,
} btmg_scan_type_t;
typedef struct {
    const char **uuid_list;
    uint32_t uuid_num;
    int16_t rssi;
    btmg_scan_type_t type;
} btmg_scan_filter_t;
typedef enum {
    BTMG_SCAN_STARTED,
    BTMG_SCAN_START_FAILED,
    BTMG_SCAN_STOPPED,
} btmg_scan_state_t;
typedef enum {
    BTMG_SCAN_MODE_NONE,
    BTMG_SCAN_MODE_CONNECTABLE,
    BTMG_SCAN_MODE_CONNECTABLE_DISCOVERABLE,
} btmg_scan_mode_t;
typedef enum {
    BTMG_IO_CAP_KEYBOARDDISPLAY = 0,
    BTMG_IO_CAP_DISPLAYONLY,
    BTMG_IO_CAP_DISPLAYYESNO,
    BTMG_IO_CAP_KEYBOARDONLY,
    BTMG_IO_CAP_NOINPUTNOOUTPUT,
} btmg_io_capability_t;
typedef enum {
    BTMG_BOND_STATE_UNBONDED,
    BTMG_BOND_STATE_BONDING,
    BTMG_BOND_STATE_BONDED,
    BTMG_BOND_STATE_BOND_FAILED,
} btmg_bond_state_t;
typedef struct {
    char *uuid;
    char *uuid_name;
} bt_dev_uuid_t;
typedef enum {
	BT_PAIR_REQUEST_REJECTED,
	BT_PAIR_REQUEST_CANCELED
} btmg_pair_request_error_t;
typedef struct {
    char *remote_address;
    char *remote_name;
    char *icon;
    bool paired;
    bool trusted;
    bool blocked;
    bool connected;
    int16_t rssi;
    uint32_t r_class;
} btmg_bt_device_t __attribute((aligned(sizeof(long))));

typedef enum {
    BTMG_ADAPTER_OFF,
    BTMG_ADAPTER_TURNING_ON,
    BTMG_ADAPTER_ON,
    BTMG_ADAPTER_TURNING_OFF,
} btmg_adapter_state_t;
typedef void (*bt_adapter_state_cb)(btmg_adapter_state_t state);
typedef struct btmg_adapter_callback_t {
    bt_adapter_state_cb adapter_state_cb;
} btmg_adapter_callback_t;

int bt_manager_set_scan_mode(btmg_scan_mode_t mode);
int bt_manager_enable_profile(int profile);
int bt_manager_enable(bool enable);
int bt_manager_set_default_profile(bool is_default);
bool bt_manager_is_enabled(void);
void bt_manager_hex_dump(char *pref, int width, unsigned char *buf, int len);

int bt_manager_scan_filter(btmg_scan_filter_t *filter);
int bt_manager_start_scan(void);
int bt_manager_stop_scan(void);
bool bt_manager_is_discovering();
int bt_manager_pair(char *addr);
int bt_manager_unpair(char *addr);
int bt_manager_get_paired_devices(btmg_bt_device_t **dev_list, int *count);
int bt_manager_free_paired_devices(btmg_bt_device_t *dev_list, int count);
int bt_manager_remove_device(const char *addr);
btmg_adapter_state_t bt_manager_get_adapter_state();
int bt_manager_get_adapter_name(char *name);
int bt_manager_set_adapter_name(const char *name);
int bt_manager_get_adapter_address(char *addr);
int bt_manager_get_device_name(const char *addr, char *name);
int bt_manager_connect(const char *addr);
int bt_manager_disconnect(const char *addr);
bool bt_manager_device_is_connected(const char *addr);
int bt_manager_get_connection_state(void);
/* 1 slot = 0.625ms, so timeout = slots * 0.625ms */
int bt_manager_set_page_timeout(int slots);
/* 1 slot = 0.625ms, so timeout = slots * 0.625ms */
int bt_manager_set_link_supervision_timeout(const char *addr, int slots);

/*BLUEZ_AGENT*/
typedef void (*bt_agent_request_pincode)(void *handle, char *device);
typedef void (*bt_agent_display_pincode)(char *device, char *pincode);
typedef void (*bt_agent_request_passkey)(void *handle, char *device);
typedef void (*bt_agent_display_passkey)(char *device, unsigned int passkey, unsigned int entered);
typedef void (*bt_agent_request_confirm_passkey)(void *handle, char *device, unsigned int passkey);
typedef void (*bt_agent_request_authorize)(void *handle, char *device);
typedef void (*bt_agent_authorize_service)(void *handle, char *device, char *uuid);
typedef struct {
    bt_agent_request_pincode agent_request_pincode;
    bt_agent_display_pincode agent_display_pincode;
    bt_agent_request_passkey agent_request_passkey;
    bt_agent_display_passkey agent_display_passkey;
    bt_agent_request_confirm_passkey agent_request_confirm_passkey;
    bt_agent_request_authorize agent_request_authorize;
    bt_agent_authorize_service agent_authorize_service;
} btmg_agent_callback_t;
int bt_manager_agent_set_io_capability(btmg_io_capability_t io_cap);
int bt_manager_agent_send_pincode(void *handle, char *pincode);
int bt_manager_agent_send_passkey(void *handle, unsigned int passkey);
int bt_manager_agent_send_pair_error(void *handle, btmg_pair_request_error_t e,
                                     const char *err_msg);
int bt_manager_agent_pair_send_empty_response(void *handle);

/*AVRCP*/
typedef enum {
    BTMG_AVRCP_PLAYSTATE_STOPPED,
    BTMG_AVRCP_PLAYSTATE_PLAYING,
    BTMG_AVRCP_PLAYSTATE_PAUSED,
    BTMG_AVRCP_PLAYSTATE_FWD_SEEK,
    BTMG_AVRCP_PLAYSTATE_REV_SEEK,
    BTMG_AVRCP_PLAYSTATE_FORWARD,
    BTMG_AVRCP_PLAYSTATE_BACKWARD,
    BTMG_AVRCP_PLAYSTATE_ERROR,
} btmg_avrcp_play_state_t;
typedef enum {
    BTMG_AVRCP_PLAY = 0,
    BTMG_AVRCP_PAUSE,
    BTMG_AVRCP_STOP,
    BTMG_AVRCP_FASTFORWARD,
    BTMG_AVRCP_REWIND,
    BTMG_AVRCP_FORWARD,
    BTMG_AVRCP_BACKWARD,
    BTMG_AVRCP_VOL_UP,
    BTMG_AVRCP_VOL_DOWN,
} btmg_avrcp_command_t;
typedef struct btmg_track_info_t {
    char title[512];
    char artist[256];
    char album[256];
    char track_num[64];
    char num_tracks[64];
    char genre[256];
    char duration[256];
} btmg_track_info_t;
typedef void (*bt_avrcp_play_state_cb)(
        const char *bd_addr,
        btmg_avrcp_play_state_t state); /*used to report play state of avrcp, recommended*/
typedef void (*bt_avrcp_track_changed_cb)(
        const char *bd_addr, btmg_track_info_t track_info); /*used to report track information*/
typedef void (*bt_avrcp_play_position_cb)(
        const char *bd_addr, int song_len,
        int song_pos); /*used to report the progress of playing music*/
typedef void (*bt_avrcp_audio_volume_cb)(const char *bd_addr, unsigned int volume);
typedef struct btmg_avrcp_callback_t {
    bt_avrcp_play_state_cb avrcp_play_state_cb;
    bt_avrcp_track_changed_cb avrcp_track_changed_cb;
    bt_avrcp_play_position_cb avrcp_play_position_cb;
    bt_avrcp_audio_volume_cb avrcp_audio_volume_cb; /*used to report the avrcp volume, range: 0~16*/
} btmg_avrcp_callback_t;
int bt_manager_avrcp_command(char *addr, btmg_avrcp_command_t command);
int bt_manager_a2dp_set_vol(int vol_value);
int bt_manager_a2dp_get_vol(void);
btmg_avrcp_play_state_t bt_manager_get_avrcp_status(void);
void bt_manager_set_avrcp_status(btmg_avrcp_play_state_t status);

/*A2DP SOURCE*/
typedef enum {
    BTMG_A2DP_SINK_DISCONNECTED,
    BTMG_A2DP_SINK_CONNECTING,
    BTMG_A2DP_SINK_CONNECTED,
    BTMG_A2DP_SINK_DISCONNECTING,
    BTMG_A2DP_SINK_CONNECT_FAILED,
    BTMG_A2DP_SINK_DISCONNEC_FAILED,
} btmg_a2dp_sink_connection_state_t;
typedef enum {
    BTMG_A2DP_SINK_AUDIO_SUSPENDED,
    BTMG_A2DP_SINK_AUDIO_STOPPED,
    BTMG_A2DP_SINK_AUDIO_STARTED,
} btmg_a2dp_sink_audio_state_t;
typedef void (*bt_a2dp_sink_connection_state_cb)(const char *bd_addr,
                                                 btmg_a2dp_sink_connection_state_t state);
typedef void (*bt_a2dp_sink_audio_state_cb)(const char *bd_addr,
                                            btmg_a2dp_sink_audio_state_t state);
typedef void (*bt_a2dp_sink_stream_cb)(const char *bd_addr, uint16_t channels, uint16_t sampling,
                                       uint8_t *data, uint32_t len);

typedef struct btmg_a2dp_sink_callback_t {
    bt_a2dp_sink_connection_state_cb
            a2dp_sink_connection_state_cb; /*used to report the a2dp_sink connection state*/
    bt_a2dp_sink_audio_state_cb
            a2dp_sink_audio_state_cb; /*used to report the a2dp_sink audio state, not recommended as mentioned before*/
    bt_a2dp_sink_stream_cb a2dp_sink_stream_cb;
} btmg_a2dp_sink_callback_t;

/*A2DP SOURCE*/
typedef enum {
    BTMG_A2DP_SOURCE_DISCONNECTED,
    BTMG_A2DP_SOURCE_CONNECTING,
    BTMG_A2DP_SOURCE_CONNECTED,
    BTMG_A2DP_SOURCE_DISCONNECTING,
    BTMG_A2DP_SOURCE_CONNECT_FAILED,
    BTMG_A2DP_SOURCE_DISCONNEC_FAILED,
} btmg_a2dp_source_connection_state_t;

typedef void (*bt_a2dp_source_connection_state_cb)(const char *bd_addr,
                               btmg_a2dp_source_connection_state_t state);
typedef struct btmg_a2dp_source_callback_t {
    bt_a2dp_source_connection_state_cb
            a2dp_source_connection_state_cb; /*used to report the a2dp_source connection state*/
} btmg_a2dp_source_callback_t;
int bt_manager_a2dp_src_init(uint16_t channels, uint16_t sampling);
int bt_manager_a2dp_src_deinit(void);
int bt_manager_a2dp_src_stream_start(uint32_t len);
int bt_manager_a2dp_src_stream_stop(bool drop);
int bt_manager_a2dp_src_stream_send(char *data, int len);
bool bt_manager_a2dp_src_is_stream_start(void);
/*HFP*/
typedef enum {
    BTMG_HFP_CONNECT,
    BTMG_HFP_CONNECT_LOST,
    BTMG_HFP_CIND = 11, /* Indicator string from AG */
    BTMG_HFP_CIEV, /* Indicator status from AG */
    BTMG_HFP_RING, /* RING alert from AG */
    BTMG_HFP_CLIP, /* Calling subscriber information from AG */
    BTMG_HFP_BSIR, /* In band ring tone setting */
    BTMG_HFP_BVRA, /* Voice recognition activation/deactivation */
    BTMG_HFP_CCWA, /* Call waiting notification */
    BTMG_HFP_CHLD, /* Call hold and multi party service in AG */
    BTMG_HFP_VGM, /* MIC volume setting */
    BTMG_HFP_VGS, /* Speaker volume setting */
    BTMG_HFP_BINP, /* Input data response from AG */
    BTMG_HFP_BTRH, /* CCAP incoming call hold */
    BTMG_HFP_CNUM, /* Subscriber number */
    BTMG_HFP_COPS, /* Operator selection info from AG */
    BTMG_HFP_CMEE, /* Enhanced error result from AG */
    BTMG_HFP_CLCC, /* Current active call list info */
    BTMG_HFP_UNAT, /* AT command response fro AG which is not specified in HFP or HSP */
    BTMG_HFP_OK, /* OK response */
    BTMG_HFP_ERROR, /* ERROR response */
    BTMG_HFP_BCS /* Codec selection from AG */
} btmg_hfp_even_t;
typedef enum {
    BTMG_HFP_VOLUE_TYPE_SPK = 0,
    BTMG_HFP_VOLUE_TYPE_MIC = 1,
} btmg_hfp_volume_type_t;
typedef struct {
    uint16_t handle;
    uint16_t status;
} btmg_hfp_hdr_t;
typedef struct {
    uint16_t handle;
    uint8_t addr[6];
    uint16_t status;
    uint32_t reserved;
    uint8_t initiator; /* 1 local, 0 peer */
} btmg_hfp_open_t;
typedef struct {
    uint16_t handle;
    uint8_t addr[6];
    uint16_t status;
    uint32_t reserved;
    uint16_t features;
} btmg_hfp_conn_t;
typedef struct {
    uint16_t handle;
    char val[255 + 1];
    uint16_t num;
} btmg_hfp_atrsp_t;
typedef union {
    btmg_hfp_hdr_t hdr;
    btmg_hfp_open_t open;
    btmg_hfp_conn_t conn;
    btmg_hfp_atrsp_t rsp;
} btmg_hfp_data_t;
typedef void (*bt_hfp_hs_event_cb)(btmg_hfp_even_t event, btmg_hfp_data_t *data);
typedef struct btmg_hfp_callback_t {
    bt_hfp_hs_event_cb hfp_hf_event_cb;
} btmg_hfp_callback_t;
int bt_manager_hfp_hf_send_at_ata(void);
int bt_manager_hfp_hf_send_at_chup(void);
int bt_manager_hfp_hf_send_at_atd(char *number);
int bt_manager_hfp_hf_send_at_bldn(void);
int bt_manager_hfp_hf_send_at_btrh(bool query, uint32_t val);
int bt_manager_hfp_hf_send_at_vts(char code);
int bt_manager_hfp_hf_send_at_bcc(void);
int bt_manager_hfp_hf_send_at_cnum(void);
int bt_manager_hfp_hf_send_at_vgs(uint32_t volume);
int bt_manager_hfp_hf_send_at_vgm(uint32_t volume);
int bt_manager_hfp_hf_send_at_cmd(char *cmd);

/*SPP SERVER*/
typedef enum {
    BTMG_SPP_SERVER_DISCONNECTED,
    BTMG_SPP_SERVER_CONNECTING,
    BTMG_SPP_SERVER_CONNECTED,
    BTMG_SPP_SERVER_DISCONNECTING,
    BTMG_SPP_SERVER_CONNECT_FAILED,
    BTMG_SPP_SERVER_DISCONNEC_FAILED,
} btmg_spp_server_connection_state_t;

typedef void (*bt_spp_server_connection_state_cb)(const char *bd_addr,
                                                  btmg_spp_server_connection_state_t state);
typedef void (*bt_spp_server_accept_cb)(const char *bd_addr, char *data, int data_len);
typedef struct btmg_spp_server_callback_t {
    bt_spp_server_connection_state_cb spp_server_connection_state_cb;
    bt_spp_server_accept_cb spp_server_accept_cb;
} btmg_spp_server_callback_t;

int bt_manager_spp_service_accept(void);
int bt_manager_spp_service_send(char *data, uint32_t len);
int bt_manager_spp_service_disconnect(const char *dst);

/*SPP CLIENT*/
typedef enum {
    BTMG_SPP_CLIENT_DISCONNECTED,
    BTMG_SPP_CLIENT_CONNECTING,
    BTMG_SPP_CLIENT_CONNECTED,
    BTMG_SPP_CLIENT_DISCONNECTING,
    BTMG_SPP_CLIENT_CONNECT_FAILED,
    BTMG_SPP_CLIENT_DISCONNEC_FAILED,
} btmg_spp_client_connection_state_t;

typedef void (*bt_spp_client_connection_state_cb)(const char *bd_addr,
                                                  btmg_spp_client_connection_state_t state);
typedef void (*bt_spp_client_recvdata_cb)(const char *bd_addr, char *data, int data_len);

typedef struct btmg_spp_client_callback_t {
    bt_spp_client_connection_state_cb spp_client_connection_state_cb;
    bt_spp_client_recvdata_cb spp_client_recvdata_cb;
} btmg_spp_client_callback_t;

int bt_manager_spp_client_connect(const char *dst);
int bt_manager_spp_client_send(char *data, uint32_t len);
int bt_manager_spp_client_disconnect(const char *dst);

/*BLE*/
typedef enum {
    BTMG_LE_PUBLIC_ADDRESS = 0x00,
    BTMG_LE_RANDOM_ADDRESS = 0x01,
    BTMG_LE_PUBLIC_ADDRESS_ID = 0x02,
    BTMG_LE_RANDOM_ADDRESS_ID = 0x03,
} btmg_le_addr_type_t;

#define BTMG_LE_ADV_CHANNEL_NONE 0x00
#define BTMG_LE_ADV_CHANNEL_37 0x01
#define BTMG_LE_ADV_CHANNEL_38 (0x01 << 1)
#define BTMG_LE_ADV_CHANNEL_39 (0x01 << 2)
#define BTMG_LE_ADV_CHANNEL_ALL                                                                    \
    (BTMG_LE_ADV_CHANNEL_NONE | BTMG_LE_ADV_CHANNEL_37 | BTMG_LE_ADV_CHANNEL_38 |                  \
     BTMG_LE_ADV_CHANNEL_39)
typedef struct {
    uint8_t data[31];
    uint8_t data_len;
} btmg_adv_data_t;
typedef struct {
    uint8_t data[31];
    uint8_t data_len;
} btmg_scan_rsp_data_t;
typedef enum {
    BTMG_LE_ADV_IND = 0x00, /*connectable and scannable undirected advertising*/
    BTMG_LE_ADV_DIRECT_HIGH_IND = 0x01, /*connectable high duty cycle directed advertising*/
    BTMG_LE_ADV_SCAN_IND = 0x02, /*scannable undirected advertising*/
    BTMG_LE_ADV_NONCONN_IND = 0x03, /*non connectable undirected advertising*/
    BTMG_LE_ADV_DIRECT_LOW_IND = 0x04, /*connectable low duty cycle directed advertising*/
    BTMG_LE_ADV_TYPE_MAX = 0x05,
} btmg_le_advertising_type_t;
typedef enum {
    BTMG_LE_PROCESS_ALL_REQ = 0x00, /*process scan and connection requests from all devices*/
    BTMG_LE_PROCESS_CONN_REQ =
            0x01, /*process connection request from all devices and scan request only from white list*/
    BTMG_LE_PROCESS_SCAN_REQ =
            0x02, /*process scan request from all devices and connection request only from white list*/
    BTMG_LE_PROCESS_WHITE_LIST_REQ = 0x03, /*process requests only from white list*/
    BTMG_LE_FILTER_POLICY_MAX = 0x04,
} btmg_le_advertising_filter_policy_t;
typedef enum {
    BTMG_LE_PEER_PUBLIC_ADDRESS =
            0x00, /*public device address(default) or public indentiy address*/
    BTMG_LE_PEER_RANDOM_ADDRESS =
            0x01, /*random device address(default) or random indentiy address*/
} btmg_le_peer_addr_type_t;
typedef struct {
    uint16_t min_interval;
    uint16_t max_interval;
    btmg_le_advertising_type_t adv_type;
    btmg_le_addr_type_t own_addr_type;
    btmg_le_peer_addr_type_t peer_addr_type;
    char peer_addr[18];
    uint8_t chan_map;
    btmg_le_advertising_filter_policy_t filter;
} btmg_le_advertising_parameters_t;
typedef enum {
    LE_ADV_DATA,
    LE_SCAN_RSP_DATA,
} btmg_adv_data_type_t;
typedef struct {
    uint8_t peer_addr[6];
    btmg_le_addr_type_t addr_type;
    btmg_adv_data_type_t adv_type;
    int8_t rssi;
    btmg_scan_rsp_data_t report;
} btmg_le_scan_report_t;
typedef struct {
    uint16_t min_conn_interval; // Range: 0x0006 to 0x0C80, Time = N * 1.25 msec, Time Range: 7.5 msec to 4 seconds.
    uint16_t max_conn_interval; // Range: 0x0006 to 0x0C80, Time = N * 1.25 msec, Time Range: 7.5 msec to 4 seconds.
    uint16_t slave_latency; // Range: 0x0000 to 0x01F3
    uint16_t conn_sup_timeout; // Range: 0x000A to 0x0C80, Time = N * 10 msec, Time Range: 100 msec to 32 seconds
} btmg_le_conn_param_t;


int bt_manager_le_set_random_address(void);
int bt_manager_le_set_adv_param(btmg_le_advertising_parameters_t *adv_param);
int bt_manager_le_set_adv_data(btmg_adv_data_t *adv_data);
int bt_manager_le_enable_adv(bool enable);
int bt_manager_le_set_scan_rsp_data(btmg_scan_rsp_data_t *rsp_data);
int bt_manager_le_disconnect(void);

/*BLE GATT Server*/
typedef enum {
    BT_GATT_CHAR_PROPERTY_BROADCAST = 0x01,
    BT_GATT_CHAR_PROPERTY_READ = 0x02,
    BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE = 0x04,
    BT_GATT_CHAR_PROPERTY_WRITE = 0x08,
    BT_GATT_CHAR_PROPERTY_NOTIFY = 0x10,
    BT_GATT_CHAR_PROPERTY_INDICATE = 0x20,
    BT_GATT_CHAR_PROPERTY_SIGNED_WRITE = 0x40
} gatt_char_properties_t;
typedef enum {
    BT_GATT_DESC_PROPERTY_READ = 0x01,
    BT_GATT_DESC_PROPERTY_WRITE = 0x02,
    BT_GATT_DESC_PROPERTY_ENCRY_READ = 0x04,
    BT_GATT_DESC_PROPERTY_ENCRY_WRITE = 0x08,
    BT_GATT_DESC_PROPERTY_ENCRY_AUTH_READ = 0x10,
    BT_GATT_DESC_PROPERTY_ENCRY_AUTH_WRITE = 0x20,
    BT_GATT_DESC_PROPERTY_SECURE_READ = 0x40,
    BT_GATT_DESC_PROPERTY_SECURE_WRITE = 0x80
} gatt_desc_properties_t;

typedef enum {
    BT_GATT_PERM_READ = 0x01,
    BT_GATT_PERM_WRITE = 0x02,
    BT_GATT_PERM_READ_ENCYPT = 0x04,
    BT_GATT_PERM_WRITE_ENCRYPT = 0x08,
    BT_GATT_PERM_ENCRYPT = 0x04 | 0x08,
    BT_GATT_PERM_READ_AUTHEN = 0x10,
    BT_GATT_PERM_WRITE_AUTHEN = 0x20,
    BT_GATT_PERM_AUTHEN = 0x10 | 0x20,
    BT_GATT_PERM_AUTHOR = 0x40,
    BT_GATT_PERM_NONE = 0x80
} gatt_permissions_t;

typedef enum {
    BT_GATT_SUCCESS = 0x00,
    BT_GATT_ERROR_INVALID_HANDLE = 0x01, /*Invalid Handle*/
    BT_GATT_ERROR_READ_NOT_PERMITTED = 0x02, /*Read not Permitted*/
    BT_GATT_ERROR_WRITE_NOT_PERMITTED = 0x03, /*Writed Not Permitted*/
    BT_GATT_ERROR_INVALID_PDU = 0x04, /*Invalid PDU*/
    BT_GATT_ERROR_AUTHENTICATION = 0x05, /*Insufficient Authentication*/
    BT_GATT_ERROR_REQUEST_NOT_SUPPORTED = 0x06, /*Request Not Supported*/
    BT_GATT_ERROR_INVALID_OFFSET = 0x07, /*Invalid Offset*/
    BT_GATT_ERROR_AUTHORIZATION = 0x08, /*Insufficient Authorization*/
    BT_GATT_ERROR_PREPARE_QUEUE_FULL = 0x09, /*Prepare Queue Full*/
    BT_GATT_ERROR_ATTRIBUTE_NOT_FOUND = 0x0A, /*Attribute Not Found*/
    BT_GATT_ERROR_ATTRIBUTE_NOT_LONG = 0x0B, /*Attribute Not Long*/
    BT_GATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE = 0x0C, /*Insufficient Encryption Key Size*/
    BT_GATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN = 0x0D, /*Invalid Attribute Value Length*/
    BT_GATT_ERROR_UNLIKELY = 0x0E, /*Unlikely Error*/
    BT_GATT_ERROR_INSUFFICIENT_ENCRYPTION = 0x0F, /*Insufficient Encryption*/
    BT_GATT_ERROR_UNSUPPORTED_GROUP_TYPE = 0x10, /*Unsupported Group Type*/
    BT_GATT_ERROR_INSUFFICIENT_RESOURCES = 0x11, /*Insufficient Resources*/
    BT_GATT_ERROR_DB_OUT_OF_SYNC = 0x12, /*Database Out of Sync*/
    BT_GATT_ERROR_VALUE_NOT_ALLOWED = 0x13 /*Value Not Allowed*/
    /*0x80-0x9F Application Error*/
    /*0xE0-0xFF Common Profile and Service Error Codes*/
} gatt_attr_res_code_t;

#define GATT_MAX_ATTR_LEN 600
typedef struct {
    int handle_num;
    int svc_handle;
} gatts_add_svc_msg_t;

typedef struct {
    char *uuid;
    int char_handle;
} gatts_add_char_msg_t;

typedef struct {
    int desc_handle;
} gatts_add_desc_msg_t;

typedef enum {
    BT_GATT_CONNECTION,
    BT_GATT_DISCONNECT,
} gatts_connection_event_t;

typedef struct {
    unsigned int trans_id;
    int attr_handle;
    int offset;
    bool is_blob_req;
} gatts_char_read_req_t;
typedef struct {
    unsigned int trans_id;
    int attr_handle;
    int offset;
    char value[GATT_MAX_ATTR_LEN];
    int value_len;
    bool need_rsp;
} gatts_char_write_req_t;
typedef struct {
    unsigned int trans_id;
    int attr_handle;
    int offset;
    bool is_blob_req;
} gatts_desc_read_req_t;
typedef struct {
    unsigned int trans_id;
    int attr_handle;
    int offset;
    char value[GATT_MAX_ATTR_LEN];
    int value_len;
    bool need_rsp;
} gatts_desc_write_req_t;

typedef struct {
} gatts_send_indication_t;

typedef void (*bt_gatts_connection_event_cb)(char *addr, gatts_connection_event_t event, int err);
typedef void (*bt_gatts_add_service_cb)(gatts_add_svc_msg_t *pData);
typedef void (*bt_gatts_add_char_cb)(gatts_add_char_msg_t *pData);
typedef void (*bt_gatts_add_desc_cb)(gatts_add_desc_msg_t *pData);
typedef void (*bt_gatts_char_read_req_cb)(gatts_char_read_req_t *char_read);
typedef void (*bt_gatts_char_write_req_cb)(gatts_char_write_req_t *char_write);
typedef void (*bt_gatts_desc_read_req_cb)(gatts_desc_read_req_t *desc_read);
typedef void (*bt_gatts_desc_write_req_cb)(gatts_desc_write_req_t *desc_write);
typedef void (*bt_gatts_send_indication_cb)(gatts_send_indication_t *send_ind);
typedef struct {
    /*gatt connection event callback*/
    bt_gatts_connection_event_cb gatts_connection_event_cb;
    /*gatt add ... callback*/
    bt_gatts_add_service_cb gatts_add_svc_cb;
    bt_gatts_add_char_cb gatts_add_char_cb;
    bt_gatts_add_desc_cb gatts_add_desc_cb;
    /*gatt characteristic request callback*/
    bt_gatts_char_read_req_cb gatts_char_read_req_cb;
    bt_gatts_char_write_req_cb gatts_char_write_req_cb;
    /*gatt descriptor request callback*/
    bt_gatts_desc_read_req_cb gatts_desc_read_req_cb;
    bt_gatts_desc_write_req_cb gatts_desc_write_req_cb;
    bt_gatts_send_indication_cb gatts_send_indication_cb;
} btmg_gatt_server_cb_t;

typedef struct {
    char *uuid; /*128-bit service UUID*/
    bool primary; /* If true, this GATT service is a primary service */
    int number;
} gatts_add_svc_t;
typedef struct {
    char *uuid; /*128-bit characteristic UUID*/
    int properties; /*The GATT characteristic properties*/
    int permissions; /*The GATT characteristic permissions*/
    int svc_handle; /*the service atr handle*/
} gatts_add_char_t;
typedef struct {
    char *uuid; /*128-bit descriptor UUID*/
    int permissions; /*he GATT descriptor  permissions*/
    int svc_handle;
} gatts_add_desc_t;
typedef struct {
    int svc_handle;
} gatts_star_svc_t;
typedef struct {
    int svc_handle;
} gatts_stop_svc_t;
typedef struct {
    int svc_handle;
} gatts_remove_svc_t;
typedef struct {
    unsigned int trans_id;
    int status;
    int attr_handle;
    char *value;
    int value_len;
    int auth_req;
} gatts_send_read_rsp_t;
typedef struct {
    unsigned int trans_id;
    int attr_handle;
    gatt_attr_res_code_t state;
} gatts_write_rsp_t;
typedef struct {
    int attr_handle;
    char *value;
    int value_len;
} gatts_notify_data_t;
typedef struct {
    int attr_handle;
    char *value;
    int value_len;
} gatts_indication_data_t;

int bt_manager_gatt_server_init(void);
int bt_manager_gatt_server_create_service(gatts_add_svc_t *svc);
int bt_manager_gatt_server_add_characteristic(gatts_add_char_t *chr);
int bt_manager_gatt_server_add_descriptor(gatts_add_desc_t *desc);
int bt_manager_gatt_server_start_service(gatts_star_svc_t *start_svc);
int bt_manager_gatt_server_stop_service(gatts_stop_svc_t *stop_svc);
int bt_manager_gatt_server_remove_service(gatts_remove_svc_t *remove_svc);
int bt_manager_gatt_server_get_mtu(void);
int bt_manager_gatt_server_send_read_response(gatts_send_read_rsp_t *pData);
int bt_manager_gatt_server_send_write_response(gatts_write_rsp_t *pData);
int bt_manager_gatt_server_send_notify(gatts_notify_data_t *pData);
int bt_manager_gatt_server_send_indication(gatts_indication_data_t *pData);
int bt_manager_gatt_server_deinit(void);

/*BLE GATT Client*/
typedef enum {
    LE_SCAN_FILTER_POLICY_ALLOW_ALL = 0,
    LE_SCAN_FILTER_POLICY_ONLY_WLIST,
    LE_SCAN_FILTER_POLICY_UND_RPA_DIR,
    LE_SCAN_FILTER_POLICY_WLIST_RPA_DIR,
} btmg_le_scan_filter_policy_t;

typedef enum {
    LE_SCAN_TYPE_PASSIVE = 0x00, // passive scanning
    LE_SCAN_TYPE_ACTIVE, // active scanning
} btmg_le_scan_type_t;

typedef enum {
    LE_SCAN_DUPLICATE_DISABLE = 0x0, // will not filter
    LE_SCAN_DUPLICATE_ENABLE = 0x1, // filter duplicate packets
} btmg_le_scan_filter_duplicate_t;

typedef struct {
    btmg_le_scan_type_t scan_type;
    uint16_t scan_interval; // Range: 0x0004 to 0x4000 Time = N * 0.625 msec Time Range: 2.5 msec to 10.24 sec
    uint16_t scan_window; // Range: 0x0004 to 0x4000 Time = N * 0.625 msec Time Range: 2.5 msec to 10.24 seconds
    btmg_le_scan_filter_duplicate_t filter_duplicate;
    btmg_le_addr_type_t own_addr_type;
    btmg_le_scan_filter_policy_t filter_policy;
    uint16_t timeout; // Scan timeout between 0x0001 and 0xFFFF in seconds, 0x0000 disables timeout.
} btmg_le_scan_param_t;

typedef enum {
    BTMG_SECURITY_LOW = 1,
    BTMG_SECURITY_MEDIUM = 2,
    BTMG_SECURITY_HIGH = 3,
    BTMG_SECURITY_FIPS = 4,
} btmg_security_level_t;
typedef struct {
    uint16_t value_handle;
    const uint8_t *value;
    uint16_t length;
    void *user_data;
} gattc_notify_indicate_cb_para_t;
typedef struct {
    bool success;
    uint8_t att_ecode;
    void *user_data;
} gattc_write_cb_para_t;
typedef struct {
    bool success;
    bool reliable_error;
    uint8_t att_ecode;
    void *user_data;
} gattc_write_long_cb_para_t;
typedef struct {
    bool success;
    uint8_t att_ecode;
    const uint8_t *value;
    uint16_t length;
    void *user_data;
} gattc_read_cb_para_t;
typedef struct {
    int conn_id;
    bool success;
    uint8_t att_ecode;
    void *user_data;
} gattc_conn_cb_para_t;
typedef struct {
    int conn_id;
    uint8_t *addr;
    int addr_type;
} gattc_connected_list_cb_para_t;
typedef enum {
    UNKNOWN_OTHER_ERROR = 0,
    CONNECTION_TIMEOUT,
    REMOTE_USER_TERMINATED,
    LOCAL_HOST_TERMINATED,
} gattc_disconnect_reason_t;
typedef struct {
    uint16_t conn_handle;
    gattc_disconnect_reason_t reason;
} gattc_disconn_cb_para_t;
typedef struct {
    uint16_t start_handle;
    uint16_t end_handle;
    void *user_data;
} gattc_service_changed_cb_para_t;
typedef struct {
    enum {
        BT_UUID_UN_SPEC = 0,
        BT_UUID_16 = 16,
        BT_UUID_32 = 32,
        BT_UUID_128 = 128,
    } type;
    union {
        uint16_t u16;
        uint32_t u32;
        uint8_t u128[16];
    } value;
} btmg_uuid_t;
typedef struct {
    int conn_id;
    uint16_t start_handle;
    uint16_t end_handle;
    bool primary;
    btmg_uuid_t uuid;
    void *attr;
} gattc_dis_service_cb_para_t;
typedef struct {
    int conn_id;
    uint16_t start_handle;
    uint16_t value_handle;
    uint8_t properties;
    uint16_t ext_prop;
    btmg_uuid_t uuid;
    void *attr;
} gattc_dis_char_cb_para_t;
typedef struct {
    int conn_id;
    uint16_t handle;
    btmg_uuid_t uuid;
} gattc_dis_desc_cb_para_t;
typedef void (*bt_gattc_notify_indicate_cb)(gattc_notify_indicate_cb_para_t *data);
typedef void (*bt_gattc_write_cb)(gattc_write_cb_para_t *data);
typedef void (*bt_gattc_write_long_cb)(gattc_write_long_cb_para_t *data);
typedef void (*bt_gattc_read_cb)(gattc_read_cb_para_t *data);
typedef void (*bt_gattc_service_changed_cb)(gattc_service_changed_cb_para_t *data);
typedef void (*bt_gattc_dis_service_cb)(gattc_dis_service_cb_para_t *data);
typedef void (*bt_gattc_dis_char_cb)(gattc_dis_char_cb_para_t *data);
typedef void (*bt_gattc_dis_desc_cb)(gattc_dis_desc_cb_para_t *data);
typedef void (*bt_gattc_connect_cb)(gattc_conn_cb_para_t *data);
typedef void (*bt_gattc_disconnect_cb)(gattc_disconn_cb_para_t *data);
typedef void (*bt_gattc_connected_list_cb)(gattc_connected_list_cb_para_t *data);
typedef struct {
    bt_gattc_connect_cb gattc_conn_cb;
    bt_gattc_disconnect_cb gattc_disconn_cb;
    bt_gattc_read_cb gattc_read_cb;
    bt_gattc_write_cb gattc_write_cb;
    bt_gattc_write_long_cb gattc_write_long_cb;
    bt_gattc_notify_indicate_cb gattc_notify_indicate_cb;
    bt_gattc_service_changed_cb gattc_service_changed_cb;
    bt_gattc_dis_service_cb gattc_dis_service_cb;
    bt_gattc_dis_char_cb gattc_dis_char_cb;
    bt_gattc_dis_desc_cb gattc_dis_desc_cb;
    bt_gattc_connected_list_cb gattc_connected_list_cb;
} btmg_gatt_client_cb_t;

int bt_manager_gatt_client_connect(uint8_t *addr, btmg_le_addr_type_t dst_type,
                                                uint16_t mtu, btmg_security_level_t sec);
int bt_manager_gatt_client_disconnect(uint8_t *addr);
int bt_manager_gatt_client_get_conn_list(void);
int bt_manager_gatt_client_set_security(int conn_id, int sec_level);
int bt_manager_gatt_client_get_security(int conn_id);
int bt_manager_gatt_client_register_notify_indicate(int conn_id, int char_handle);
int bt_manager_gatt_client_unregister_notify_indicate(int conn_id, int id);
int bt_manager_gatt_client_write_request(int conn_id, int char_handle, uint8_t *value,
                                         uint16_t len);
int bt_manager_gatt_client_write_command(int conn_id, int char_handle, bool signed_write,
                                         uint8_t *value, uint16_t len);
int bt_manager_gatt_client_write_long_request(int conn_id, bool reliable_writes, int char_handle, int offset, uint8_t *value, uint16_t len);
int bt_manager_gatt_client_read_long_request(int conn_id, int char_handle, int offset);
int bt_manager_gatt_client_read_request(int conn_id, int char_handle);
int bt_manager_gatt_client_discover_all_services(int conn_id, uint16_t start_handle,
                                                         uint16_t end_handle);
int bt_manager_gatt_client_discover_services_by_uuid(int conn_id, const char *uuid,
                                                             uint16_t start_handle, uint16_t end_handle);
int bt_manager_gatt_client_discover_service_all_char(int conn_id, void *svc_handle);
int bt_manager_gatt_client_discover_char_all_descriptor(int conn_id, void *char_handle);
int bt_manager_gatt_client_get_mtu(int conn_id);
int bt_manager_gatt_client_init(void);
int bt_manager_gatt_client_deinit(void);
const char *bt_manager_gatt_client_ecode_to_string(uint8_t ecode);
void bt_manager_uuid_to_uuid128(const btmg_uuid_t *src, btmg_uuid_t *dst);
int bt_manager_uuid_to_string(const btmg_uuid_t *uuid, char *str, size_t n);
int bt_manager_le_scan_start(btmg_le_scan_param_t *scan_param);
int bt_manager_le_scan_stop(void);
int bt_manager_le_set_scan_parameters(btmg_le_scan_param_t *scan_param);
int bt_manager_le_update_conn_params(btmg_le_conn_param_t *conn_params);

typedef void (*bt_gap_scan_status_cb)(btmg_scan_state_t status);
typedef void (*bt_gap_dev_device_add_cb)(btmg_bt_device_t *device);
typedef void (*bt_gap_dev_device_remove_cb)(btmg_bt_device_t *device);
typedef void (*bt_gap_le_scan_report_cb)(btmg_le_scan_report_t *data);
typedef void (*bt_gap_update_rssi_cb)(const char *address, int rssi);
typedef void (*bt_gap_bond_state_cb)(btmg_bond_state_t state, const char *bd_addr);
typedef struct {
    bt_gap_scan_status_cb gap_scan_status_cb; /*used for return scan status of BT*/
    bt_gap_dev_device_add_cb gap_device_add_cb; /*used for device found event*/
    bt_gap_dev_device_remove_cb gap_device_remove_cb; /*used for device found event*/
    bt_gap_le_scan_report_cb gap_le_scan_report_cb;
    bt_gap_update_rssi_cb gap_update_rssi_cb; /*update rssi for scan and bonded devices*/
    bt_gap_bond_state_cb gap_bond_state_cb; /*used for bond state event*/
} btmg_gap_callback_t;

typedef struct btmg_callback_t {
    btmg_adapter_callback_t btmg_adapter_cb;
    btmg_gap_callback_t btmg_gap_cb;
    btmg_agent_callback_t btmg_agent_cb;
    btmg_a2dp_sink_callback_t btmg_a2dp_sink_cb;
    btmg_a2dp_source_callback_t btmg_a2dp_source_cb;
    btmg_avrcp_callback_t btmg_avrcp_cb;
    btmg_hfp_callback_t btmg_hfp_cb;
    btmg_spp_server_callback_t btmg_spp_server_cb;
    btmg_spp_client_callback_t btmg_spp_client_cb;
    btmg_gatt_server_cb_t btmg_gatt_server_cb;
    btmg_gatt_client_cb_t btmg_gatt_client_cb;
} btmg_callback_t;

int bt_manager_preinit(btmg_callback_t **btmg_cb);
int bt_manager_init(btmg_callback_t *btmg_cb);
int bt_manager_deinit(btmg_callback_t *btmg_cb);

#if __cplusplus
}; // extern "C"
#endif

#endif
