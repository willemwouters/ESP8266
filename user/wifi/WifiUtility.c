#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"


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
GetIpSettings(char * data)
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
