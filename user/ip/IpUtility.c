#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "io/io.h"
#include "espmissingincludes.h"


int GetWifiMode() {
  return wifi_get_opmode();
}

int GetWifiStatus() {
  int x=wifi_station_get_connect_status();
  return x;
}
/**
  * @brief  Execution commad of get module ip.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdCifsr(uint8_t id)//add get station ip and ap ip
{
  struct ip_info pTempIp;
  char temp[64];

  if((GetWifiMode() == 3)||(GetWifiMode() == 2))
  {
    wifi_get_ip_info(0x01, &pTempIp);
    os_sprintf(temp, "%d.%d.%d.%d\r\n",
               IP2STR(&pTempIp.ip));
  // uart0_sendStr(temp);
//    mdState = m_gotip; /////////
  }
  if((GetWifiMode() == 1)||(GetWifiMode() == 2))
  {
    wifi_get_ip_info(0x00, &pTempIp);
    os_sprintf(temp, "%d.%d.%d.%d\r\n",
               IP2STR(&pTempIp.ip));
    //uart0_sendStr(temp);
//    mdState = m_gotip; /////////
  }
 // mdState = m_gotip;
 // at_backOk;
}

