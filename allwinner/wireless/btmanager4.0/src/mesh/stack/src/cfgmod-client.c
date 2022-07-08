/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <ell/ell.h>

#include "json-c/json.h"

#include "mesh/mesh-defs.h"
#include "mesh/mesh.h"
#include "mesh/node.h"
#include "mesh/net.h"
#include "mesh/appkey.h"
#include "mesh/model.h"

#include "mesh/cfgmod.h"
#include "mesh/dbus.h"
#include "mesh/util.h"
#include "mesh/mesh-config.h"
#include "mesh/xr829-patch.h"
#include "mesh/keyring.h"
#include "mesh/error.h"

#ifdef CONFIG_XR829_BT
#define MESH_ADAPTER_INTERFACE "org.bluez.mesh.Adapter"
#define BLUEZ_MESH_ADAPTER_NAME "org.mesh.adapter"
#define BLUEZ_MESH_ADAPTER_PATH "/adapter"
#define BLUEZ_MESH_ADAPTER_INTERFACE "org.mesh.adapter"

#define CFG_MAX_MSG_LEN 380
#define MIN_COMPOSITION_LEN 16
//static bool dev_comp_get;
struct mesh_adapter {
	char *path;
	char *owner;
	uint32_t disc_watch;
	struct adapter_request *req;
	struct mesh_agent *agent;
};

static uint32_t print_mod_id(const uint8_t *data, bool vid)
{
	uint32_t mod_id;

	if (!vid) {
		mod_id = l_get_le16(data);
		l_debug("Model Id\t%4.4x", mod_id);
		mod_id = 0xffff0000 | mod_id;
	} else {
		mod_id = l_get_le16(data + 2);
		l_debug("vendor Model Id\t%4.4x %4.4x",
				l_get_le16(data), mod_id);
		mod_id = l_get_le16(data) << 16 | mod_id;
	}
	return mod_id;
}

static bool cfg_cli_pkt(uint16_t src, uint32_t dst, uint16_t unicast,
				uint16_t app_idx, uint16_t net_idx,
				const uint8_t *data, uint16_t size,
				uint8_t ttl, const void *user_data)
{
	struct mesh_node *node = (struct mesh_node *) user_data;
	const uint8_t *pkt;
    uint8_t nrst_s = 0;
	uint32_t opcode,mod_id;
	uint16_t company_id,m_opcode,ele_addr,addr,i,n;
    uint8_t rssi = 0;
	struct mesh_model_pub pub = {
        .credential= 0
    };

	struct l_dbus_message *dmsg;
	struct l_dbus *dbus = dbus_get_bus();
	struct l_dbus_message_builder *builder;

	pkt = data;
	if ((app_idx != APP_IDX_DEV_REMOTE) && (app_idx != APP_IDX_DEV_LOCAL))
		return false;

	if (mesh_model_opcode_get(pkt, size, &opcode, &n)) {
		size -= n;
		pkt += n;
	} else {
		l_info("mesh_model_opcode_get return false\n");
		return false;
	}

	l_info("cfgmod-client  CONFIG-CLI-opcode 0x%x size %u idx %3.3x", opcode, size, app_idx);

	if (n == 3) {
		company_id = ((opcode & 0xff)<< 8) | ((opcode >> 8) & 0xff);
		m_opcode = opcode >> 16;
	} else {
		company_id = 0xffff;
		m_opcode = opcode;
	}

	n = 0;

	l_info("opcode : %x meta_data: src_addr %x dst_addr %x appkey_index %x net_index %x\n", opcode, src, dst, app_idx, net_idx);
	switch (opcode) {
	default:
		return false;

	case OP_DEV_COMP_STATUS:
		print_packet("dev comp status data: ", (const void *)data, size);
		break;

    case OP_APPKEY_LIST:
        l_debug("appkey list: size %d",size);
        break;

	case OP_APPKEY_STATUS:
		if (size != 4)
			break;

		l_debug("Node %4.4x AppKey status %s", src,
						mesh_status_str(pkt[0]));
		net_idx = l_get_le16(pkt + 1) & 0xfff;
		app_idx = l_get_le16(pkt + 2) >> 4;

		l_debug("==NetKey\t%4.4x", net_idx);
		l_debug("==AppKey\t%4.4x", app_idx);
		break;
    case OP_NETKEY_LIST:
        l_debug("netkey list: size %d",size);
        break;

	case OP_NETKEY_STATUS:
		if (size != 3)
			break;

		l_debug("Node %4.4x NetKey status %s", src,
						mesh_status_str(pkt[0]));
		net_idx = l_get_le16(pkt + 1) & 0xfff;

		l_debug("\tNetKey %3.3x", net_idx);
		break;

	case OP_MODEL_APP_STATUS:
		if (size != 7 && size != 9)
			break;

		l_debug("Node %4.4x Model App status %s", src,
						mesh_status_str(pkt[0]));
		addr = l_get_le16(pkt + 1);
		app_idx = l_get_le16(pkt + 3);

		l_debug("Element Addr\t%4.4x", addr);

		mod_id = print_mod_id(pkt + 5, (size == 9) ? true : false);
		l_debug("AppIdx\t\t%3.3x ", app_idx);
		break;

	case OP_NODE_IDENTITY_STATUS:
		if (size != 4)
			return true;
		l_debug("Network index 0x%04x "
				"Node Identity state 0x%02x status %s",
				l_get_le16(pkt + 1), pkt[3],
				mesh_status_str(pkt[0]));
		break;

	case OP_CONFIG_BEACON_STATUS:
		if (size != 1)
			return true;
		l_debug("Node %4.4x Config Beacon Status 0x%02x",
				src, pkt[0]);
		break;

	case OP_CONFIG_RELAY_STATUS:
		if (size != 2)
			return true;
		l_debug("Node %4.4x Relay state 0x%02x"
				" count %d steps %d",
				src, pkt[0], pkt[1] & 0x7, pkt[1] >> 3);
		break;

	case OP_CONFIG_NETWORK_TRANSMIT_STATUS:
		if (size != 1)
			return true;

		l_debug("Node %4.4x network transmit"
				" count %d steps %d",
				src, pkt[0] & 0x7, pkt[0] >> 3);
		break;
	case OP_CONFIG_PROXY_STATUS:
		if (size != 1)
			return true;
		l_debug("Node %4.4x Proxy state 0x%02x",
				src, pkt[0]);
		break;

	case OP_CONFIG_DEFAULT_TTL_STATUS:
		if (size != 1)
			return true;
		l_debug("Node %4.4x Default TTL %d", src, pkt[0]);
		break;

	case OP_CONFIG_FRIEND_STATUS:
		if (size != 1)
			return true;
		l_debug("Node %4.4x friend status %d", src, pkt[0]);
		break;
	case OP_CONFIG_MODEL_PUB_STATUS:
		if (size != 12 && size != 14)
			return true;

		l_debug("\nNode %4.4x Publication status %s",
				src, mesh_status_str(pkt[0]));

		ele_addr = l_get_le16(pkt + 1);

		l_debug("Element Addr\t%04x", ele_addr);

		mod_id = print_mod_id(pkt + 10, (size == 14) ? true : false);

		pub.addr = l_get_le16(pkt + 3);
		pub.idx = l_get_le16(pkt + 5);
		pub.ttl = pkt[7];
		pub.period = pkt[8];
		l_debug("sub addr %x idx %d ttl %d period %x\n", pub.addr, pub.idx, pub.ttl, pub.period);
		pub.retransmit = pkt[9];
		l_debug("Rexmit count\t%d", pkt[9] & 0x7);
		l_debug("Rexmit steps\t%d", pkt[9] >> 3);
		break;

	/* Per Mesh Profile 4.3.2.19 */
	case OP_CONFIG_MODEL_SUB_STATUS:
		l_debug("\nNode %4.4x Subscription status %s",
				src, mesh_status_str(pkt[0]));
		ele_addr = l_get_le16(pkt + 1);
		addr = l_get_le16(pkt + 3);
		l_debug("Element Addr\t%4.4x size %d", ele_addr, size);
		mod_id = print_mod_id(pkt + 5, (size == 9) ? true : false);
		l_debug("Subscr Addr\t%4.4x mod_id %x", addr, mod_id);
		l_info("sub_status: addr %x ele_addr %x status %x company %x mod_id %x", addr, ele_addr, pkt[0], mod_id >> 16, mod_id & 0xffff);
		break;

	/* Per Mesh Profile 4.3.2.27 */
	case OP_CONFIG_MODEL_SUB_LIST:

		l_debug("\nNode %4.4x Subscription List status %s",
				src, mesh_status_str(pkt[0]));
		l_debug("Element Addr\t%4.4x", l_get_le16(pkt + 1));
		l_debug("Model ID\t%4.4x", l_get_le16(pkt + 3));

		for (i = 5; i < size; i += 2)
			l_debug("Subscr Addr\t%4.4x",
					l_get_le16(pkt + i));
		break;

	case OP_CONFIG_VEND_MODEL_SUB_LIST:

		l_debug("\nNode %4.4x vendor Subscription List status %s",
				src, mesh_status_str(pkt[0]));
		l_debug("Element Addr\t%4.4x", l_get_le16(pkt + 1));

		mod_id = l_get_le16(pkt + 5);
		l_debug("vendor Model Id\t%4.4x %4.4x",
				l_get_le16(pkt + 3), mod_id);

		mod_id = l_get_le16(pkt + 3) << 16 | mod_id;
		l_debug("sub vendor Model ID\t%4.4x", mod_id);

		for (i = 7; i < size; i += 2)
			l_debug("Subscr Addr\t%4.4x",
					l_get_le16(pkt + i));
		break;

	/* Per Mesh Profile 4.3.2.50 */
	case OP_MODEL_APP_LIST:
		l_debug("\nNode %4.4x Model AppIdx "
				"status %s", src,
				mesh_status_str(pkt[0]));
		l_debug("Element Addr\t%4.4x", l_get_le16(pkt + 1));
		l_debug("Model ID\t%4.4x", l_get_le16(pkt + 3));

		for (i = 5; i < size; i += 2)
			l_debug("Model AppIdx\t%4.4x",
					l_get_le16(pkt + i));
		break;

	case OP_VEND_MODEL_APP_LIST:
		l_debug("\nNode %4.4x Model AppIdx "
				"status %s", src,
				mesh_status_str(pkt[0]));
		l_debug("Element Addr\t%4.4x", l_get_le16(pkt + 1));

		mod_id = l_get_le16(pkt + 5);
		l_debug("vendor Model Id\t%4.4x %4.4x",
				l_get_le16(pkt + 3), mod_id);

		mod_id = l_get_le16(pkt + 3) << 16 | mod_id;
		l_debug("vendor Model ID\t%4.4x", mod_id);

		for (i = 7; i < size; i += 2)
			l_debug("Model AppIdx\t%4.4x",
					l_get_le16(pkt + i));
		break;

	/* Per Mesh Profile 4.3.2.63 */
	case OP_CONFIG_HEARTBEAT_PUB_STATUS:
		l_debug("\nNode %4.4x Heartbeat publish status %s",
				src, mesh_status_str(pkt[0]));

		if (pkt[0] != MESH_STATUS_SUCCESS)
			return true;

		l_debug("Destination\t%4.4x", l_get_le16(pkt + 1));
		l_debug("Count\t\t%2.2x", pkt[3]);
		l_debug("Period\t\t%2.2x", pkt[4]);
		l_debug("TTL\t\t%2.2x", pkt[5]);
		l_debug("Features\t%4.4x", l_get_le16(pkt + 6));
		l_debug("Net_Idx\t%4.4x", l_get_le16(pkt + 8));
		break;

	/* Per Mesh Profile 4.3.2.66 */
	case OP_CONFIG_HEARTBEAT_SUB_STATUS:
		l_debug("\nNode %4.4x Heartbeat subscribe status %s",
				src, mesh_status_str(pkt[0]));

		if (pkt[0] != MESH_STATUS_SUCCESS)
			return true;

		l_debug("Source\t\t%4.4x", l_get_le16(pkt + 1));
		l_debug("Destination\t%4.4x", l_get_le16(pkt + 3));
		l_debug("Period\t\t%2.2x", pkt[5]);
		l_debug("Count\t\t%2.2x", pkt[6]);
		l_debug("Min Hops\t%2.2x", pkt[7]);
		l_debug("Max Hops\t%2.2x", pkt[8]);
		break;
    case OP_CONFIG_KEY_REFRESH_PHASE_STATUS:
        l_debug("key refresh status %x",pkt[0]);
        break;
	/* Per Mesh Profile 4.3.2.54 */
	case OP_NODE_RESET_STATUS:
       {
		l_debug("Node %4.4x reset status %s",
				src, mesh_status_str(pkt[0]));
            pkt = &nrst_s;
            size = 1;
       }
		break;
	}

	char *method_name = "ConfigMessageCb";/*mi_mesh_event_to_dbus_method(MIBLE_MESH_EVENT_GENERIC_MESSAGE_CB);*/
	if (node_get_owner(node) == NULL || node_get_app_path(node) == NULL) {
		l_info("cfg_cli_pkt node is not normal\n");
		return true;
	}
	dmsg = l_dbus_message_new_method_call(dbus, node_get_owner(node),
								node_get_app_path(node),
								/*MESH_PROVISIONER_INTERFACE,*/
								MESH_APPLICATION_INTERFACE,
								method_name);

	builder = l_dbus_message_builder_new(dmsg);
	l_dbus_message_builder_append_basic(builder, 'q', &company_id);
	l_dbus_message_builder_append_basic(builder, 'q', &m_opcode);
	l_dbus_message_builder_append_basic(builder, 'q', &src);
	l_dbus_message_builder_append_basic(builder, 'q', &dst);
	l_dbus_message_builder_append_basic(builder, 'q', &app_idx);
	l_dbus_message_builder_append_basic(builder, 'q', &net_idx);
	l_dbus_message_builder_append_basic(builder, 'y', &rssi);
	l_dbus_message_builder_append_basic(builder, 'y', &ttl);

	dbus_append_byte_array(builder, pkt, size);

	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_debug("cfg_cli_pkt Send \" cfg msg \"  to %s %s",
											node_get_owner(node), node_get_app_path(node));
	l_dbus_send(dbus, dmsg);
	return true;
}

static void cfgmod_cli_unregister(void *user_data)
{
	struct mesh_node *node = user_data;
	struct mesh_net *net = node_get_net(node);
	struct mesh_net_heartbeat *hb = mesh_net_heartbeat_get(net);

	l_timeout_remove(hb->pub_timer);
	l_timeout_remove(hb->sub_timer);
	hb->pub_timer = hb->sub_timer = NULL;
}

static const struct mesh_model_ops ops = {
	.unregister = cfgmod_cli_unregister,
	.recv = cfg_cli_pkt,
	.bind = NULL,
	.sub = NULL,
	.pub = NULL
};

bool mesh_config_cli_msg_handler(struct mesh_node *node, uint16_t src, uint16_t dst,uint16_t global_net_idx,
			uint8_t ttl, const void *data, uint32_t len)
{
	uint32_t opcode;
	uint32_t size = len;
	uint16_t n;
	int b_res = MESH_STATUS_SUCCESS;
	uint8_t *pkt = (uint8_t *)data;
	//struct mesh_net *net = node_get_net(node);

	if (mesh_model_opcode_get(data, len, &opcode, &n)) {
		len -= n;
		// data += n; //TODO
	} else
		return false;

    l_info("mesh_config_cli_msg_handler opcode is %x len %d\n", opcode,len);

	switch (opcode) {
	default:
        l_debug("%s default",__func__);
		return false;
	case OP_DEV_COMP_GET:
	case OP_CONFIG_DEFAULT_TTL_GET:
    case OP_CONFIG_MODEL_PUB_GET:
    case OP_CONFIG_VEND_MODEL_SUB_GET:
    case OP_CONFIG_MODEL_SUB_GET:
    case OP_CONFIG_RELAY_GET:
    case OP_CONFIG_PROXY_GET:
    case OP_CONFIG_NETWORK_TRANSMIT_GET:
    case OP_NODE_IDENTITY_GET:
    case OP_CONFIG_BEACON_GET:
    case OP_CONFIG_FRIEND_GET:
    case OP_CONFIG_KEY_REFRESH_PHASE_GET:
    case OP_APPKEY_GET:
	case OP_NETKEY_GET:
    case OP_VEND_MODEL_APP_GET:
    case OP_MODEL_APP_GET:
    case OP_CONFIG_HEARTBEAT_PUB_GET:
    case OP_CONFIG_HEARTBEAT_SUB_GET:
        l_debug("opcode %x",opcode);
        cfg_srv_pkt(src,dst,0,APP_IDX_DEV_LOCAL,global_net_idx,pkt,size,0x7f,(const void*)node);
		break;

    case OP_CONFIG_PROXY_SET:
    case OP_NODE_IDENTITY_SET:
    case OP_CONFIG_BEACON_SET:
    case OP_CONFIG_FRIEND_SET:
    case OP_CONFIG_KEY_REFRESH_PHASE_SET:
    case OP_NODE_RESET:
    case OP_CONFIG_POLL_TIMEOUT_LIST:
    case OP_CONFIG_DEFAULT_TTL_SET:
    case OP_CONFIG_MODEL_PUB_VIRT_SET:
    case OP_CONFIG_MODEL_PUB_SET:
    case OP_CONFIG_RELAY_SET:
	case OP_CONFIG_NETWORK_TRANSMIT_SET:
    case OP_APPKEY_UPDATE:
    case OP_APPKEY_ADD:
    case OP_APPKEY_DELETE:
    case OP_NETKEY_ADD:
    case OP_NETKEY_UPDATE:
    case OP_NETKEY_DELETE:
    case OP_MODEL_APP_BIND:
    case OP_MODEL_APP_UNBIND:
    case OP_CONFIG_HEARTBEAT_PUB_SET:
    case OP_CONFIG_HEARTBEAT_SUB_SET:
	case OP_CONFIG_MODEL_SUB_VIRT_OVERWRITE:
	case OP_CONFIG_MODEL_SUB_VIRT_DELETE:
	case OP_CONFIG_MODEL_SUB_VIRT_ADD:
	case OP_CONFIG_MODEL_SUB_OVERWRITE:
	case OP_CONFIG_MODEL_SUB_DELETE:
	case OP_CONFIG_MODEL_SUB_ADD:
	case OP_CONFIG_MODEL_SUB_DELETE_ALL:
        l_debug("opcode %x",opcode);
        cfg_srv_pkt(src,dst,0,APP_IDX_DEV_LOCAL,global_net_idx,pkt,size,0x7f,(const void*)node);
        break;
	}

    l_debug("%s end ret %d",__func__,b_res);
    return (MESH_STATUS_SUCCESS == b_res);
}

void cfgmod_cli_init(struct mesh_node *node, uint8_t ele_idx)
{
	l_debug("%2.2x", ele_idx);
	mesh_model_register(node, ele_idx, CONFIG_CLI_MODEL, &ops, node);
}
#endif
