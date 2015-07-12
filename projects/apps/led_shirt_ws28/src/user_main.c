#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "ws2812.h"
#include "driver/uart.h"
static ETSTimer tickTimer;

void ReceiveUART(char * val) {
	os_printf("Received stuff: %s \r\n", val);
}

static int cnt2 = 0;
static int cnt = 0;
static int cnt3 = 0;
static int blink_cnt = 0;
uint8_t empty[3] = { 0, 0, 0 };
int speed = 12;
void tickCb() {
	os_timer_disarm(&tickTimer);

	os_printf("Tick %d, %d, %d\r\n", cnt, cnt2, cnt3);

	ets_wdt_disable();
	os_intr_lock();

	uint8_t buf[3] = { cnt, cnt2, cnt3 };
	if(speed != 0 && blink_cnt % speed > ((speed /2)-1)) {
		WS2812OutBuffer(empty, 3);
	 } else {
		 WS2812OutBuffer(buf, 3);
	 }

	os_intr_unlock();

	os_timer_arm(&tickTimer, 10, 0);
	if(cnt2 < 0)
		cnt2 = 0;

	if(cnt >= 250) {
		if(cnt2 < 250)
		cnt2++;
	}

	if(cnt2 == 0) {
		if(cnt3 > 0)
			cnt3--;
		if(cnt < 250)
		cnt++;
	}

	if(cnt2 >= 250) {
		cnt--;
		if(cnt3 < 250)
			cnt3++;
	}
 	if(cnt3 >= 250) {


		if(cnt < 250) {
			cnt++;
			cnt2--;
		}
	}



	blink_cnt++;
}

void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200); //ReceiveUART, true); // baudrate, callback, eolchar, printftouart
	os_printf("Starting \r\n");



   // PIN_FUNC_SELECT(PERIPHS_IO_MUX_CONF_U, FUNC_GPIO12);
 	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);

   // PIN_PULLUP_EN(12);
   // gpio16_output_conf();
   // gpio16_output_set(0x00);
   // platform_gpio_mode(2, 1, 0);
    // platform_gpio_write(2, 0);

//
//  void    gpio_output_set(uint32 set_mask, uint32 clear_mask,  uint32 enable_mask, uint32 disable_mask)
//
// set status and direction pin by mask gpio id pin. 

  gpio_output_set(0, GPIO_ID_PIN(12), GPIO_ID_PIN(12), GPIO_ID_PIN(0)); // set gpio 2 as output and low. set gpio 0 as input

  GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);



	char outbuffer[] = { 0xff, 0xff, 0x00 };
	WS2812OutBuffer( outbuffer, 1 ); //Initialize the output.
//
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_printf("Main timer running\r\n");
	os_timer_arm(&tickTimer, 5000, 0);
}

