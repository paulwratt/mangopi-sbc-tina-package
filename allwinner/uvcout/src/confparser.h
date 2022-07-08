#ifndef __CONFIG_PARSER_H__
#define __CONFIG_PARSER_H__



#include "iniparser.h"


typedef struct confparser_s
{
	dictionary *pDict;
}CONFPARSER_S, *PTR_CONFPARSER_S;


int createConfParser(const char *conf_path, CONFPARSER_S *pCfg);
void destroyConfParser(CONFPARSER_S *pCfg);

int GetConfParaInt(CONFPARSER_S *pCfg, const char * key,int notfound);
unsigned int GetConfParaUInt(CONFPARSER_S *pCfg, const char * key,int notfound);
const char *GetConfParaString(CONFPARSER_S *pCfg, const char *key, const char *def);
double GetConfParaDouble(CONFPARSER_S *pCfg, const char *key, double notfound);
int GetConfParaBoolean(CONFPARSER_S *pCfg, const char *key, int notfound);




#endif //__CONFIG_PARSER_H__
