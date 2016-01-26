#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "GDBStub.h"

static ETSTimer tickTimer;


int state;
uint32 time = 0;
void tickCb() {

	uint16 adc = system_adc_read();
	if(adc > 600 && adc < 1000) {
		if(state == 0  && system_get_rtc_time() - time > 40000) {
			os_printf("ADC %d \r\n", adc);
			os_printf("Tick\r\n");
			state = 1;
			time = system_get_rtc_time();
		}
	} else {
		state = 0;
	}
	os_timer_arm(&tickTimer, 10, 0);
}


void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	gdb_init();

	os_printf("Starting \r\n");

	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 1000, 0);
}

