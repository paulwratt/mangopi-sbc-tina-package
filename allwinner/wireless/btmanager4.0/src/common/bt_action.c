#include <stdio.h>
#include <string.h>
#include <api_action.h>

#include "common.h"
#include "platform.h"
#include "bt_log.h"
#include "bt_device.h"
#include "bt_adapter.h"
#include "bt_bluez.h"
#include "bt_avrcp.h"
#include "bt_avrcp_tg.h"
#include "bt_config_json.h"
#include "bt_a2dp_sink.h"
#include "bt_a2dp_source.h"
#include "bt_hfp.h"
#include "bt_agent.h"
#include "bt_rfcomm.h"
#include "bt_manager.h"
#include "bt_action.h"
#include "bt_bluez_signals.h"

#define WAIT_HCI_COUNT_MAX 6

dev_list_t *connected_devices = NULL;
static btmg_adapter_state_t btmg_adapter_state = BTMG_ADAPTER_OFF;

int _check_bt_is_on(void **call_args, void **cb_args)
{
    bool is_on;

    if (btmg_adapter_state == BTMG_ADAPTER_ON) {
        is_on = true;
    } else {
        BTMG_WARNG("bt is not turned on");
        is_on = false;
    }
    memcpy(cb_args[0], (void *)(&is_on), sizeof(bool));

    return 0;
}

int _bt_manager_set_loglevel(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    btmg_log_level_t log_level = *(btmg_log_level_t *)call_args[0];
    ret = btmg_set_debug_level(log_level);

    RETURN_INT(ret);
}

int _bt_manager_get_loglevel(void **call_args, void **cb_args)
{
    int level = 0;

    level = btmg_get_debug_level();

    if (level >= MSG_DEBUG)
        level = BTMG_LOG_LEVEL_DEBUG;

    RETURN_INT(level);
}

int _bt_manager_set_ex_debug_mask(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    int mask = *(int *)call_args[0];
    ret = btmg_set_ex_debug_mask((enum ex_debug_mask)mask);

    RETURN_INT(ret);
}

int _bt_manager_get_ex_debug_mask(void **call_args, void **cb_args)
{
    int mask;

    mask = btmg_get_ex_debug_mask();

    RETURN_INT(mask);
}

int _bt_manager_enable_profile(void **call_args, void **cb_args)
{
    int profile = *(int *)call_args[0];

    if (bt_pro_info == NULL) {
        RETURN_INT(BT_ERROR_NULL_VARIABLE);
    }

    BTMG_DEBUG("profile mask:%d", profile);

    if (profile & BTMG_A2DP_SINK_ENABLE) {
        BTMG_DEBUG("a2dp sink profile enable.");
        bt_pro_info->is_a2dp_sink_enabled = true;
    }
    if (profile & BTMG_A2DP_SOUCE_ENABLE) {
        BTMG_DEBUG("a2dp source profile enable.");
        bt_pro_info->is_a2dp_source_enabled = true;
    }
    if (profile & BTMG_AVRCP_ENABLE) {
        BTMG_DEBUG("avrcp profile enable.");
        bt_pro_info->is_avrcp_enabled = true;
    }
    if (profile & BTMG_HFP_HF_ENABLE) {
        BTMG_DEBUG("hfp client profile enable.");
        bt_pro_info->is_hfp_hf_enabled = true;
    }
    if (profile & BTMG_GATT_SERVER_ENABLE) {
        BTMG_DEBUG("gatt profile enable.");
        bt_pro_info->is_gatts_enabled = true;
    }
    if (profile & BTMG_GATT_CLIENT_ENABLE) {
        BTMG_DEBUG("gatt profile enable.");
        bt_pro_info->is_gattc_enabled = true;
    }
    if (profile & BTMG_SPP_ENABLE) {
        BTMG_DEBUG("spp profile enable.");
        bt_pro_info->is_sppc_enabled = true;
        bt_pro_info->is_spps_enabled = true;
    }

    RETURN_INT(BT_OK);
}

static int bt_init_profile(void)
{
    btmg_profile_info_t profie_cmp;

    if (bt_pro_info == NULL) {
        BTMG_ERROR("BT manager is not preinit");
        goto fail;
    }

    memset(&profie_cmp, 0, sizeof(btmg_profile_info_t));
    if (memcmp(bt_pro_info, &profie_cmp, sizeof(btmg_profile_info_t)) == 0) {
        BTMG_ERROR("all profiles are disable.");
        goto fail;
    }

    if (get_process_run_state("bluealsa", 8) == 0) {
        system("killall -9 bluealsa");
        ms_sleep(50);
    }
    if (bt_pro_info->is_a2dp_sink_enabled || bt_pro_info->is_a2dp_source_enabled) {
        if (get_process_run_state("bluealsa", 8) != 0) {
            char para[128] = "bluealsa ";
            if (bt_pro_info->is_a2dp_sink_enabled) {
                strcat(para, "-p a2dp-sink ");
            }
            if (bt_pro_info->is_a2dp_source_enabled) {
                strcat(para, "-p a2dp-source ");
            }

            strcat(para, "&");
            BTMG_DEBUG("start bluealsa:%s", para);
            system(para);
            ms_sleep(1000);
        }
        if (bt_pro_info->is_a2dp_sink_enabled) {
            if (bt_a2dp_sink_init() == -1) {
                BTMG_ERROR("a2dp sink init fail");
                goto fail;
            }
        }
    }

    if (bt_pro_info->is_hfp_hf_enabled) {
        if (bt_hfp_hf_init() == -1) {
            BTMG_ERROR("hfp init fail");
            goto fail;
        }
    }

    if (bt_pro_info->is_sppc_enabled || bt_pro_info->is_spps_enabled) {
        if (rfcomm_init() == -1) {
            BTMG_ERROR("spp init fail");
            goto fail;
        }
    }

    return BT_OK;

fail:

    return BT_ERROR;
}

static int bt_deinit_profile(void)
{
    if (bt_pro_info->is_hfp_hf_enabled) {
        if (bt_hfp_hf_deinit() < 0)
            goto fail;
    }
    if (bt_pro_info->is_sppc_enabled) {
        if (rfcomm_deinit() < 0)
            goto fail;
    }
    if (bt_pro_info->is_a2dp_sink_enabled) {
        if (bt_a2dp_sink_deinit() < 0)
            goto fail;
    }

    if (get_process_run_state("bluealsa", 8) == 0) {
        system("killall -9 bluealsa");
        if (access("/tmp/run/bluealsa/hci0", F_OK) == 0)
            system("rm -rf /tmp/run/bluealsa");
        ms_sleep(100);
    }

    return BT_OK;

fail:

    BTMG_ERROR("deinit profile fail");
    return BT_ERROR;
}

int _bt_manager_enable(void **call_args, void **cb_args)
{
    bool enable = *(bool *)call_args[0];

    BTMG_INFO("btmanager version: %s, builed time: %s-%s", BTMGVERSION, __DATE__, __TIME__);
    BTMG_INFO("enable state: %d, now bt adapter state : %d", enable, btmg_adapter_state);

    if (enable == true) {
        if (btmg_adapter_state == BTMG_ADAPTER_ON) {
            BTMG_WARNG("BT is already ON!");
            return BT_OK;
        }
        if (btmg_adapter_state == BTMG_ADAPTER_TURNING_ON) {
            BTMG_WARNG("BT is already in process of TURNING ON!");
            return BT_ERROR_IN_PROCESS;
        }
        if (btmg_adapter_state == BTMG_ADAPTER_TURNING_OFF) {
            BTMG_WARNG("BT is already in process of TURNING OFF!");
            return BT_ERROR_IN_PROCESS;
        }

        btmg_adapter_state = BTMG_ADAPTER_TURNING_ON;
        if (btmg_cb_p && btmg_cb_p->btmg_adapter_cb.adapter_state_cb)
            btmg_cb_p->btmg_adapter_cb.adapter_state_cb(BTMG_ADAPTER_TURNING_ON);

        if (bt_platform_init() < 0) {
            BTMG_ERROR("BT turn on fail");
            if (btmg_cb_p && btmg_cb_p->btmg_adapter_cb.adapter_state_cb)
                btmg_cb_p->btmg_adapter_cb.adapter_state_cb(BTMG_ADAPTER_OFF);
            btmg_adapter_state = BTMG_ADAPTER_OFF;
            goto fail;
        }
        if (bt_bluez_init() < 0) {
            bt_platform_deinit();
            BTMG_ERROR("BT turn on fail");
            if (btmg_cb_p && btmg_cb_p->btmg_adapter_cb.adapter_state_cb)
                btmg_cb_p->btmg_adapter_cb.adapter_state_cb(BTMG_ADAPTER_OFF);
            btmg_adapter_state = BTMG_ADAPTER_OFF;
            goto fail;
        }
        if (bt_init_profile() < 0) {
            bt_platform_deinit();
            bt_bluez_deinit();
            BTMG_ERROR("BT turn on fail");
            if (btmg_cb_p && btmg_cb_p->btmg_adapter_cb.adapter_state_cb)
                btmg_cb_p->btmg_adapter_cb.adapter_state_cb(BTMG_ADAPTER_OFF);
            btmg_adapter_state = BTMG_ADAPTER_OFF;
            goto fail;
        }

        btmg_adapter_state = bt_adapter_get_power_state();
        if (btmg_adapter_state == BTMG_ADAPTER_ON) {
            if (btmg_cb_p && btmg_cb_p->btmg_adapter_cb.adapter_state_cb)
                btmg_cb_p->btmg_adapter_cb.adapter_state_cb(BTMG_ADAPTER_ON);
        } else {
            BTMG_ERROR("BT turn on fail");
            if (btmg_cb_p && btmg_cb_p->btmg_adapter_cb.adapter_state_cb)
                btmg_cb_p->btmg_adapter_cb.adapter_state_cb(BTMG_ADAPTER_OFF);
            btmg_adapter_state = BTMG_ADAPTER_OFF;
            goto fail;
        }

    } else {
        if (btmg_adapter_state == BTMG_ADAPTER_OFF) {
            BTMG_WARNG("BT is already OFF!");
            RETURN_INT(0);
        } else if (btmg_adapter_state == BTMG_ADAPTER_TURNING_OFF) {
            BTMG_WARNG("BT is already in process of TURNING_OFF!");
            goto fail;
        } else if (btmg_adapter_state == BTMG_ADAPTER_TURNING_ON) {
            BTMG_WARNG("BT is already in process of TURNING ON!");
            goto fail;
        } else if (btmg_adapter_state != BTMG_ADAPTER_ON) {
            BTMG_WARNG("BT state is not right,try later!");
            goto fail;
        }
        btmg_adapter_state = BTMG_ADAPTER_TURNING_OFF;
        if (btmg_cb_p && btmg_cb_p->btmg_adapter_cb.adapter_state_cb)
            btmg_cb_p->btmg_adapter_cb.adapter_state_cb(BTMG_ADAPTER_TURNING_OFF);
        if (bt_agent_unregister() < 0)
            BTMG_ERROR("unregister agent fail");
        if (bt_deinit_profile() < 0)
            BTMG_ERROR("profile deinit fail");
        if (bt_bluez_deinit() < 0)
            BTMG_ERROR("bluez deinit fail");
        bt_platform_deinit();
        sleep(1);
        btmg_adapter_state = BTMG_ADAPTER_OFF;
        if (btmg_cb_p && btmg_cb_p->btmg_adapter_cb.adapter_state_cb)
            btmg_cb_p->btmg_adapter_cb.adapter_state_cb(BTMG_ADAPTER_OFF);
    }

    RETURN_INT(BT_OK);

fail:
    RETURN_INT(BT_ERROR);
}

int _bt_manager_set_default_profile(void **call_args, void **cb_args)
{
    /*BT is configed to enable defaultly*/
    bool is_default = *(bool *)call_args[0];

    if (is_default) {
        struct bt_profile_cf profile_cf;
        if (bt_config_read_profile(&profile_cf) == -1) {
            BTMG_ERROR("read bt config failed");
            RETURN_INT(BT_ERROR);
        }
        /*enable a2dp_sink and disable HFP as default*/
        bt_pro_info->is_a2dp_sink_enabled = profile_cf.a2dp_sink;
        bt_pro_info->is_a2dp_source_enabled = profile_cf.a2dp_source;
        bt_pro_info->is_avrcp_enabled = profile_cf.avrcp;
        bt_pro_info->is_hfp_hf_enabled = profile_cf.hfp_hf;
        bt_pro_info->is_sppc_enabled = profile_cf.spp_client;
        bt_pro_info->is_sppc_enabled = profile_cf.spp_server;

        BTMG_INFO("enable default profile from bt config");
    }

    RETURN_INT(BT_OK);
}

int _bt_manager_set_scan_mode(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    btmg_scan_mode_t mode = *(btmg_scan_mode_t *)call_args[0];

    if (BTMG_SCAN_MODE_NONE == mode) {
        BTMG_DEBUG("Scan mode:Discoverable(no),Connectable(no)");
        ret = bt_adapter_set_scan_mode("nopiscan");
    } else if (BTMG_SCAN_MODE_CONNECTABLE == mode) {
        BTMG_DEBUG("Scan mode:Discoverable(no),Connectable(yes)");
        return bt_adapter_set_scan_mode("pscan");
    } else if (BTMG_SCAN_MODE_CONNECTABLE_DISCOVERABLE == mode) {
        BTMG_DEBUG("Scan mode:Discoverable(yes),Connectable(yes)");
        ret = bt_adapter_set_scan_mode("piscan");
    }

    RETURN_INT(ret);
}

int _bt_manager_scan_filter(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    btmg_scan_filter_t *filter = (btmg_scan_filter_t *)call_args[0];

    if (bt_adapter_is_scanning() == true) {
        BTMG_WARNG("Currently scanning, please stop and try again");
        RETURN_INT(BT_ERROR_IN_PROCESS);
    }
    ret = bt_adapter_scan_filter(filter);

    RETURN_INT(BT_OK);
}

int _bt_manager_start_scan(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    if (bt_adapter_is_scanning() == true) {
        BTMG_WARNG("scan is already started");
        RETURN_INT(-1);
    }
    ret = bt_adapter_start_scan();

    RETURN_INT(ret);
}

int _bt_manager_stop_scan(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    if (bt_adapter_is_scanning() == false) {
        BTMG_WARNG("scan is already stopped");
        RETURN_INT(ret);
    }

    ret = bt_adapter_stop_scan();

    RETURN_INT(ret);
}

int _bt_manager_is_scanning(void **call_args, void **cb_args)
{
    bool ret;

    ret = bt_adapter_is_scanning();
    memcpy(cb_args[0], (void *)(&ret), sizeof(bool));

    return 0;
}

int _bt_manager_pair(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    char *addr = (char *)call_args[0];
    ret = bt_device_pair(addr);

    RETURN_INT(ret);
}

int _bt_manager_unpair(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    char *addr = (char *)call_args[0];
    ret = bt_remove_device(addr, true);

    RETURN_INT(ret);
}

int _bt_manager_get_paired_devices(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    int count = 0;

    btmg_bt_device_t **dev_list = (btmg_bt_device_t **)call_args[0];

    ret = bt_get_paired_devices(dev_list, &count);
    memcpy(cb_args[1], (void *)&count, sizeof(int));

    RETURN_INT(ret);
}

int _bt_manager_free_paired_devices(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    btmg_bt_device_t *dev_list = (btmg_bt_device_t *)call_args[0];
    int count = *(int *)call_args[1];
    ret = bt_free_paired_devices(dev_list, count);

    RETURN_INT(ret);
}

int _bt_manager_get_adapter_state(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    ret = btmg_adapter_state;

    RETURN_INT(ret);
}

int _bt_manager_get_adapter_name(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    char name[256] = { 0 };

    ret = bt_adapter_get_name(name);
    strcpy(cb_args[1], (char *)name);

    RETURN_INT(ret);
}

int _bt_manager_set_adapter_name(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    const char *name = (const char *)call_args[0];
    ret = bt_adapter_set_alias(name);

    RETURN_INT(ret);
}

int _bt_manager_get_adapter_address(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    char addr[18] = { 0 };

    ret = bt_adapter_get_address(addr);
    strcpy(cb_args[1], (char *)addr);

    RETURN_INT(ret);
}

int _bt_manager_get_device_name(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    char name[256] = { 0 };

    const char *addr = (const char *)call_args[0];
    ret = bt_device_get_name(addr, name);

    strcpy(cb_args[1], (char *)name);

    RETURN_INT(ret);
}

int _bt_manager_connect(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    const char *addr = (const char *)call_args[0];
    ret = bt_device_connect(addr);

    RETURN_INT(ret);
}

int _bt_manager_disconnect(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    const char *addr = (const char *)call_args[0];
    ret = bt_device_disconnect(addr);

    RETURN_INT(ret);
}

int _bt_manager_device_is_connected(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    const char *addr = (const char *)call_args[0];
    ret = bt_device_is_connected(addr);
    memcpy(cb_args[0], (void *)(&ret), sizeof(bool));

    return 0;
}

int _bt_manager_get_connection_state(void)
{
    //TODO:for all profile device:
    return 0;
}

int _bt_manager_set_page_timeout(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    int slots = *(int *)call_args[0];
    ret = bt_adapter_set_page_timeout(slots);

    RETURN_INT(ret);
}

int _bt_manager_remove_device(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    const char *addr = (const char *)call_args[0];
    ret = bt_remove_device(addr, false);

    RETURN_INT(ret);
}

int _bt_manager_a2dp_src_init(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    uint16_t channels = *(uint16_t *)call_args[0];
    uint16_t sampling = *(uint16_t *)call_args[1];

    ret = bt_a2dp_src_init(channels, sampling);
    if (ret < 0) {
        BTMG_ERROR("a2dp src init fail");
        RETURN_INT(ret);
    }
    ret = bluez_avrcp_tg_init();
    if (ret < 0) {
        BTMG_ERROR("avrcp tg init fail");
    }

    RETURN_INT(ret);
}

int _bt_manager_a2dp_src_deinit(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    ret = bluez_avrcp_tg_deinit();
    if (ret < 0) {
        BTMG_ERROR("avrcp tg deinit fail");
        RETURN_INT(ret);
    }
    ret = bt_a2dp_src_deinit();
    if (ret < 0) {
        BTMG_ERROR("a2dp src deinit fail");
    }

    RETURN_INT(ret);
}

int _bt_manager_a2dp_src_stream_start(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    uint32_t len = *(uint32_t *)call_args[0];
    ret = bt_a2dp_src_stream_start(len);
    if (ret < 0) {
        BTMG_ERROR("a2dp src stream start fail");
    }

    RETURN_INT(ret);
}

int _bt_manager_a2dp_src_stream_stop(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    bool drop = *(bool *)call_args[0];

    ret = bt_a2dp_src_stream_stop(drop);

    RETURN_INT(ret);
}

int _bt_manager_a2dp_src_stream_send(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    char *data = (char *)call_args[0];
    int len = *(int *)call_args[1];

    ret = bt_a2dp_src_stream_send(data, len);

    RETURN_INT(ret);
}

int _bt_manager_a2dp_src_is_stream_start(void **call_args, void **cb_args)
{
    bool start = false;

    start = bt_a2dp_src_is_stream_start();
    memcpy(cb_args[0], (void *)(&start), sizeof(bool));

    return BT_OK;
}

int _bt_manager_avrcp_command(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    char *addr = (char *)call_args[0];
    btmg_avrcp_command_t command = *(btmg_avrcp_command_t *)call_args[1];

    if (btmg_string_is_bdaddr(addr)) {
        switch (command) {
        case BTMG_AVRCP_PLAY:
            bluez_avrcp_play();
            break;
        case BTMG_AVRCP_PAUSE:
            bluez_avrcp_pause();
            break;
        case BTMG_AVRCP_STOP:
            bluez_avrcp_pause();
            break;
        case BTMG_AVRCP_FASTFORWARD:
            bluez_avrcp_fastforward();
            break;
        case BTMG_AVRCP_REWIND:
            bluez_avrcp_rewind();
            break;
        case BTMG_AVRCP_FORWARD:
            bluez_avrcp_next();
            break;
        case BTMG_AVRCP_BACKWARD:
            bluez_avrcp_previous();
            break;
        case BTMG_AVRCP_VOL_UP:
            break;
        case BTMG_AVRCP_VOL_DOWN:
            break;
        default:
            BTMG_ERROR("Unsupported avrcp command!");
            ret = BT_ERROR;
        }
    } else {
        BTMG_ERROR("address is invalid");
        ret = BT_ERROR;
    }

    RETURN_INT(ret);
}

int _bt_manager_a2dp_set_vol(void **call_args, void **cb_args)
{
    char vol[4] = { 0 };
    char cmd_para[64] = "amixer -D bluealsa sset '";
    char remote_name[128] = { 0 };
    char control_name[24] = { 0 };
    bool is_a2dp_device = false;
    dev_node_t *dev_node = NULL;

    int vol_value = *(int *)call_args[0];

    dev_node = connected_devices->head;
    while (dev_node != NULL) {
        if (dev_node->profile & BTMG_REMOTE_DEVICE_A2DP) {
            memcpy(remote_name, dev_node->dev_name, sizeof(remote_name));
            is_a2dp_device = true;
            break;
        }
        dev_node = dev_node->next;
    }

    if (is_a2dp_device == false) {
        BTMG_ERROR("No connected a2dp device");
        RETURN_INT(-1);
    }

    string_left_cut(control_name, remote_name, 20);
    if (vol_value > 100)
        vol_value = 100;

    if (vol_value < 0)
        vol_value = 0;

    sprintf(vol, "%d", vol_value);
    strcat(cmd_para, control_name);
    strcat(cmd_para, " - A2DP' ");
    strcat(cmd_para, vol);
    strcat(cmd_para, "%");
    system(cmd_para);

    RETURN_INT(BT_OK);
}

int _bt_manager_a2dp_get_vol(void **call_args, void **cb_args)
{
    int vol_value = 0;
    char vol[4] = { 0 };
    char cmd_para[64] = "amixer -D bluealsa sget '";
    char remote_name[128] = { 0 };
    char control_name[24] = { 0 };
    bool is_a2dp_device = false;
    dev_node_t *dev_node = NULL;

    dev_node = connected_devices->head;
    while (dev_node != NULL) {
        if (dev_node->profile & BTMG_REMOTE_DEVICE_A2DP) {
            memcpy(remote_name, dev_node->dev_name, sizeof(remote_name));
            is_a2dp_device = true;
            break;
        }
        dev_node = dev_node->next;
    }

    if (is_a2dp_device == false) {
        BTMG_ERROR("No connected a2dp device");
        RETURN_INT(BT_ERROR);
    }

    string_left_cut(control_name, remote_name, 20);
    strcat(cmd_para, control_name);
    strcat(cmd_para, " - A2DP'");
    get_vol_from_bluealsa(cmd_para, vol);

    vol_value = atoi(vol);

    RETURN_INT(vol_value);
}

/*
 1 slot = 0.625ms
 timeout = slots * 0.625ms;
*/
int _bt_manager_set_link_supervision_timeout(void **call_args, void **cb_args)
{
    char cmd_para[64] = "hcitool lst ";
    char timeout[5] = { 0 };

    const char *addr = (const char *)call_args[0];
    int slots = *(int *)call_args[1];

    if (slots <= 0 || slots > 65535) {
        BTMG_ERROR("slots value is invalid,must be between 1-65535.");
        RETURN_INT(BT_ERROR_INVALID_ARGS);
    }

    sprintf(timeout, "%d", slots);
    strcat(cmd_para, addr);
    strcat(cmd_para, " ");
    strcat(cmd_para, timeout);
    BTMG_DEBUG("link supervision timeout cmd:%s", cmd_para);
    system(cmd_para);

    RETURN_INT(BT_OK);
}

int _bt_manager_hfp_hf_send_at_ata(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    ret = bt_hfp_hf_send_at_ata();

    RETURN_INT(ret);
}

int _bt_manager_hfp_hf_send_at_chup(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    ret = bt_hfp_hf_send_at_chup();

    RETURN_INT(ret);
}

int _bt_manager_hfp_hf_send_at_atd(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    char *number = (char *)call_args[0];

    ret = bt_hfp_hf_send_at_atd(number);

    RETURN_INT(ret);
}

int _bt_manager_hfp_hf_send_at_bldn(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    ret = bt_hfp_hf_send_at_bldn();

    RETURN_INT(ret);
}

int _bt_manager_hfp_hf_send_at_btrh(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    bool query = *(bool *)call_args[0];
    uint32_t val = *(uint32_t *)call_args[1];

    ret = bt_hfp_hf_send_at_btrh(query, val);

    RETURN_INT(ret);
}

int _bt_manager_hfp_hf_send_at_vts(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    char code = *(char *)call_args[0];

    ret = bt_hfp_hf_send_at_vts(code);

    RETURN_INT(ret);
}

int _bt_manager_hfp_hf_send_at_bcc(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    ret = bt_hfp_hf_send_at_bcc();

    RETURN_INT(ret);
}

int _bt_manager_hfp_hf_send_at_cnum(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    ret = bt_hfp_hf_send_at_cnum();

    RETURN_INT(ret);
}

int _bt_manager_hfp_hf_send_at_vgs(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    uint32_t volume = *(uint32_t *)call_args[0];

    ret = bt_hfp_hf_send_at_vgs(volume);
}

int _bt_manager_hfp_hf_send_at_vgm(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    uint32_t volume = *(uint32_t *)call_args[0];

    ret = bt_hfp_hf_send_at_vgm(volume);

    RETURN_INT(ret);
}

int _bt_manager_hfp_hf_send_at_cmd(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    char *cmd = (char *)call_args[0];

    ret = bt_hfp_hf_send_at_cmd(cmd);

    RETURN_INT(ret);
}

int _bt_manager_agent_set_io_capability(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    btmg_io_capability_t io_cap = *(btmg_io_capability_t *)call_args[0];

    if (bt_agent_register(io_cap) == 0) {
        ret = bt_agent_set_default();
    }

    RETURN_INT(ret);
}

int _bt_manager_agent_send_pincode(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    void *handle = (void *)call_args[0];
    char *pincode = (char *)call_args[1];

    ret = bt_agent_send_pincode(handle, pincode);

    RETURN_INT(ret);
}

int _bt_manager_agent_send_passkey(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    void *handle = (void *)call_args[0];
    unsigned int passkey = *(unsigned int *)call_args[1];

    ret = bt_agent_send_passkey(handle, passkey);

    RETURN_INT(ret);
}

int _bt_manager_agent_send_pair_error(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    void *handle = (void *)call_args[0];
    btmg_pair_request_error_t e = *(btmg_pair_request_error_t *)call_args[1];
    const char *err_msg = (const char *)call_args[2];

    ret = bt_agent_send_error(handle, e, err_msg);

    RETURN_INT(ret);
}

int _bt_manager_agent_pair_send_empty_response(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    void *handle = (void *)call_args[0];

    ret = bt_agent_send_empty_response(handle);

    RETURN_INT(ret);
}

int _bt_manager_spp_client_connect(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    const char *dst = (const char *)call_args[0];

    if (bt_device_is_paired(dst) != true) {
        BTMG_ERROR("Please pair the device %s first", dst);
        RETURN_INT(BT_ERROR);
    }
    if (bt_device_is_connected(dst) == true) {
        BTMG_ERROR("Device %s is connected", dst);
        RETURN_INT(BT_ERROR);
    }
    ret = spp_client_connect(0, dst);

    RETURN_INT(ret);
}

int _bt_manager_spp_client_send(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    char *data = (char *)call_args[0];
    uint32_t len = *(uint32_t *)call_args[1];

    ret = spp_client_send(data, len);

    RETURN_INT(ret);
}

int _bt_manager_spp_client_disconnect(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    const char *dst = (const char *)call_args[0];

    if (bt_device_is_connected(dst) == false) {
        BTMG_ERROR("Device %s is not connected", dst);
        RETURN_INT(-1);
    }

    ret = spp_client_disconnect(0, dst);

    RETURN_INT(ret);
}

int _bt_manager_spp_service_accept(void **call_args, void **cb_args)
{
    int ret = BT_OK;

    ret = spp_service_accept(20);

    RETURN_INT(ret);
}

int _bt_manager_spp_service_send(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    char *data = (char *)call_args[0];
    uint32_t len = *(uint32_t *)call_args[1];

    ret = spp_service_send(data, len);

    RETURN_INT(ret);
}

int _bt_manager_spp_service_disconnect(void **call_args, void **cb_args)
{
    int ret = BT_OK;
    const char *dst = (const char *)call_args[0];

    if (bt_device_is_connected(dst) == false) {
        BTMG_ERROR("Device %s is not connected", dst);
        RETURN_INT(BT_ERROR);
    }

    ret = spp_service_disconnect();
    RETURN_INT(BT_OK);
}

act_func_t bt_table[] = {
    [BT_CHECK_STATE] = { _check_bt_is_on, "check_bt_is_on" },
    [BT_GET_LOGLEVEL] = { _bt_manager_get_loglevel, "get_loglevel" },
    [BT_SET_EX_DEBUG] = { _bt_manager_set_ex_debug_mask, "set_ex_debug_mask" },
    [BT_GET_EX_DEBUG] = { _bt_manager_get_ex_debug_mask, "get_ex_debug_mask" },
    [BT_ENABLE_PROFILE] = { _bt_manager_enable_profile, "enable_profile" },
    [BT_BTMG_ENABLE] = { _bt_manager_enable, "enable" },
    [BT_SET_DEFAULT_PROFILE] = { _bt_manager_set_default_profile, "set_default_profile" },
    [BT_SET_SCAN_MODE] = { _bt_manager_set_scan_mode, "set_scan_mode" },
    [BT_SET_SCAN_FILTER] = { _bt_manager_scan_filter, "scan_filter" },
    [BT_START_SCAN] = { _bt_manager_start_scan, "start_scan" },
    [BT_STOP_SCAN] = { _bt_manager_stop_scan, "stop_scan" },
    [BT_IS_SCAN] = { _bt_manager_is_scanning, "is_scanning" },
    [BT_PAIR] = { _bt_manager_pair, "pair" },
    [BT_UNPAIR] = { _bt_manager_unpair, "unpair" },
    [BT_GET_PAIRED_DEV] = { _bt_manager_get_paired_devices, "get_paired_devices" },
    [BT_FREE_PAIRED_DEV] = { _bt_manager_free_paired_devices, "free_paired_devices" },
    [BT_GET_ADATER_STATE] = { _bt_manager_get_adapter_state, "get_adater_state" },
    [BT_GET_ADATER_NAME] = { _bt_manager_get_adapter_name, "get_adapter_name" },
    [BT_SET_ADATER_NAME] = { _bt_manager_set_adapter_name, "set_adapter_name" },
    [BT_GET_ADATER_ADDRESS] = { _bt_manager_get_adapter_address, "get_adapter_address" },
    [BT_GET_DEVICE_NAME] = { _bt_manager_get_device_name, "get_device_name" },
    [BT_DEV_CONNECT] = { _bt_manager_connect, "connect" },
    [BT_DEV_DISCONNECT] = { _bt_manager_disconnect, "disconnect" },
    [BT_IS_CONNECTED] = { _bt_manager_device_is_connected, "device_is_connected" },
    [BT_SET_PAGE_TO] = { _bt_manager_set_page_timeout, "set_page_timeout" },
    [BT_REMOVE_DEV] = { _bt_manager_remove_device, "remove_device" },

    [BT_A2DP_SRC_INIT] = { _bt_manager_a2dp_src_init, "a2dp_src_init" },
    [BT_A2DP_SRC_DEINIT] = { _bt_manager_a2dp_src_deinit, "a2dp_src_deinit" },
    [BT_A2DP_SRC_STREAM_START] = { _bt_manager_a2dp_src_stream_start, "a2dp_src_stream_start" },
    [BT_A2DP_SRC_STREAM_STOP] = { _bt_manager_a2dp_src_stream_stop, "a2dp_src_stream_stop" },
    [BT_A2DP_SRC_STREAM_SEND] = { _bt_manager_a2dp_src_stream_send, "a2dp_src_stream_send" },
    [BT_A2DP_SRC_IS_STREAM_START] = { _bt_manager_a2dp_src_is_stream_start,
                                      "a2dp_src_is_stream_start" },
    [BT_A2DP_SET_VOL] = { _bt_manager_a2dp_set_vol, "a2dp_set_vol" },
    [BT_A2DP_GET_VOL] = { _bt_manager_a2dp_get_vol, "a2dp_get_vol" },
    [BT_AVRCP_COMMAND] = { _bt_manager_avrcp_command, "avrcp_command" },
    [BT_SET_LINK_SUP_TO] = { _bt_manager_set_link_supervision_timeout,
                             "set_link_supervision_timeout" },

    [BT_HFP_AT_ATA] = { _bt_manager_hfp_hf_send_at_ata, "hfp_hf_send_at_ata" },
    [BT_HFP_AT_CHUP] = { _bt_manager_hfp_hf_send_at_chup, "hfp_hf_send_at_chup" },
    [BT_HFP_AT_ATD] = { _bt_manager_hfp_hf_send_at_atd, "hfp_hf_send_at_atd" },
    [BT_HFP_AT_BLDN] = { _bt_manager_hfp_hf_send_at_bldn, "hfp_hf_send_at_bldn" },
    [BT_HFP_AT_BTRH] = { _bt_manager_hfp_hf_send_at_btrh, "hfp_hf_send_at_btrh" },
    [BT_HFP_AT_VTS] = { _bt_manager_hfp_hf_send_at_vts, "hfp_hf_send_at_vts" },
    [BT_HFP_AT_BCC] = { _bt_manager_hfp_hf_send_at_bcc, "_hfp_hf_send_at_bcc" },
    [BT_HFP_AT_CNUM] = { _bt_manager_hfp_hf_send_at_cnum, "hfp_hf_send_at_cnum" },
    [BT_HFP_AT_VGS] = { _bt_manager_hfp_hf_send_at_vgs, "hf_send_at_vgs" },
    [BT_HFP_AT_VGM] = { _bt_manager_hfp_hf_send_at_vgm, "hfp_hf_send_at_vgm" },
    [BT_HFP_AT_CMD] = { _bt_manager_hfp_hf_send_at_cmd, "hfp_hf_send_at_cmd" },
    [BT_SPP_CLI_CONNECT] = { _bt_manager_spp_client_connect, "spp_client_connect" },
    [BT_SPP_CLI_SEND] = { _bt_manager_spp_client_send, "spp_client_send" },
    [BT_SPP_CLI_DISCONNECT] = { _bt_manager_spp_client_disconnect, "spp_client_disconnect" },
    [BT_SPP_SVC_ACCEPT] = { _bt_manager_spp_service_accept, "spp_service_accept" },
    [BT_SPP_SVC_SEND] = { _bt_manager_spp_service_send, "spp_service_send" },
    [BT_SPP_SVC_DISCONNECT] = { _bt_manager_spp_service_disconnect, "spp_service_disconnect" },
};
