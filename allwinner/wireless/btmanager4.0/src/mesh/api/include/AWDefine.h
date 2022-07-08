#ifndef __AW_DEFINE_H__
#define __AW_DEFINE_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOOL	bool
#define BOOLEAN	bool
#define CHAR	char
#define VOID	void
#define INT8    int8_t
#define INT16	int16_t
#define INT32	int32_t
#define UINT8	uint8_t
#define UINT16	uint16_t
#define UINT32	uint32_t

#ifndef _PACKED_
#define _PACKED_  __attribute__ ((packed))
#endif

#ifdef __cplusplus
}
#endif

#endif
