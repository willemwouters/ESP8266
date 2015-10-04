/*
 * user_cpp.c
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */
#include "UserMain.h"


extern "C" {
	#include "espressif/esp_common.h"
}


extern "C" void user_init(void);
extern "C" void uart_div_modify(int no, unsigned int freq);

void  user_init(void) {
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	os_delay_us(500);
	os_printf("\r\n");
	new UserMain();
}
