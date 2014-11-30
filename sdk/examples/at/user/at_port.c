#include "at.h"
#include "user_interface.h"
#include "osapi.h"
#include "driver/uart.h"

/** @defgroup AT_PORT_Defines
  * @{
  */
#define at_cmdLenMax 128
#define at_dataLenMax 2048
/**
  * @}
  */

/** @defgroup AT_PORT_Extern_Variables
  * @{
  */
extern uint16_t at_sendLen;
extern uint16_t at_tranLen;
//extern UartDevice UartDev;
//extern bool IPMODE;
extern os_timer_t at_delayChack;
/**
  * @}
  */

/** @defgroup AT_PORT_Extern_Functions
  * @{
  */
extern void at_ipDataSending(uint8_t *pAtRcvData);
/**
  * @}
  */

os_event_t    at_busyTaskQueue[at_busyTaskQueueLen];
os_event_t    at_procTaskQueue[at_procTaskQueueLen];

BOOL specialAtState = TRUE;
at_stateType  at_state;
uint8_t *pDataLine;
BOOL echoFlag = TRUE;

static uint8_t at_cmdLine[at_cmdLenMax];
uint8_t at_dataLine[at_dataLenMax];/////
//uint8_t *at_dataLine;

/** @defgroup AT_PORT_Functions
  * @{
  */

static void at_procTask(os_event_t *events);
static void at_busyTask(os_event_t *events);

/**
  * @brief  Uart receive task.
  * @param  events: contain the uart receive data
  * @retval None
  */
void at_recvTask(void)
{
  static uint8_t atHead[2];
  static uint8_t *pCmdLine;
  uint8_t temp;

//  temp = events->par;
  temp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
//  temp = 'X';
  //add transparent determine
  if((at_state != at_statIpTraning) && (temp != '\n') && (echoFlag))
  {
      uart_tx_one_char(temp); //display back
  }

  switch (at_state)
  {
  case at_statIdle: //serch "AT" head
    atHead[0]=atHead[1];
    atHead[1]=temp;
    if((os_memcmp(atHead, "AT", 2) == 0) || (os_memcmp(atHead, "at", 2) == 0))
    {
      at_state = at_statRecving;
      pCmdLine = at_cmdLine;
      atHead[1] = 0x00;
    }
    else if(temp == '\n') //only get enter
    {
      uart0_sendStr("\r\nError\r\n");
    }
    break;

  case at_statRecving: //push receive data to cmd line
    *pCmdLine = temp;
    if(temp == '\n')
    {
      system_os_post(at_procTaskPrio, 0, 0);
      at_state = at_statProcess;
      if(echoFlag)
      {
        uart0_sendStr("\r\n"); ///////////
      }
    }
    else if (pCmdLine == &at_cmdLine[at_cmdLenMax-1])
    {
      at_state = at_statIdle;
    }
    pCmdLine++;
    break;

  case at_statProcess: //process data
    if(temp == '\n')
    {
      system_os_post(at_busyTaskPrio, 0, 1);
//      uart0_sendStr("\r\nbusy\r\n");
    }
    break;

  case at_statIpSending:
  	*pDataLine = temp;
		if ((pDataLine == &at_dataLine[at_sendLen-1]) ||
		    (pDataLine == &at_dataLine[at_dataLenMax-1]))
    {
      system_os_post(at_procTaskPrio, 0, 0);
      at_state = at_statIpSended;
    }
    pDataLine++;
//    *pDataLine = temp;
//    if (pDataLine == &UartDev.rcv_buff.pRcvMsgBuff[at_sendLen-1])
//    {
//      system_os_post(at_procTaskPrio, 0, 0);
//      at_state = at_statIpSended;
//    }
//    pDataLine++;
    break;

  case at_statIpSended: //send data
    if(temp == '\n')
    {
      system_os_post(at_busyTaskPrio, 0, 2);
//      uart0_sendStr("busy\r\n");
    }
    break;

  case at_statIpTraning:
    os_timer_disarm(&at_delayChack);
    *pDataLine = temp;
    if (pDataLine == &at_dataLine[at_dataLenMax-1])
    {
      ETS_UART_INTR_DISABLE();
//      pDataLine++;
      at_tranLen++;
      os_timer_arm(&at_delayChack, 1, 0);
      break;
    }
    pDataLine++;
    at_tranLen++;
    os_timer_arm(&at_delayChack, 20, 0);
    break;

  default:
    if(temp == '\n')
    {
    }
    break;
  }
}

/**
  * @brief  Task of process command or txdata.
  * @param  events: no used
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_procTask(os_event_t *events)
{
  if(at_state == at_statProcess)
  {
    at_cmdProcess(at_cmdLine);
    if(specialAtState)
    {
      at_state = at_statIdle;
    }
  }
  else if(at_state == at_statIpSended)
  {
    at_ipDataSending(at_dataLine);//UartDev.rcv_buff.pRcvMsgBuff);
    if(specialAtState)
    {
      at_state = at_statIdle;
    }
  }
}

static void ICACHE_FLASH_ATTR
at_busyTask(os_event_t *events)
{
  switch(events->par)
  {
  case 1:
    uart0_sendStr("\r\nbusy p...\r\n");
    break;

  case 2:
    uart0_sendStr("\r\nbusy s...\r\n");
    break;
  }

}

/**
  * @brief  Initializes build two tasks.
  * @param  None
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_init(void)
{
  system_os_task(at_busyTask, at_busyTaskPrio, at_busyTaskQueue, at_busyTaskQueueLen);
  system_os_task(at_procTask, at_procTaskPrio, at_procTaskQueue, at_procTaskQueueLen);
}

/**
  * @}
  */
