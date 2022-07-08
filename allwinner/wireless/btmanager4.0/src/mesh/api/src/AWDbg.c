#include "AWDbg.h"
#include "AWmeshDefine.h"
#include "AWmeshTypes.h"
#include "AWmeshApi.h"
static AWDBG_T aw_dbg_cfg[AW_MODULE_NB] = {
    [AW_APP_MODULE]          =  {"AWAPP ",AW_DBG_VERB_LEVE},
    [AW_APP_DB_MODULE]       =  {"AWAPP_DB",AW_DBG_VERB_LEVE},
    [AW_APP_PROV_MODULE]     =  {"AWAPP_PROV",AW_DBG_VERB_LEVE},
    [AW_APP_GATEWAY_MODULE]  =  {"AWAPP_GATEWAY",AW_DBG_VERB_LEVE},
    [AW_APP_NODE_MODULE]     =  {"AWAPP_NODE",AW_DBG_VERB_LEVE},
    [AW_APP_GAP_MODULE]      =  {"AWAPP_GAP",AW_DBG_VERB_LEVE},
    [AW_APP_CFGMODEL_MODULE] =  {"AWAPP_CFGMODEL",AW_DBG_VERB_LEVE},
    [AW_APP_EVENT_CB_MODULE] =  {"AWAPP_EVENTCB",AW_DBG_VERB_LEVE},
    [AW_UNKNOW_MODULE]       =  {"AWLOG_UNKNOW",AW_DBG_VERB_LEVE}
};

void aw_mesh_dbg_cfg_set()
{
    aw_mesh_debug_config(aw_dbg_cfg,sizeof(aw_dbg_cfg)/sizeof(AWDBG_T),false);
}
