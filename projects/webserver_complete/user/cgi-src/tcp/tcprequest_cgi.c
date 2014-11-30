
#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "wifi/WifiUtility.h"
#include "ip/TcpClient.h"


//Template code for the Tcp request
void ICACHE_FLASH_ATTR tplTcpRequest(HttpdConnData *connData, char *token, void **arg) {
	char buff[1024];
	static struct station_config stconf;
	os_printf("-%s-%s \r\n", __FILE__, __func__);
	if (token==NULL) return;
	wifi_station_get_config(&stconf);
	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "WiFiMode")==0) {
		os_strcpy(buff, "GET / HTTP/1.1\r\nCache-Control: max-age=3600\r\nUser-Agent: esp8266\r\n\r\n");
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}


char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = os_strlen(rep);
    if (!with)
        with = "";
    len_with = os_strlen(with);

    ins = orig;
    for (count = 0; (tmp = os_strstr(ins, rep)) != NULL; ++count) {
        ins = tmp + len_rep;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = os_malloc(os_strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = os_strstr(orig, rep);
        len_front = ins - orig;
        tmp = os_strncpy(tmp, orig, len_front) + len_front;
        tmp = os_strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    os_strcpy(tmp, orig);
    return result;
}

//This cgi for creating TCP Request
int ICACHE_FLASH_ATTR cgiTcpRequest(HttpdConnData *connData) {
	char ip[128];
	char port[128];
	char cmd[128];
	char *cmdp;
	os_memset(cmd, 0, 128);
	httpdFindArg(connData->postBuff, "ip", ip, sizeof(ip));
	httpdFindArg(connData->postBuff, "port", port, sizeof(port));
	httpdFindArg(connData->postBuff, "cmd", cmd, sizeof(cmd));
	cmdp = str_replace((char*) cmd, "+", " ");
   	os_printf("-%s-%s Sending to:%s %d %s\r\n", __FILE__, __func__, ip, atoi(port), cmdp);
	TcpSend(TCP, ip, atoi(port),  cmd);
	httpdRedirect(connData, "/tcp/request.tpl");
	return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR cgiTcpResponse(HttpdConnData *connData) {
	int len;
	char buff[1024];
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/html");
	httpdEndHeaders(connData);
	len=os_sprintf(buff, "{\n \"result\": { \n\"inProgress\": \"1\"\n }\n}\n");
	os_printf("-%s-%s response: %s \r\n", __FILE__, __func__, buff);		
	espconn_sent(connData->conn, (uint8 *)buff, len);
	return HTTPD_CGI_DONE;
}