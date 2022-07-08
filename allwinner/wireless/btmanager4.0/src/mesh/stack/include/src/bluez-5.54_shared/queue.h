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

typedef void (*bluez554_queue_destroy_func_t)(void *data);

struct queue;

struct queue_entry {
	void *data;
	struct queue_entry *next;
};

struct queue *bluez554_queue_new(void);
void bluez554_queue_destroy(struct queue *queue, bluez554_queue_destroy_func_t destroy);

bool bluez554_queue_push_tail(struct queue *queue, void *data);
bool bluez554_queue_push_head(struct queue *queue, void *data);
bool bluez554_queue_push_after(struct queue *queue, void *entry, void *data);
void *bluez554_queue_pop_head(struct queue *queue);
void *bluez554_queue_peek_head(struct queue *queue);
void *bluez554_queue_peek_tail(struct queue *queue);

typedef void (*bluez554_queue_foreach_func_t)(void *data, void *user_data);

void bluez554_queue_foreach(struct queue *queue, bluez554_queue_foreach_func_t function,
							void *user_data);

typedef bool (*queue_match_func_t)(const void *data, const void *match_data);

void *bluez554_queue_find(struct queue *queue, queue_match_func_t function,
							const void *match_data);

bool bluez554_queue_remove(struct queue *queue, void *data);
void *bluez554_bluez554_queue_remove_if(struct queue *queue, queue_match_func_t function,
							void *user_data);
unsigned int bluez554_queue_remove_all(struct queue *queue, queue_match_func_t function,
				void *user_data, bluez554_queue_destroy_func_t destroy);

const struct queue_entry *bluez554_queue_get_entries(struct queue *queue);

unsigned int bluez554_queue_length(struct queue *queue);
bool bluez554_queue_isempty(struct queue *queue);
