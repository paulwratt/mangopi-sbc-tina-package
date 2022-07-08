#ifndef __AW_MESH_TEST_LOG_H__
#define __AW_MESH_TEST_LOG_H__
//tag
#define STR_GOO_CL_TAG "goo-client:"
#define STR_APPKEY_ADD_TAG "appkey-add:"
#define STR_APPKEY_UPDATE_TAG "appkey-update:"
#define STR_APPKEY_DEL_TAG "appkey-del:"
#define STR_APPKEY_GET_TAG "appkey-get:"
#define STR_NETKEY_ADD_TAG "netkey-add:"
#define STR_NETKEY_UPDATE_TAG "netkey-update:"
#define STR_NETKEY_DEL_TAG "netkey-del:"
#define STR_NETKEY_GET_TAG "netkey-get:"
#define STR_APP_BIND_TAG "app-bind:"
#define STR_APP_UNBIND_TAG "app-unbind:"
#define STR_APP_VDN_GET_TAG "app-vnd-get:"
#define STR_APP_GET_TAG "app-get:"

#define STR_NODE_RESET_TAG "node-reset:"
#define STR_DEV_COMP_GET_TAG "device-composition-get:"

//network transmit
#define STR_NETWORK_TRANSMIT_SET_TAG "network-transmit-set"
#define STR_NETWORK_TRANSMIT_GET_TAG "network-tranmit-get"

//heartbeat
#define STR_HEARTBEAT_PUB_SET_TAG "heartbeat-pub-set:"
#define STR_HEARTBEAT_PUB_GET_TAG "beartbeat-pub-get:"
#define STR_HEARTBEAT_SUB_SET_TAG "heartbeat-sub-set:"
#define STR_HEARTBEAT_SUB_GET_TAG "heartbeat-sub-get:"
#define STR_HEARTBEAT_PKT_TAG "heartbeat-pkt:"
//keyrefresh
#define STR_KEY_REFRESH_SET_TAG "key-refresh-set:"
#define STR_KEY_REFRESH_GET_TAG "key-refresh-get:"

//iv update
#define STR_IV_UPDATE_GET_TAG "iv-update-get:"
#define STR_IV_UPDATE_SET_TAG "iv-update-set:"
#define STR_IV_TEST_MODE_SET_TAG "iv-test-mode-set:"

//friend
#define STR_FRIEND_SET_TAG "friend-set:"
#define STR_FRIEND_GET_TAG "friend-get:"
#define STR_FRIEND_STATUS_TAG "friend-status:"
#define STR_POLL_TIMEOUT_LIST_TAG "poll-timeout-list:"

//ttl
#define STR_DEFAULT_TTL_SET_TAG "default-ttl-set:"
#define STR_DEFAULT_TTL_GET_TAG "default-ttl-get:"
#define STR_RELAY_SET_TAG "relay-set:"
#define STR_OP_RELAY_GET_TAG "relay-get:"

#define STR_MODEL_PUB_SET_TAG "model-pub-set:"
#define STR_MODEL_PUB_GET_TAG "model-pub-get:"
#define STR_MODEL_SUB_ADD_TAG "model-sub-add:"
#define STR_MODEL_SUB_GET_TAG "model-sub-get:"
#define STR_MODEL_SUB_DELETE_TAG "model-sub-del:"
#define STR_MODEL_SUB_DELETE_ALL_TAG "model-sub-del-all:"
#define STR_MODEL_SUB_OVERWRITE_TAG "model-sub-overwrite:"
#define STR_VEND_MODEL_SUB_GET_TAG "vendor-model-sub-get:"

//proxy
#define STR_PROXY_SET_TAG "proxy-set:"
#define STR_PROXY_GET_TAG "proxy-get:"
#define STR_NODE_IDENTITY_SET_TAG "node-identity-set:"
#define STR_NODE_IDENTITY_GET_TAG "node-identity-get:"

//beacon
#define STR_BEACON_SET_TAG "beacon-set:"
#define STR_BEACON_GET_TAG "beacon-get:"


//tag for status
#define STR_COMPOSITION_STATUS "composition-status:"

//network setting status
#define STR_NETWORK_TRANS_STATUS "network-trans-status:"
#define STR_BEACON_STATUS "beacon-status:"
#define STR_TTL_STATUS "ttl-status:"
#define STR_RELAY_STATUS "relay-status:"

//node status
#define STR_NODE_RESET_STATUS "node-reset-status:"
//key status
#define STR_APP_STATUS "app-status:"
#define STR_APPKEY_STATUS "appkey-status:"
#define STR_NETKEY_STATUS "netkey-status:"
#define STR_NETKEY_LIST_STATUS "netkey-list-status:"
#define STR_APPKEY_LIST_STATUS "appkey-list-status:"
#define STR_VND_APP_LIST_STATUS "vnd-app-list-status:"
#define STR_SIG_MODEL_APPKEY_LIST_STATUS "sig-model-appkey-list-status:"
//key refresh status
#define STR_KEYREFRESH_PHASE_STATUS "keyrefresh-phase-status:"

//sub-pub status
#define STR_PUB_STATUS "pub-status:"
#define STR_SUB_STATUS "sub-status:"
#define STR_SIG_SUB_LIST_STATUS "sub-list-status:"
#define STR_VND_SUB_LIST_STATUS "vnd-sub-list-status:"

//heartbeat status
#define STR_HEARTBEAT_PUB_STATUS "heartbeat-pub-status:"
#define STR_HEARTBEAT_SUB_STATUS "heartbeat-sub-status:"

//friend status
#define STR_LPN_POLLTIMEOUT_STATUS "lpn-polltimeout-status:"

//proxy status
#define STR_NODE_IDENTITY_STATUS "identity-status:"
#define STR_PROXY_STATUS "proxy-status:"


//aw-mesh-app status
#define STR_AW_MESH_APP_VERIFY_VERSION_ID       "V1.0"
#define STR_AW_MESH_APP_RUN_STATUS              "app-status:"
#define STR_AW_MESH_APP_ATTACHED                "app-attach-status:"
#define STR_AW_MESH_APP_ACCESS_RX               "app-access-rx:"
#define STR_AW_MESH_HEALTH_CLIENT_RX            "app-health-client-rx:"
#define STR_AW_MESH_UD_SCAN_RESULT              "app-unprov-scan-result:"
#define STR_AW_MESH_PROV_DONE                   "app-prov-status:"
#define STR_AW_MESH_FRIEND_STATUS               "app-friend-status:"
#define STR_AW_MESH_FRIEND_ESTABLISHED          "app-friend-establish:"
#define STR_AW_MESH_FRIEND_TERMINATED           "app-friend-terminated:"

#define STR_AW_MESH_IV_UPDATE_STATUS            "app-iv-update-status:"
#define STR_AW_MESH_IV_TEST_MODE_STATUS         "app-iv-test-mode-status:"
//aw-mesh-goo-control
#define STR_APP_CLIENT_GOO_SET          "app-client-goo-set:"
#define STR_APP_CLIENT_GOO_GET          "app-client-goo-get:"

//aw-mesh-goo-status
#define STR_APP_SERVER_GOO_SET          "app-server-goo-set:"
#define STR_APP_SERVER_GOO_OP           "app-server-goo-op:"
#define STR_APP_CLIENT_GOO_STATUS       "app-client-goo-status:"

//aw-mesh-prov-operation
#define STR_APP_UD_SCAN_START   "unprov-device-scan-start:"
#define STR_APP_UD_SCAN_STOP    "unprov-device-scan-stop:"
#define STR_APP_UD_SCAN_TIMEOUT "unprov-device-scan-timeout:"
#define STR_APP_PROV_START  "prov-start:"
#define STR_APP_PROV_STOP   "prov-stop:"
#define STR_APP_PROV_TO     "prov-timeout:"
#define STR_APP_PROV_DATA_SET "prov-data-set:"
#define STR_APP_PROV_DATA_GET "prov-data-get:"

//aw-mesh-vendor-model-operation
#define STR_APP_CLIENT_XRADIO_VENDOR_NAME_SET "app-vendor-cliet-name-set:"
#define STR_APP_CLIENT_XRADIO_VENDOR_NAME_GET "app-vendor-cliet-name-get:"
#define STR_APP_CLIENT_XRADIO_VENDOR_NAME_SET_UNACK "app-vendor-cliet-name-set-unack:"
#define STR_APP_SERVER_XRADIO_VENDOR_NAME_STATUS "app-vendor-server-name-status:"
#endif
