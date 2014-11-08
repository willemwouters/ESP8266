#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "ip/TcpClient.h"
#include "ip/TcpServer.h"
#include "wifi/WifiUtility.h"






//This cgi uses the routines above to connect to a specific access point with the
//given ESSID using the given password.
int ICACHE_FLASH_ATTR cgiPushIndexRequest(HttpdConnData *connData) {
	char text[128];
	httpdFindArg(connData->postBuff, "text", text, sizeof(text));
	httpdPushMessage("/push/listen.push", text);
	espconn_sent(connData->conn, (uint8 *) " ", 1);
	return HTTPD_CGI_DONE;
}

//Template code for the WLAN page.
void ICACHE_FLASH_ATTR tplPushIndex(HttpdConnData *connData, char *token, void **arg) {
	char buff[1024];
   
	if (token==NULL) return;
	
	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "WiFiMode")==0) {
			os_strcpy(buff, "Unknown");
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}

