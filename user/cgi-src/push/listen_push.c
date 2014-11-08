#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "wifi/WifiUtility.h"

//This cgi uses the routines above to connect to a specific access point with the
//given ESSID using the given password.
int ICACHE_FLASH_ATTR cgiPushListen(HttpdConnData *connData) {
	char * msg = "data: connection opened\n\n";
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", httpdGetMimetype(connData->url));
	espconn_sent(connData->conn, (uint8 *)msg, os_strlen(msg));
	return HTTPD_CGI_DONE;
}