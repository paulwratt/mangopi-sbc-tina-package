#include <glib.h>
#include <gio/gio.h>

#ifndef BT_BLUEZ_H_
#define BT_BLUEZ_H_

#include "bt_manager.h"

int bt_bluez_init(void);
int bt_bluez_deinit(void);

#endif
