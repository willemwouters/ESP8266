#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "GDBStub.h"
#include "driver/spi.h"
#include "gpio.h"


static ETSTimer tickTimer;


void tickCb() {

	char t = 0x00;
	spi_byte_read_espslave(HSPI, &t);
	os_printf("%d", t);
	os_timer_arm(&tickTimer, 100, 0);
}

void dat() {
	char t = 0x00;
	spi_byte_read_espslave(HSPI, &t);

	os_printf("I:%d", t);
}

void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	gdb_init();


	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDO_U); // disable pullodwn
	 GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT15);
	 GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);



	os_printf("Starting \r\n");
	spi_master_init(HSPI);
	//spi_mast_byte_write(HSPI, 0x55);

	spi_slave_isr_handler(dat);



	//spi_slave_init(HSPI,1);
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 1000, 0);
}

