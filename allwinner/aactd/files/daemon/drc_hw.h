#ifndef __AACTD_DRC_HW_H__
#define __AACTD_DRC_HW_H__

#include "aactd/communicate.h"

#if defined (TARGET_PLATFORM_r328) \
    || defined (TARGET_PLATFORM_r818) \
    || defined (TARGET_PLATFORM_mr813)
#define DRC_HW_REG_BASE_ADDR 0x05096000
#elif defined (TARGET_PLATFORM_r329)
#define DRC_HW_REG_BASE_ADDR 0x07032000
#elif defined (TARGET_PLATFORM_r528) \
    || defined (TARGET_PLATFORM_r528s2) \
    || defined (TARGET_PLATFORM_d1) \
    || defined (TARGET_PLATFORM_v853)
#define DRC_HW_REG_BASE_ADDR 0x02030000
#else
#error "Not supported platform"
#endif /* TARGET_PLATFORM */

int drc_hw_local_init(void);
int drc_hw_local_release(void);

int drc_hw_write_com_to_local(const struct aactd_com *com);

#endif /* ifndef __AACTD_DRC_HW_H__ */
