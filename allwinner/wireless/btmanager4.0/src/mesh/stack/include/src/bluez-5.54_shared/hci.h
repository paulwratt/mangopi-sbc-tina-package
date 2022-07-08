/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdbool.h>
#include <stdint.h>

typedef void (*bluez554_bt_hci_destroy_func_t)(void *user_data);

struct bluez554_bt_hci;

struct bluez554_bt_hci *bluez554_bt_hci_new(int fd);
struct bluez554_bt_hci *bluez554_bt_hci_new_user_channel(uint16_t index);
struct bluez554_bt_hci *bluez554_bt_hci_new_raw_device(uint16_t index);

struct bluez554_bt_hci *bluez554_bt_hci_ref(struct bluez554_bt_hci *hci);
void bluez554_bt_hci_unref(struct bluez554_bt_hci *hci);

bool bluez554_bt_hci_set_close_on_unref(struct bluez554_bt_hci *hci, bool do_close);

typedef void (*bt_hci_callback_func_t)(const void *data, uint8_t size,
							void *user_data);

unsigned int bluez554_bt_hci_send(struct bluez554_bt_hci *hci, uint16_t opcode,
				const void *data, uint8_t size,
				bt_hci_callback_func_t callback,
				void *user_data, bluez554_bt_hci_destroy_func_t destroy);
bool bluez554_bt_hci_cancel(struct bluez554_bt_hci *hci, unsigned int id);
bool bluez554_bt_hci_flush(struct bluez554_bt_hci *hci);

unsigned int bluez554_bt_hci_register(struct bluez554_bt_hci *hci, uint8_t event,
				bt_hci_callback_func_t callback,
				void *user_data, bluez554_bt_hci_destroy_func_t destroy);
bool bluez554_bt_hci_unregister(struct bluez554_bt_hci *hci, unsigned int id);
