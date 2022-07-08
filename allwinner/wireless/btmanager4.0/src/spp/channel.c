#include <stdlib.h>
#include <errno.h>

#include "channel.h"
#include "bt_log.h"

int getChannel(uint8_t *uuid, const char *device_address)
{
    // connect to an SDP server
    uint8_t address[6];
    // create query lists
    int range = 0x0000ffff;
    int channel = 0;
    sdp_session_t *session;
    sdp_list_t *responseList;
    sdp_list_t *searchList;
    sdp_list_t *attrIdList;
    sdp_list_t *responses;
    uuid_t uuid128;

    str2ba(device_address, (bdaddr_t *)&address);
    session = sdp_connect(BDADDR_ANY, (bdaddr_t *)&address, SDP_RETRY_IF_BUSY);
    if (!session) {
        BTMG_ERROR("can't connect to sdp server on device %s, %s", device_address, strerror(errno));
        return BT_ERROR;
    }
    sdp_uuid128_create(&uuid128, uuid);
    attrIdList = sdp_list_append(NULL, &range);
    searchList = sdp_list_append(NULL, &uuid128);
    // search for records
    int success = sdp_service_search_attr_req(session, searchList, SDP_ATTR_REQ_RANGE, attrIdList,
                                              &responseList);

    if (success) {
        BTMG_ERROR("search failed: %s", strerror(errno));
        sdp_list_free(attrIdList, 0);
        sdp_list_free(searchList, 0);
        sdp_close(session);
        return BT_ERROR;
    }

    sdp_list_free(attrIdList, 0);
    sdp_list_free(searchList, 0);

    // check responses
    success = sdp_list_len(responseList);
    if (success <= 0) {
        sdp_close(session);
        BTMG_ERROR("no responses: %s", strerror(errno));
        return BT_ERROR;
    }

    // process responses
    responses = responseList;
    while (responses) {
        sdp_record_t *record = (sdp_record_t *)responses->data;
        sdp_list_t *protoList;
        success = sdp_get_access_protos(record, &protoList);
        if (success) {
            BTMG_ERROR("can't access protocols: %s", strerror(errno));
            return BT_ERROR;
        }

        sdp_list_t *protocol = protoList;
        while (protocol) {
            sdp_list_t *pds;
            int protocolCount = 0;
            pds = (sdp_list_t *)protocol->data;

            while (pds) { // loop thru all pds
                sdp_data_t *d;
                int dtd;
                d = pds->data;
                while (d) {
                    dtd = d->dtd;
                    switch (dtd) {
                    case SDP_UUID16:
                    case SDP_UUID32:
                    case SDP_UUID128:
                        protocolCount = sdp_uuid_to_proto(&d->val.uuid);
                        break;
                    case SDP_UINT8:
                        if (protocolCount == RFCOMM_UUID) {
                            channel = d->val.uint8; // save channel id
                        }
                        break;
                    default:
                        break;
                    }
                    d = d->next; // to next data unit
                }
                pds = pds->next; // to next pds
            }
            sdp_list_free((sdp_list_t *)protocol->data, 0);

            protocol = protocol->next; // to next protocol
        }
        sdp_list_free(protoList, 0);

        responses = responses->next; // to next response
    }

    // assert channel id not less that 1:
    // - in principle, a channel id ranges between 1 and 30
    if (channel < 1) {
        BTMG_ERROR("no rfcomm channel found using UUID:");
        BTMG_ERROR("00001101-0000-1000-8000-00805F9B34FB");
        return BT_ERROR;
    }
    BTMG_DEBUG("rfcomm channel id: %d", channel);

    return channel;
}
