/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     platform_misc.h
  * @brief    Head file for platform miscellaneous.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2017-3-3
  * @version  v1.0
  * *************************************************************************************
  */

#ifndef _PLATFORM_TYPES_H_
#define _PLATFORM_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "platform_macros.h"

/** @addtogroup Mesh_Platform_TYPES Mesh Platform Types
  * @brief data types related
  * @{
  */

/** @defgroup Mesh_Platform_Queue_Exported_Macros Mesh Platform Queue Exported Macros
  * @brief
  * @{
  */

/* boolean definition */
#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    (0L)
#else
#define NULL    ((void *)0)
#endif
#endif

/** @} */

/** @} End of group Mesh_Platform_TYPES */

#endif /* _PLATFORM_TYPES_H_ */
