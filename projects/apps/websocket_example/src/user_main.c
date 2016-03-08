#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "lwip_websocket.h"

static ETSTimer tickTimer;


void ICACHE_FLASH_ATTR connectToAp() {
    char * ap = "Ziggo23445";
    char * pass = "0616539549";
    wifi_set_phy_mode( PHY_MODE_11N );
    struct station_config apconf;
    wifi_station_set_auto_connect(true);
    wifi_set_opmode(STATION_MODE);
    wifi_station_get_config(&apconf);
    os_strncpy((char*)apconf.ssid, ap, 32);
    os_strncpy((char*)apconf.password, pass, 64);
    wifi_station_set_config(&apconf);
   // wifi_set_event_handler_cb(wifi_event_cb);
}


void tickCb() {
	os_timer_disarm(&tickTimer);
	uint8_t buffer[16];
	uint16 adc = system_adc_read();

	ets_sprintf( buffer, "%d", adc);
	//os_printf("%s\r\n", buffer);
	int ret = writeToWebsocket(buffer);
	if(ret == 0) {
	os_timer_arm(&tickTimer, 33, 0);
	} else {
		//os_printf("EEERRRRRROOOOOORRRRRR \r\n");
		os_timer_arm(&tickTimer, 100, 0);
	}
}

void user_done() {
	os_printf("Starting \r\n");
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 100, 0);
}

void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	//gdb_init();
	connectToAp();
	my_server_init();
	system_init_done_cb(user_done);
}

