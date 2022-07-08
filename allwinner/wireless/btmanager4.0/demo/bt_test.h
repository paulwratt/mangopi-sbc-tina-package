#ifndef __BT_TEST_H__
#define __BT_TEST_H__

#include "bt_dev_list.h"

typedef struct {
    const char *cmd;
    const char *arg;
    void (*func)(int argc, char *args[]);
    const char *desc;
} cmd_tbl_t;

extern dev_list_t *discovered_devices;
extern btmg_profile_info_t *bt_pro_info;

extern cmd_tbl_t bt_cmd_table[];
extern cmd_tbl_t bt_gatts_cmd_table[];
extern cmd_tbl_t bt_gattc_cmd_table[];

/*gatt client*/
void bt_gatt_client_register_callback(btmg_callback_t *cb);
int bt_gatt_client_init();
int bt_gatt_client_deinit();

/*gatt server*/
void bt_gatt_server_register_callback(btmg_callback_t *cb);
int bt_gatt_server_init();
int bt_gatt_server_deinit();

/*a2dp src*/
bool bt_a2dp_src_is_run(void);
void bt_a2dp_src_stop(void);

#include <stddef.h>
#include <stdio.h>
#include <errno.h>

#undef MIN
#undef MAX
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

#if 1
static int char2hex(char c, uint8_t *x)
{
    if (c >= '0' && c <= '9') {
        *x = c - '0';
    } else if (c >= 'a' && c <= 'f') {
        *x = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        *x = c - 'A' + 10;
    } else {
        return -EINVAL;
    }

    return 0;
}

static int hex2char(uint8_t x, char *c)
{
    if (x <= 9) {
        *c = x + '0';
    } else if (x >= 10 && x <= 15) {
        *c = x - 10 + 'a';
    } else {
        return -EINVAL;
    }

    return 0;
}

static size_t hex2bin(const char *hex, size_t hexlen, uint8_t *buf, size_t buflen)
{
    uint8_t dec;

    if (buflen < hexlen / 2 + hexlen % 2) {
        return 0;
    }

    /* if hexlen is uneven, insert leading zero nibble */
    if (hexlen % 2) {
        if (char2hex(hex[0], &dec) < 0) {
            return 0;
        }
        buf[0] = dec;
        hex++;
        buf++;
    }

    /* regular hex conversion */
    for (size_t i = 0; i < hexlen / 2; i++) {
        if (char2hex(hex[2 * i], &dec) < 0) {
            return 0;
        }
        buf[i] = dec << 4;

        if (char2hex(hex[2 * i + 1], &dec) < 0) {
            return 0;
        }
        buf[i] += dec;
    }

    return hexlen / 2 + hexlen % 2;
}

static size_t bin2hex(const uint8_t *buf, size_t buflen, char *hex, size_t hexlen)
{
    if ((hexlen + 1) < buflen * 2) {
        return 0;
    }

    for (size_t i = 0; i < buflen; i++) {
        if (hex2char(buf[i] >> 4, &hex[2 * i]) < 0) {
            return 0;
        }
        if (hex2char(buf[i] & 0xf, &hex[2 * i + 1]) < 0) {
            return 0;
        }
    }

    hex[2 * buflen] = '\0';
    return 2 * buflen;
}
#endif

#endif