#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "lwip_websocket.h"
#include "gpio.h"
#include "driver/pwm.h"

static ETSTimer tickTimer;


void ICACHE_FLASH_ATTR connectToAp() {
    char * ap = "Ziggo23445";
    char * pass = "0616539549";
    wifi_set_phy_mode( PHY_MODE_11N );
    struct station_config apconf;
    wifi_station_set_auto_connect(true);
    wifi_set_opmode(STATION_MODE);
    wifi_station_get_config(&apconf);
    os_strncpy((char*)apconf.ssid, ap, 32);
    os_strncpy((char*)apconf.password, pass, 64);
    wifi_station_set_config(&apconf);
   // wifi_set_event_handler_cb(wifi_event_cb);
}


void tickCb() {
	os_timer_disarm(&tickTimer);
	uint8_t buffer[16];
	uint16 adc = system_adc_read();

	ets_sprintf( buffer, "%d", adc);
	int ret = websocket_writedata(buffer);
	if(ret == 0) {
		os_timer_arm(&tickTimer, 33, 0);
	} else {
		os_timer_arm(&tickTimer, 100, 0);
	}
}
void websocket_callback(char * data, int size) {
	if(strstr(data, "R:") != 0) {
		char dat[3] = { 0 };
		os_memcpy(dat, &data[2], size);
		os_printf("%s\r\n", dat);
		int i = atoi(dat);
		os_printf("%d\r\n", i);
		pwm_set_duty(i, 1);
		pwm_start();
	} else if(strstr(data, "G:") != 0) {
		char dat[3] = { 0 };
		os_memcpy(dat, &data[2], size);
		os_printf("%s\r\n", dat);
		int i = atoi(dat);
		os_printf("%d\r\n", i);
		pwm_set_duty(i, 0);
		pwm_start();
	} else if(strstr(data, "B:") != 0) {
		char dat[3] = { 0 };
		os_memcpy(dat, &data[2], size);
		os_printf("%s\r\n", dat);
		int i = atoi(dat);
		os_printf("%d\r\n", i);
		pwm_set_duty(i, 2);
		pwm_start();
	}
}
void user_done() {
	os_printf("Starting \r\n");
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 100, 0);
}

void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDI_U); // disable pullodwn
	GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT12);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTCK_U); // disable pullodwn
	GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT13);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDO_U); // disable pullodwn
	GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT15);

	GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);  // SET
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);  // SET
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);  // SET


	uint8_t PWM_CH[]= {0, 0, 0};   // PIN CONFIG IS IN PWM.h
	uint16_t freq = 100;
	pwm_init(freq, PWM_CH);
	pwm_start();

	connectToAp();
	server_init(websocket_callback);
	system_init_done_cb(user_done);
}

