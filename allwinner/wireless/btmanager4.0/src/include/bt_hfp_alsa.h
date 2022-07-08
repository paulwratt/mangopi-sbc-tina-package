#ifndef __BT_HFP_ALSA_H
#define __BT_HFP_ALSA_H
#if __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <bt_log.h>

enum hfp_thread {
    PTHONE_STREAM_PLAY = 0,
    DEV_STREAM_RECORD,
};

struct hfp_hf_worker {
    bool enable;
    enum hfp_thread thread_flag;
    struct pcm_config *read_pcm;
    struct pcm_config *write_pcm;
    pthread_t thread;
    bool thread_loop;
    uint8_t read_write_frams;
};

int bt_hfp_alsa_init(void);
void bt_hfp_alsa_deinit(void);
void bt_hfp_hf_pcm_start(void);
void bt_hfp_hf_pcm_stop(void);

#if __cplusplus
}; // extern "C"
#endif

#endif
