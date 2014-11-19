#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "wifi/WifiUtility.h"

//This cgi is when initial connection is setup with push example page   listen.push
int ICACHE_FLASH_ATTR cgiPushConsole(HttpdConnData *connData) {
	char * msg = "data: connection opened\n\n";
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", httpdGetMimetype(connData->url));
	espconn_sent(connData->conn, (uint8 *)msg, os_strlen(msg));
	return HTTPD_CGI_DONE;
}