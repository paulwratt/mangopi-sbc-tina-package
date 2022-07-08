#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "bt_log.h"
#include "bt_alarm.h"
#include "bt_adapter.h"
#include "bt_bluez_signals.h"
#include "bt_device.h"
#include "common.h"

enum bluez_state {
    MG_IDLE = 0,
    MG_INITIALIZING,
    MG_INIT_FAILED,
    MG_RUNNING,
};

struct bluez_worker {
    pthread_t thread;
    GMainLoop *loop;
    enum bluez_state state;
};

static struct bluez_worker workers;

bluez_mg_t bluez_mg = {
    .dbus = NULL,
};

static void *bt_routine(void *arg)
{
    int try = 0;
    gchar *address;
    GError *err = NULL;
    struct hci_dev_info hci_devs;
    struct bt_adapter adapter;

    pthread_detach(pthread_self());
    workers.state = MG_INITIALIZING;
    BTMG_DEBUG("bt main loop thread start,thread id:%d", (int)pthread_self());
    if (hci_devinfo(0, &hci_devs) != 0) {
        BTMG_ERROR("hci device is null");
        workers.state = MG_INIT_FAILED;
        goto end;
    }

    address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    if ((bluez_mg.dbus = g_dbus_connection_new_for_address_sync(
                 address,
                 G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                         G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
                 NULL, NULL, &err)) == NULL) {
        BTMG_ERROR("Couldn't obtain D-Bus connection: %s", err->message);
        g_error_free(err);
        workers.state = MG_INIT_FAILED;
        goto final;
    }
    bluez_subscribe_signals();
    workers.loop = g_main_loop_new(NULL, FALSE);
    for (try = 0; try < 3; try++) {
        BTMG_DEBUG("set adapter power,try:%d", try + 1);
        if (bt_adapter_set_power(true) != -1)
            break;
        ms_sleep(500);
    }

    if (bt_adapter_set_scan_mode("piscan") < 0) {
        BTMG_ERROR("set scan mode fail");
        workers.state = MG_INIT_FAILED;
        goto final;
    }
    BTMG_DEBUG("Scan mode:Discoverable(yes),Connectable(yes)");

    if (bt_adapter_get_info(&adapter) < 0) {
        BTMG_ERROR("get adapter info fail");
        workers.state = MG_INIT_FAILED;
        goto final;
    }

    BTMG_INFO("bt adapter info: \n \
            address:%s\n \
            Name: %s\n \
            Alias: %s\n \
            Discoverable: %d\n \
            DiscoverableTimeout: %d\n",
               adapter.address, adapter.name, adapter.alias, adapter.discoverable,
               adapter.discover_timeout);
    bt_adapter_free(&adapter);
    workers.state = MG_RUNNING;
    g_main_loop_run(workers.loop);

    BTMG_DEBUG("bt main loop start quit");
    g_main_loop_unref(workers.loop);
    workers.loop = NULL;

    if (bt_adapter_set_scan_mode("nopiscan") < 0) {
        BTMG_ERROR("set scan mode fail");
        goto end;
    }
    BTMG_DEBUG("Scan mode:Discoverable(no),Connectable(no)");

    bt_adapter_set_power(false);
    bluez_unsubscribe_signals();
    if (bt_adapter_get_power_state() == BTMG_ADAPTER_ON)
        BTMG_ERROR("adapter set power off fail");

    err = NULL;
    g_dbus_connection_close_sync(bluez_mg.dbus, NULL, &err);
    if (err != NULL) {
        BTMG_ERROR("close gdbus failed: %s", err->message);
        g_error_free(err);
        err = NULL;
    }
    g_free(address);
    g_object_unref(bluez_mg.dbus);
    bluez_mg.dbus = NULL;

end:

    workers.state = MG_IDLE;
final:

    BTMG_DEBUG("bt main loop thread exit");
    pthread_exit(NULL);
}

int bt_bluez_init(void)
{
    int ret = BT_OK;

    BTMG_DEBUG("enter");
    if ((ret = pthread_create(&workers.thread, NULL, bt_routine, NULL)) != 0) {
        BTMG_ERROR("Couldn't create bt_routine thread:%s", strerror(ret));
        return BT_ERROR;
    }

    while (workers.state != MG_RUNNING) {
        ms_sleep(500);
        BTMG_DEBUG("wait bt main loop init..., state:%d", workers.state);
        if (workers.state == MG_INIT_FAILED) {
            BTMG_ERROR("bt main loop init fail");
            ret = BT_ERROR;
            break;
        }
    }

    return ret;
}

int bt_bluez_deinit(void)
{
    BTMG_DEBUG("enter");

    if (workers.state == MG_RUNNING) {
        g_main_loop_quit(workers.loop);
        while (workers.state != MG_IDLE) {
            ms_sleep(500);
            BTMG_DEBUG("wait bt main loop quit,state:%d......", workers.state);
        }
    } else {
        BTMG_DEBUG("bluez_worker don't running,state:%d", workers.state);
        return BT_ERROR;
    }

    return BT_OK;
}
