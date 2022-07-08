
#ifndef __BT_SPP_SERVICE_H__
#define __BT_SPP_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

sdp_session_t *spp_service_register(uint8_t rfcomm_channel);
void spp_service_regesiter_dev(void);
int spp_service_accept(int channelID);
int spp_service_send(char *data, uint32_t len);
int spp_service_disconnect(void);
int spp_service_bt_disconnect(const char *addr);
bool is_spp_service_run(void);
int spp_service_start_recv_thread(void);
void spp_service_stop_recv_thread(void);

#ifdef __cplusplus
}
#endif
#endif
