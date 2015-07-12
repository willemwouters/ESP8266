#include <string.h>
#include <osapi.h>
#include <user_interface.h>
#include "mem.h"
#include "espmissingincludes.h"

#include <espconn.h>
#include "TcpClient.h"


char data[1024] = { 0 };
espConnectionType singleConnection;
static ip_addr_t host_ip;

/**
  * @brief  Client received callback function.
  * @param  arg: contain the ip link information
  * @param  pdata: received data
  * @param  len: the lenght of received data
  * @retval None
  */
static void ICACHE_FLASH_ATTR TcpClient_Receive(void *arg, char *pdata, unsigned short len)
{
 os_printf("-%s-%s Received data: %s \r\n", __FILE__, __func__, pdata);
 return;
}

/**
  * @brief  Client send over callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
TcpClient_Send_Cb(void *arg) {
  os_printf("-%s-%s \r\n", __FILE__, __func__);
}


/**
  * @brief  Tcp client disconnect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
TcpClient_Disconnect_Cb(void *arg) {
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;

  if(pespconn == NULL)  {
    return;
  }
  if(pespconn->proto.tcp != NULL)  {
    os_free(pespconn->proto.tcp);
  }
  os_free(pespconn);
  linkTemp->linkEn = FALSE;
  os_printf("-%s-%s \r\n", __FILE__, __func__);
}

/**
  * @brief  Tcp client connect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
TcpClient_Connect_Cb(void *arg) {
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;

  os_printf("-%s-%s \r\n", __FILE__, __func__);

  linkTemp->linkEn = TRUE;
  linkTemp->teType = teClient;
  linkTemp->repeaTime = 0;
  
  os_printf("-%s-%s data: %s \r\n", __FILE__, __func__, data);

  espconn_sent(pespconn, (uint8_t *) data, os_strlen(data));
}

/**
  * @brief  Tcp client connect repeat callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
TcpClient_Reconnect_Cb(void *arg, sint8 errType) {
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *) pespconn->reverse;
  if(linkTemp->linkEn) {
    os_printf("-%s-%s ALREADY CONNECTED \r\n", __FILE__, __func__);
    return;
  }
    os_printf("-%s-%s  \r\n", __FILE__, __func__);

  if(linkTemp->teToff == TRUE) {
    linkTemp->teToff = FALSE;
    linkTemp->repeaTime = 0;
    if(pespconn->proto.tcp != NULL) {
      os_free(pespconn->proto.tcp);
    }
    os_free(pespconn);
    linkTemp->linkEn = false;
  }   else   {
    linkTemp->repeaTime++;
    if(linkTemp->repeaTime >= 1)  {
      os_printf("-%s-%s repeat over %d \r\n", __FILE__, __func__, linkTemp->repeaTime);
      linkTemp->repeaTime = 0;
      if(pespconn->proto.tcp != NULL)  {
        os_free(pespconn->proto.tcp);
      }
      os_free(pespconn);
      linkTemp->linkEn = false;
      os_printf("-%s-%s disconnected \r\n", __FILE__, __func__);      
      return;
    }
    os_printf("-%s-%s repeat: %d \r\n", __FILE__, __func__, linkTemp->repeaTime);


    pespconn->proto.tcp->local_port = espconn_port();
    espconn_connect(pespconn);
  }
}


/******************************************************************************
 * FunctionName : user_esp_platform_dns_found
 * Description  : dns found callback
 * Parameters   : name -- pointer to the name that was looked up.
 *                ipaddr -- pointer to an ip_addr_t containing the IP address of
 *                the hostname, or NULL if the name could not be found (or on any
 *                other error).
 *                callback_arg -- a user-specified callback argument passed to
 *                dns_gethostbyname
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
TcpClient_Dns_Cb(const char *name, ip_addr_t *ipaddr, void *arg) {
  struct espconn *pespconn = (struct espconn *) arg;
  at_linkConType *linkTemp = (at_linkConType *) pespconn->reverse;
  if(ipaddr == NULL)  {
    linkTemp->linkEn = FALSE;
    os_printf("-%s-%s DNS Fail \r\n", __FILE__, __func__);      

    return;
  }


  os_printf("-%s-%s  DNS found: %d.%d.%d.%d\n", 
            __FILE__, __func__,
            *((uint8 *) &ipaddr->addr),
            *((uint8 *) &ipaddr->addr + 1),
            *((uint8 *) &ipaddr->addr + 2),
            *((uint8 *) &ipaddr->addr + 3));

  if(host_ip.addr == 0 && ipaddr->addr != 0) {
    if(pespconn->type == ESPCONN_TCP)  {
      os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);
      espconn_connect(pespconn);
    } else {
      os_memcpy(pespconn->proto.udp->remote_ip, &ipaddr->addr, 4);
      espconn_connect(pespconn);
    }
  }
}


/******************************************************************************
 * FunctionName : TcpSend
 * Description  : tcp full send
 * Parameters   : type -- type of call UDP / TCP
 *                ipAddress -- char * of the ip or host of the tcp server
 *                port -- port of the tcp server
 *                cmd -- will be send to tcp server
 * Returns      : none -- output will be send over uart
*******************************************************************************/
void TcpSend(espTypes type, char * ipAddress, int32_t port, char * cmd) {
  char ipTemp[128];
  uint32_t ip = 0;
  os_memcpy(data, cmd, os_strlen(cmd));
  singleConnection.pCon = (struct espconn *)os_zalloc(sizeof(struct espconn));
  singleConnection.pCon->state = ESPCONN_NONE;
  singleConnection.linkId = 0;
  ip = ipaddr_addr(ipAddress);
  singleConnection.pCon->type = type;
  switch(type)
  {
    case ESPCONN_TCP:
      os_printf("-%s-%s DNS Fail Setting up connection to: %s %d  \r\n", __FILE__, __func__, ipAddress, (int) port);      
      singleConnection.pCon->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
      singleConnection.pCon->proto.tcp->local_port = espconn_port();
      singleConnection.pCon->proto.tcp->remote_port = port;
      os_memcpy(singleConnection.pCon->proto.tcp->remote_ip, &ip, 4);
      singleConnection.pCon->reverse = &singleConnection;

      espconn_regist_recvcb(singleConnection.pCon, TcpClient_Receive);////////
      espconn_regist_sentcb(singleConnection.pCon, TcpClient_Send_Cb);///////
      espconn_regist_connectcb(singleConnection.pCon, TcpClient_Connect_Cb);
      espconn_regist_disconcb(singleConnection.pCon, TcpClient_Disconnect_Cb);
      espconn_regist_reconcb(singleConnection.pCon, TcpClient_Reconnect_Cb);
      if((ip == 0xffffffff) && (os_memcmp(ipTemp,"255.255.255.255",16) != 0))  {
        os_printf("-%s-%s Troubles with ip, going to resolve... to:: %s \r\n", __FILE__, __func__, ipAddress);      
        espconn_gethostbyname(singleConnection.pCon, ipAddress, &host_ip, TcpClient_Dns_Cb);
      } else  {
        os_printf("-%s-%s Connecting via tcp to: %s \r\n", __FILE__, __func__, ipAddress);      
        espconn_connect(singleConnection.pCon);
      }
    break;

    case ESPCONN_UDP:
      singleConnection.pCon->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
      singleConnection.pCon->proto.udp->local_port = espconn_port();
      singleConnection.pCon->proto.udp->remote_port = port;
      os_memcpy(singleConnection.pCon->proto.udp->remote_ip, &ip, 4);
      singleConnection.pCon->reverse = &singleConnection;
      os_printf("%d\r\n", singleConnection.pCon->proto.udp->remote_port);///
      singleConnection.linkEn = TRUE;
      singleConnection.teType = teClient;
      espconn_regist_recvcb(singleConnection.pCon, TcpClient_Receive);
      espconn_regist_sentcb(singleConnection.pCon, TcpClient_Send_Cb);
      espconn_regist_connectcb(singleConnection.pCon, TcpClient_Connect_Cb);
      espconn_regist_disconcb(singleConnection.pCon, TcpClient_Disconnect_Cb);
      espconn_regist_reconcb(singleConnection.pCon, TcpClient_Reconnect_Cb);
      if((ip == 0xffffffff) && (os_memcmp(ipTemp,"255.255.255.255",16) != 0))  {
        espconn_gethostbyname(singleConnection.pCon, ipTemp, &host_ip, TcpClient_Dns_Cb);
      } else {
        os_printf("-%s-%s Connecting via udp to:%s local port:%d remote port:%lu \r\n", __FILE__, __func__, ipAddress, singleConnection.pCon->proto.udp->local_port, port);      
        espconn_create(singleConnection.pCon);
        //espconn_sent(singleConnection.pCon, (uint8_t *) data, os_strlen(data));
      }
    break;

    default:
    break;
  }

}


