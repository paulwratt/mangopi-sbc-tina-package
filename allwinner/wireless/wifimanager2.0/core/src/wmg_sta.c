#include <wifimg.h>
#include <wmg_common.h>
#include <wifi_log.h>
#include <wmg_sta.h>
#include <linux/linux_wpa.h>

#include <string.h>
#include <unistd.h>

#define STA_TO_WPS_CMD_MAX_LEN     100

static wmg_sta_object_t sta_object;

const char *wmg_sta_state_to_str(wifi_sta_state_t state)
{
	switch (state) {
	case WIFI_STA_IDLE:
		return "IDLE";
	case WIFI_STA_CONNECTING:
		return "CONNECTING";
	case WIFI_STA_CONNECTED:
		return "CONNECTED";
	case WIFI_STA_OBTAINING_IP:
		return "OBTAINING_IP";
	case WIFI_STA_NET_CONNECTED:
		return "NETWORK_CONNECTED";
	case WIFI_STA_DISCONNECTING:
		return "DISCONNECTING";
	case WIFI_STA_DISCONNECTED:
		return "DISCONNECTED";
	default:
		return "UNKNOWN";
	}
}

static void sta_state_notify(wifi_sta_state_t state)
{
	wifi_msg_data_t msg;

	if (sta_object.sta_msg_cb) {
		msg.id = WIFI_MSG_ID_STA_STATE_CHANGE;
		msg.data.state = state;
		sta_object.sta_msg_cb(&msg);
	}
}

static void sta_wpa_event_notify(wifi_sta_event_t event)
{
	wifi_msg_data_t msg;
	int recall_flag = 1;

	switch (event) {
	case WIFI_CONNECTED:
		sta_object.sta_state = WIFI_STA_CONNECTED;
		break;
	case WIFI_DHCP_START:
		sta_object.sta_state = WIFI_STA_OBTAINING_IP;
		break;
	case WIFI_DHCP_TIMEOUT:
		sta_object.sta_state = WIFI_STA_DHCP_TIMEOUT;
		break;
	case WIFI_DHCP_SUCCESS:
		sta_object.sta_state = WIFI_STA_NET_CONNECTED;
		break;
	case WIFI_TERMINATING:
		sta_object.wpas_run = WMG_FALSE;
	case WIFI_DISCONNECTED:
	case WIFI_ASSOC_REJECT:
	case WIFI_NETWORK_NOT_FOUND:
	case WIFI_PASSWORD_INCORRECT:
		sta_object.sta_state = WIFI_STA_DISCONNECTED;
		break;
	default:
		break;
	}

	if (sta_object.sta_msg_cb && recall_flag) {
		msg.id = WIFI_MSG_ID_STA_CN_EVENT;
		msg.data.event = event;
		sta_object.sta_msg_cb(&msg);
	}
}

static int sta_connect(void **call_argv,void **cb_argv)
{
	wmg_status_t ret;

	wifi_sta_cn_para_t *cn_para = (wifi_sta_cn_para_t *)call_argv[0];
	wmg_status_t *cb_args = (wmg_status_t *)cb_argv[0];

	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}
	if (sta_object.sta_enable == WMG_FALSE) {
		WMG_WARNG("wifi station already disabled\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}

	if (sta_object.sta_state == WIFI_STA_CONNECTING ||
		sta_object.sta_state == WIFI_STA_OBTAINING_IP) {
		WMG_ERROR("device is busy\n");
		*cb_args = WMG_STATUS_UNHANDLED;
		return WMG_STATUS_UNHANDLED;
	}

	if (!cn_para->ssid || !cn_para->ssid[0]) {
		WMG_ERROR("invalid connect parameters - ssid\n");
		*cb_args = WMG_STATUS_INVALID;
		return WMG_STATUS_INVALID;
	}

	sta_object.sta_state = WIFI_STA_CONNECTING;

	ret = sta_object.platform_inf[PLATFORM_LINUX]->sta_wpa_connect_to_ap(cn_para);

	*cb_args = ret;
	return ret;
}

static int sta_disconnect(void **call_argv,void **cb_argv)
{
	wmg_status_t *cb_args = (wmg_status_t *)cb_argv[0];
	if(sta_object.sta_state != WIFI_STA_DISCONNECTED) {
		sta_object.sta_state = WIFI_STA_DISCONNECTING;
		sta_object.platform_inf[PLATFORM_LINUX]->sta_wpa_disconnect();
		*cb_args = WMG_STATUS_SUCCESS;
		return WMG_STATUS_SUCCESS;
	}

	WMG_ERROR("sta already disconnect,need not to disconnect\n");
	*cb_args = WMG_STATUS_UNHANDLED;
	return WMG_STATUS_UNHANDLED;
}

static int sta_auto_reconnect(void **call_argv,void **cb_argv)
{
	wmg_bool_t *enable = (wmg_bool_t *)call_argv[0];
	wmg_status_t *cb_args = (wmg_status_t *)cb_argv[0];

	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}
	if(sta_object.sta_enable == WMG_FALSE) {
		WMG_WARNG("wifi station already disabled\n");
		*cb_args = WMG_STATUS_UNHANDLED;
		return WMG_STATUS_UNHANDLED;
	}

	sta_object.sta_auto_reconn = *enable;
	sta_object.platform_inf[PLATFORM_LINUX]->sta_platform_extension(WPA_CMD_SET_AUTO_RECONN, enable, cb_args);

	WMG_DUMP("sta auto reconn set to %d\n", *enable);

	return WMG_STATUS_SUCCESS;
}

static int sta_get_info(void **call_argv,void **cb_argv)
{
	wmg_status_t ret = WMG_STATUS_SUCCESS;
	wifi_sta_info_t *sta_info = (wifi_sta_info_t *)call_argv[0];
	wmg_status_t *cb_args = (wmg_status_t *)cb_argv[0];

	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}
	if (sta_object.sta_enable == WMG_FALSE) {
		WMG_WARNG("wifi station already disabled\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}

	if (sta_object.sta_state < WIFI_STA_CONNECTED ||
		sta_object.sta_state > WIFI_STA_NET_CONNECTED) {
		WMG_WARNG("wifi station already disconnected\n");
		*cb_args = WMG_STATUS_UNHANDLED;
		return WMG_STATUS_UNHANDLED;
	}

	if (sta_info == NULL) {
		WMG_ERROR("sta info is NULL\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}

	int erro_code;
	ret = sta_object.platform_inf[PLATFORM_LINUX]->sta_platform_extension(WPA_CMD_GET_INFO, (void *)sta_info, &erro_code);
	if(ret) {
		WMG_ERROR("failed to parse station info\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}

	*cb_args = ret;
	return ret;
}

static int sta_list_networks(void **call_argv,void **cb_argv)
{
	wmg_status_t ret = WMG_STATUS_SUCCESS;
	wifi_sta_list_t *sta_list = (wifi_sta_list_t *)call_argv[0];
	wmg_status_t *cb_args = (wmg_status_t *)cb_argv[0];

	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}
	if (sta_object.sta_enable == WMG_FALSE) {
		WMG_WARNG("wifi station already disabled\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}

	if (sta_list == NULL) {
		WMG_ERROR("sta list is NULL\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}

	int erro_code;
	ret = sta_object.platform_inf[PLATFORM_LINUX]->sta_platform_extension(WPA_CMD_LIST_NETWORKS, (void *)sta_list, &erro_code);
	if(ret) {
		WMG_ERROR("failed to list networks info\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}

	*cb_args = ret;
	return ret;
}

static int sta_remove_networks(void **call_argv,void **cb_argv)
{
	wmg_status_t ret = WMG_STATUS_SUCCESS;
	char *ssid = (char *)call_argv[0];
	wmg_status_t *cb_args = (wmg_status_t *)cb_argv[0];

	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}
	if (sta_object.sta_enable == WMG_FALSE) {
		WMG_WARNG("wifi station already disabled\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}

	int erro_code;
	ret = sta_object.platform_inf[PLATFORM_LINUX]->sta_platform_extension(WPA_CMD_REMOVE_NETWORKS, (void *)ssid, &erro_code);
	if(ret) {
		WMG_ERROR("failed to remove networks\n");
		*cb_args = WMG_STATUS_FAIL;
		return WMG_STATUS_FAIL;
	}

	*cb_args = ret;
	return ret;
}

static int sta_set_scan_param(void **call_argv,void **cb_argv)
{
	return WMG_STATUS_FAIL;
}

static int sta_get_scan_results(void **call_argv,void **cb_argv)
{
	wmg_status_t ret;

	sta_get_scan_results_para_t *sta_scan_results_para = (sta_get_scan_results_para_t *)call_argv[0];
	wmg_status_t *cb_args = (wmg_status_t *)cb_argv[0];

	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		*cb_args = WMG_STATUS_UNHANDLED;
		return WMG_STATUS_UNHANDLED;
	}
	if (sta_object.sta_enable == WMG_FALSE) {
		WMG_WARNG("wifi station already disabled\n");
		*cb_args = WMG_STATUS_UNHANDLED;
		return WMG_STATUS_UNHANDLED;
	}

	if (sta_object.sta_state >= WIFI_STA_CONNECTED &&
		sta_object.sta_state < WIFI_STA_NET_CONNECTED) {
		WMG_WARNG("device is busy(state:%d)\n",sta_object.sta_state);
		*cb_args = WMG_STATUS_UNHANDLED;
		return WMG_STATUS_UNHANDLED;
	}

	ret = sta_object.platform_inf[PLATFORM_LINUX]->sta_platform_extension(WPA_CMD_GET_SCAN_RESULTS, (void *)sta_scan_results_para, NULL);

	*cb_args = ret;
	return ret;
}

static int sta_register_msg_cb(void **call_argv,void **cb_argv)
{
	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		return WMG_FALSE;
	}

	wifi_msg_cb_t *msg_cb = (wifi_msg_cb_t *)call_argv[0];

	if (*msg_cb == NULL) {
		WMG_WARNG("message callback is NULL\n");
		return WMG_STATUS_UNHANDLED;
	}

	sta_object.sta_msg_cb = *msg_cb;

	return WMG_STATUS_SUCCESS;
}

static int sta_set_mac(void **call_argv,void **cb_argv)
{
	wmg_status_t ret;

	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		return WMG_FALSE;
	}

	common_mac_para_t *common_mac_para = (common_mac_para_t *)call_argv[0];
	wmg_status_t *cb_args = (wmg_status_t *)cb_argv[0];

	ret = sta_object.platform_inf[PLATFORM_LINUX]->sta_platform_extension(WPA_CMD_SET_MAC, (void *)common_mac_para, NULL);

	*cb_args = ret;
	return ret;
}

static int sta_get_mac(void **call_argv,void **cb_argv)
{
	wmg_status_t ret;

	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		return WMG_FALSE;
	}

	common_mac_para_t *common_mac_para = (common_mac_para_t *)call_argv[0];
	wmg_status_t *cb_args = (wmg_status_t *)cb_argv[0];

	ret = sta_object.platform_inf[PLATFORM_LINUX]->sta_platform_extension(WPA_CMD_GET_MAC, (void *)common_mac_para, NULL);

	*cb_args = ret;
	return ret;
}

act_func_t sta_action_table[] = {
	[WMG_STA_ACT_CONNECT] = {sta_connect, "sta_connect"},
	[WMG_STA_ACT_DISCONNECT] = {sta_disconnect, "sta_disconnect"},
	[WMG_STA_ACT_AUTO_RECONNECT] = {sta_auto_reconnect, "sta_auto_reconnect"},
	[WMG_STA_ACT_GET_INFO] = {sta_get_info, "sta_get_info"},
	[WMG_STA_ACT_LIST_NETWORKS] = {sta_list_networks, "sta_list_networks"},
	[WMG_STA_ACT_REMOVE_NETWORKS] = {sta_remove_networks, "sta_remove_networks"},
	[WMG_STA_ACT_SCAN_PARAM] = {sta_set_scan_param, "sta_set_scan_param"},
	[WMG_STA_ACT_SCAN_RESULTS] = {sta_get_scan_results, "sta_get_scan_results"},
	[WMG_STA_ACT_REGISTER_MSG_CB] = {sta_register_msg_cb, "sta_register_msg_cb"},
	[WMG_STA_ACT_SET_MAC] = {sta_set_mac, "sta_set_mac"},
	[WMG_STA_ACT_GET_MAC] = {sta_get_mac, "sta_get_mac"},
};

static wmg_status_t sta_mode_init(void)
{
	if(sta_object.init_flag == WMG_FALSE) {
		//初始化平台wpa接口
		WMG_INFO("sta mode init now\n");
		sta_object.platform_inf[PLATFORM_LINUX] = NULL;
		sta_object.platform_inf[PLATFORM_RTOS] = NULL;
		//#ifde PLATFFORM_LINUX_WPA
		sta_object.platform_inf[PLATFORM_LINUX] = sta_linux_inf_object_register();
		if(sta_object.platform_inf[PLATFORM_LINUX]->sta_wpa_init != NULL){
			if(sta_object.platform_inf[PLATFORM_LINUX]->sta_wpa_init(sta_wpa_event_notify)){
				return WMG_STATUS_FAIL;
			}
		}
		sta_object.init_flag = WMG_TRUE;
	} else {
		WMG_INFO("sta mode already init\n");
	}
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t sta_mode_deinit(void)
{
	if(sta_object.init_flag == WMG_TRUE) {
		WMG_INFO("sta mode deinit now\n");
		if(sta_object.platform_inf[PLATFORM_LINUX]->sta_wpa_deinit != NULL){
			sta_object.platform_inf[PLATFORM_LINUX]->sta_wpa_deinit();
		}
		sta_object.init_flag = WMG_FALSE;
		sta_object.wpas_run = WMG_FALSE;
		sta_object.sta_state = WIFI_STA_IDLE;
		sta_object.sta_msg_cb = NULL;
		sta_object.sta_auto_reconn = WMG_FALSE;
		sta_object.sta_active_recv = WMG_FALSE;
	} else {
		WMG_INFO("sta mode already deinit\n");
	}
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t sta_mode_enable(int* erro_code)
{
	wmg_status_t ret;

	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		return WMG_STATUS_FAIL;
	}
	if (sta_object.sta_enable == WMG_TRUE) {
		WMG_WARNG("wifi station already enabled\n");
		return WMG_STATUS_SUCCESS;
	}

	WMG_DUMP("wifi station enabling...\n");

	ret = sta_object.platform_inf[PLATFORM_LINUX]->sta_wpa_connect();
	if (ret) {
		WMG_ERROR("failed to connect to wpa_supplicant\n");
		return WMG_STATUS_FAIL;
	}
	sta_object.wpas_run = WMG_TRUE;
	WMG_DUMP("start wpa_supplicant success\n");

	act_register_handler(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,sta_action_table);

	sta_object.sta_enable = WMG_TRUE;

	WMG_DUMP("wifi station enable success\n");
	return ret;
}

static wmg_status_t sta_mode_disable(int* erro_code)
{
	wmg_status_t ret;

	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		return WMG_STATUS_SUCCESS;
	}
	if (sta_object.sta_enable == WMG_FALSE) {
		WMG_WARNG("wifi station already disabled\n");
		return WMG_STATUS_SUCCESS;
	}

	WMG_DUMP("wifi station disabling...\n");

	sta_object.platform_inf[PLATFORM_LINUX]->sta_wpa_disconnect();
	sta_object.sta_enable = WMG_FALSE;

	WMG_DUMP("wifi station disabled\n");

	return WMG_STATUS_SUCCESS;
}


static wmg_status_t sta_mode_ctl(int cmd, void *param, void *cb_msg)
{
	if(sta_object.init_flag == WMG_FALSE) {
		WMG_ERROR("wifi station has not been initialized\n");
		return WMG_STATUS_FAIL;
	}
	if (sta_object.sta_enable == WMG_FALSE) {
		WMG_WARNG("wifi station already disabled\n");
		return WMG_STATUS_SUCCESS;
	}

	WMG_DEBUG("=====sta_mode_ctl  cmd: %d=====\n", cmd);

	switch (cmd) {
		case WMG_STA_CMD_CONNECT:
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_CONNECT,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		case WMG_STA_CMD_DISCONNECT:
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_DISCONNECT,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		case WMG_STA_CMD_AUTO_RECONNECT:
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_AUTO_RECONNECT,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		case WMG_STA_CMD_GET_INFO:
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_GET_INFO,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		case WMG_STA_CMD_LIST_NETWORKS:
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_LIST_NETWORKS,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		case WMG_STA_CMD_REMOVE_NETWORKS:
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_REMOVE_NETWORKS,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		case WMG_STA_CMD_SCAN_PARAM:
			//sta_object.wmg_sta_set_scan_param((wifi_scan_param_t *)param);
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_SCAN_PARAM,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		case WMG_STA_CMD_SCAN_RESULTS:
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_SCAN_RESULTS,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		case WMG_STA_CMD_REGISTER_MSG_CB:
			//sta_object.wmg_sta_register_msg_cb((*(wifi_msg_cb_t *)param));
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_REGISTER_MSG_CB,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		case WMG_STA_CMD_SET_MAC:
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_SET_MAC,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		case WMG_STA_CMD_GET_MAC:
			if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_GET_MAC,1,1,param,cb_msg)){
				return WMG_STATUS_FAIL;
			}
			break;
		default:
		return WMG_STATUS_FAIL;
	}
	return WMG_STATUS_SUCCESS;
}

static wmg_sta_object_t sta_object = {
	.init_flag = WMG_FALSE,
	.sta_enable = WMG_FALSE,
	.wpas_run = WMG_FALSE,
	.sta_state = WIFI_STA_IDLE,
	.sta_msg_cb = NULL,
	.sta_auto_reconn = WMG_FALSE,
	.sta_active_recv = WMG_FALSE,
};

static mode_opt_t sta_mode_opt = {
	.mode_enable = sta_mode_enable,
	.mode_disable = sta_mode_disable,
	.mode_ctl = sta_mode_ctl,
};

static mode_object_t sta_mode_object = {
	.mode_name = "sta",
	.init = sta_mode_init,
	.deinit = sta_mode_deinit,
	.mode_opt = &sta_mode_opt,
	.private_data = &sta_object,
};

mode_object_t* wmg_sta_register_object(void)
{
	return &sta_mode_object;
}
