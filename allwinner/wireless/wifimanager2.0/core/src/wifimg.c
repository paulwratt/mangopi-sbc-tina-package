/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * File: wifimg.c
 *
 * Description:
 *		This file implements the core APIs of wifi manager.
 */
#include <wifimg.h>
#include <wmg_common.h>
#include <wifi_log.h>
#include <string.h>

wmg_status_t wifi_on(wifi_mode_t mode)
{
	wmg_status_t ret;

	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		ret = wifimg_object->init();
		if (ret != WMG_STATUS_SUCCESS) {
			return ret;
		}
	}

	ret = wifimg_object->switch_mode(mode);
	if ((ret == WMG_STATUS_UNHANDLED) || (ret == WMG_STATUS_SUCCESS)) {
		WMG_DEBUG("switch wifi mode success\n");
		ret = WMG_STATUS_SUCCESS;
	} else {
		WMG_ERROR("failed to switch wifi mode\n");
	}

	return ret;
}

wmg_status_t wifi_off(void)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(wifimg_object->is_init()){
		wifimg_object->deinit();
	} else {
		WMG_DEBUG("wifimg is already deinit\n");
	}

	return WMG_STATUS_SUCCESS;
}

wmg_status_t wifi_sta_connect(wifi_sta_cn_para_t *cn_para)
{
	wmg_status_t ret;
	wifi_mode_t current_mode;

	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_STATION) {
		WMG_ERROR("wifi station mode is not enabled\n");
		return WMG_STATUS_NOT_READY;
	}

	if (cn_para == NULL) {
		WMG_ERROR("invalid connect parameters\n");
		return WMG_STATUS_INVALID;
	}

	ret = wifimg_object->sta_connect(cn_para);
	if (!ret)
		WMG_DEBUG("wifi station connect success\n");
	else
		WMG_ERROR("wifi station connect fail\n");

	return ret;
}

wmg_status_t wifi_sta_disconnect(void)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_DEBUG("wifi manager is not running\n");
	}

	return wifimg_object->sta_disconnect();
}

wmg_status_t wifi_sta_auto_reconnect(wmg_bool_t enable)
{
	wmg_status_t ret;
	wifi_mode_t current_mode;

	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_STATION) {
		WMG_ERROR("wifi station mode is not enabled\n");
		return WMG_STATUS_NOT_READY;
	}

	return wifimg_object->sta_auto_reconnect(enable);
}

wmg_status_t wifi_sta_get_info(wifi_sta_info_t *sta_info)
{
	wmg_status_t ret;
	wifi_mode_t current_mode;

	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_STATION) {
		WMG_ERROR("wifi station mode is not enabled\n");
		return WMG_STATUS_NOT_READY;
	}

	if (sta_info == NULL) {
		WMG_ERROR("invalid station info parameters\n");
		return WMG_STATUS_INVALID;
	}

	return wifimg_object->sta_get_info(sta_info);
}

wmg_status_t wifi_sta_list_networks(wifi_sta_list_t *sta_list_networks)
{
	wmg_status_t ret;
	wifi_mode_t current_mode;

	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_STATION) {
		WMG_ERROR("wifi station mode is not enabled\n");
		return WMG_STATUS_NOT_READY;
	}

	if (sta_list_networks == NULL) {
		WMG_ERROR("invalid list networks parameters\n");
		return WMG_STATUS_INVALID;
	}

	return wifimg_object->sta_list_networks(sta_list_networks);
}

wmg_status_t wifi_sta_remove_networks(char *ssid)
{
	wmg_status_t ret;
	wifi_mode_t current_mode;

	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_STATION) {
		WMG_ERROR("wifi station mode is not enabled\n");
		return WMG_STATUS_NOT_READY;
	}

	return wifimg_object->sta_remove_networks(ssid);
}

wmg_status_t wifi_ap_enable(wifi_ap_config_t *ap_config)
{
	wifi_mode_t current_mode;

	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_AP) {
		WMG_ERROR("wifi ap mode is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	if (ap_config == NULL) {
		WMG_ERROR("invalid ap config parameters\n");
		return WMG_STATUS_INVALID;
	}

	return wifimg_object->ap_enable(ap_config);
}

wmg_status_t wifi_ap_disable(void)
{
	wifi_mode_t current_mode;
	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_DEBUG("wifi manager is not running\n");
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_AP) {
		WMG_ERROR("wifi ap mode is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	return wifimg_object->ap_disable();
}

wmg_status_t wifi_ap_get_config(wifi_ap_config_t *ap_config)
{
	wifi_mode_t current_mode;

	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_AP) {
		WMG_ERROR("wifi ap mode is not enabled\n");
		return WMG_STATUS_NOT_READY;
	}

	return wifimg_object->ap_get_config(ap_config);
}

wmg_status_t wifi_monitor_enable(uint8_t channel)
{
	wifi_mode_t current_mode;

	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_MONITOR) {
		WMG_ERROR("wifi monitor mode is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	return wifimg_object->monitor_enable(channel);
}

wmg_status_t wifi_monitor_set_channel(uint8_t channel)
{
	wifi_mode_t current_mode;

	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_MONITOR) {
		WMG_ERROR("wifi monitor mode is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	return wifimg_object->monitor_set_channel(channel);
}

wmg_status_t wifi_monitor_disable(void)
{
	wifi_mode_t current_mode;
	wifimg_object_t* wifimg_object = get_wifimg_object();

	if(!wifimg_object->is_init()){
		WMG_DEBUG("wifi manager is not running\n");
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_MONITOR) {
		WMG_ERROR("wifi monitor mode is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	return wifimg_object->monitor_disable();
}

wmg_status_t wifi_register_msg_cb(wifi_msg_cb_t msg_cb)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();

	wmg_status_t ret;

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	if (msg_cb == NULL) {
		WMG_ERROR("invalid parameters, msg_cb is NULL\n");
		return WMG_STATUS_INVALID;
	}

	ret = wifimg_object->register_msg_cb(msg_cb);
	if (!ret)
		WMG_DEBUG("register msg cb success\n");
	else
		WMG_ERROR("failed to register msg cb\n");

	return ret;
}

wmg_status_t wifi_set_scan_param(wifi_scan_param_t *scan_param)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();
	return WMG_STATUS_FAIL;
}

wmg_status_t wifi_get_scan_results(wifi_scan_result_t *scan_results, uint32_t *bss_num, uint32_t arr_size)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();

	wmg_status_t ret;
	wifi_mode_t current_mode;

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	current_mode = wifimg_object->get_current_mode();
	if (current_mode != WIFI_STATION) {
		WMG_ERROR("wifi station is not enabled\n");
		return WMG_STATUS_NOT_READY;
	}

	if (scan_results == NULL || arr_size == 0 || bss_num == NULL) {
		WMG_ERROR("invalid parameters\n");
		return WMG_STATUS_INVALID;
	}

	ret = wifimg_object->get_scan_results(scan_results, bss_num, arr_size);
	if (!ret)
		WMG_DEBUG("get scan results success\n");
	else
		WMG_ERROR("failed to get scan results\n");

	return ret;
}

wmg_status_t wifi_set_mac(const char *ifname, uint8_t *mac_addr)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();

	wmg_status_t ret = WMG_STATUS_FAIL;
	char wmg_ifname[32];

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	if(ifname == NULL) {
		strncpy(wmg_ifname, WMG_DEFAULT_INF, 6);
	} else {
		if(strlen(ifname) > 32){
			WMG_ERROR("infname longer than 32\n");
			return WMG_STATUS_FAIL;
		} else {
			strncpy(wmg_ifname, ifname, strlen(ifname));
		}
	}

	ret = wifimg_object->set_mac(wmg_ifname, mac_addr);
	if (!ret)
		WMG_DEBUG("set mac addr success\n");
	else
		WMG_ERROR("failed to set mac addr\n");

	return ret;
}

wmg_status_t wifi_get_mac(const char *ifname, uint8_t *mac_addr)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();

	wmg_status_t ret = WMG_STATUS_FAIL;
	char wmg_ifname[32];

	if(!wifimg_object->is_init()){
		WMG_ERROR("wifi manager is not running\n");
		return WMG_STATUS_NOT_READY;
	}

	if(ifname == NULL) {
		strncpy(wmg_ifname, WMG_DEFAULT_INF, 6);
	} else {
		if(strlen(ifname) > 32){
			WMG_ERROR("infname longer than 32\n");
			return WMG_STATUS_FAIL;
		} else {
			strncpy(wmg_ifname, ifname, strlen(ifname));
		}
	}

	ret = wifimg_object->get_mac(wmg_ifname, mac_addr);
	if (!ret)
		WMG_DEBUG("get mac addr success\n");
	else
		WMG_ERROR("failed to get mac addr\n");

	return ret;
}

wmg_status_t wifi_send_80211_raw_frame(uint8_t *data, uint32_t len, void *priv)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();
	return WMG_STATUS_FAIL;
}

wmg_status_t wifi_set_country_code(const char *country_code)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();
	return WMG_STATUS_FAIL;
}

wmg_status_t wifi_get_country_code(char *country_code)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();
	return WMG_STATUS_FAIL;
}

wmg_status_t wifi_set_ps_mode(wmg_bool_t enable)
{
	wifimg_object_t* wifimg_object = get_wifimg_object();
	return WMG_STATUS_FAIL;
}
