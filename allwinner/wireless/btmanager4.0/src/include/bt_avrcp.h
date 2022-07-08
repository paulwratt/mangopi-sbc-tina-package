/*
* btmanager - bt_avrcp.h
*
* Copyright (c) 2018 Allwinner Technology. All Rights Reserved.
*
* Author        laumy    liumingyuan@allwinnertech.com
* verision      0.01
* Date          2019.1.10
* Desciption    avrcp control
*
* History:
*    1. date
*    2. Author
*    3. modification
*/

#ifndef __BT_AVRCP_H
#define __BT_AVRCP_H
#if __cplusplus
extern "C" {
#endif
int bluez_avrcp_play();
int bluez_avrcp_pause();
int bluez_avrcp_next();
int bluez_avrcp_previous();
int bluez_avrcp_fastforward();
int bluez_avrcp_rewind();

#if __cplusplus
}; // extern "C"
#endif

#endif
