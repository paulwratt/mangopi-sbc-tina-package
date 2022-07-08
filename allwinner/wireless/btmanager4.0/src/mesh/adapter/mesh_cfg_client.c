#include "dbus.h"
#include "AWTypes.h"
#include "mesh_internal_api.h"

//bluez h files
#include "error.h"
#include "cfgmod.h"

#define LOCAL_MODEL AW_APP_CFGMODEL_MODULE
#define LOG_PRINTF(LEVEL,FMT,...)   mesh_log(LEVEL,LOCAL_MODEL,FMT,##__VA_ARGS__)

static bool config_client_build_msg(AW_MESH_CONFIGURATION_MSG_TX_T* cc_msg,uint8_t *buf, uint16_t *len, uint16_t *ele_addr)
{
	uint16_t n;
	uint32_t opcode = cc_msg->opcode;
	uint16_t tmp;
	bool is_virtual = false;
	bool is_vendor = false;

	switch (opcode) {
	default:
		return false;

	case OP_DEV_COMP_GET:
		n = mesh_model_opcode_set(opcode, buf);
		buf[n++] = cc_msg->data.composition_data_get.page;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x page:%x]\n",STR_DEV_COMP_GET_TAG,cc_msg->meta.dst_addr,cc_msg->data.composition_data_get.page);
#endif

		break;

	case OP_CONFIG_DEFAULT_TTL_SET:
		n = mesh_model_opcode_set(opcode, buf);
		buf[n++] = cc_msg->data.default_ttl_set.TTL;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x ttl %x]\n",STR_DEFAULT_TTL_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.default_ttl_set.TTL);
#endif

		break;

	case OP_CONFIG_DEFAULT_TTL_GET:
		n = mesh_model_opcode_set(opcode, buf);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x]\n",STR_DEFAULT_TTL_GET_TAG,cc_msg->meta.dst_addr);
#endif

		break;

	case OP_CONFIG_MODEL_PUB_VIRT_SET:
		/* TODO */
		is_virtual = true;
	case OP_CONFIG_MODEL_PUB_SET:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.model_pub_set.state->element_address, buf + n);
	    n += 2;
	    /* Publish address */
		if (is_virtual) {
			memcpy(buf + n, cc_msg->data.model_pub_set.state->publish_address.virtual_uuid, 16);
			n += 16;
		} else {
		l_put_le16(cc_msg->data.model_pub_set.state->publish_address.value, buf + n);
			n += 2;
		}
	    /* AppKey index + credential (set to 0) */
	    l_put_le16(cc_msg->data.model_pub_set.state->appkey_index, buf + n);
	    n += 2;
	    /* TTL */
	    buf[n++] = cc_msg->data.model_pub_set.state->publish_ttl;
	    /* Publish period  step count and step resolution */
	    buf[n++] = cc_msg->data.model_pub_set.state->publish_period;
	    /* Publish retransmit count & interval steps */
	    buf[n++] = (cc_msg->data.model_pub_set.state->retransmit_count & 0x7) |
				(cc_msg->data.model_pub_set.state->retransmit_interval_steps << 3);

		is_vendor = cc_msg->data.model_pub_set.state->model_id < VENDOR_ID_MASK ;//&& cc_msg->data.model_pub_set.state->model_id > 0xffff;
	    /* Model Id */
	    if (is_vendor) {
		    l_put_le16(cc_msg->data.model_pub_set.state->model_id >> 16, buf + n);
			n += 2;
		    l_put_le16(cc_msg->data.model_pub_set.state->model_id & 0xffff, buf + n);
		    n += 2;
	    } else {
		    l_put_le16(cc_msg->data.model_pub_set.state->model_id & 0xffff, buf + n);
		    n += 2;
	    }
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        if(is_virtual)
            mesh_test_log("%s[dst:%x ele_addr:%x uuid:%s app_idx:%x ttl %x period %x count:%x interval_steps:%x mod_id:%x]\n",STR_MODEL_PUB_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.model_pub_set.state->element_address,    \
                getstr_hex2str((uint8_t*)(cc_msg->data.model_pub_set.state->publish_address.virtual_uuid),16),cc_msg->data.model_pub_set.state->appkey_index,cc_msg->data.model_pub_set.state->publish_ttl, \
                cc_msg->data.model_pub_set.state->publish_period,cc_msg->data.model_pub_set.state->retransmit_count,cc_msg->data.model_pub_set.state->retransmit_interval_steps,    \
                cc_msg->data.model_pub_set.state->model_id);
        else
            mesh_test_log("%s[dst:%x ele_addr:%x value:%x app_idx:%x ttl %x period %x count:%x interval_steps:%x mod_id:%x]\n",STR_MODEL_PUB_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.model_pub_set.state->element_address,    \
                cc_msg->data.model_pub_set.state->publish_address.value,cc_msg->data.model_pub_set.state->appkey_index,cc_msg->data.model_pub_set.state->publish_ttl, \
                cc_msg->data.model_pub_set.state->publish_period,cc_msg->data.model_pub_set.state->retransmit_count,cc_msg->data.model_pub_set.state->retransmit_interval_steps,    \
                cc_msg->data.model_pub_set.state->model_id);
#endif
        *ele_addr = cc_msg->data.model_pub_set.state->element_address;
		break;

	case OP_CONFIG_MODEL_PUB_GET:
		n = mesh_model_opcode_set(opcode, buf);

		/* Element Address */
	    l_put_le16(cc_msg->data.model_pub_get.element_addr, buf + n);
	    n += 2;
		is_vendor = cc_msg->data.model_pub_get.model_id < VENDOR_ID_MASK && cc_msg->data.model_pub_get.model_id > 0xffff;
	    /* Model Id */
	    if (is_vendor) {
		    l_put_le16(cc_msg->data.model_pub_get.model_id >> 16, buf + n);
			n += 2;
		    l_put_le16(cc_msg->data.model_pub_get.model_id & 0xffff, buf + n);
		    n += 2;
	    } else {
		    l_put_le16(cc_msg->data.model_pub_get.model_id & 0xffff, buf + n);
		    n += 2;
	    }
		*len = n;
        *ele_addr = cc_msg->data.model_pub_get.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x ele_addr:%x mod_id:%x]\n",STR_MODEL_PUB_GET_TAG,cc_msg->meta.dst_addr,cc_msg->data.model_pub_get.element_addr,    \
            cc_msg->data.model_pub_get.model_id);
#endif

		break;

	case OP_CONFIG_VEND_MODEL_SUB_GET:
		n = mesh_model_opcode_set(opcode, buf);
		/* Per Mesh Profile 4.3.2.19 */
	    /* Element Address */
	    l_put_le16(cc_msg->data.vendor_model_sub_get.element_addr, buf + n);
	    n += 2;
	    /* VENDOR Model ID */
		tmp = cc_msg->data.vendor_model_sub_get.model_id >> 16;
	    l_put_le16(tmp, buf + n);
	    n += 2;
		tmp = cc_msg->data.vendor_model_sub_get.model_id & 0xffff;
	    l_put_le16(tmp, buf + n);
	    n += 2;
		*len = n;
        *ele_addr = cc_msg->data.vendor_model_sub_get.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x ele_addr:%x mod_id:%x]\n",STR_VEND_MODEL_SUB_GET_TAG,cc_msg->meta.dst_addr,cc_msg->data.vendor_model_sub_get.element_addr,    \
            cc_msg->data.vendor_model_sub_get.model_id);
#endif
		break;

	case OP_CONFIG_MODEL_SUB_GET:
		n = mesh_model_opcode_set(opcode, buf);
		/* Per Mesh Profile 4.3.2.19 */
	    /* Element Address */
	    l_put_le16(cc_msg->data.sig_model_sub_get.element_addr, buf + n);
	    n += 2;
	    /* SIG Model ID */
	    l_put_le16(cc_msg->data.sig_model_sub_get.model_id, buf + n);
	    n += 2;
		*len = n;
        *ele_addr = cc_msg->data.sig_model_sub_get.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x ele_addr:%x mod_idx:%x]\n",STR_MODEL_SUB_GET_TAG,cc_msg->meta.dst_addr,cc_msg->data.sig_model_sub_get.element_addr,    \
            cc_msg->data.sig_model_sub_get.model_id);
#endif
		break;
	case OP_CONFIG_MODEL_SUB_VIRT_OVERWRITE:
		is_virtual = true;
	case OP_CONFIG_MODEL_SUB_OVERWRITE:
		n = mesh_model_opcode_set(opcode, buf);
		l_info("MODEL_SUB_ADD element_addr %x\n", cc_msg->data.model_sub_add.element_addr);
		is_vendor = cc_msg->data.model_sub_ow.model_id < VENDOR_ID_MASK && cc_msg->data.model_sub_ow.model_id > 0xffff;
		/* Per Mesh Profile 4.3.2.19 */
	    /* Element Address */
	    l_put_le16(cc_msg->data.model_sub_ow.element_addr, buf + n);
	    n += 2;
		if (is_virtual) {
			memcpy(buf + n, cc_msg->data.model_sub_ow.address.virtual_uuid, 16);
		n += 16;
		} else {
			l_put_le16(cc_msg->data.model_sub_ow.address.value, buf + n);
			n += 2;
		}
	    /* SIG Model ID */
	    if (is_vendor) {
		l_put_le16(cc_msg->data.model_sub_ow.model_id >> 16, buf + n);
		n += 2;
			l_put_le16(cc_msg->data.model_sub_ow.model_id & 0xffff, buf + n);
		n += 2;
		} else {
			l_put_le16(cc_msg->data.model_sub_ow.model_id, buf + n);
			n += 2;
		}
		*len = n;
        *ele_addr = cc_msg->data.model_sub_ow.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
        if(is_virtual)
            mesh_test_log("%s[dst:%x ele_addr:%x is_vendor:%x uuid:%s mod_id:%x]\n",STR_MODEL_SUB_OVERWRITE_TAG,cc_msg->meta.dst_addr,cc_msg->data.model_sub_ow.element_addr,    \
                is_vendor,getstr_hex2str((uint8_t *)(cc_msg->data.model_sub_ow.address.virtual_uuid),16),cc_msg->data.model_sub_ow.model_id);
        else
            mesh_test_log("%s[dst:%x ele_addr:%x is_vendor:%x value:%x mod_id:%x]\n",STR_MODEL_SUB_OVERWRITE_TAG,cc_msg->meta.dst_addr,cc_msg->data.model_sub_ow.element_addr,    \
                is_vendor,cc_msg->data.model_sub_ow.address.value,cc_msg->data.model_sub_ow.model_id);
#endif

		break;
	case OP_CONFIG_MODEL_SUB_VIRT_DELETE:
		is_virtual = true;
	case OP_CONFIG_MODEL_SUB_DELETE:
		n = mesh_model_opcode_set(opcode, buf);
		is_vendor = cc_msg->data.model_sub_del.model_id < VENDOR_ID_MASK && cc_msg->data.model_sub_del.model_id > 0xffff;
		/* Per Mesh Profile 4.3.2.19 */
	    /* Element Address */
	    l_put_le16(cc_msg->data.model_sub_del.element_addr, buf + n);
	    n += 2;
	    /* Subscription Address */
		if (is_virtual) {
			memcpy(buf + n, cc_msg->data.model_sub_del.address.virtual_uuid, 16);
		n += 16;
		} else {
		l_put_le16(cc_msg->data.model_sub_del.address.value, buf + n);
		n += 2;
		}
	    /* SIG Model ID */
	    if (is_vendor) {
		l_put_le16(cc_msg->data.model_sub_del.model_id >> 16, buf + n);
		n += 2;
			l_put_le16(cc_msg->data.model_sub_del.model_id & 0xffff, buf + n);
		n += 2;
		} else {
			l_put_le16(cc_msg->data.model_sub_del.model_id, buf + n);
			n += 2;
		}
		*len = n;
        *ele_addr = cc_msg->data.model_sub_del.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
        if(is_virtual)
            mesh_test_log("%s[dst:%x is_vendor:%x ele_addr:%x is_virtual:%x uuid:%s mod_id:%x]\n",STR_MODEL_SUB_DELETE_TAG,cc_msg->meta.dst_addr,is_vendor,cc_msg->data.model_sub_del.element_addr,  \
                is_virtual,getstr_hex2str((uint8_t  *)(cc_msg->data.model_sub_del.address.virtual_uuid),16),cc_msg->data.model_sub_del.model_id);
        else
            mesh_test_log("%s[dst:%x is_vendor:%x ele_addr:%x is_virtual:%x value:%x mod_id:%x]\n",STR_MODEL_SUB_DELETE_TAG,cc_msg->meta.dst_addr,is_vendor,cc_msg->data.model_sub_del.element_addr,  \
                is_virtual,cc_msg->data.model_sub_del.address.value,cc_msg->data.model_sub_del.model_id);
#endif

		break;

	case OP_CONFIG_MODEL_SUB_DELETE_ALL:
		n = mesh_model_opcode_set(opcode, buf);
		is_vendor = cc_msg->data.model_sub_del_all.model_id < VENDOR_ID_MASK && cc_msg->data.model_sub_del_all.model_id > 0xffff;
		/* Per Mesh Profile 4.3.2.19 */
	    /* Element Address */
	    l_put_le16(cc_msg->data.model_sub_del_all.element_addr, buf + n);
	    n += 2;
	    /* SIG Model ID */
	     if (is_vendor) {
		l_put_le16(cc_msg->data.model_sub_del_all.model_id >> 16, buf + n);
		n += 2;
			l_put_le16(cc_msg->data.model_sub_del_all.model_id & 0xffff, buf + n);
		n += 2;
		} else {
			l_put_le16(cc_msg->data.model_sub_del_all.model_id, buf + n);
			n += 2;
		}
		*len = n;
        *ele_addr = cc_msg->data.model_sub_del_all.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x is_vendor:%x ele_addr:%x mod_idx:%x]\n",STR_MODEL_SUB_DELETE_ALL_TAG,cc_msg->meta.dst_addr,is_vendor,cc_msg->data.model_sub_del_all.element_addr,    \
            cc_msg->data.model_sub_del_all.model_id);
#endif

		break;

	case OP_CONFIG_MODEL_SUB_VIRT_ADD:
		is_virtual = true;
	case OP_CONFIG_MODEL_SUB_ADD:
		n = mesh_model_opcode_set(opcode, buf);
		is_vendor = cc_msg->data.model_sub_add.model_id < VENDOR_ID_MASK && cc_msg->data.model_sub_add.model_id > 0xffff;
		/* Per Mesh Profile 4.3.2.19 */
	    /* Element Address */
		l_info("MODEL_SUB_ADD ele_addr %x addr.type %d value %x model_id %x\n", cc_msg->data.model_sub_add.element_addr, cc_msg->data.model_sub_add.address.type, cc_msg->data.model_sub_add.address.value, cc_msg->data.model_sub_add.model_id);
	    l_put_le16(cc_msg->data.model_sub_add.element_addr, buf + n);
	    n += 2;
	    /* Subscription Address */
		if (is_virtual) {
			memcpy(buf + n, cc_msg->data.model_sub_add.address.virtual_uuid, 16);
			n += 16;
		} else {
			l_put_le16(cc_msg->data.model_sub_add.address.value, buf + n);
		n += 2;
		}
	    /* SIG Model ID */
	    if (is_vendor) {
		l_put_le16(cc_msg->data.model_sub_add.model_id >> 16, buf + n);
		n += 2;
			l_put_le16(cc_msg->data.model_sub_add.model_id & 0xffff, buf + n);
		n += 2;
		} else {
			l_put_le16(cc_msg->data.model_sub_add.model_id, buf + n);
			n += 2;
		}
		*len = n;
        *ele_addr = cc_msg->data.model_sub_add.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
        if(is_virtual)
            mesh_test_log("%s[dst:%x is_virtual:%x is_vendor:%x ele_addr:%x virtual:%s mod_id:%x]\n",STR_MODEL_SUB_ADD_TAG,cc_msg->meta.dst_addr,is_virtual,is_vendor,cc_msg->data.model_sub_add.element_addr,    \
                getstr_hex2str((uint8_t *)(cc_msg->data.model_sub_add.address.virtual_uuid),16),cc_msg->data.model_sub_add.model_id);
        else
            mesh_test_log("%s[dst:%x is_virtual:%x is_vendor:%x ele_addr:%x value:%x mod_id:%x]\n",STR_MODEL_SUB_ADD_TAG,cc_msg->meta.dst_addr,is_virtual,is_vendor,cc_msg->data.model_sub_add.element_addr,    \
                cc_msg->data.model_sub_add.address.value,cc_msg->data.model_sub_add.model_id);
#endif

		break;

	case OP_CONFIG_RELAY_SET:
		n = mesh_model_opcode_set(opcode, buf);
		buf[n++] = cc_msg->data.relay_set.relay;
		buf[n++] = (cc_msg->data.relay_set.retransmit_count & 0x7) |
					((cc_msg->data.relay_set.retransmit_interval_steps & 0x1f) << 3) ;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x relay:%x count:%x interval_steps:%x]\n",STR_RELAY_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.relay_set.relay,    \
            cc_msg->data.relay_set.retransmit_count,cc_msg->data.relay_set.retransmit_interval_steps);
#endif

        break;

	case OP_CONFIG_RELAY_GET:
		n = mesh_model_opcode_set(opcode, buf);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x]\n",STR_OP_RELAY_GET_TAG,cc_msg->meta.dst_addr);
#endif

		break;

	case OP_CONFIG_NETWORK_TRANSMIT_SET:
		n = mesh_model_opcode_set(opcode, buf);
		buf[n++] = (cc_msg->data.net_trans_set.count & 0x7) |
					((cc_msg->data.net_trans_set.interval_steps&0x1f) << 3);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x count:%x interval_steps:%x]\n",STR_NETWORK_TRANSMIT_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.net_trans_set.count,    \
            cc_msg->data.net_trans_set.interval_steps);
#endif

		break;

	case OP_CONFIG_NETWORK_TRANSMIT_GET:
		n = mesh_model_opcode_set(opcode, buf);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x]\n",STR_NETWORK_TRANSMIT_GET_TAG,cc_msg->meta.dst_addr);
#endif
		break;

	case OP_CONFIG_PROXY_SET:
		n = mesh_model_opcode_set(opcode, buf);
		buf[n++] = cc_msg->data.gatt_proxy_set.gatt_proxy;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x net_idx:%x gatt_proxy:%x]\n",STR_PROXY_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.gatt_proxy_set.gatt_proxy);
#endif
		break;

	case OP_CONFIG_PROXY_GET:
		n = mesh_model_opcode_set(opcode, buf);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x net_idx:%x]\n",STR_PROXY_GET_TAG,cc_msg->meta.dst_addr);
#endif
		break;

	case OP_NODE_IDENTITY_SET:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.node_identity_set.netkey_index, buf + n);
	    n += 2;
		buf[n++] = cc_msg->data.node_identity_set.identity;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x net_idx:%x identity:%x]\n",STR_NODE_IDENTITY_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.node_identity_set.netkey_index,    \
            cc_msg->data.node_identity_set.identity);
#endif

		break;

	case OP_NODE_IDENTITY_GET:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.node_identity_get.netkey_index, buf + n);
	    n += 2;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x net_idx:%x]\n",STR_NODE_IDENTITY_GET_TAG,cc_msg->meta.dst_addr,cc_msg->data.node_identity_get.netkey_index);
#endif

		break;

	case OP_CONFIG_BEACON_SET:
		n = mesh_model_opcode_set(opcode, buf);
		buf[n++] = cc_msg->data.beacon_set.beacon;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x beacon:%x]\n",STR_BEACON_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.beacon_set.beacon);
#endif

        break;

	case OP_CONFIG_BEACON_GET:
		n = mesh_model_opcode_set(opcode, buf);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x]\n",STR_BEACON_GET_TAG,cc_msg->meta.dst_addr);
#endif

		break;

	case OP_CONFIG_KEY_REFRESH_PHASE_SET:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.key_ref_pha_set.netkey_index, buf + n);
	    n += 2;
		buf[n++] = cc_msg->data.key_ref_pha_set.transition;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x net_idx:%x transition:%x]\n",STR_KEY_REFRESH_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.key_ref_pha_set.netkey_index,    \
            cc_msg->data.key_ref_pha_set.transition);
#endif

		break;

	case OP_CONFIG_KEY_REFRESH_PHASE_GET:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.key_ref_pha_get.netkey_index, buf + n);
	    n += 2;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x net_idx:%x]\n",STR_KEY_REFRESH_GET_TAG,cc_msg->meta.dst_addr,cc_msg->data.key_ref_pha_get.netkey_index);
#endif

		break;

	case OP_APPKEY_ADD:
	case OP_APPKEY_UPDATE:
		n = mesh_model_opcode_set(opcode, buf);
		buf[n++] = cc_msg->data.appkey_add.netkey_index & 0xff;
		buf[n++] = ((cc_msg->data.appkey_add.netkey_index >> 8) & 0xf) |
			((cc_msg->data.appkey_add.appkey_index << 4) & 0xf0);
		buf[n++] = cc_msg->data.appkey_add.appkey_index >> 4;
		memcpy(buf + n, cc_msg->data.appkey_add.appkey, 16);
		n += 16;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        if(opcode == OP_APPKEY_ADD)
        {
            mesh_test_log("%s[dst:%x net_idx:%x app_idx:%x key:%s]\n",STR_APPKEY_ADD_TAG,cc_msg->meta.dst_addr,cc_msg->data.appkey_add.netkey_index,   \
                cc_msg->data.appkey_add.appkey_index,getstr_hex2str(cc_msg->data.appkey_add.appkey,16));
        }
        else
        {
            mesh_test_log("%s[dst:%x net_idx:%x app_idx:%x key:%s]\n",STR_APPKEY_UPDATE_TAG,cc_msg->meta.dst_addr,cc_msg->data.appkey_add.netkey_index,   \
                cc_msg->data.appkey_add.appkey_index,getstr_hex2str(cc_msg->data.appkey_add.appkey,16));
        }
#endif
		break;

	case OP_APPKEY_DELETE:
		n = mesh_model_opcode_set(opcode, buf);
        buf[n++] = cc_msg->data.appkey_del.netkey_index & 0xff;
		buf[n++] = ((cc_msg->data.appkey_del.netkey_index >> 8) & 0xf) |
					((cc_msg->data.appkey_del.appkey_index << 4) & 0xf0);
		buf[n++] = cc_msg->data.appkey_del.appkey_index >> 4;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x net_idx:%x app_idx:%x ]\n",STR_APPKEY_DEL_TAG,cc_msg->meta.dst_addr,cc_msg->data.appkey_del.netkey_index,   \
            cc_msg->data.appkey_del.appkey_index);
#endif

		break;

	case OP_APPKEY_GET:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.appkey_get.netkey_index, &buf[n]);
        n += 2;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x net_idx:%x]\n",STR_APPKEY_GET_TAG,cc_msg->meta.dst_addr,cc_msg->data.appkey_get.netkey_index);
#endif
		break;

	case OP_NETKEY_ADD:
	case OP_NETKEY_UPDATE:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.netkey_add.netkey_index, &buf[n]);
		n += 2;
		memcpy(buf + n, cc_msg->data.netkey_add.netkey, 16);
		n += 16;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        if(opcode == OP_NETKEY_ADD)
            mesh_test_log("%s[dst:%x net_idx:%x key:%s]\n",STR_NETKEY_ADD_TAG,cc_msg->meta.dst_addr,cc_msg->data.netkey_add.netkey_index,getstr_hex2str(cc_msg->data.netkey_add.netkey,16));
        else
            mesh_test_log("%s[dst:%x net_idx:%x key:%s]\n",STR_NETKEY_UPDATE_TAG,cc_msg->meta.dst_addr,cc_msg->data.netkey_add.netkey_index,getstr_hex2str(cc_msg->data.netkey_add.netkey,16));
#endif
		break;

	case OP_NETKEY_DELETE:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.netkey_del.netkey_index, &buf[n]);
		n += 2;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
        mesh_test_log("%s[dst:%x net_idx:%x]\n",STR_NETKEY_DEL_TAG,cc_msg->meta.dst_addr,cc_msg->data.netkey_del.netkey_index);
#endif
		break;

	case OP_NETKEY_GET:
		n = mesh_model_opcode_set(opcode, buf);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x net_idx:%x]\n",STR_NETKEY_GET_TAG,cc_msg->meta.dst_addr,cc_msg->meta.msg_netkey_index);
#endif
		break;

	case OP_MODEL_APP_BIND:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.model_app_bind.element_addr, buf + n);
		n += 2;
		l_put_le16(cc_msg->data.model_app_bind.appkey_index, buf + n);
		n += 2;
        if(cc_msg->data.model_app_bind.model_id & VENDOR_ID_MASK)
        {
		    l_put_le16((cc_msg->data.model_app_bind.model_id & VENDOR_ID_MASK) >> 16, buf + n);
		    n += 2;
		    l_put_le16(cc_msg->data.model_app_bind.model_id, buf + n);
		    n += 2;
        }
        else
        {
		    l_put_le16(cc_msg->data.model_app_bind.model_id, buf + n);
		    n += 2;
        }
		*len = n;
        *ele_addr = cc_msg->data.model_app_bind.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x ele_addr:%x app_idx:%x mod_id:%x]\n",STR_APP_BIND_TAG,cc_msg->meta.dst_addr,cc_msg->data.model_app_bind.element_addr, \
       cc_msg->data.model_app_bind.appkey_index,cc_msg->data.model_app_bind.model_id);
#endif

		break;

	case OP_MODEL_APP_UNBIND:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.model_app_unbind.element_addr, buf + n);
		n += 2;
		l_put_le16(cc_msg->data.model_app_unbind.appkey_index, buf + n);
		n += 2;
        if(cc_msg->data.model_app_unbind.model_id & VENDOR_ID_MASK)
        {
		    l_put_le16((cc_msg->data.model_app_unbind.model_id & VENDOR_ID_MASK) >> 16, buf + n);
		    n += 2;
		    l_put_le16(cc_msg->data.model_app_unbind.model_id, buf + n);
		    n += 2;
        }
        else
        {
            l_put_le16(cc_msg->data.model_app_unbind.model_id, buf + n);
            n += 2;
        }
		*len = n;
        *ele_addr = cc_msg->data.model_app_unbind.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x ele_addr:%x app_idx:%x mod_id:%x]\n",STR_APP_UNBIND_TAG,cc_msg->meta.dst_addr,cc_msg->data.model_app_unbind.element_addr, \
       cc_msg->data.model_app_unbind.appkey_index,cc_msg->data.model_app_unbind.model_id);
#endif

		break;

	case OP_VEND_MODEL_APP_GET:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.vendor_model_app_get.element_addr, buf + n);
		n += 2;
		tmp = cc_msg->data.vendor_model_app_get.model_id >> 16;
		l_put_le16(tmp, buf + n);
		n += 2;
		tmp = cc_msg->data.vendor_model_app_get.model_id & 0xffff;
		l_put_le16(tmp, buf + n);
		n += 2;
		*len = n;
        *ele_addr = cc_msg->data.vendor_model_app_get.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x ele_addr:%x mod_id:%x]\n",STR_APP_VDN_GET_TAG,cc_msg->meta.dst_addr,cc_msg->data.vendor_model_app_get.element_addr, \
       cc_msg->data.vendor_model_app_get.model_id);
#endif

		break;

	case OP_MODEL_APP_GET:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.sig_model_app_get.element_addr, buf + n);
		n += 2;
		l_put_le16(cc_msg->data.sig_model_app_get.model_id, buf + n);
		n += 2;
		*len = n;
        *ele_addr = cc_msg->data.sig_model_app_get.element_addr;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x ele_addr:%x mod_id:%x]\n",STR_APP_GET_TAG,cc_msg->meta.dst_addr,cc_msg->data.sig_model_app_get.element_addr, \
       cc_msg->data.sig_model_app_get.model_id);
#endif

		break;

	case OP_CONFIG_HEARTBEAT_PUB_SET:
		n = mesh_model_opcode_set(opcode, buf);
		/* Per Mesh Profile 4.3.2.62 */
		/* Publish address */
		l_put_le16(cc_msg->data.hb_pub_set.publication->destination, buf + n);
		n += 2;
		/* Count Log */
		buf[n++] = cc_msg->data.hb_pub_set.publication->count_log;
		/* Period Log */
		buf[n++] = cc_msg->data.hb_pub_set.publication->period_log;
		/* Heartbeat TTL */
		buf[n++] = cc_msg->data.hb_pub_set.publication->ttl;
		/* Features */
		l_put_le16(cc_msg->data.hb_pub_set.publication->features, buf + n);
		n += 2;
		/* NetKey Index */
		l_put_le16(cc_msg->data.hb_pub_set.publication->netkey_index, buf + n);
		n += 2;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x pub_dst:%x count_log:%x period_log:%x ttl:%x feat:%x net_idx:%x]\n",STR_HEARTBEAT_PUB_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.hb_pub_set.publication->destination, \
       cc_msg->data.hb_pub_set.publication->count_log,cc_msg->data.hb_pub_set.publication->period_log,cc_msg->data.hb_pub_set.publication->ttl, \
       cc_msg->data.hb_pub_set.publication->features,cc_msg->data.hb_pub_set.publication->netkey_index);
#endif

		break;

	case OP_CONFIG_HEARTBEAT_PUB_GET:
		n = mesh_model_opcode_set(opcode, buf);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x]\n",STR_HEARTBEAT_PUB_GET_TAG,cc_msg->meta.dst_addr);
#endif
		break;

	case OP_CONFIG_HEARTBEAT_SUB_SET:
		n = mesh_model_opcode_set(opcode, buf);
		/* Per Mesh Profile 4.3.2.65 */
	    /* Source address */
	    l_put_le16(cc_msg->data.hb_sub_set.subscription->source, buf + n);
	    n += 2;
	    /* Destination address */
	    l_put_le16(cc_msg->data.hb_sub_set.subscription->destination, buf + n);
	    n += 2;
	    /* Period log */
	    buf[n++] = cc_msg->data.hb_sub_set.subscription->period_log;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x sub_src:%x sub_dst:%x period_log:%x]\n",STR_HEARTBEAT_SUB_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.hb_sub_set.subscription->source,   \
       cc_msg->data.hb_sub_set.subscription->destination,cc_msg->data.hb_sub_set.subscription->period_log);
#endif

		break;

	case OP_CONFIG_HEARTBEAT_SUB_GET:
		n = mesh_model_opcode_set(opcode, buf);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x]\n",STR_HEARTBEAT_SUB_GET_TAG,cc_msg->meta.dst_addr);
#endif
		break;
	case OP_CONFIG_FRIEND_SET:
		n = mesh_model_opcode_set(opcode, buf);
		buf[n++] = cc_msg->data.friend_set.mesh_friend;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x friend:%x]\n",STR_FRIEND_SET_TAG,cc_msg->meta.dst_addr,cc_msg->data.friend_set.mesh_friend);
#endif
		break;
	case OP_CONFIG_FRIEND_GET:
		n = mesh_model_opcode_set(opcode, buf);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x]\n",STR_FRIEND_GET_TAG,cc_msg->meta.dst_addr);
#endif
		break;
	case OP_CONFIG_POLL_TIMEOUT_LIST:
		n = mesh_model_opcode_set(opcode, buf);
		l_put_le16(cc_msg->data.poll_timeout.address, buf + n);
	    n += 2;
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x poll_timeout.address:%x]\n",STR_POLL_TIMEOUT_LIST_TAG,cc_msg->meta.dst_addr,cc_msg->data.poll_timeout.address);
#endif
		break;

	case OP_NODE_RESET:
		n = mesh_model_opcode_set(opcode, buf);
		*len = n;
#ifdef MEST_TEST_LOG_ENABLE
       mesh_test_log("%s[dst:%x]\n",STR_NODE_RESET_TAG,cc_msg->meta.dst_addr);
#endif
		break;
	}

	return true;
}

#define MESH_STATUS_SUCCESS		0x00
#define MESH_STATUS_INVALID_ADDRESS	0x01
#define MESH_STATUS_INVALID_MODEL	0x02
#define MESH_STATUS_INVALID_APPKEY	0x03
#define MESH_STATUS_INVALID_NETKEY	0x04
#define MESH_STATUS_INSUFF_RESOURCES	0x05
#define MESH_STATUS_IDX_ALREADY_STORED	0x06
#define MESH_STATUS_INVALID_PUB_PARAM	0x07
#define MESH_STATUS_NOT_SUB_MOD		0x08
#define MESH_STATUS_STORAGE_FAIL	0x09
#define MESH_STATUS_FEATURE_NO_SUPPORT	0x0a
#define MESH_STATUS_CANNOT_UPDATE	0x0b
#define MESH_STATUS_CANNOT_REMOVE	0x0c
#define MESH_STATUS_CANNOT_BIND		0x0d
#define MESH_STATUS_UNABLE_CHANGE_STATE	0x0e
#define MESH_STATUS_CANNOT_SET		0x0f
#define MESH_STATUS_UNSPECIFIED_ERROR	0x10
#define MESH_STATUS_INVALID_BINDING	0x11

static const char *mesh_status_str(uint8_t err)
{
	switch (err) {
	case MESH_STATUS_SUCCESS: return "Success";
	case MESH_STATUS_INVALID_ADDRESS: return "Invalid Address";
	case MESH_STATUS_INVALID_MODEL: return "Invalid Model";
	case MESH_STATUS_INVALID_APPKEY: return "Invalid AppKey";
	case MESH_STATUS_INVALID_NETKEY: return "Invalid NetKey";
	case MESH_STATUS_INSUFF_RESOURCES: return "Insufficient Resources";
	case MESH_STATUS_IDX_ALREADY_STORED: return "Key Idx Already Stored";
	case MESH_STATUS_INVALID_PUB_PARAM: return "Invalid Publish Parameters";
	case MESH_STATUS_NOT_SUB_MOD: return "Not a Subscribe Model";
	case MESH_STATUS_STORAGE_FAIL: return "Storage Failure";
	case MESH_STATUS_FEATURE_NO_SUPPORT: return "Feature Not Supported";
	case MESH_STATUS_CANNOT_UPDATE: return "Cannot Update";
	case MESH_STATUS_CANNOT_REMOVE: return "Cannot Remove";
	case MESH_STATUS_CANNOT_BIND: return "Cannot bind";
	case MESH_STATUS_UNABLE_CHANGE_STATE: return "Unable to change state";
	case MESH_STATUS_CANNOT_SET: return "Cannot set";
	case MESH_STATUS_UNSPECIFIED_ERROR: return "Unspecified error";
	case MESH_STATUS_INVALID_BINDING: return "Invalid Binding";

	default: return "Unknown";
	}
}

bool config_client_msg_receive(uint16_t src,uint16_t net_idx,const uint8_t *data, uint16_t size)
{
	const uint8_t *pkt = data;
	uint32_t opcode;
	//int b_res = MESH_STATUS_SUCCESS;
	//uint8_t msg[11];
	uint16_t app_idx;
	//uint8_t state, status;
	//uint8_t phase;
	//bool virt = false;
	//uint8_t count;
	//uint16_t interval;
	uint16_t n;
	//uint32_t mod_id;
	//uint16_t ele_addr;
	//uint8_t ele_idx;
	uint16_t i;

	if (mesh_model_opcode_get(pkt, size, &opcode, &n)) {
		size -= n;
		pkt += n;
	} else
		return false;


	l_info("CONFIG-CLI-opcode 0x%x size %u idx %3.3x", opcode, size, net_idx);

	n = 0;

	switch (opcode) {
	default:
		return false;

	case OP_DEV_COMP_STATUS:
		break;

	case OP_APPKEY_STATUS:
		if (size != 4)
			break;

		l_info("Node %4.4x AppKey status %s", src,
						mesh_status_str(pkt[0]));
		net_idx = l_get_le16(pkt + 1) & 0xfff;
		app_idx = l_get_le16(pkt + 2) >> 4;

		l_info("NetKey\t%4.4x", net_idx);
		l_info("AppKey\t%4.4x", app_idx);
        //if(pkt[0] == 0)
        //    mesh_test_log("appkey success\n");
       // else
        //    mesh_test_log("appkey fail\n");
#if 0
		if (pkt[0] != MESH_STATUS_SUCCESS &&
				pkt[0] != MESH_STATUS_IDX_ALREADY_STORED)
			prov_appkey_key_delete(node_get_net(target_node), net_idx, app_idx);
#endif
		break;

	case OP_NETKEY_STATUS:
        //if(pkt[0] == 0)
          //  mesh_test_log("netkey success\n");
        //else
          ///  mesh_test_log("netkey fail\n");
#if 0
		if (size != 3)
			break;
		l_debug("Node %4.4x NetKey status %s", src,
						mesh_status_str(pkt[0]));
		net_idx = l_get_le16(pkt + 1) & 0xfff;
		l_debug("\tNetKey %3.3x", net_idx);
#endif
		break;

	case OP_MODEL_APP_STATUS:
		if (size != 7 && size != 9)
			break;
        //mod_id = print_mod_id(pkt + 5, (size == 9) ? true : false);
       // if(pkt[0] == 0)
       //     mesh_test_log("bind %x success\n",mod_id);
        //else
        //    mesh_test_log("bind %x fail\n",mod_id);

#if 0
		l_debug("Node %4.4x Model App status %s", src,
						mesh_status_str(pkt[0]));
		addr = l_get_le16(pkt + 1);
		app_idx = l_get_le16(pkt + 3);

		l_debug("Element Addr\t%4.4x", addr);

		mod_id = print_mod_id(pkt + 5, (size == 9) ? true : false);

		l_debug("AppIdx\t\t%3.3x ", app_idx);

		if (pkt[0] != MESH_STATUS_SUCCESS)
			prov_mesh_model_binding_del(target_node, addr, mod_id, app_idx);
#endif
		break;

	case OP_NODE_IDENTITY_STATUS:
#if 0
		if (size != 4)
			return true;
		l_debug("Network index 0x%04x "
				"Node Identity state 0x%02x status %s",
				l_get_le16(pkt + 1), pkt[3],
				mesh_status_str(pkt[0]));
#endif
		break;

	case OP_CONFIG_BEACON_STATUS:
#if 0
		if (size != 1)
			return true;
		l_debug("Node %4.4x Config Beacon Status 0x%02x",
				src, pkt[0]);
#endif
		break;

	case OP_CONFIG_RELAY_STATUS:
		if (size != 2)
			return true;
		l_debug("Node %4.4x Relay state 0x%02x"
				" count %d steps %d",
				src, pkt[0], pkt[1]>>5, pkt[1] & 0x1f);
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
/*
		if (node_set_default_ttl (target_node, pkt[0]))
			prov_db_node_set_ttl(target_node, pkt[0]);
*/
		break;

	case OP_CONFIG_MODEL_PUB_STATUS:
 #if 0
		if (size != 12 && size != 14)
			return true;

		l_debug("\nNode %4.4x Publication status %s",
				src, mesh_status_str(pkt[0]));

		if (pkt[0] != MESH_STATUS_SUCCESS)
			return true;

		ele_addr = l_get_le16(pkt + 1);

		l_debug("Element Addr\t%04x", ele_addr);

		mod_id = print_mod_id(pkt + 10, (size == 14) ? true : false);

		pub.addr = l_get_le16(pkt + 3);
		pub.idx = l_get_le16(pkt + 5);
		pub.ttl = pkt[7];
		pub.period = pkt[8];
		n = (pkt[8] & 0x3f);
		l_debug("Pub Addr\t%04x", pub.addr);
		switch (pkt[8] >> 6) {
		case 0:
			l_debug("Period\t\t%d ms", n * 100);
			break;
		case 2:
			n *= 10;
			/* fall through */
		case 1:
			l_debug("Period\t\t%d sec", n);
			break;
		case 3:
			l_debug("Period\t\t%d min", n * 10);
			break;
		}

		pub.retransmit = pkt[9];
		l_debug("Rexmit count\t%d", pkt[9] >> 5);
		l_debug("Rexmit steps\t%d", pkt[9] & 0x1f);

		ele_idx = ele_addr - node_get_primary(target_node);

		/* Local configuration is saved by server */
/*
		if (node == node_get_local_node())
			break;

		if (node_model_pub_set(node, ele_idx, mod_id, &pub))
			prov_db_node_set_model_pub(node, ele_idx, mod_id,
				     node_model_pub_get(node, ele_idx, mod_id));
*/
#endif
		break;

	/* Per Mesh Profile 4.3.2.19 */
	case OP_CONFIG_MODEL_SUB_STATUS:
#if 0
		l_debug("\nNode %4.4x Subscription status %s",
				src, mesh_status_str(pkt[0]));

		if (pkt[0] != MESH_STATUS_SUCCESS)
			return true;

		ele_addr = l_get_le16(pkt + 1);
		addr = l_get_le16(pkt + 3);
		ele_idx = ele_addr - node_get_primary(target_node);

		l_debug("Element Addr\t%4.4x", ele_addr);

		mod_id = print_mod_id(pkt + 5, (size == 9) ? true : false);

		l_debug("Subscr Addr\t%4.4x", addr);

		/* Save subscriptions in node and database */
/*
		if (node_add_subscription(target_node, ele_idx, mod_id, addr))
			prov_db_add_subscription(target_node, ele_idx, mod_id, addr);
*/
#endif
		break;

	/* Per Mesh Profile 4.3.2.27 */
	case OP_CONFIG_MODEL_SUB_LIST:

		l_debug("\nNode %4.4x Subscription List status %s",
				src, mesh_status_str(pkt[0]));

		if (pkt[0] != MESH_STATUS_SUCCESS)
			return true;

		l_debug("Element Addr\t%4.4x", l_get_le16(pkt + 1));
		l_debug("Model ID\t%4.4x", l_get_le16(pkt + 3));

		for (i = 5; i < size; i += 2)
			l_debug("Subscr Addr\t%4.4x",
					l_get_le16(pkt + i));
		break;

	/* Per Mesh Profile 4.3.2.50 */
	case OP_MODEL_APP_LIST:
		l_debug("\nNode %4.4x Model AppIdx "
				"status %s", src,
				mesh_status_str(pkt[0]));

		if (pkt[0] != MESH_STATUS_SUCCESS)
			return true;

		l_debug("Element Addr\t%4.4x", l_get_le16(pkt + 1));
		l_debug("Model ID\t%4.4x", l_get_le16(pkt + 3));

		for (i = 5; i < size; i += 2)
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

	/* Per Mesh Profile 4.3.2.54 */
	case OP_NODE_RESET_STATUS:
		l_debug("Node %4.4x reset status %s",
				src, mesh_status_str(pkt[0]));
/*
		net_release_address(node_get_primary(target_node),
				(node_get_num_elements(target_node)));

		node_free(target_node);
*/
		break;
    case OP_CONFIG_KEY_REFRESH_PHASE_STATUS:
/*
        if (pkt[0] == MESH_STATUS_SUCCESS)
        {
            mesh_test_log("key refresh success");
        }
        else
        {
            mesh_test_log("key refresh fail");
        }
*/
        break;
	}
#if 0
	if (pkt[0])
		mesh_evt_to_app(AG_MESH_AW_CFG_FAIL_EVENT);
	else
		mesh_evt_to_app(AG_MESH_AW_CFG_SUCCESS_EVENT);
#endif
	return true;
}

static void dev_key_send_call_reply(struct l_dbus_message *reply, void *user_data)
{
	int err;

	err = dbus_get_reply_error(reply);
    mesh_stack_reply(AW_MESH_NODE_CFG_REQ, __func__, err);
	if (err == MESH_ERROR_NONE) {
		LOG_PRINTF(AW_DBG_VERB_LEVE,"dev_key_send_call_reply error %d", err);
	}
	else {
		LOG_PRINTF(AW_DBG_WARN_LEVEL,"dev_key_send_call_reply error %d", err);
	}
}

//Public API
int32_t aw_mesh_send_config_client_msg(AW_MESH_CONFIGURATION_MSG_TX_T* cc_msg)
{
    struct l_dbus_message *msg;
    struct l_dbus_message_builder *builder;
    struct mesh_application *app = mesh_application_get_instance();
    struct node_element *element = NULL;
    bool remote = true;
    uint16_t dst, net_idx,len,ele_idx,ele_addr = 0;
    uint8_t buf[AW_MESH_CFG_MMDL_MSG_LEN];
    if(app == NULL)
        return AW_ERROR_MESH_NOT_INIT;

    if(cc_msg == NULL)
        return AW_ERROR_INVALID_ARGS;

    ele_addr = app->primary_addr;

    if (config_client_build_msg(cc_msg,buf, &len,&ele_addr) == false)
        return AW_MESH_ERROR_NOT_FOUND_OPCODE;
    if(cc_msg->meta.dst_addr)
    {
        element = mesh_element_find_by_idx(AW_MESH_FOUNDATION_MMDL_INDEX);
    }
    else
    {
        ele_idx = ele_addr - app->primary_addr;
        element = mesh_element_find_by_idx(ele_idx);
    }

    if(element == NULL)
    {
        l_info("element not exist ele_addr %d",ele_addr);
        return AW_MESH_ERROR_NOT_FOUND_ELEMENT;
    }
    dst = cc_msg->meta.dst_addr;
    net_idx = cc_msg->meta.msg_netkey_index;
    msg = l_dbus_message_new_method_call(app->dbus, BLUEZ_MESH_NAME,
                    app->node_path, MESH_NODE_INTERFACE,
                    "DevKeySend");

    builder = l_dbus_message_builder_new(msg);
    l_dbus_message_builder_append_basic(builder, 'o', element->path);
    l_dbus_message_builder_append_basic(builder, 'q', &dst);
    l_dbus_message_builder_append_basic(builder, 'b', &remote);
    l_dbus_message_builder_append_basic(builder, 'q', &net_idx);
    dbus_append_byte_array(builder, buf, len);
    l_dbus_message_builder_finalize(builder);
    l_dbus_message_builder_destroy(builder);
    l_dbus_send_with_reply(app->dbus, msg, dev_key_send_call_reply, NULL, NULL);

    return 0;
}
