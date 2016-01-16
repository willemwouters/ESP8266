#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "GDBStub.h"

static ETSTimer tickTimer;

void Crash() {
	int * i =  (int*)0;
	*i = 1;
}


void tickCb() {
	os_printf("Tick\r\n");
	Crash();
	os_timer_arm(&tickTimer, 1000, 0);
}

void user_done() {
	os_printf("Starting \r\n");
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 1000, 0);
}

void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	gdb_init();
	system_init_done_cb(user_done);
}

