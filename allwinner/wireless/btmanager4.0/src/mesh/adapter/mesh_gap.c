#include "dbus.h"
#include "bluez/include/error.h"
#include "mesh_internal_api.h"

struct l_dbus_message *dbus_adv_packet_cb(struct l_dbus *dbus,
						struct l_dbus_message *message,void *user_data)
{
	struct l_dbus_message *reply;
	struct l_dbus_message_iter iter_data, iter_data_addr;
	uint8_t *data;
	uint8_t *mac;
	uint8_t addr_type, adv_type, data_len;
	uint32_t len;
	int16_t rssi;
	//int8_t *rssi_p;

    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;

	if (l_dbus_message_is_error(message)) {
	   mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,AW_MESH_STACK_REPLY_FAILED);
	   return dbus_error(message, MESH_ERROR_NONE,
				 "Mesh message is empty");
	}

	if (!l_dbus_message_get_arguments(message, "ayyynyay", &iter_data_addr,
					 &addr_type, &adv_type, &rssi,
					 &data_len, &iter_data))
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
	   return dbus_error(message, MESH_ERROR_INVALID_ARGS, NULL);
    }

	l_dbus_message_iter_get_fixed_array(&iter_data_addr, &mac, &len);
	if (len)
    {
        memcpy(&mesh_event.param.adv_report.peer_addr.addr[0], mac, len);
    }
    else
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
	   return dbus_error(message, MESH_ERROR_INVALID_ARGS,
				 "Mesh message device key is empty");
    }


	l_dbus_message_iter_get_fixed_array(&iter_data, &data, &len);
	if (len)
    {
        memcpy(&mesh_event.param.adv_report.data[0], data, len);
    }
    else
    {
        mesh_stack_reply(AW_MESH_UNKNOW_REQ,__func__,MESH_ERROR_INVALID_ARGS);
        return dbus_error(message, MESH_ERROR_INVALID_ARGS,
			 "Mesh message device key is empty");
    }


	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

    if(event_cb)
    {
        mesh_event.evt_code = AW_MESH_ADV_REPORT;
	mesh_event.param.adv_report.rssi = (int8_t)rssi;
        mesh_event.param.adv_report.peer_addr.addr_type = addr_type;
        mesh_event.param.adv_report.type = adv_type;
        mesh_event.param.adv_report.dlen = data_len;
        event_cb(&mesh_event,NULL);
    }

	return reply;
}
