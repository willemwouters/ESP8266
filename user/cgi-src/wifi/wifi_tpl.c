#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "ip/TcpClient.h"
#include "ip/TcpServer.h"
#include "wifi/WifiUtility.h"



//Template code for the WLAN page.
void ICACHE_FLASH_ATTR tplWlan(HttpdConnData *connData, char *token, void **arg) {
	char buff[1024];
	int x;
    struct softap_config apConfig;
	static struct station_config stconf;
    os_printf("-%s-%s  \r\n", __FILE__, __func__);
	if (token==NULL) return;
	wifi_station_get_config(&stconf);
    wifi_softap_get_config(&apConfig);

	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "WiFiMode")==0) {
		x=wifi_get_opmode();
		if (x==1) os_strcpy(buff, "Client");
		if (x==2) os_strcpy(buff, "SoftAP");
		if (x==3) os_strcpy(buff, "STA+AP");

		//
		//SetupServer("1,333");
	} else if (os_strcmp(token, "ssid")==0) {
		os_strcpy(buff, (char*)apConfig.ssid);
	} else if (os_strcmp(token, "password")==0) {
		os_strcpy(buff, (char*)apConfig.password);
	} else if (os_strcmp(token, "currSsid")==0) {
		os_strcpy(buff, (char*)stconf.ssid);
	} else if (os_strcmp(token, "WiFiPasswd")==0) {
		os_strcpy(buff, (char*)stconf.password);
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}

