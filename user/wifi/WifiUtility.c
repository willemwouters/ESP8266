#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
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
GetIpSettings()//add get station ip and ap ip
{
  struct ip_info pTempIp;
  //char temp[64];

  if((GetWifiMode() == 3)||(GetWifiMode() == 2))
  {
    wifi_get_ip_info(0x01, &pTempIp);
    os_printf("%d.%d.%d.%d\r\n",
               IP2STR(&pTempIp.ip));
  // uart0_sendStr(temp);
//    mdState = m_gotip; /////////
  }
  if((GetWifiMode() == 1)||(GetWifiMode() == 2))
  {
    wifi_get_ip_info(0x00, &pTempIp);
    os_printf("%d.%d.%d.%d\r\n",
               IP2STR(&pTempIp.ip));
    //uart0_sendStr(temp);
//    mdState = m_gotip; /////////
  }
 // mdState = m_gotip;
 // at_backOk;
}



/**
  * @brief  Query commad of set wifi mode.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
PrintIpMode()
{
  int at_wifiMode = wifi_get_opmode();
  os_printf("%d\r\n", at_wifiMode);
}

/**
  * @brief  Query commad of module as wifi ap.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
GetIpConfig(char * data)
{
  struct softap_config apConfig;
  struct station_config stationConfig;

  wifi_station_get_config(&stationConfig);
  wifi_softap_get_config(&apConfig);
  os_sprintf(data, "\"%s\",\"%s\",%d,%d <br />\r\n",
             apConfig.ssid,
             apConfig.password,
             apConfig.channel,
             apConfig.authmode);

  os_sprintf(data, "%s \"%s\",\"%s\"<br />\r\n",
             data,
             stationConfig.ssid,
             stationConfig.password);
}
