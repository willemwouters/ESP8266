#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "GDBStub.h"
#include "httpclient.h"

static ETSTimer tickTimer;

void Crash() {
	int * i =  (int*)0;
	*i = 1;
}



  void http_callback2(char * response, int http_status, char * full_response)
  {
	  os_printf("-%s- \r\n",  response);
  }


void tickCb() {
	http_get("http://footscapesofcrete.com/time.php", "", http_callback2);
	os_timer_arm(&tickTimer, 1000, 0);
}


void ICACHE_FLASH_ATTR connectToAp(char * ap, char * pass) {
	wifi_set_opmode(STATION_MODE);
	struct station_config apconf;
	wifi_station_set_auto_connect(true);
	wifi_station_get_config(&apconf);
	//apconf.beacon_interval = 100;
	os_strncpy((char*)apconf.ssid, ap, 32);
	os_strncpy((char*)apconf.password, pass, 64);
	wifi_station_set_config(&apconf);
	wifi_station_set_auto_connect(true);
	wifi_station_connect();
}

void user_done() {
	os_printf("Starting \r\n");
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 10000, 0);
	connectToAp("Ziggo23445", "0616539549");
}



void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	gdb_init();
	system_init_done_cb(user_done);
}

