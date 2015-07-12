#ifndef __IPCONFIG_H
#define __IPCONFIG_H

#include "c_types.h"
#define at_busyTaskPrio        1
#define at_busyTaskQueueLen    4

#define at_procTaskPrio        0
#define at_procTaskQueueLen    1

#define at_backOk        uart0_sendStr("\r\nOK\r\n")
#define at_backError     uart0_sendStr("\r\nERROR\r\n")
#define at_backTeError   "+CTE ERROR: %d\r\n"

#define at_linkMax 5

//static BOOL at_ipMux = FALSE;
//static BOOL disAllFlag = FALSE;


//static int at_linkNum = 0;

typedef enum
{
  teClient,
  teServer
}teType;

typedef struct
{
  BOOL linkEn;
  BOOL teToff;
  uint8_t linkId;
  teType teType;
  uint8_t repeaTime;
  struct espconn *pCon;
}at_linkConType;

//static at_linkConType pLink[at_linkMax];


typedef enum{
  at_statIdle,
  at_statRecving,
  at_statProcess,
  at_statIpSending,
  at_statIpSended,
  at_statIpTraning
}at_stateType;

typedef enum{
  m_init,
  m_wact,
  m_gotip,
  m_linked,
  m_unlink,
  m_wdact
}at_mdStateType;


extern at_mdStateType mdState;


typedef struct
{
	char *at_cmdName;
	int8_t at_cmdLen;
  void (*at_testCmd)(uint8_t id);
  void (*at_queryCmd)(uint8_t id);
  void (*at_setupCmd)(uint8_t id, char *pPara);
  void (*at_exeCmd)(uint8_t id);
}at_funcationType;

#endif
