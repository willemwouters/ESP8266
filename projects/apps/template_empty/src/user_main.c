#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
static ETSTimer tickTimer;

void ReceiveUART(char * val) {
	os_printf("Received stuff: %s \r\n", val);
}

void tickCb() {
	os_timer_disarm(&tickTimer);
	ets_wdt_disable();
	os_intr_lock();

	os_printf("Tick\r\n");

	os_intr_unlock();
	ets_wdt_enable();
	os_timer_arm(&tickTimer, 1000, 0);
	
}

void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200); //ReceiveUART, true); // baudrate, callback, eolchar, printftouart
	os_printf("Starting \r\n");
 	
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 1000, 0);
}

