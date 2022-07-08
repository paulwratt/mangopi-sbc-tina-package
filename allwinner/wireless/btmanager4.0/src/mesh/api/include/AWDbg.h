#ifndef __AW_DBG_H__
#define __AW_DBG_H__
#include <stdio.h>

typedef enum {
    AW_DBG_NONE_LEVEL = 0,
    AW_DBG_ERR_LEVEL,
    AW_DBG_WARN_LEVEL,
    AW_DBG_INFO_LEVEL,
    AW_DBG_DBG_LEVEL,
    AW_DBG_FULL_LEVEL,
    AW_DBG_VERB_LEVE,
}AW_DBG_LEVEL_T;

typedef enum {
    AW_APP_MODULE = 0,
    AW_APP_DB_MODULE,
    AW_APP_PROV_MODULE,
    AW_APP_GATEWAY_MODULE,
    AW_APP_NODE_MODULE,
    AW_APP_GAP_MODULE,
    AW_APP_CFGMODEL_MODULE,
    AW_APP_EVENT_CB_MODULE,
    AW_APP_SIG_MDL_MODULE,
    AW_UNKNOW_MODULE,
    AW_MODULE_NB
}AW_MODULE_T;

typedef struct
{
    char *module_name;
    AW_DBG_LEVEL_T level;
}AWDBG_T;

#define AW_MESH_LOG_SIZE 1024

void aw_mesh_dbg_cfg_set();

#endif
