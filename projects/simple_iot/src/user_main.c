#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "httpd/httpd.h"
#include "tcp/TcpClient.h"
#include "tty/uart.h"
#include "pages/index_cgi.h"
static ETSTimer ipTimer;


int noIpTimer = 0;

HttpdBuiltInUrl builtInUrls[]={
	{"/", cgiIndex},
	{"/save.cgi", cgiSave},
	{"*", cgiRedirect}
};

static struct softap_config apconf;


void WifiConnectCb(void* arg) {
	char * ap = "ESPAp";
	char * pass = "00000000";
	int x=wifi_station_get_connect_status();
	if (x==STATION_GOT_IP) {
		os_printf("-%s-%s Got an ip\r\n", __FILE__, __func__);
		os_timer_disarm(&ipTimer);
	} else {
		os_timer_disarm(&ipTimer);
		os_timer_setfn(&ipTimer, WifiConnectCb, NULL);
		os_printf("-%s-%s  Wait for IP \r\n", __FILE__, __func__);
		os_timer_arm(&ipTimer, 1000, 0);
		noIpTimer++;
		if(noIpTimer == 60) {
			wifi_softap_get_config(&apconf);
			os_strncpy((char*)apconf.ssid, ap, 32);
			os_strncpy((char*)apconf.password, pass, 64);
			apconf.authmode = AUTH_WPA_WPA2_PSK;
			apconf.max_connection = 2;
			noIpTimer = 0;
			//apconf.ssid_hidden = 0;
			wifi_softap_set_config(&apconf);
			//wifi_set_opmode(3);
			system_restart();
		}
	}
}


void user_init(void) {
	uart_init(BIT_RATE_115200);
	wifi_set_opmode(3);
	httpdInit(builtInUrls, 80);
	noIpTimer = 0;
	os_timer_disarm(&ipTimer);
	os_timer_setfn(&ipTimer, WifiConnectCb, NULL);
	os_printf("-%s-%s Schedule main loop after Wifi connect \r\n", __FILE__, __func__);
	os_timer_arm(&ipTimer, 5000, 0);
	os_printf("\nReady \n");
}

