#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"
#include "config.h"
#include "driver/uart.h"
#include "driver/pwm.h"

static ETSTimer writeudpTimer;

int GammaE[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
					2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
					6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11,
					11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
					19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
					29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
					40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
					55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
					71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
					90, 91, 93, 94, 95, 96, 98, 99,100,102,103,104,106,107,109,110,
					111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,
					135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,
					162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,
					191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
					222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255};


void ReceiveUART(char * val) {
	os_printf("Received stuff: %s \r\n", val);
}


int prev = 0;


int brightness = 0;
void btnCallback(int btn) {

	  brightness = brightness + 30;
	  if(brightness > 255) {
		  brightness = 0;
	  }
	  os_printf("SWITCH %d \r\n", brightness);

	  pwm_set_duty(GammaE[brightness], 2);
	pwm_set_duty(GammaE[brightness], 1);
	pwm_set_duty(GammaE[brightness], 0);
	pwm_start();

}
int thres = 120;
void readButton() {
	os_timer_disarm(&writeudpTimer);
	int c = 0;
	int val = 0;
	c = 0;
	GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << 12);
	while(GPIO_INPUT_GET(2) == 0 && c < 20000) {
	GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << 12);
	  c++;
	}
	os_printf("%d \r\n", c);
	if(c < thres && c != 20000) {
	  val = 1;
	} else {
	  val = 0;
	}

	if(prev != val) {
	  prev = val;
	  if(val == 0) {
		 btnCallback(0);
	  }
	}
	c = 0;
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);
	while(GPIO_INPUT_GET(2) == 1);
	os_timer_arm(&writeudpTimer, 100, 0);
}

int state = 0;
void test() {
	os_timer_disarm(&writeudpTimer);
	state = !state;
	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO5_U); // disable pullodwn
	 PIN_PULLDWN_EN(PERIPHS_IO_MUX_GPIO5_U); // pull - up pin

	// GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << 4);
	 GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 1);

	 os_printf("WW SET %d \r\n", 1);
	 os_timer_arm(&writeudpTimer, 100, 0);
}



void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200); //ReceiveUART, true); // baudrate, callback, eolchar, printftouart
//gpio_init();
//
//	  	  uint8_t PWM_CH[]= {0, 0, 0};
//	  	uint16_t freq = 100;
//	  	pwm_init(freq, PWM_CH);
//	  	pwm_start();
//
//	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
//	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDI_U); // disable pullodwn
//	 PIN_PULLDWN_EN(PERIPHS_IO_MUX_MTDI_U); // pull - up pin
//
//	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
//	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO2_U); // disable pullodwn
//	 PIN_PULLDWN_DIS(PERIPHS_IO_MUX_GPIO2_U); // pull - up pin
//	 GPIO_DIS_OUTPUT(2);
//
//
//	 GPIO_OUTPUT_SET(GPIO_ID_PIN(16), 0);

//
//	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
//	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDO_U); // disable pullodwn
//	 PIN_PULLDWN_EN(PERIPHS_IO_MUX_MTDO_U); // pull - up pin
//
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
//	PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTCK_U); // disable pullodwn
//	PIN_PULLDWN_EN(PERIPHS_IO_MUX_MTCK_U); // pull - up pin
//
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
//	PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTMS_U); // disable pullodwn
//	PIN_PULLDWN_EN(PERIPHS_IO_MUX_MTMS_U); // pull - up pin
//



	os_timer_disarm(&writeudpTimer);
	os_timer_setfn(&writeudpTimer,(os_timer_func_t *) test, NULL);
	os_timer_arm(&writeudpTimer, 100, 0);




}

