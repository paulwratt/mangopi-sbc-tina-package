/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     generic_property.h
  * @brief    Head file for generic property models.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2017-12-25
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _GENERIC_PROPERTY_H
#define _GENERIC_PROPERTY_H

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include "platform_misc.h"
//#include "mesh_api.h"
#include "mesh_access.h"

/** @defgroup GENERIC_ON_OFF_OPCODE generic on off opcode
  * @brief
  * @{
  */
#define MESH_MSG_GENERIC_MANUFACTURER_PROPERTIES_GET        0x822A
#define MESH_MSG_GENERIC_MANUFACTURER_PROPERTIES_STAT       0x43
#define MESH_MSG_GENERIC_MANUFACTURER_PROPERTY_GET          0x822B
#define MESH_MSG_GENERIC_MANUFACTURER_PROPERTY_SET          0x44
#define MESH_MSG_GENERIC_MANUFACTURER_PROPERTY_SET_UNACK    0x45
#define MESH_MSG_GENERIC_MANUFACTURER_PROPERTY_STAT         0x46

#define MESH_MSG_GENERIC_ADMIN_PROPERTIES_GET               0x822C
#define MESH_MSG_GENERIC_ADMIN_PROPERTIES_STAT              0x47
#define MESH_MSG_GENERIC_ADMIN_PROPERTY_GET                 0x822D
#define MESH_MSG_GENERIC_ADMIN_PROPERTY_SET                 0x48
#define MESH_MSG_GENERIC_ADMIN_PROPERTY_SET_UNACK           0x49
#define MESH_MSG_GENERIC_ADMIN_PROPERTY_STAT                0x4A

#define MESH_MSG_GENERIC_USER_PROPERTIES_GET                0x822E
#define MESH_MSG_GENERIC_USER_PROPERTIES_STAT               0x4B
#define MESH_MSG_GENERIC_USER_PROPERTY_GET                  0x822F
#define MESH_MSG_GENERIC_USER_PROPERTY_SET                  0x4C
#define MESH_MSG_GENERIC_USER_PROPERTY_SET_UNACK            0x4D
#define MESH_MSG_GENERIC_USER_PROPERTY_STAT                 0x4E

#define MESH_MSG_GENERIC_CLIENT_PROPERTIES_GET              0x4F
#define MESH_MSG_GENERIC_CLIENT_PROPERTIES_STAT             0x50

#define MESH_MODEL_GENERIC_ADMIN_PROPERTY_SERVER            0x1011FFFF
#define MESH_MODEL_GENERIC_MANUFACTURER_PROPERTY_SERVER     0x1012FFFF
#define MESH_MODEL_GENERIC_USER_PROPERTY_SERVER             0x1013FFFF
#define MESH_MODEL_GENERIC_CLIENT_PROPERTY_SERVER           0x1014FFFF
#define MESH_MODEL_GENERIC_PROPERTY_CLIENT                  0x1015FFFF
/** @} */

/** @defgroup GENERIC_ON_OFF_Msg generic on off msg
  * @brief Msg types used by generic on off models
  * @{
  */
typedef enum
{
    GENERIC_USER_ACCESS_READ = 0x1,
    GENERIC_USER_ACCESS_WRITE = 0x2,
    GENERIC_USER_ACCESS_READ_AND_WRITE = 0x3
} _PACKED_ generic_user_access_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_USER_PROPERTIES_GET)];
} _PACKED_ generic_user_properties_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_USER_PROPERTIES_STAT)];
    uint16_t user_property_ids[1]; //!< variable length
} _PACKED_ generic_user_properties_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_USER_PROPERTY_GET)];
    uint16_t user_property_id;
} _PACKED_ generic_user_property_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_USER_PROPERTY_SET)];
    uint16_t user_property_id;
    uint8_t user_property_value[1]; //!< variable length
} _PACKED_ generic_user_property_set_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_USER_PROPERTY_STAT)];
    uint16_t user_property_id;
    generic_user_access_t user_access;
    uint8_t user_property_value[1]; //!< variable length
} _PACKED_ generic_user_property_stat_t;

typedef enum
{
    GENERIC_ADMIN_USER_ACCESS_NOT_GENERIC_USER_PROPERTY = 0x0,
    GENERIC_ADMIN_USER_ACCESS_READ = 0x1,
    GENERIC_ADMIN_USER_ACCESS_WRITE = 0x2,
    GENERIC_ADMIN_USER_ACCESS_READ_AND_WRITE = 0x3
} _PACKED_ generic_admin_user_access_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_ADMIN_PROPERTIES_GET)];
} _PACKED_ generic_admin_properties_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_ADMIN_PROPERTIES_STAT)];
    uint16_t admin_property_ids[1]; //!< variable length
} _PACKED_ generic_admin_properties_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_ADMIN_PROPERTY_GET)];
    uint16_t admin_property_id;
} _PACKED_ generic_admin_property_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_ADMIN_PROPERTY_SET)];
    uint16_t admin_property_id;
    generic_admin_user_access_t admin_user_access;
    uint8_t admin_property_value[1]; //!< variable length
} _PACKED_ generic_admin_property_set_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_ADMIN_PROPERTY_STAT)];
    uint16_t admin_property_id;
    generic_admin_user_access_t admin_user_access;
    uint8_t admin_property_value[1]; //!< variable length
} _PACKED_ generic_admin_property_stat_t;

typedef enum
{
    GENERIC_MANUFACTURER_USER_ACCESS_NOT_GENERIC_USER_PROPERTY = 0x0,
    GENERIC_MANUFACTURER_USER_ACCESS_READ = 0x1,
} _PACKED_ generic_manufacturer_user_access_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_MANUFACTURER_PROPERTIES_GET)];
} _PACKED_ generic_manufacturer_properties_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_MANUFACTURER_PROPERTIES_STAT)];
    uint16_t manufacturer_property_ids[1]; //!< variable length
} _PACKED_ generic_manufacturer_properties_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_MANUFACTURER_PROPERTY_GET)];
    uint16_t manufacturer_property_id;
} _PACKED_ generic_manufacturer_property_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_MANUFACTURER_PROPERTY_SET)];
    uint16_t manufacturer_property_id;
    generic_manufacturer_user_access_t manufacturer_user_access;
} _PACKED_ generic_manufacturer_property_set_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_MANUFACTURER_PROPERTY_STAT)];
    uint16_t manufacturer_property_id;
    generic_manufacturer_user_access_t manufacturer_user_access;
    uint8_t manufacturer_property_value[1]; //!< variable length
} _PACKED_ generic_manufacturer_property_stat_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_CLIENT_PROPERTIES_GET)];
    uint16_t client_property_id; //!< A starting Client Property ID present within an element
} _PACKED_ generic_client_properties_get_t;

typedef struct
{
    uint8_t opcode[ACCESS_OPCODE_SIZE(MESH_MSG_GENERIC_CLIENT_PROPERTIES_STAT)];
    uint16_t client_property_ids[1]; //!< variable length
} _PACKED_ generic_client_properties_stat_t;
/** @} */

/** @defgroup GENERIC_ON_OFF_SERVER_API   generic on off Server API
  * @brief Functions declaration
  * @{
  */

/** @} */

/** @defgroup GENERIC_ON_OFF_CLIENT_API   generic on off Client API
  * @brief Functions declaration
  * @{
  */

/** @} */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif /* _GENERIC_PROPERTY_H */
