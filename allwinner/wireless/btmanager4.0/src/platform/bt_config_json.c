/*
* Copyright (c) 2019-2020 Allwinner Technology Co., Ltd. ALL rights reserved.
* Author: laumy liumingyuan@allwinnertech.com
* Date: 2019.11.26
* Description:bluetooth config handle.
*/

#include <json-c/json.h>
#include <stdbool.h>
#include <string.h>
#include "bt_config_json.h"
#include "bt_log.h"

int bt_config_read_profile(struct bt_profile_cf *cf)
{
    struct json_object *json_bt = NULL;
    struct json_object *json_profile = NULL;

    if (cf == NULL)
        return -1;

    memset(cf, 0, sizeof(struct bt_profile_cf));

    json_bt = json_object_from_file(BT_JSON_FILE_PATH);
    if (!json_bt)
        return -1;

    json_profile = json_object_object_get(json_bt, "profile");
    if (!json_profile)
        return -1;

    cf->a2dp_sink = json_object_get_int(json_object_object_get(json_profile, "a2dp_sink")),
    cf->a2dp_source = json_object_get_int(json_object_object_get(json_profile, "a2dp_source")),
    cf->avrcp = json_object_get_int(json_object_object_get(json_profile, "avrcp")),
    cf->hfp_hf = json_object_get_int(json_object_object_get(json_profile, "hfp_hf")),
    cf->gatt_client = json_object_get_int(json_object_object_get(json_profile, "gatt_client")),
    cf->gatt_server = json_object_get_int(json_object_object_get(json_profile, "gatt_server")),

    BTMG_DEBUG("profile:a2dp_sink:%d,a2dp_source:%d,avrcp:%d\n", cf->a2dp_sink, cf->a2dp_source,
               cf->avrcp);

    BTMG_DEBUG("profile:hfp_hf:%d,gatt_client:%d,gatt_server:%d", cf->hfp_hf, cf->gatt_client,
               cf->gatt_server);
    return 0;
}

int bt_config_read_a2dp_sink(struct bt_a2dp_sink_cf *cf)
{
    struct json_object *json_bt = NULL;
    struct json_object *js_a2dp_sink = NULL;
    struct json_object *js_dev = NULL;
    struct json_object *js_buffer_time = NULL;
    struct json_object *js_period_time = NULL;
    int len;
    if (cf == NULL)
        return -1;
    memset(cf, 0, sizeof(struct bt_a2dp_sink_cf));

    json_bt = json_object_from_file(BT_JSON_FILE_PATH);
    if (!json_bt)
        return -1;

    js_a2dp_sink = json_object_object_get(json_bt, "a2dp_sink");

    if (!js_a2dp_sink)
        return -1;

    js_dev = json_object_object_get(js_a2dp_sink, "device");
    if (!js_dev)
        return -1;

    js_buffer_time = json_object_object_get(js_a2dp_sink, "buffer_time");
    if (!js_buffer_time)
        return -1;

    js_period_time = json_object_object_get(js_a2dp_sink, "period_time");
    if (!js_period_time)
        return -1;

    len = json_object_get_string_len(js_dev);
    if (len > PCM_MAX_DEVICE_LEN) {
        BTMG_ERROR("device name is too loog(%d)", len);
        return -1;
    }

    strncpy(cf->device, json_object_get_string(js_dev), len);
    cf->buffer_time = json_object_get_int(js_buffer_time);
    cf->period_time = json_object_get_int(js_period_time);

    BTMG_DEBUG("a2dp-sink,dev:%s buffer_time:%d,period_time:%d\n", cf->device, cf->buffer_time,
               cf->period_time);

    return 0;
}

int bt_config_read_a2dp_source(struct bt_a2dp_source_cf *cf)
{
    struct json_object *json_bt = NULL;
    struct json_object *js_a2dp_source = NULL;
    struct json_object *js_hci_index = NULL;
    struct json_object *js_mac = NULL;
    struct json_object *js_delay = NULL;

    if (cf == NULL)
        return -1;

    memset(cf, 0, sizeof(struct bt_a2dp_sink_cf));

    json_bt = json_object_from_file(BT_JSON_FILE_PATH);
    if (!json_bt)
        return -1;

    js_a2dp_source = json_object_object_get(json_bt, "a2dp_source");

    if (!js_a2dp_source)
        return -1;

    js_hci_index = json_object_object_get(js_a2dp_source, "device");
    if (!js_hci_index)
        return -1;

    js_mac = json_object_object_get(js_a2dp_source, "buffer_time");
    if (!js_mac)
        return -1;

    js_delay = json_object_object_get(js_a2dp_source, "period_time");
    if (!js_delay)
        return -1;

    cf->hci_index = json_object_get_int(js_hci_index);
    strncpy(cf->remote_mac, json_object_get_string(js_mac), 17);
    cf->delay = json_object_get_int(js_delay);

    BTMG_DEBUG("a2dp-source,hci index:%d,mac:%s,delay:%d\n", cf->hci_index, cf->remote_mac,
               cf->delay);
    return 0;
}

int bt_config_read_hfp(struct hfp_pcm *cf)
{
    struct json_object *json_bt = NULL;
    struct json_object *hfp_pcm = NULL;
    int len;
    int ret = -1;

    if (NULL == cf) {
        return -1;
    }

    memset(cf, 0, sizeof(struct hfp_pcm));

    json_bt = json_object_from_file(BT_JSON_FILE_PATH);
    if (!json_bt)
        return -1;

    /*parse hfp pcm config*/
    {
        struct json_object *js_hfp_pcm = NULL;

        struct json_object *js_rate = NULL;

        struct json_object *js_p_to_dev_c = NULL;
        struct json_object *js_p_to_dev_p = NULL;
        struct json_object *js_d_to_p_c = NULL;
        struct json_object *js_d_to_p_p = NULL;

        js_hfp_pcm = json_object_object_get(json_bt, "hfp_pcm");
        if (!js_hfp_pcm)
            return -1;

        js_rate = json_object_object_get(js_hfp_pcm, "rate");
        if (!js_rate)
            return -1;

        js_p_to_dev_c = json_object_object_get(js_hfp_pcm, "phone_to_dev_cap");
        if (!js_p_to_dev_c)
            return -1;

        js_p_to_dev_p = json_object_object_get(js_hfp_pcm, "phone_to_dev_play");
        if (!js_p_to_dev_p)
            return -1;

        js_d_to_p_c = json_object_object_get(js_hfp_pcm, "dev_to_phone_cap");
        if (!js_d_to_p_c)
            return -1;

        js_d_to_p_p = json_object_object_get(js_hfp_pcm, "dev_to_phone_play");
        if (!js_d_to_p_p)
            return -1;

        cf->rate = json_object_get_int(js_rate);

        /*parse phone to device capture audio hardware name*/
        len = json_object_get_string_len(js_p_to_dev_c);
        if (len > PCM_MAX_DEVICE_LEN) {
            BTMG_ERROR("device name is too loog(%d)", len);
            return -1;
        }
        strncpy(cf->phone_to_dev_cap, json_object_get_string(js_p_to_dev_c), len);

        /*parse phone to device play audio hardware name*/
        len = json_object_get_string_len(js_p_to_dev_p);
        if (len > PCM_MAX_DEVICE_LEN) {
            BTMG_ERROR("device name is too loog(%d)", len);
            return -1;
        }
        strncpy(cf->phone_to_dev_play, json_object_get_string(js_p_to_dev_p), len);

        /*parse device to phone capture audio hardware name*/
        len = json_object_get_string_len(js_d_to_p_c);
        if (len > PCM_MAX_DEVICE_LEN) {
            BTMG_ERROR("device name is too loog(%d)", len);
            return -1;
        }
        strncpy(cf->dev_to_phone_cap, json_object_get_string(js_d_to_p_c), len);

        /*parse device to phone play audio hardware name*/
        len = json_object_get_string_len(js_d_to_p_p);
        if (len > PCM_MAX_DEVICE_LEN) {
            BTMG_ERROR("device name is too loog(%d)", len);
            return -1;
        }
        strncpy(cf->dev_to_phone_play, json_object_get_string(js_d_to_p_p), len);

        BTMG_DEBUG("hfp:%s-%s-%s-%s", cf->phone_to_dev_cap, cf->phone_to_dev_play,
                   cf->dev_to_phone_cap, cf->dev_to_phone_play);
    }
    return 0;
}
