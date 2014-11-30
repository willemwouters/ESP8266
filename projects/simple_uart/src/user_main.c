#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"


#include "tty/uart.h"

void user_init(void) {
	uart_init(BIT_RATE_115200);
	os_printf("\nReady \n");
}

