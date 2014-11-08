#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "wifi/WifiUtility.h"


//Template code for the WLAN page.
void ICACHE_FLASH_ATTR tplTcpServerStatus(HttpdConnData *connData, char *token, void **arg) {
	char buff[1024];
	//int x;
	static struct station_config stconf;


	if (token==NULL) return;
	wifi_station_get_config(&stconf);

	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "WiFiMode")==0) {
		
		char ipSettings[256] = { 0};
		GetIpSettings(ipSettings);
		os_strcpy(buff, ipSettings);
		//delete ipSettings;
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}