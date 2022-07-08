#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "bt_log.h"

#define WAIT_HCI_COUNT_MAX 6

int bt_platform_init(void)
{
    struct hci_dev_info hci_devs;
    int i = 0;
    int ctl = -1;

check_hci:
    if (i == 0) {
        /*bring up hci*/
        system("/etc/bluetooth/bt_init.sh start");
    }
    /*check hci0 device*/
    if (hci_devinfo(0, &hci_devs) != 0) {
        if (i < WAIT_HCI_COUNT_MAX) {
            BTMG_ERROR("detect hci0......");
            i++;
            sleep(3);
            goto check_hci;
        } else {
            BTMG_ERROR("init hci device failed");
            return -1;
        }
    }
    BTMG_DEBUG("init hci device successful");

    return 0;
}

void bt_platform_deinit(void)
{
    system("/etc/bluetooth/bt_init.sh stop");
}
