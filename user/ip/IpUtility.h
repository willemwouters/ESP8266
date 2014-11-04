#ifndef __IP_UTILITY_H
#define __IP_UTILITY_H


int GetWifiMode();
int GetWifiStatus();
void at_exeCmdCifsr(uint8_t id); //add get station ip and ap ip
int8_t at_dataStrCpy(void *pDest, const void *pSrc, int8_t maxLen);

#endif