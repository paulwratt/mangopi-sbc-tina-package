#ifndef __BT_LOG_H
#define __BT_LOG_H

#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include "log_core.h"

#ifndef DEFAULT_LOG_FILE
#define DEFAULT_LOG_FILE "/tmp/bt_log.log"
#endif

#ifndef DEFAULT_LOG_CMD_FIFO
#define DEFAULT_LOG_CMD_FIFO "/tmp/bt_debug"
#endif

#ifndef DEBUG_TAG
#define DEBUG_TAG ""
#endif

#if __cplusplus
extern "C" {
#endif

enum ex_debug_mask {
    EX_DBG_RING_BUFF_WRITE = 1 << 0,
    EX_DBG_RING_BUFF_READ = 1 << 1,
    EX_DBG_A2DP_SINK_RATE = 1 << 2,
    EX_DBG_A2DP_SOURCE_LOW_RATE = 1 << 3,
    EX_DBG_A2DP_SOURCE_UP_RATE = 1 << 4,
    EX_DBG_A2DP_SINK_DUMP_RB = 1 << 5,
    EX_DBG_A2DP_SINK_DUMP_HW = 1 << 6,
    EX_DBG_A2DP_SOURCE_DUMP_UP = 1 << 7,
    EX_DBG_A2DP_SOURCE_DUMP_LOW = 1 << 8,
    EX_DBG_HFP_HF_DUMP_Capture = 1 << 9,
    EX_DBG_MASK_MAX = 1 << 10,
};

extern int btmg_ex_debug_mask;

typedef void (*timer_debug_callback_t)(void *data);

#define CONFIG_DEBUG_FUNCTION_LINE 1
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

extern int btmg_debug_level;
extern int btmg_debug_show_keys;
extern int btmg_debug_timestamp;

#ifdef __GNUC__
#define PRINTF_FORMAT(a,b) __attribute__ ((format (printf, (a), (b))))
#define STRUCT_PACKED __attribute__ ((packed))
#else
#define PRINTF_FORMAT(a,b)
#define STRUCT_PACKED
#endif

typedef enum btmg_prtk_level {
    MSG_NONE = 0,
    MSG_ERROR,
    MSG_WARNING,
    MSG_INFO,
    MSG_DEBUG,
    MSG_MSGDUMP,
    MSG_EXCESSIVE
} btmg_prtk_level;

#ifdef CONFIG_NO_STDOUT_DEBUG

#define btmg_printf(args...)                                                                       \
    do {                                                                                           \
    } while (0)
#define btmg_debug_open_file(p)                                                                    \
    do {                                                                                           \
    } while (0)
#define btmg_debug_close_file()                                                                    \
    do {                                                                                           \
    } while (0)

#else
// int btmg_debug_open_file(const char *path);
// void btmg_debug_close_file(void);
// void btmg_debug_open_syslog(void);
// void btmg_debug_close_syslog(void);
int btmg_set_debug_level(int level);
int btmg_get_debug_level();
int btmg_set_ex_debug_mask(int mask);
int btmg_get_ex_debug_mask(void);

#ifdef CONFIG_DEBUG_FUNCTION_LINE
#define btmg_printf(level, fmt, arg...)                                                            \
    btmg_print(level, "%s:%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#else
#define btmg_printf(level, fmt, arg...) btmg_print(level, fmt "\n", ##arg)
#endif /*CONFIG_DEBUG_FUNCTION_LINE*/

#define BTMG_PRTK(level, fmt, arg...)                                                              \
    btmg_print(level, "BTMG[%s:%u]:  " fmt "", __func__, __LINE__, ##arg)
#define BTMG_DEBUG(fmt, arg...)                                                                    \
    btmg_print(MSG_DEBUG, "BTMG[%s:%u]:  " fmt "\n", __func__, __LINE__, ##arg)
#define BTMG_INFO(fmt, arg...)                                                                     \
    btmg_print(MSG_INFO, "BTMG[%s:%u]:  " fmt "\n", __func__, __LINE__, ##arg)
#define BTMG_WARNG(fmt, arg...)                                                                    \
    btmg_print(MSG_WARNING, "BTMG[%s:%u]:  " fmt "\n", __func__, __LINE__, ##arg)
#define BTMG_ERROR(fmt, arg...)                                                                    \
    btmg_print(MSG_ERROR, "BTMG[%s:%u]:  " fmt "\n", __func__, __LINE__, ##arg)
#define BTMG_DUMP(fmt, arg...)                                                                     \
    btmg_print(MSG_MSGDUMP, "BTMG[%s:%u]:	" fmt "\n", __func__, __LINE__, ##arg)
#define BTMG_EXCESSIVE(fmt, arg...)                                                                \
    btmg_print(MSG_EXCESSIVE, "BTMG[%s:%u]:	" fmt "\n", __func__, __LINE__, ##arg)

// void btmg_print(int level, const char *fmt, ...)
// PRINTF_FORMAT(2, 3);
#define btmg_print(level, fmt, arg...) log_print(DEBUG_TAG, level, fmt, ##arg)

#endif /* CONFIG_NO_STDOUT_DEBUG */

#define BT_OK (0)
#define BT_ERROR (-1)
#define BT_ERROR_ADATER_NOT_ON (-2)
#define BT_ERROR_NO_MEMORY (-3)
#define BT_ERROR_INVALID_ARGS (-4)
#define BT_ERROR_IN_PROCESS (-5)
#define BT_ERROR_TIME_OUT (-6)
#define BT_ERROR_NULL_VARIABLE (-7)

#define BT_ERROR_DEVICE_NOT_FOUND (-8)
#define BT_ERROR_A2DP_DEVICE_NOT_CONNECTED (-9)
#define BT_ERROR_A2DP_PCM_OPEN_FAIL (-10)
#define BT_ERROR_A2DP_SRC_STREAM_NOT_START (-11)
#define BT_ERROR_A2DP_SRC_NOT_INIT (-12)
#define BT_ERROR_A2DP_SINK_NOT_INIT (-13)

#define BT_ERROR_BLUEZ_HOST_IS_DOWN (-14)
#define BT_ERROR_BLUEZ_OPERATION_IN_PROGRESS (-15)
#define BT_ERROR_BLUEZ_RESOURCE_UNAVAILABLE (-16)
#define BT_ERROR_BLUEZ_MES_RECIPIENT_DISCONNECTED_FROM_MES_BUS_WITHOUT_REPLY (-17)

typedef struct {
    int error;
    char info[128];
} bt_error_info_string;

const char *btmg_get_error_info(int error);
int btmg_parse_ex_debug_mask(char *key, char *op, char *val);

#if __cplusplus
}; // extern "C"
#endif

#endif
