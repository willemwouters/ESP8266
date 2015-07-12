#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "httpd/httpd.h"
#include "io/io.h"
#include "httpd/httpdespfs.h"
#include "user_interface.h"
#include "cgi-src/wifi/wifi_cgi.h"
#include "cgi-src/wifi/wifi_tpl.h"
#include "cgi-src/wifi/status_tpl.h"
#include "cgi-src/tcp/tcprequest_cgi.h"
#include "cgi-src/push/listen_push.h"
#include "cgi-src/push/index_tpl.h"
#include "cgi-src/push/console_tpl.h"
#include "cgi-src/push/console_push.h"
#include "cgi-src/push/io_push.h"
#include "cgi-src/tcp/serverconfig_tpl.h"
#include "cgi-src/tcp/serverstatus_tpl.h"
#include "tty/stdout.h"

#include "TcpClient.h"

HttpdBuiltInUrl builtInUrls[]={
	{"/", cgiRedirect, "/index.tpl"},
	//Routines to make the /wifi URL and everything beneath it work.
	{"/wifi", cgiRedirect, "/wifi/wifi.tpl"},
	{"/wifi/", cgiRedirect, "/wifi/wifi.tpl"},
	{"/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
	{"/wifi/status.tpl", cgiEspFsTemplate, tplStatus},
	{"/wifi/WiFiScan.cgi", cgiWiFiScan, NULL},
	{"/wifi/WiFiConnect.cgi", cgiWiFiConnect},
	{"/wifi/WiFiApSave.cgi", cgiWiFiApSave},

	{"/push", cgiRedirect, "/push/index.tpl"},
	{"/push/", cgiRedirect, "/push/index.tpl"},
	{"/push/listen.push", cgiPushListen},
	{"/push/io.push", cgiIOListen},
	{"/push/index.tpl", cgiEspFsTemplate, tplPushIndex},
	{"/push/console.push", cgiPushConsole},
	{"/push/console.tpl", cgiEspFsTemplate, tplPushConsole},
	{"/push/index.cgi", cgiPushIndexRequest},

	{"/tcp", cgiRedirect, "/tcp/request.tpl"},
	{"/tcp/", cgiRedirect, "/tcp/request.tpl"},
	{"/tcp/request.tpl", cgiEspFsTemplate, tplTcpRequest},
	{"/tcp/serverconfig.tpl", cgiEspFsTemplate, tplTcpServerConfig},
	{"/tcp/TcpServerSave.cgi", cgiTcpServerSave},
	{"/tcp/TcpServerEnable.cgi", cgiTcpServerEnable},
	
	{"/tcp/serverstatus.tpl", cgiEspFsTemplate, tplTcpServerStatus},
	{"/tcp/TcpResponse.cgi", cgiTcpResponse, NULL},
	{"/tcp/TcpRequest.cgi", cgiTcpRequest},

	{"/index.tpl", cgiEspFsTemplate, NULL},

	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL}
};

static ETSTimer resetTimer;

void mainUser(void * arg) {
	char    *hostname=(char *)"200.20.186.76";
	char msg[48]={010,0,0,0,0,0,0,0,0};    // the packet we send
	int portno=123;     //NTP is port 123
//int maxlen=1024;        //check our buffers

	TcpSend(UDP, hostname, portno,  msg);

		os_timer_disarm(&resetTimer);
		os_timer_setfn(&resetTimer, mainUser, NULL);
		//os_printf("-%s-%s  main loop \r\n", __FILE__, __func__);
		//time(NULL);


		os_timer_arm(&resetTimer, 20000, 0);
}

void WifiConnectCb(void* arg) {
	int x=wifi_station_get_connect_status();
	if (x==STATION_GOT_IP) {
		os_printf("-%s-%s Got an ip\r\n", __FILE__, __func__);
		os_timer_disarm(&resetTimer);
		os_timer_setfn(&resetTimer, mainUser, NULL);
		os_printf("-%s-%s  Main loop starting. \r\n", __FILE__, __func__);
		os_timer_arm(&resetTimer, 20000, 0);
	} else {
		os_timer_disarm(&resetTimer);
		os_timer_setfn(&resetTimer, WifiConnectCb, NULL);
		os_printf("-%s-%s  Wait for IP \r\n", __FILE__, __func__);
		os_timer_arm(&resetTimer, 10000, 0);
	}
}

void user_init(void) {
	stdoutInit();
	ioInit();
	httpdInit(builtInUrls, 80);
	os_timer_disarm(&resetTimer);
	os_timer_setfn(&resetTimer, WifiConnectCb, NULL);
	os_printf("-%s-%s Schedule main loop after Wifi connect \r\n", __FILE__, __func__);
	os_timer_arm(&resetTimer, 5000, 0);
	os_printf("\nReady \n");
}

