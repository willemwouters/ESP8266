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
GetIpConfig(char * data)//add get station ip and ap ip
{
  struct ip_info pTempIp;
  os_printf("-%s-%s  Wifi mode 1= %d\r\n", __FILE__, __func__, GetWifiMode());   
  if((GetWifiMode() == 2)||(GetWifiMode() == 3))
  {
    wifi_get_ip_info(0x01, &pTempIp);
    os_sprintf(data, "%d.%d.%d.%d\r\n", IP2STR(&pTempIp.ip));
  }
  if((GetWifiMode() == 1)||(GetWifiMode() == 3))
  {
    wifi_get_ip_info(0x00, &pTempIp);
    os_sprintf(data, "%s %d.%d.%d.%d\r\n", data, IP2STR(&pTempIp.ip));
  }
}



void ICACHE_FLASH_ATTR
PrintIpMode()
{
  int at_wifiMode = wifi_get_opmode();
  os_printf("-%s-%s   %d\r\n", __FILE__, __func__, at_wifiMode);   
}

void ICACHE_FLASH_ATTR
GetWifiConfig(char * data)
{
  struct softap_config apConfig;
  struct station_config stationConfig;
  os_printf("-%s-%s \r\n", __FILE__, __func__); 
  wifi_station_get_config(&stationConfig);
  wifi_softap_get_config(&apConfig);
  os_sprintf(data, " \"%s\",\"%s\",%d,%d <br />\r\n",
             apConfig.ssid,
             apConfig.password,
             apConfig.channel,
             apConfig.authmode);

  os_sprintf(data, "%s \"%s\",\"%s\"<br />\r\n",
             data,
             stationConfig.ssid,
             stationConfig.password);
}
