#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "GDBStub.h"
#include "gpio.h"
static ETSTimer tickTimer;

void Crash() {
	int * i =  (int*)0;
	*i = 1;
}

int state = 0;

void tickCb() {
	os_printf("Tick\r\n");
	os_timer_arm(&tickTimer, 100, 0);


	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);  // SET
	GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);  // SET
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);  // SET


	//rg GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 0);  // CLEAR


//	 GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << 5);  // SET
//	 GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << 5);  // CLEAR
	 uint8_t buffer[16];
		 uint16 adc = system_adc_read();

		 if(adc < 400) {
			 switch (state) {
				case 0:
					 GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);  // SET
					break;
				case 1:
					GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);  // SET
					break;
				case 2:
					GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);  // SET
					break;
				default:
					break;
			}
		 }

	 ets_sprintf( buffer, "%d\r\n", adc );
	 uart0_sendStr(buffer);

	 state++;
	 if(state > 3)
		 state = 0;
}

void user_done() {
	os_printf("Starting \r\n");
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 1000, 0);

	 	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
		 PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDI_U); // disable pullodwn
		 GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT12);

		 PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
				 PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTCK_U); // disable pullodwn
				 GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT13);

				 PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
						 PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDO_U); // disable pullodwn
						 GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT15);
}

void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	gdb_init();
	system_init_done_cb(user_done);
}

