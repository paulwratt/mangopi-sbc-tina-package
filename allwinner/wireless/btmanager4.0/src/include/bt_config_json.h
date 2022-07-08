#ifndef __BT_CONFIG_JSON_H__
#define __BT_CONFIG_JSON_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#define BT_JSON_FILE_PATH "/etc/bluetooth/bluetooth.json"
#define PCM_MAX_DEVICE_LEN 36

struct bt_profile_cf {
    bool a2dp_sink;
    bool a2dp_source;
    bool avrcp;
    bool hfp_hf;
    bool gatt_client;
    bool gatt_server;
    bool spp_client;
    bool spp_server;
};

struct bt_a2dp_sink_cf {
    char device[PCM_MAX_DEVICE_LEN];
    int buffer_time;
    int period_time;
};

struct bt_a2dp_source_cf {
    int hci_index;
    char remote_mac[18];
    int delay;
};

struct hfp_pcm {
    int rate;
    char phone_to_dev_cap[PCM_MAX_DEVICE_LEN];
    char phone_to_dev_play[PCM_MAX_DEVICE_LEN];
    char dev_to_phone_cap[PCM_MAX_DEVICE_LEN];
    char dev_to_phone_play[PCM_MAX_DEVICE_LEN];
};

int bt_config_read_hfp(struct hfp_pcm *cf);
int bt_config_read_a2dp_source(struct bt_a2dp_source_cf *cf);
int bt_config_read_a2dp_sink(struct bt_a2dp_sink_cf *cf);
int bt_config_read_profile(struct bt_profile_cf *cf);

#ifdef __cplusplus
}
#endif

#endif
