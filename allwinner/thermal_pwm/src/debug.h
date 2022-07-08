
#ifndef _ipc_debug_h_
#define _ipc_debug_h_

#include <stdio.h>

#define INFO(fmt, ...) \
    printf("%s:line %d: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DLOGE INFO
#define DLOGW INFO

#endif
