#ifndef __BT_A2DP_SOURCE_H__
#define __BT_A2DP_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif
int bt_a2dp_src_init(uint16_t channels, uint16_t sampling);
int bt_a2dp_src_deinit(void);
int bt_a2dp_src_stream_send(char *data, int len);
int bt_a2dp_src_stream_start(uint32_t len);
int bt_a2dp_src_stream_stop(bool drop);
bool bt_a2dp_src_is_stream_start(void);

#ifdef __cplusplus
}
#endif
#endif
