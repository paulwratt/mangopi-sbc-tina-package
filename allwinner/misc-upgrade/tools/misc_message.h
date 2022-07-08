#ifndef __MISC_MESSAGE_H__
#define __MISC_MESSAGE_H__

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define COMPATIBLE_ANDROID_UBOOT

#define MISC_LIMITED_SIZE (4*1024)
//#define MISC_DEVICE "/dev/by-name/misc"

#define LOGE(...) printf( __VA_ARGS__)

#define CHECK_MISC_MESSAGE \
    ((sizeof(bootloader_message) > MISC_LIMIT_SIZE) ? -1 : 0)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef COMPATIBLE_ANDROID_UBOOT
/*
 * uboot need to support both tina and android, now uboot2018 will
 * clear misc partition unless misc->recovery contain update_package.
 * to prevent clear misc, let's always set version(android's recovery)
 * to "update_package".
 *
 * the android bootloader_message is:
 * struct bootloader_message {
 * 	char command[32];
 * 	char status[32];
 * 	char recovery[768];
 *
 * 	// The 'recovery' field used to be 1024 bytes.  It has only ever
 * 	// been used to store the recovery command line, so 768 bytes
 * 	// should be plenty.  We carve off the last 256 bytes to store the
 * 	// stage string (for multistage packages) and possible future
 * 	// expansion.
 * 	char stage[32];
 *
 * 	// The 'reserved' field used to be 224 bytes when it was initially
 * 	// carved off from the 1024-byte recovery field. Bump it up to
 * 	// 1184-byte so that the entire bootloader_message struct rounds up
 * 	// to 2048-byte.
 * 	char reserved[1184];
 * };
 */
#endif

struct bootloader_message {
    char command[32];
    char status[32];
    char version[32];
    //char reserved[1024];
};

int get_bootloader_message_block(struct bootloader_message *out,
                                 const char* misc);

int set_bootloader_message_block(const struct bootloader_message *in,
                                 const char* misc);
#ifdef __cplusplus
}
#endif
#endif /*__MISC_MESSAGE_H__*/
