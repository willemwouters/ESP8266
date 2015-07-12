#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"


#include "io/io.h"
#include "httpd/httpd.h"
#include "tcp/TcpClient.h"
#include "tty/uart.h"
#include "pages/index_cgi.h"
static ETSTimer ipTimer;

#define PLUG_KEY_NUM            1

LOCAL struct keys_param keys;
LOCAL struct single_key_param *single_key[PLUG_KEY_NUM];


#define PLUG_KEY_2_IO_MUX     PERIPHS_IO_MUX_MTMS_U
#define PLUG_KEY_2_IO_NUM     2
#define PLUG_KEY_2_IO_FUNC    FUNC_GPIO2

#define PLUG_KEY_0_IO_MUX     PERIPHS_IO_MUX_MTMS_U
#define PLUG_KEY_0_IO_NUM     0
#define PLUG_KEY_0_IO_FUNC    FUNC_GPIO0

int noIpTimer = 0;

HttpdBuiltInUrl builtInUrls[]={
	{"/", cgiIndex},
	{"/save.cgi", cgiSave},
	{"*", cgiRedirect}
};

LOCAL void ICACHE_FLASH_ATTR user_plug_long_press(void)
{
	char * msg = "{ \"GPIO0\": \"LONG PRESS\" }" ;
   	os_printf("-%s-%s %s \r\n", __FILE__, __func__, msg);
}

static struct softap_config apconf;


void ioIntterupt() {
	int x;
	char    *hostname=(char *)"200.20.186.76";
	char msg[48]={010,0,0,0,0,0,0,0,0};    // the packet we send
	int portno=123;     //NTP is port 123
	TcpSend(UDP, hostname, portno,  msg);
	x=wifi_station_get_connect_status();
	if (x!=STATION_GOT_IP) {
		TcpSend(TCP, hostname, portno,  msg);
	}
}

void initIntGpio() {
   	os_printf("-%s-%s \r\n", __FILE__, __func__);
	single_key[0] = key_init_single(PLUG_KEY_0_IO_NUM, PLUG_KEY_0_IO_MUX, PLUG_KEY_0_IO_FUNC,
                                    ioIntterupt, user_plug_long_press, 50, 1000);
    keys.key_num = PLUG_KEY_NUM;
    keys.single_key = single_key;

    key_init(&keys);
}

void WifiConnectCb(void* arg) {
	char * ap = "ESPAp";
	char * pass = "00000000";
	int x=wifi_station_get_connect_status();
	if (x==STATION_GOT_IP) {
		os_printf("-%s-%s Got an ip\r\n", __FILE__, __func__);
		os_timer_disarm(&ipTimer);
		ioInit();
		initIntGpio();
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

