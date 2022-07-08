#include "AWTypes.h"
#include "mesh_internal.h"

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

static char aw_level_name[]={
    [AW_DBG_NONE_LEVEL] = 'N',
    [AW_DBG_ERR_LEVEL]  = 'E',
    [AW_DBG_WARN_LEVEL] = 'W',
    [AW_DBG_INFO_LEVEL] = 'I',
    [AW_DBG_DBG_LEVEL]  = 'D',
    [AW_DBG_FULL_LEVEL] = 'F',
    [AW_DBG_VERB_LEVE]  = 'V'
};

static bool is_syslog = false;

void mesh_log(AW_DBG_LEVEL_T level,AW_MODULE_T module,const char *fmt,...)
{
    char buf[AW_MESH_LOG_SIZE] = {'\0'};
    uint32_t size = 0;
    va_list args;
    va_start(args,fmt);

    if(module >= AW_MODULE_NB)
        module = AW_UNKNOW_MODULE;

    if(aw_dbg_cfg[module].level >= level)
    {
        sprintf(buf,"[%s %c]%s\n",aw_dbg_cfg[module].module_name,aw_level_name[level],fmt);
        if(is_syslog == true)
        {
            size = strlen(buf);
            vsprintf(&buf[size],fmt,args);
            l_info("%s",buf);
        }
        else
        {
            vprintf(buf,args);
        }
    }

    va_end(args);
}

int32_t aw_mesh_debug_config(AWDBG_T *cfg_list, uint8_t size,bool enable_syslog)
{
    uint8_t i = 0;
    uint8_t len = sizeof(aw_dbg_cfg)/sizeof(AWDBG_T);
    is_syslog = enable_syslog;
    if(size > len)
        size = len;
    for(i = 0 ; i < size; i++)
    {
        if(cfg_list[i].module_name)
        {
            aw_dbg_cfg[i].module_name = cfg_list[i].module_name;
        }
        aw_dbg_cfg[i].level = cfg_list[i].level;
        l_info("%s debug level %d\n",cfg_list[i].module_name,cfg_list[i].level);
    }
    return AW_ERROR_NONE;
}
