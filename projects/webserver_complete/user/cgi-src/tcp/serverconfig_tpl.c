#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "ip/TcpServer.h"

//This cgi uses the routines POST save tcp server enable
int ICACHE_FLASH_ATTR cgiTcpServerEnable(HttpdConnData *connData) {
	int typeI = 0;
	char state[128];
	httpdFindArg(connData->postBuff, "state", state, sizeof(state));
	os_printf("-%s-%s Change to tcp state: %s \r\n", __FILE__, __func__, state);
	//TcpSend(TCP, ip, atoi(port),  cmd);
	if(os_strcmp(state, "1") == 0) {
		typeI = 1;
	}
	SetStateServer(typeI);
	httpdRedirect(connData, "/tcp/serverconfig.tpl");
	return HTTPD_CGI_DONE;
}

//This cgi uses the routines POST save tcp server settings
int ICACHE_FLASH_ATTR cgiTcpServerSave(HttpdConnData *connData) {
	char type[128];
	char port[128];
	int typeI  = 3;
	httpdFindArg(connData->postBuff, "type", type, sizeof(type));
	httpdFindArg(connData->postBuff, "port", port, sizeof(port));
	if(os_strcmp(type, "TCP") == 0) {
		typeI = 1;
	}
	if(os_strcmp(type, "UDP") == 0) {
		typeI=2;
	}
	os_printf("-%s-%s Saving tcp settings: type=%d, port=%s\r\n", __FILE__, __func__, typeI, port);
	SetupServer(1, atoi(port), typeI);
	httpdRedirect(connData, "/tcp/serverconfig.tpl");
	return HTTPD_CGI_DONE;
}

//This tpl page for TCP server config
void ICACHE_FLASH_ATTR tplTcpServerConfig(HttpdConnData *connData, char *token, void **arg) {
	char buff[1024];
	os_printf("-%s-%s \r\n", __FILE__, __func__);
	static struct station_config stconf;
	if (token==NULL) return;
	wifi_station_get_config(&stconf);

	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "status")==0) {
		if(IsServerRunning()) {
			os_strcpy(buff, "Running");
		} else {
			os_strcpy(buff, "Not Running");
		}
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}