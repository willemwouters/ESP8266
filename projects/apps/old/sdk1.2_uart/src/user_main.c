#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "uart_custom.h"

#include "driver/uart.h"

//////// NEEDED IF MODULES_1_2_0 IS USED
void user_rf_pre_init(void)
{
} 


void uart_receive(char * line) {
	// send_to_all(line, port); not implemented in dhcpserver
	//SendBroadcast(line);
}

void user_init(void) {
	
	//uart_init(BIT_RATE_115200, uart_receive, false);
	system_set_os_print(1);
	//os_printf("\nUart init done... \n");


	

}

