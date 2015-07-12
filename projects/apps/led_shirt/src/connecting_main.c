/*
 * startup_main.c
 *
 *  Created on: Mar 14, 2015
 *      Author: wouters
 */

#include "config.h"
#include "user_interface.h"
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"

#define SEARCH_TRIES 2
int scan_tries = 0;
void scan();

void tick_connecting() {
	uint8 at_wifiMode = wifi_get_opmode();
	if(at_wifiMode == SOFTAP_MODE)
	{
		os_printf("Cannot list ap in softap mode\r\n");
		return;
	}
	else
	{
	  if(scan_tries >= SEARCH_TRIES) {
		  config_mode = MODE_MASTER_S;
	  } else {
		  os_printf("Starting scan, tries: %d \r\n", scan_tries);
		  scan();
	  }
	}
}


/**
  * @brief  Wifi ap scan over callback to display.
  * @param  arg: contain the aps information
  * @param  status: scan over status
  * @retval None
  */
static void ICACHE_FLASH_ATTR
scan_done(void *arg, STATUS status)
{
  uint8 ssid[33];
  char temp[128];

  if (status == OK)
  {
    struct bss_info *bss_link = (struct bss_info *)arg;
    bss_link = bss_link->next.stqe_next;//ignore first
    scan_tries++;

    while (bss_link != 0)
    {
      os_memset(ssid, 0, 33);
      if (os_strlen(bss_link->ssid) <= 32)
      {
        os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
      }
      else
      {
        os_memcpy(ssid, bss_link->ssid, 32);
      }
      //os_printf("%s \r\n", bss_link->ssid);
      if(os_strcmp(bss_link->ssid, MASTER_AP) == 0) {
    	  os_printf("FOUND MASTER AP\r\n");
    	  scan_tries = 0;
    	  config_mode = MODE_SLAVE;
      }
      os_sprintf(temp,"+CWLAP:(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
                 bss_link->authmode, ssid, bss_link->rssi,
                 MAC2STR(bss_link->bssid),bss_link->channel);
      os_printf(temp);
      bss_link = bss_link->next.stqe_next;
    }
  }

}


void scan() {
	struct scan_config config;

	wifi_station_scan(NULL, scan_done);


}

