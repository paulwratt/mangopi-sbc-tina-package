#include "va_wifi.h"

/*
**************************************************************************
*							wifi										*
**************************************************************************
*/
static char *sto_utf8(const char *buf, char *dst)
{
        char buf_temp[20148] = {0};
        int x = 0;
        unsigned long i;
        while(*buf != '\0')
        {
                if(*buf == '\\')
                {
                        strcpy(buf_temp,buf);
                        *buf_temp = '0';
                        *(buf_temp + 4) = '\0';
                        i = strtoul(buf_temp, NULL, 16);
                        dst[x] = i;
                        buf += 3;
                }
                else
                {
                        dst[x] = *buf;
                }
                x++;
                buf++;


        }
	dst[x] = '\0';
        return dst;
}

char *get_str_pos(char *s,char c,int n)
{
	int i = 0;
	char *obj_ptr = NULL;
	if(NULL == s)
		return NULL;
	while((obj_ptr=strchr(s,c)) != NULL){
		s=++obj_ptr;
		i++;
		if(i == n)
			return obj_ptr;
	}
	return NULL;
}

static void wifi_state_handle(struct Manager *w, int event_label)
{
    //app_debug("event_label 0x%x\n", event_label);

    switch(w->StaEvt.state)
    {
		 case CONNECTING:
		 {
			 //app_debug("Connecting to the network(%s)......\n",w->ssid);
			 break;
		 }
		 case CONNECTED:
		 {
			 //app_debug("Connected to the AP(%s)\n",w->ssid);
			 start_udhcpc();
			 extern void ota_check(void);
			 ota_check();
			 break;
		 }

		 case OBTAINING_IP:
		 {
			 //app_debug("Getting ip address(%s)......\n",w->ssid);
			 break;
		 }

		 case NETWORK_CONNECTED:
		 {
			 //app_debug("Successful network connection(%s)\n",w->ssid);
			 break;
		 }
		case DISCONNECTED:
		{
		    //app_debug("Disconnected\n");
		    break;
		}
		default:
			break;
    }
}

aw_wifi_interface_t *net_wifi_on(int event_label)
{
	aw_wifi_interface_t *p_wifi_hd= NULL;

    p_wifi_hd = (aw_wifi_interface_t *)aw_wifi_on(wifi_state_handle, event_label);
    if(p_wifi_hd == NULL) {
        return NULL;
    }

	return p_wifi_hd;
}

int net_wifi_off(aw_wifi_interface_t *p_wifi_hd)
{
	int ret = 0;

	if(p_wifi_hd == NULL) {
         return -1;
    }

	ret = aw_wifi_off(p_wifi_hd);
    if(ret < 0)
    {
        return -1;
    }
	return 0;
}

int net_wifi_connect_ap(aw_wifi_interface_t *p_wifi_hd, const char *ssid, const char *passwd, int event_label)
{
	if(p_wifi_hd == NULL) {
         return 0;
    }

	p_wifi_hd->connect_ap(ssid, passwd, event_label);
	if(aw_wifi_get_wifi_state() == NETWORK_CONNECTED)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int net_wifi_disconnect_ap(aw_wifi_interface_t *p_wifi_hd, int event_label)
{
	if(p_wifi_hd == NULL) {
         return -1;
    }

	p_wifi_hd->disconnect_ap(event_label);
	if(aw_wifi_get_wifi_state() == DISCONNECTED)
	{
		return 0;
	}
    else
	{
		return -1;
    }
}

int net_wifi_reconnect_ap(aw_wifi_interface_t *p_wifi_hd, int event_label)
{
	if(p_wifi_hd == NULL) {
         return 0;
    }

	p_wifi_hd->connect_ap_auto(event_label);
	if(aw_wifi_get_wifi_state() == NETWORK_CONNECTED)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int net_wifi_remove_all_networks(aw_wifi_interface_t *p_wifi_hd)
{
	int ret;
	if(p_wifi_hd == NULL) {
         return -1;
    }

	ret = p_wifi_hd->remove_all_networks();
	if(ret == 0) {
	return 0;
    }else {
		return -1;
	}
}

enum wmgState net_wifi_get_wifi_state()
{
	return aw_wifi_get_wifi_state();
}

int net_wifi_get_scan_results(aw_wifi_interface_t *p_wifi_hd, char *scan_results)
{
	int ret;
	int len;

	if(p_wifi_hd == NULL) {
         return -1;
    }

	len = WIFI_MAX_SCAN_SIZE;
	ret = p_wifi_hd->get_scan_results(scan_results, &len);
	#if 0
	printf("step--------1\n");
	printf("ret=%d, len=%d,\n", ret, len);
	printf("%s\n", scan_results);
	printf("\n\n");
	#endif
    if(ret == 0)
	{
		if(len >= WIFI_MAX_SCAN_SIZE) {
			//app_warn("len=%d\n", len);
			len = WIFI_MAX_SCAN_SIZE;
		}
		return len;
	}
	else
	{
		return -1;
	}
}

int net_wifi_parse_scan_results(net_wifi_scan_info_t *p_scan_info, int num, char *scan_results, int len)
{
	int i = 0, j = 0;
	int max_value = 0, tmp = 0;
	char *current_ptr = NULL;
	char *obj_ptr = NULL;
	char *obj_ptr_2 = NULL;
	char stohex_ssid[128] = {0x00};
	char level[4];
	int n_count = 0;
	int scan_number = 0;
	net_wifi_scan_info_t *p_tmp = NULL;

	if(num > WIFI_MAX_SCAN_NUM || len > WIFI_MAX_SCAN_SIZE) {
		scan_number = 0;
		goto end;
	}

	if(NULL == scan_results){
		scan_number = 0;
		goto end;
	}

	if(strncmp(scan_results,"bssid",5)){
		scan_number = 0;
		goto end;
	}

	obj_ptr = scan_results;
	while(NULL != (current_ptr=strchr(obj_ptr,'\n'))){
		obj_ptr = current_ptr+1;
		n_count++;
	}
	if(n_count == 1){
		scan_number = 0;
		goto end;
	}

	p_tmp = malloc(sizeof(net_wifi_scan_info_t) * num);
	if(p_tmp == NULL) {
		scan_number = 0;
		goto end;
	}
	memset(p_tmp, 0, sizeof(net_wifi_scan_info_t) * num);

	while((NULL != (current_ptr=strsep(&scan_results,"\n")))){
		if(!strncmp(current_ptr,"bssid",5))
			continue;

		if(*current_ptr == '\0')
			break;

		obj_ptr = get_str_pos(current_ptr,'\t',2);
		obj_ptr_2 = get_str_pos(current_ptr,'\t',4);
		if(obj_ptr != NULL && obj_ptr_2 != NULL){
			strncpy(level,obj_ptr+1,2);
			p_tmp[i].level = 100 - atoi(level);

			/*change_utf8*/
			sto_utf8(obj_ptr_2, stohex_ssid);
			strncpy(p_tmp[i].ssid, stohex_ssid, (strlen(stohex_ssid) >= 64) ? 64 : strlen(stohex_ssid));

			i++;
			if(i >= num)
				break;
		}
	}
	scan_number = i;

	/*remove duplicates*/
    for(i = 0; i < scan_number; i++){
		for(j = i+1; j < scan_number; j++){
			if(strcmp(p_tmp[i].ssid, p_tmp[j].ssid) == 0){
				memset(p_tmp[j].ssid, 0, strlen(p_tmp[j].ssid));
			}
        }
    }
	#if 0
	printf("step--------2\n");
	for(i = 0; i< scan_number; ++i){
		printf("p_tmp[%d].ssid=[%s]\n", i, p_tmp[i].ssid);
	}
	printf("\n\n");
	#endif

	/*Remove empty string*/
    j = 0;
    for(i = 0; i < scan_number; ++i) {
		if(strlen(p_tmp[i].ssid) != 0){
			strncpy(p_scan_info[j].ssid, p_tmp[i].ssid, strlen(p_tmp[i].ssid));
			p_scan_info[j].level = p_tmp[i].level;
            j++;
        }
    }
	scan_number = j;
	#if 0
	printf("step--------3\n");
	for(i = 0; i< scan_number; ++i) {
		printf("p_scan_info[%d].ssid=%s, p_scan_info[%d].level=%d\n", i, p_scan_info[i].ssid, i, p_scan_info[i].level);
    }
	printf("\n\n");
	#endif

	/*sort*/
    for(i = 0; i < scan_number-1; i++)
    {
        max_value = i;
        for(j = i + 1; j < scan_number; j++)
        {
            if(p_scan_info[max_value].level < p_scan_info[j].level)
            {
                max_value = j;
            }
        }

        if(max_value != i)
        {
            tmp = p_scan_info[i].level;
            p_scan_info[i].level = p_scan_info[max_value].level;
            p_scan_info[max_value].level = tmp;

			memset(stohex_ssid, '\0', strlen(stohex_ssid));
			strncpy(stohex_ssid, p_scan_info[i].ssid, strlen(p_scan_info[i].ssid));

			memset(p_scan_info[i].ssid, '\0', strlen(p_scan_info[i].ssid));
			strncpy(p_scan_info[i].ssid, p_scan_info[max_value].ssid, strlen(p_scan_info[max_value].ssid));

			memset(p_scan_info[max_value].ssid, '\0', strlen(p_scan_info[max_value].ssid));
			strncpy(p_scan_info[max_value].ssid, stohex_ssid,  strlen(stohex_ssid));
        }
    }
	#if 0
	printf("step--------4\n");
	printf("scan_number=%d\n", scan_number);

	for(i = 0; i< scan_number; ++i) {
		printf("p_scan_info[%d].ssid=%s, p_scan_info[%d].level=%d\n", i, p_scan_info[i].ssid, i, p_scan_info[i].level-100);
    }
	printf("\n\n");
	#endif

	end:
		if(p_tmp) {
			free(p_tmp);
		}
		return scan_number;

}

int net_wifi_get_mac_adrress(char *buf, int len)
{
	int fd;
	int ret;
	fd = open("/tmp/xr_wifi.conf", O_RDONLY);
	if(fd < 0) {
		//app_error("\n");
		return 0;
	}

	ret = read(fd, buf, 17);

	close(fd);
	return ret;
}

int net_wifi_get_connect_info(aw_wifi_interface_t *p_wifi_hd, connection_status *info)
{
	int ret;

	if(p_wifi_hd == NULL) {
         return -1;
    }

	ret = p_wifi_hd->get_connection_info(info);
	#if 0
	printf("\n*******************************************************************************\n");
	printf("wifi_get_connection_info_test: get connection infomation successfully!\n");
	printf("Connected AP: %s\n", info->ssid);
	printf("IP address: %s\n", info->ip_address);
	printf("frequency: %d\n", info->freq);
	printf("RSSI: %d\n", info->rssi);
	printf("link_speed: %d\n", info->link_speed);
	printf("noise: %d\n", info->noise);
	printf("**********************************************************************************\n");
	#endif
	return ret;
}

int net_wifi_is_ap_connected(aw_wifi_interface_t *p_wifi_hd, char *ssid, int *len)
{
	int ret;
	int is_connected = 0;

	if(p_wifi_hd == NULL) {
         return is_connected;
    }

	ret = p_wifi_hd->is_ap_connected(ssid, len);
	if(ret > 0) {
		is_connected = 1;
	}
	else {
		is_connected = 0;
	}

	return is_connected;
}
