#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "wifi/WifiUtility.h"


//Template code for the WLAN status page.
void ICACHE_FLASH_ATTR tplStatus(HttpdConnData *connData, char *token, void **arg) {
	char buff[1024];
	static struct station_config stconf;
		os_printf("-%s-%s \r\n", __FILE__, __func__);

	if (token==NULL) return;
	wifi_station_get_config(&stconf);
	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "WiFiMode")==0) {
		char wifiConfig[256] = { 0};
		GetWifiConfig(wifiConfig);
		os_strcpy(buff, wifiConfig);		
	} else if (os_strcmp(token, "IpSettings")==0) {
		char wifiConfig[256] = { 0};
		GetIpConfig(wifiConfig);
		os_strcpy(buff, wifiConfig);
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}