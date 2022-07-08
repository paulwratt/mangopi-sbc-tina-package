#ifndef __BT_HFP_H
#define __BT_HFP_H

#if __cplusplus
extern "C" {
#endif

#include <stdatomic.h>
#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <glib-object.h>
#include <glib.h>
#include <stdint.h>

/* information exposed via Apple AT extension */
extern unsigned int xapl_vendor_id;
extern unsigned int xapl_product_id;
extern const char *xapl_software_version;
extern const char *xapl_product_name;
extern unsigned int xapl_features;

#define BLUETOOTH_UUID_HSP_HS "00001108-0000-1000-8000-00805F9B34FB"
#define BLUETOOTH_UUID_HSP_AG "00001112-0000-1000-8000-00805F9B34FB"
#define BLUETOOTH_UUID_HFP_HF "0000111E-0000-1000-8000-00805F9B34FB"
#define BLUETOOTH_UUID_HFP_AG "0000111F-0000-1000-8000-00805F9B34FB"

/* HFP codec IDs */
#define HFP_CODEC_UNDEFINED 0x00
#define HFP_CODEC_CVSD 0x01
#define HFP_CODEC_MSBC 0x02

/* SDP HF feature flags */
#define SDP_HFP_HF_FEAT_ECNR (1 << 0)
#define SDP_HFP_HF_FEAT_TWC (1 << 1)
#define SDP_HFP_HF_FEAT_CLI (1 << 2)
#define SDP_HFP_HF_FEAT_VREC (1 << 3)
#define SDP_HFP_HF_FEAT_VOLUME (1 << 4)
#define SDP_HFP_HF_FEAT_WBAND (1 << 5)

/* HF feature flags */
#define HFP_HF_FEAT_ECNR (1 << 0)
#define HFP_HF_FEAT_3WC (1 << 1)
#define HFP_HF_FEAT_CLI (1 << 2)
#define HFP_HF_FEAT_VOICE (1 << 3)
#define HFP_HF_FEAT_VOLUME (1 << 4)
#define HFP_HF_FEAT_ECS (1 << 5)
#define HFP_HF_FEAT_ECC (1 << 6)
#define HFP_HF_FEAT_CODEC (1 << 7)
#define HFP_HF_FEAT_HFIND (1 << 8)
#define HFP_HF_FEAT_ESCO (1 << 9)

/* AG feature flags */
#define HFP_AG_FEAT_3WC (1 << 0)
#define HFP_AG_FEAT_ECNR (1 << 1)
#define HFP_AG_FEAT_VOICE (1 << 2)
#define HFP_AG_FEAT_RING (1 << 3)
#define HFP_AG_FEAT_VTAG (1 << 4)
#define HFP_AG_FEAT_REJECT (1 << 5)
#define HFP_AG_FEAT_ECS (1 << 6)
#define HFP_AG_FEAT_ECC (1 << 7)
#define HFP_AG_FEAT_EERC (1 << 8)
#define HFP_AG_FEAT_CODEC (1 << 9)
#define HFP_AG_FEAT_HFIND (1 << 10)
#define HFP_AG_FEAT_ESCO (1 << 11)

/* Apple's extension feature flags */
#define XAPL_FEATURE_BATTERY (1 << 1)
#define XAPL_FEATURE_DOCKING (1 << 2)
#define XAPL_FEATURE_SIRI (1 << 3)
#define XAPL_FEATURE_DENOISE (1 << 4)

/**
* HFP Service Level Connection States */
enum __attribute__((packed)) hfp_slc_state {
    HFP_DISCONNECTED,
    HFP_SLC_BRSF_SET,
    HFP_SLC_BRSF_SET_OK,
    HFP_SLC_BAC_SET_OK,
    HFP_SLC_CIND_TEST,
    HFP_SLC_CIND_TEST_OK,
    HFP_SLC_CIND_GET,
    HFP_SLC_CIND_GET_OK,
    HFP_SLC_CMER_SET_OK,
    HFP_SLC_CONNECTED,
};

/**
* Initial BT accessory setup */
enum __attribute__((packed)) hfp_setup {
    HFP_SETUP_GAIN_MIC,
    HFP_SETUP_GAIN_SPK,
    HFP_SETUP_ACCESSORY_XAPL,
    HFP_SETUP_ACCESSORY_BATT,
    HFP_SETUP_COMPLETE,
};

/**
 * HFP Indicators */
enum __attribute__((packed)) hfp_ind {
    HFP_IND_NULL = 0,
    HFP_IND_SERVICE,
    HFP_IND_CALL,
    HFP_IND_CALLSETUP,
    HFP_IND_CALLHELD,
    HFP_IND_SIGNAL,
    HFP_IND_ROAM,
    HFP_IND_BATTCHG,
    __HFP_IND_MAX
};

/* no Home/Roam network available */
#define HFP_IND_SERVICE_NONE 0
/* Home/Roam network available */
#define HFP_IND_SERVICE_ACTIVE 1
/* no calls in progress */
#define HFP_IND_CALL_NONE 0
/* at least one call is in progress */
#define HFP_IND_CALL_ACTIVE 1
/* currently not in call set up */
#define HFP_IND_CALLSETUP_NONE 0
/* an incoming call process ongoing */
#define HFP_IND_CALLSETUP_IN 1
/* an outgoing call set up is ongoing */
#define HFP_IND_CALLSETUP_OUT 2
/* remote party being alerted in an outgoing call */
#define HFP_IND_CALLSETUP_OUT_ALERT 3
/* no calls held */
#define HFP_IND_CALLHELD_NONE 0
/* call on hold with other active call */
#define HFP_IND_CALLHELD_SWAP 1
/* call on hold, no active call */
#define HFP_IND_CALLHELD_HOLD 2
/* roaming is not active */
#define HFP_IND_ROAM_NONE 0
/* a roaming is active */
#define HFP_IND_ROAM_ACTIVE 1

int get_hfp_features_hf(void);
int bt_hfp_hf_init();
int bt_hfp_hf_deinit();
int bt_hfp_hf_send_at_ata(void);
int bt_hfp_hf_send_at_chup(void);
int bt_hfp_hf_send_at_atd(char *number);
int bt_hfp_hf_send_at_bldn(void);
int bt_hfp_hf_send_at_btrh(bool query, uint32_t val);
int bt_hfp_hf_send_at_vts(char code);
int bt_hfp_hf_send_at_bcc(void);
int bt_hfp_hf_send_at_cnum(void);
int bt_hfp_hf_send_at_vgs(uint32_t volume);
int bt_hfp_hf_send_at_vgm(uint32_t volume);
int bt_hfp_hf_send_at_cmd(char *cmd);

#if __cplusplus
}; // extern "C"
#endif

#endif
