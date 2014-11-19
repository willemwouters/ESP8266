#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "wifi/WifiUtility.h"

//This cgi is when initial connection is setup with push page   io.push
int ICACHE_FLASH_ATTR cgiIOListen(HttpdConnData *connData) {
	char * msg = "data: connection opened\n\n";
   	os_printf("-%s-%s \r\n", __FILE__, __func__);	
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", httpdGetMimetype(connData->url));
	espconn_sent(connData->conn, (uint8 *)msg, os_strlen(msg));
	return HTTPD_CGI_DONE;
}