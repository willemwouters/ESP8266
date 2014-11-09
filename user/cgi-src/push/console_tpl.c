#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "ip/TcpClient.h"
#include "ip/TcpServer.h"
#include "wifi/WifiUtility.h"
//#include "io/io.h"
#include "gpio.h"
#include "io/key.h"


//Template code for the WLAN page.
void ICACHE_FLASH_ATTR tplPushConsole(HttpdConnData *connData, char *token, void **arg) {
	char buff[1024];
	if (token==NULL) return;
	
	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "GPIO0")==0) {
		
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}

