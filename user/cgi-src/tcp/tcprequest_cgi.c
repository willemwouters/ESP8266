
#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "wifi/WifiUtility.h"
#include "ip/TcpClient.h"


///Template code for the WLAN page.
void ICACHE_FLASH_ATTR tplTcpRequest(HttpdConnData *connData, char *token, void **arg) {
	char buff[1024];
	static struct station_config stconf;
	if (token==NULL) return;
	wifi_station_get_config(&stconf);
	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "WiFiMode")==0) {
		
		char ipSettings[256] = { 0};
		GetIpSettings(ipSettings);
		os_strcpy(buff, ipSettings);
	} else if (os_strcmp(token, "cmd")==0) {
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

//This cgi uses the routines above to connect to a specific access point with the
//given ESSID using the given password.
int ICACHE_FLASH_ATTR cgiTcpRequest(HttpdConnData *connData) {
	char ip[128];
	char port[128];
	char cmd[128];
	char *cmdp;
	os_memset(cmd, 0, 128);
	//	os_memset(cmdp, 0, 128);

	// if (connData->conn==NULL) {
	// 	//Connection aborted. Clean up.
	// 	return HTTPD_CGI_DONE;
	// }

	//wifi_softap_get_config(&apconf);

	httpdFindArg(connData->postBuff, "ip", ip, sizeof(ip));
	httpdFindArg(connData->postBuff, "port", port, sizeof(port));
	httpdFindArg(connData->postBuff, "cmd", cmd, sizeof(cmd));
	cmdp = str_replace((char*) cmd, "+", " ");
		// cmdp = str_replace((char*) cmd, "%0D", "\r");
		// cmdp = str_replace((char*) cmd, "%0A", "\n");

	//cmd = str_replace((char*) cmd, "%2f", "/");

	os_printf("Sending to: \r\n%s %d %s--- \r\n", ip, atoi(port), cmdp );
	//TcpSend(TCP, "192.168.178.1", 80, "GET / HTTP/1.0\r\n\r\n");
	TcpSend(TCP, ip, atoi(port),  cmd);

	httpdRedirect(connData, "/tcp/request.tpl");
	return HTTPD_CGI_DONE;
}

//This CGI is called from the bit of AJAX-code in wifi.tpl. It will initiate a
//scan for access points and if available will return the result of an earlier scan.
//The result is embedded in a bit of JSON parsed by the javascript in wifi.tpl.
int ICACHE_FLASH_ATTR cgiTcpResponse(HttpdConnData *connData) {
	int len;
	char buff[1024];
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/html");
	httpdEndHeaders(connData);
	len=os_sprintf(buff, "{\n \"result\": { \n\"inProgress\": \"1\"\n }\n}\n");
	espconn_sent(connData->conn, (uint8 *)buff, len);
	return HTTPD_CGI_DONE;
}