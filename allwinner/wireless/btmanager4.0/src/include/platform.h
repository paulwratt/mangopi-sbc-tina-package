#ifndef __PLATFORM_H
#define __PLATFORM_H

#if __cplusplus
extern "C" {
#endif

int bt_platform_init(void);
void bt_platform_deinit(void);

#if __cplusplus
}; // extern "C"
#endif

#endif
