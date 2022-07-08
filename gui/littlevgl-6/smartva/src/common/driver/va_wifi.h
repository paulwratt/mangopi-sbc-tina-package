#ifndef _VA_WIFI_H_
#define _VA_WIFI_H_
#include "smt_config.h"
#include <wifi_intf.h>

/*
**************************************************************************
*							wifi											*
**************************************************************************
*/
#define WIFI_MAX_SCAN_NUM 100
#define WIFI_MAX_SCAN_SIZE 4096
#define WIFI_MAX_SSID_SIZE	48
#define WIFI_MAX_BSSID_SIZE	32
#define WIFI_MAX_PASSWORD_SIZE	48

typedef struct
{
	char ssid[WIFI_MAX_SSID_SIZE];
	int level;
} net_wifi_scan_info_t;

typedef struct
{
	char ssid[WIFI_MAX_SSID_SIZE];
	char bssid[WIFI_MAX_BSSID_SIZE];
	int freq;
	int rssi;
	int speed;
	int noise;
} net_wifi_cur_info_t;

aw_wifi_interface_t *net_wifi_on(int event_label);
int net_wifi_off(aw_wifi_interface_t *p_wifi_hd);
int net_wifi_connect_ap(aw_wifi_interface_t *p_wifi_hd, const char *ssid, const char *passwd, int event_label);
int net_wifi_disconnect_ap(aw_wifi_interface_t *p_wifi_hd, int event_label);
int net_wifi_reconnect_ap(aw_wifi_interface_t *p_wifi_hd, int event_label);
int net_wifi_remove_all_networks(aw_wifi_interface_t *p_wifi_hd);
enum wmgState net_wifi_get_wifi_state();
int net_wifi_get_scan_results(aw_wifi_interface_t *p_wifi_hd, char *scan_results);
int net_wifi_parse_scan_results(net_wifi_scan_info_t *p_scan_info, int num, char *scan_results, int len);
int net_wifi_get_connect_info(aw_wifi_interface_t *p_wifi_hd, connection_status *info);
int net_wifi_is_ap_connected(aw_wifi_interface_t *p_wifi_hd, char *ssid, int *len);
int net_wifi_get_mac_adrress(char *buf, int len);

#endif