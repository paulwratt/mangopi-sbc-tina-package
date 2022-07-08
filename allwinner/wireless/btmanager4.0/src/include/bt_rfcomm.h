#ifndef __BT_RFCOMM_H
#define __BT_RFCOMM_H

#include <stdbool.h>
#include "bt_spp_client.h"
#include "bt_spp_service.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int rfcomm_fd;
    int sock;
    bool connected;
    int dev;
    char dev_name[128];
    char dst_addr[18];
} bt_rfcomm_t;

extern bt_rfcomm_t *rfcomm_t;

int rfcomm_init(void);
int rfcomm_deinit(void);
int rfcomm_release_dev(int ctl, int dev);
int rfcomm_release_all_dev(int ctl);
int find_conn(int s, int dev_id, long arg);

#ifdef __cplusplus
}; /*extern "C"*/
#endif

#endif /*__BT_RFCOMM_H */
