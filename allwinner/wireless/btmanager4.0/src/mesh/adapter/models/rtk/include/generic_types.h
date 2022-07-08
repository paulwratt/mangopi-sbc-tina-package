/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     generic_types.h
  * @brief    Head file for types used by generic models.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2017-12-22
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _GENERIC_TYPES_H
#define _GENERIC_TYPES_H

//#include "platform_types.h"

BEGIN_DECLS


typedef enum
{
    GENERIC_STAT_SUCCESS,
    GENERIC_STAT_CANNOT_SET_RANGE_MIN,
    GENERIC_STAT_CANNOT_SET_RANGE_MAX
}_PACKED_ generic_stat_t;


/** @defgroup MODEL_ERROR_CODE model error code
  * @brief Error code for models
  * @{
  */
/**
 * @note
 * code equals to 0 means success
 * code greater than 0 means success but will do some special processing
 * code less than 0 means error happened
 */
#define MODEL_SUCCESS               0
#define MODEL_STOP_TRANSITION       1
/** @} */

END_DECLS

#endif /* _GENERIC_TYPES_H */
