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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include "monitor/bt.h"
#include "src/bluez-5.54_shared/io.h"
#include "src/bluez-5.54_shared/util.h"
#include "src/bluez-5.54_shared/queue.h"
#include "src/bluez-5.54_shared/hci.h"

#define BTPROTO_HCI	1
struct sockaddr_hci {
	sa_family_t	hci_family;
	unsigned short	hci_dev;
	unsigned short  hci_channel;
};
#define HCI_CHANNEL_RAW		0
#define HCI_CHANNEL_USER	1

#define SOL_HCI		0
#define HCI_FILTER	2
struct hci_filter {
	uint32_t type_mask;
	uint32_t event_mask[2];
	uint16_t opcode;
};

struct bluez554_bt_hci {
	int ref_count;
	struct io *io;
	bool is_stream;
	bool writer_active;
	uint8_t num_cmds;
	unsigned int next_cmd_id;
	unsigned int next_evt_id;
	struct queue *cmd_queue;
	struct queue *rsp_queue;
	struct queue *evt_list;
};

struct cmd {
	unsigned int id;
	uint16_t opcode;
	void *data;
	uint8_t size;
	bt_hci_callback_func_t callback;
	bluez554_bt_hci_destroy_func_t destroy;
	void *user_data;
};

struct evt {
	unsigned int id;
	uint8_t event;
	bt_hci_callback_func_t callback;
	bluez554_bt_hci_destroy_func_t destroy;
	void *user_data;
};

static void cmd_free(void *data)
{
	struct cmd *cmd = data;

	if (cmd->destroy)
		cmd->destroy(cmd->user_data);

	free(cmd->data);
	free(cmd);
}

static void evt_free(void *data)
{
	struct evt *evt = data;

	if (evt->destroy)
		evt->destroy(evt->user_data);

	free(evt);
}

static void send_command(struct bluez554_bt_hci *hci, uint16_t opcode,
						void *data, uint8_t size)
{
	uint8_t type = BT_H4_CMD_PKT;
	struct bt_hci_cmd_hdr hdr;
	struct iovec iov[3];
	int iovcnt;

	if (hci->num_cmds < 1)
		return;

	hdr.opcode = cpu_to_le16(opcode);
	hdr.plen = size;

	iov[0].iov_base = &type;
	iov[0].iov_len  = 1;
	iov[1].iov_base = &hdr;
	iov[1].iov_len  = sizeof(hdr);

	if (size > 0) {
		iov[2].iov_base = data;
		iov[2].iov_len  = size;
		iovcnt = 3;
	} else
		iovcnt = 2;

	if (bluez554_io_send(hci->io, iov, iovcnt) < 0)
		return;

	hci->num_cmds--;
}

static bool io_write_callback(struct io *io, void *user_data)
{
	struct bluez554_bt_hci *hci = user_data;
	struct cmd *cmd;

	cmd = bluez554_queue_pop_head(hci->cmd_queue);
	if (cmd) {
		send_command(hci, cmd->opcode, cmd->data, cmd->size);
		bluez554_queue_push_tail(hci->rsp_queue, cmd);
	}

	hci->writer_active = false;

	return false;
}

static void wakeup_writer(struct bluez554_bt_hci *hci)
{
	if (hci->writer_active)
		return;

	if (hci->num_cmds < 1)
		return;

	if (bluez554_queue_isempty(hci->cmd_queue))
		return;

	if (!bluez554_io_set_write_handler(hci->io, io_write_callback, hci, NULL))
		return;

	hci->writer_active = true;
}

static bool match_cmd_opcode(const void *a, const void *b)
{
	const struct cmd *cmd = a;
	uint16_t opcode = PTR_TO_UINT(b);

	return cmd->opcode == opcode;
}

static void process_response(struct bluez554_bt_hci *hci, uint16_t opcode,
					const void *data, size_t size)
{
	struct cmd *cmd;

	if (opcode == BT_HCI_CMD_NOP)
		goto done;

	cmd = bluez554_bluez554_queue_remove_if(hci->rsp_queue, match_cmd_opcode,
						UINT_TO_PTR(opcode));
	if (!cmd)
		return;

	if (cmd->callback)
		cmd->callback(data, size, cmd->user_data);

	cmd_free(cmd);

done:
	wakeup_writer(hci);
}

static void process_notify(void *data, void *user_data)
{
	struct bt_hci_evt_hdr *hdr = user_data;
	struct evt *evt = data;

	if (evt->event == hdr->evt)
		evt->callback(user_data + sizeof(struct bt_hci_evt_hdr),
						hdr->plen, evt->user_data);
}

static void process_event(struct bluez554_bt_hci *hci, const void *data, size_t size)
{
	const struct bt_hci_evt_hdr *hdr = data;
	const struct bt_hci_evt_cmd_complete *cc;
	const struct bt_hci_evt_cmd_status *cs;

	if (size < sizeof(struct bt_hci_evt_hdr))
		return;

	data += sizeof(struct bt_hci_evt_hdr);
	size -= sizeof(struct bt_hci_evt_hdr);

	if (hdr->plen != size)
		return;

	switch (hdr->evt) {
	case BT_HCI_EVT_CMD_COMPLETE:
		if (size < sizeof(*cc))
			return;
		cc = data;
		hci->num_cmds = cc->ncmd;
		process_response(hci, le16_to_cpu(cc->opcode),
						data + sizeof(*cc),
						size - sizeof(*cc));
		break;

	case BT_HCI_EVT_CMD_STATUS:
		if (size < sizeof(*cs))
			return;
		cs = data;
		hci->num_cmds = cs->ncmd;
		process_response(hci, le16_to_cpu(cs->opcode), &cs->status, 1);
		break;

	default:
		bluez554_queue_foreach(hci->evt_list, process_notify, (void *) hdr);
		break;
	}
}

static bool io_read_callback(struct io *io, void *user_data)
{
	struct bluez554_bt_hci *hci = user_data;
	uint8_t buf[512];
	ssize_t len;
	int fd;

	fd = bluez554_io_get_fd(hci->io);
	if (fd < 0)
		return false;

	if (hci->is_stream)
		return false;

	len = read(fd, buf, sizeof(buf));
	if (len < 0)
		return false;

	if (len < 1)
		return true;

	switch (buf[0]) {
	case BT_H4_EVT_PKT:
		process_event(hci, buf + 1, len - 1);
		break;
	}

	return true;
}

static struct bluez554_bt_hci *create_hci(int fd)
{
	struct bluez554_bt_hci *hci;

	if (fd < 0)
		return NULL;

	hci = new0(struct bluez554_bt_hci, 1);
    printf("%s\n",__func__);
	hci->io = bluez554_io_new(fd);
	if (!hci->io) {
		free(hci);
		return NULL;
	}

	hci->is_stream = true;
	hci->writer_active = false;
	hci->num_cmds = 1;
	hci->next_cmd_id = 1;
	hci->next_evt_id = 1;

	hci->cmd_queue = bluez554_queue_new();
	hci->rsp_queue = bluez554_queue_new();
	hci->evt_list = bluez554_queue_new();

	if (!bluez554_io_set_read_handler(hci->io, io_read_callback, hci, NULL)) {
		bluez554_queue_destroy(hci->evt_list, NULL);
		bluez554_queue_destroy(hci->rsp_queue, NULL);
		bluez554_queue_destroy(hci->cmd_queue, NULL);
		bluez554_io_destroy(hci->io);
		free(hci);
		return NULL;
	}

	return bluez554_bt_hci_ref(hci);
}

struct bluez554_bt_hci *bluez554_bt_hci_new(int fd)
{
	struct bluez554_bt_hci *hci;
    printf("%s\n",__func__);
	hci = create_hci(fd);
	if (!hci)
		return NULL;

	return hci;
}

static int create_socket(uint16_t index, uint16_t channel)
{
	struct sockaddr_hci addr;
	int fd;

	fd = socket(PF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC | SOCK_NONBLOCK,
								BTPROTO_HCI);
	if (fd < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = index;
	addr.hci_channel = channel;

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

struct bluez554_bt_hci *bluez554_bt_hci_new_user_channel(uint16_t index)
{
	struct bluez554_bt_hci *hci;
	int fd;

	fd = create_socket(index, HCI_CHANNEL_USER);
	if (fd < 0)
		return NULL;
    printf("%s\n",__func__);

	hci = create_hci(fd);
	if (!hci) {
		close(fd);
		return NULL;
	}

	hci->is_stream = false;

	bluez554_bt_hci_set_close_on_unref(hci, true);

	return hci;
}

struct bluez554_bt_hci *bluez554_bt_hci_new_raw_device(uint16_t index)
{
	struct bluez554_bt_hci *hci;
	struct hci_filter flt;
	int fd;

	fd = create_socket(index, HCI_CHANNEL_RAW);
	if (fd < 0)
		return NULL;

	memset(&flt, 0, sizeof(flt));
	flt.type_mask = 1 << BT_H4_EVT_PKT;
	flt.event_mask[0] = 0xffffffff;
	flt.event_mask[1] = 0xffffffff;

	if (setsockopt(fd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		close(fd);
		return NULL;
	}
    printf("%s\n",__func__);

	hci = create_hci(fd);
	if (!hci) {
		close(fd);
		return NULL;
	}

	hci->is_stream = false;

	bluez554_bt_hci_set_close_on_unref(hci, true);

	return hci;
}

struct bluez554_bt_hci *bluez554_bt_hci_ref(struct bluez554_bt_hci *hci)
{
	if (!hci)
		return NULL;

	__sync_fetch_and_add(&hci->ref_count, 1);

	return hci;
}

void bluez554_bt_hci_unref(struct bluez554_bt_hci *hci)
{
	if (!hci)
		return;

	if (__sync_sub_and_fetch(&hci->ref_count, 1))
		return;

	bluez554_queue_destroy(hci->evt_list, evt_free);
	bluez554_queue_destroy(hci->cmd_queue, cmd_free);
	bluez554_queue_destroy(hci->rsp_queue, cmd_free);

	bluez554_io_destroy(hci->io);

	free(hci);
}

bool bluez554_bt_hci_set_close_on_unref(struct bluez554_bt_hci *hci, bool do_close)
{
	if (!hci)
		return false;

	return bluez554_io_set_close_on_destroy(hci->io, do_close);
}

unsigned int bluez554_bt_hci_send(struct bluez554_bt_hci *hci, uint16_t opcode,
				const void *data, uint8_t size,
				bt_hci_callback_func_t callback,
				void *user_data, bluez554_bt_hci_destroy_func_t destroy)
{
	struct cmd *cmd;

	if (!hci)
		return 0;

	cmd = new0(struct cmd, 1);
	cmd->opcode = opcode;
	cmd->size = size;

	if (cmd->size > 0) {
		cmd->data = malloc(cmd->size);
		if (!cmd->data) {
			free(cmd);
			return 0;
		}

		memcpy(cmd->data, data, cmd->size);
	}

	if (hci->next_cmd_id < 1)
		hci->next_cmd_id = 1;

	cmd->id = hci->next_cmd_id++;

	cmd->callback = callback;
	cmd->destroy = destroy;
	cmd->user_data = user_data;

	if (!bluez554_queue_push_tail(hci->cmd_queue, cmd)) {
		free(cmd->data);
		free(cmd);
		return 0;
	}

	wakeup_writer(hci);

	return cmd->id;
}

static bool match_cmd_id(const void *a, const void *b)
{
	const struct cmd *cmd = a;
	unsigned int id = PTR_TO_UINT(b);

	return cmd->id == id;
}

bool bluez554_bt_hci_cancel(struct bluez554_bt_hci *hci, unsigned int id)
{
	struct cmd *cmd;

	if (!hci || !id)
		return false;

	cmd = bluez554_bluez554_queue_remove_if(hci->cmd_queue, match_cmd_id, UINT_TO_PTR(id));
	if (!cmd) {
		cmd = bluez554_bluez554_queue_remove_if(hci->rsp_queue, match_cmd_id,
							UINT_TO_PTR(id));
		if (!cmd)
			return false;
	}

	cmd_free(cmd);

	wakeup_writer(hci);

	return true;
}

bool bluez554_bt_hci_flush(struct bluez554_bt_hci *hci)
{
	if (!hci)
		return false;

	if (hci->writer_active) {
		bluez554_io_set_write_handler(hci->io, NULL, NULL, NULL);
		hci->writer_active = false;
	}

	bluez554_queue_remove_all(hci->cmd_queue, NULL, NULL, cmd_free);
	bluez554_queue_remove_all(hci->rsp_queue, NULL, NULL, cmd_free);

	return true;
}

unsigned int bluez554_bt_hci_register(struct bluez554_bt_hci *hci, uint8_t event,
				bt_hci_callback_func_t callback,
				void *user_data, bluez554_bt_hci_destroy_func_t destroy)
{
	struct evt *evt;

	if (!hci)
		return 0;

	evt = new0(struct evt, 1);
	evt->event = event;

	if (hci->next_evt_id < 1)
		hci->next_evt_id = 1;

	evt->id = hci->next_evt_id++;

	evt->callback = callback;
	evt->destroy = destroy;
	evt->user_data = user_data;

	if (!bluez554_queue_push_tail(hci->evt_list, evt)) {
		free(evt);
		return 0;
	}

	return evt->id;
}

static bool match_evt_id(const void *a, const void *b)
{
	const struct evt *evt = a;
	unsigned int id = PTR_TO_UINT(b);

	return evt->id == id;
}

bool bluez554_bt_hci_unregister(struct bluez554_bt_hci *hci, unsigned int id)
{
	struct evt *evt;

	if (!hci || !id)
		return false;

	evt = bluez554_bluez554_queue_remove_if(hci->evt_list, match_evt_id, UINT_TO_PTR(id));
	if (!evt)
		return false;

	evt_free(evt);

	return true;
}
