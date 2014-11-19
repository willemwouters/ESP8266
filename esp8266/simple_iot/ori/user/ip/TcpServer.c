#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd.h"
#include "io.h"
#include "espmissingincludes.h"

#include "ipconfig.h"
#include "ip/TcpServer.h"
static BOOL serverEn = false;
BOOL IPMODE;

static BOOL at_ipMux = true;

//static uint8_t repeat_time = 0;
static uint16_t server_timeover = 180;
static int at_linkNum = 0;
 BOOL specialAtState;
 at_stateType at_state;

static struct espconn *pTcpServer;
static struct espconn *pUdpServer;

os_timer_t at_delayChack;
char dataServer[1024] = { 0 };
espServerConnectionType singleConnection;
static espServerConnectionType pLink[at_linkMax];


/**
  * @brief  Tcp server disconnect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_tcpserver_discon_cb(void *arg)
{
  struct espconn *pespconn = (struct espconn *) arg;
  espServerConnectionType *linkTemp = (espServerConnectionType *) pespconn->reverse;

         os_printf("-%s-%s  \r\n", __FILE__, __func__);      

  if (pespconn == NULL)
  {
    return;
  }
  os_free(pespconn);
  linkTemp->linkEn = FALSE;
  linkTemp->pCon = NULL;
  if(linkTemp->teToff == TRUE)
  {
    linkTemp->teToff = FALSE;
   // specialAtState = true;
    //  at_state = at_statIdle;
    //  at_backOk;
  }
  
}


/**
  * @brief  Client received callback function.
  * @param  arg: contain the ip link information
  * @param  pdata: received data
  * @param  len: the lenght of received data
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_tcpclient_recv(void *arg, char *pdata, unsigned short len)
{
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;
  //char temp[32];
  os_printf("-%s-%s data:%s \r\n", __FILE__, __func__, pdata);      

  if(at_ipMux)
  {
 os_printf("-%s-%s  +IPD,%d,%d: %s\r\n", __FILE__, __func__,  linkTemp->linkId, len, pdata);      

   // uart0_sendStr(temp);
    //uart0_tx_buffer(pdata, len);
  }
  else if(IPMODE == FALSE)
  {
    os_printf("-%s-%s  +IPD %d: %s\r\n", __FILE__, __func__,  len, pdata);      
 //   uart0_sendStr(temp);
  ///  uart0_tx_buffer(pdata, len);
  }
  else
  {
    return;
  }
  //at_backOk;
}


/**
  * @brief  Client send over callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_tcpclient_sent_cb(void *arg)
{
//  os_free(at_dataLine);
  if(IPMODE == TRUE)
  {
    os_timer_disarm(&at_delayChack);
    os_timer_arm(&at_delayChack, 20, 0);
    ETS_UART_INTR_ENABLE();
    return;
  }
 os_printf("-%s-%s  \r\n", __FILE__, __func__);      
  specialAtState = TRUE;
  at_state = at_statIdle;
}
/**
  * @brief  Tcp server connect repeat callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_tcpserver_recon_cb(void *arg, sint8 errType)
{
  struct espconn *pespconn = (struct espconn *)arg;
  espServerConnectionType *linkTemp = (espServerConnectionType *)pespconn->reverse;

 os_printf("-%s-%s  \r\n", __FILE__, __func__);      

  if(pespconn == NULL)
  {
    return;
  }

  linkTemp->linkEn = false;
  linkTemp->pCon = NULL;
  
  if(linkTemp->teToff == TRUE)
  {
    linkTemp->teToff = FALSE;
   // specialAtState = true;
    //  at_state = at_statIdle;
    //  at_backOk;
  }
}
LOCAL void ICACHE_FLASH_ATTR
at_tcpserver_listen(void *arg)
{
  struct espconn *pespconn = (struct espconn *)arg;
  uint8_t i;

 os_printf("-%s-%s  \r\n", __FILE__, __func__);      
  for(i=0;i<at_linkMax;i++)
  {
    if(pLink[i].linkEn == FALSE)
    {
      pLink[i].linkEn = TRUE;
      break;
    }
  }
  if(i>=5)
  {
    return;
  }
  pLink[i].teToff = FALSE;
  pLink[i].linkId = i;
  pLink[i].teType = teServer;
  pLink[i].repeaTime = 0;
  pLink[i].pCon = pespconn;
  //mdState = m_linked;
  at_linkNum++;
  pespconn->reverse = &pLink[i];
  espconn_regist_recvcb(pespconn, at_tcpclient_recv);
  espconn_regist_reconcb(pespconn, at_tcpserver_recon_cb);
  espconn_regist_disconcb(pespconn, at_tcpserver_discon_cb);
  espconn_regist_sentcb(pespconn, at_tcpclient_sent_cb);///////

}


/**
  * @brief  Udp server receive data callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
LOCAL void ICACHE_FLASH_ATTR
at_udpserver_recv(void *arg, char *pusrdata, unsigned short len)
{
  struct espconn *pespconn = (struct espconn *)arg;
  espServerConnectionType *linkTemp;
 // char temp[32];
  uint8_t i;

 os_printf("-%s-%s  \r\n", __FILE__, __func__);      

  if(pespconn->reverse == NULL)
  {
    for(i = 0;i < at_linkMax;i++)
    {
      if(pLink[i].linkEn == FALSE)
      {
        pLink[i].linkEn = TRUE;
        break;
      }
    }
    if(i >= 5)
    {
      return;
    }
    pLink[i].teToff = FALSE;
    pLink[i].linkId = i;
    pLink[i].teType = teServer;
    pLink[i].repeaTime = 0;
    pLink[i].pCon = pespconn;
    espconn_regist_sentcb(pLink[i].pCon, at_tcpclient_sent_cb);
    //mdState = m_linked;
    at_linkNum++;
    pespconn->reverse = &pLink[i];
    os_printf("-%s-%s Link \r\n", __FILE__, __func__);      

  }
  linkTemp = (espServerConnectionType *)pespconn->reverse;
  if(pusrdata == NULL)
  {
    return;
  }
  os_printf("-%s-%s DNS Fail +IPD,%d,%d: %s \r\n", __FILE__, __func__, linkTemp->linkId, len, pusrdata);      
  //uart0_sendStr(temp);
 // uart0_tx_buffer(pusrdata, len);
 // at_backOk;
}

BOOL IsServerRunning() {
  return serverEn;
}



void ICACHE_FLASH_ATTR SetStateServer(BOOL enable) {
 os_printf("-%s-%s  \r\n", __FILE__, __func__);      
  int i;
  if(enable == false) {
     for(i = 0;i < at_linkMax;i++)
      {
        if(pLink[i].linkEn == TRUE)
        {
         espconn_disconnect(pLink[i].pCon);
         os_free(pLink[i].pCon);
         pLink[i].pCon = NULL;
        }
      }
      espconn_disconnect(pTcpServer);
      espconn_delete(pUdpServer);
      serverEn = false;
      os_free(pTcpServer);
      pTcpServer = NULL;
      os_free(pUdpServer);
      pUdpServer = NULL;
  } else {
    espconn_create(pUdpServer);
    espconn_accept(pTcpServer);
  }
}

void ICACHE_FLASH_ATTR GetTcpServerStatus(char * data) 
{
  os_printf("-%s-%s Tcp Server running:  %d\r\n", __FILE__, __func__, serverEn); 
  os_sprintf(data," Tcp Server running: %d \r\n", serverEn);
  if(serverEn) {
    if(pTcpServer != NULL) {
      os_printf("-%s-%s Tcp Server \r\n", __FILE__, __func__); 
      os_sprintf(data, " Tcp Server port: %d \r\n", pTcpServer->proto.tcp->local_port);
    }
    if(pUdpServer != NULL) {
      os_printf("-%s-%s Udp Server \r\n", __FILE__, __func__); 
      os_sprintf(data, " Udp Server port: %d \r\n", pUdpServer->proto.udp->local_port);
    }
  }
}
/**
  * @brief  Setup commad of module as server.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
SetupServer(int enable, int port, int type)
{

  //char temp[32];
  if(at_ipMux == FALSE)
  {
   // at_backError;
    return;
  }

  if(enable == 0)
  {
    os_printf("-%s-%s SetupServer error 1 \r\n", __FILE__, __func__);      
      return;
    
  }
   
  if(enable == serverEn)
  {
    os_printf("-%s-%s ERROR ERROR no change \r\n", __FILE__, __func__);      
    return;
  }

  if(enable)
  {
    

    if(type != 2) {
      pTcpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
      if (pTcpServer == NULL)
      {
        os_printf("-%s-%s TcpServer Failure \r\n", __FILE__, __func__);      
        return;
      }
        pTcpServer->type = ESPCONN_TCP;
        pTcpServer->state = ESPCONN_NONE;
        pTcpServer->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
        pTcpServer->proto.tcp->local_port = port;
        espconn_regist_connectcb(pTcpServer, at_tcpserver_listen);
        espconn_accept(pTcpServer);
        espconn_regist_time(pTcpServer, server_timeover, 0);
        os_printf("-%s-%sTcpServer success port: %d \r\n", __FILE__, __func__, (int) port);      
      }

      if(type != 1) {
        pUdpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
        if (pUdpServer == NULL)
        {
          os_printf("-%s-%s UdpServer Failure \r\n", __FILE__, __func__);      
          return;
        }
        pUdpServer->type = ESPCONN_UDP;
        pUdpServer->state = ESPCONN_NONE;
        pUdpServer->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
        pUdpServer->proto.udp->local_port = port;
        pUdpServer->reverse = NULL;
        espconn_regist_recvcb(pUdpServer, at_udpserver_recv);
        espconn_create(pUdpServer);
        os_printf("-%s-%s UdpServer success port: %d \r\n", __FILE__, __func__, (int) port);      
      }
  }
  else
  {
    /* restart */
    os_printf("-%s-%s  we must restart \r\n", __FILE__, __func__);          
    return;
  }
  serverEn = enable;
 // at_backOk;
}


//static void ICACHE_FLASH_ATTR
//user_tcp_discon_cb(void *arg)
//{
//  struct espconn *pespconn = (struct espconn *)arg;
//
//  if(pespconn == NULL)
//  {
//    return;
//  }
////  if(pespconn->proto.tcp != NULL)
////  {
////    os_free(pespconn->proto.tcp);
////  }
////  os_free(pespconn);
////  pServerCon = NULL;
//  os_printf("disconnect\r\n");
//  os_printf("pespconn %p\r\n", pespconn);
//}
