#ifndef __BLUEA_A2DP_SINK_H__
#define __BLUEA_A2DP_SINK_H__

#ifdef __cplusplus
extern "C" {
#endif
int bt_a2dp_sink_init(void);
int bt_a2dp_sink_deinit(void);
void bt_a2dp_sink_stream_cb_enable(bool enable);
#ifdef __cplusplus
}
#endif
#endif
