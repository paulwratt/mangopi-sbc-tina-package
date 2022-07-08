/*
 * BlueALSA - bluez.h
 * Copyright (c) 2016-2017 Arkadiusz Bokowy
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#ifndef BLUEALSA_BLUEZ_H_
#define BLUEALSA_BLUEZ_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    BT_MEDIA_STOP,
    BT_MEDIA_PLAYING,
    BT_MEDIA_PAUSE,
} bt_media_state_t;
typedef void (*media_change_t)(bt_media_state_t state);
void bluez_register_media_change_cb(media_change_t cb);
void bluez_unregister_media_change_cb(void);
void bluez_subscribe_signals(void);
void bluez_unsubscribe_signals(void);
#ifdef __cplusplus
}
#endif
#endif
