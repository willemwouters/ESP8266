#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"


#include "io/io.h"
#include "tcp/TcpClient.h"
#include "tty/stdout.h"

static ETSTimer ipTimer;

#include "gpio.h"
#include "io/key.h"


#define PLUG_KEY_NUM            1

LOCAL struct keys_param keys;
LOCAL struct single_key_param *single_key[PLUG_KEY_NUM];


#define PLUG_KEY_2_IO_MUX     PERIPHS_IO_MUX_MTMS_U
#define PLUG_KEY_2_IO_NUM     2
#define PLUG_KEY_2_IO_FUNC    FUNC_GPIO2

#define PLUG_KEY_0_IO_MUX     PERIPHS_IO_MUX_MTMS_U
#define PLUG_KEY_0_IO_NUM     0
#define PLUG_KEY_0_IO_FUNC    FUNC_GPIO0


LOCAL void ICACHE_FLASH_ATTR user_plug_long_press(void)
{
	char * msg = "{ \"GPIO0\": \"LONG PRESS\" }" ;
   	os_printf("-%s-%s %s \r\n", __FILE__, __func__, msg);
}



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
		os_timer_arm(&ipTimer, 10000, 0);
	}
}


void user_init(void) {
	stdoutInit();
	wifi_set_opmode(3);
	os_timer_disarm(&ipTimer);
	os_timer_setfn(&ipTimer, WifiConnectCb, NULL);
	os_printf("-%s-%s Schedule main loop after Wifi connect \r\n", __FILE__, __func__);
	os_timer_arm(&ipTimer, 5000, 0);
	os_printf("\nReady \n");
}

