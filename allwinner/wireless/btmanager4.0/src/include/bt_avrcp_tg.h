/*
* btmanager - bt_avrcp.h
*
* Copyright (c) 2018 Allwinner Technology. All Rights Reserved.
*
* Author        cai    caizepeng@allwinnertech.com
* verision      0.01
* Date          2021.05.07
* Desciption    avrcp target
*
* History:
*    1. date
*    2. Author
*    3. modification
*/

#ifndef __BT_AVRCP_TG_H
#define __BT_AVRCP_TG_H
#if __cplusplus
extern "C" {
#endif

extern int avrcp_status;
int bluez_avrcp_tg_init();
int bluez_avrcp_tg_deinit();
#if __cplusplus
}; // extern "C"
#endif

#endif
