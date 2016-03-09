#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "GDBStub.h"
#include "ws2812_i2s.h"
static ETSTimer tickTimer;



void tickCb() {
	os_printf("Tick\r\n");

	uint8_t ledout[] = { 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 };
	ws2812_push( ledout, sizeof( ledout ) );

	os_timer_arm(&tickTimer, 1000, 0);
}


void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	gdb_init();

	os_printf("Starting \r\n");
 	
	ws2812_init();

//
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 1000, 0);
}

