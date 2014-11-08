#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "httpd/httpd.h"
#include "io/io.h"
#include "httpd/httpdespfs.h"
#include "cgi-src/wifi/wifi_cgi.h"
#include "cgi-src/wifi/wifi_tpl.h"
#include "cgi-src/wifi/status_tpl.h"
#include "cgi-src/tcp/tcprequest_cgi.h"
#include "cgi-src/push/listen_push.h"
#include "cgi-src/push/index_tpl.h"
#include "cgi-src/tcp/serverconfig_tpl.h"
#include "cgi-src/tcp/serverstatus_tpl.h"
#include "tty/stdout.h"

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
	{"/push/io.push", cgiEspFsTemplate, tplPushIndex},
	{"/push/index.tpl", cgiEspFsTemplate, tplPushIndex},
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


	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL}
};


void user_init(void) {
	stdoutInit();
	ioInit();
	httpdInit(builtInUrls, 80);
	os_printf("\nReady\n");
}

