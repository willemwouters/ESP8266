#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "log.h"
#include "config.h"
#include "gpio.h"
#include "driver/uart.h"
#include "driver/pwm.h"
#include "mem.h"
#include "stdlib.h"
#include "ff.h"
#include "log.h"



 ETSTimer tickTimer;
 ETSTimer buttonTimer;
 ETSTimer processTimer;
#include "list.h"
struct linked_list* scan_list;
struct linked_list* friend_list;
char link_lock = 0;
extern void tick_master();
char * ownap;
char * connectedap = NULL;


void ReceiveUART(char * val) {
	os_printf("Received stuff: %s \r\n", val);
}
char processbtn = 0;

int blinkcount = 0;
int strobespeed = 0;
 ETSTimer blinkTimer;

int blinky = 0;


char oldbtnstate = 0;
uint32 totaltime = 0;
uint32 lasttime = 0;
char finding =0;



void blinktimert() {
	int val = 1;
	int s;
	if(blinky) {
		log_d(LOG_INFO, "Normally blinky is bussy \r\n");
		os_timer_arm(&blinkTimer, 10, 0);
		return;
	}

	if(strobespeed != 0 && blinkcount % 2 == 0) {
		val = 0;
	}

	blinkcount++;
	if(blinkcount > 250) {
		blinkcount = 0;
	}

	GPIO_OUTPUT_SET(13, val);
	GPIO_OUTPUT_SET(14, val);
	s = strobespeed;
	if(strobespeed == 0)
		s = 1000;

	os_timer_arm(&blinkTimer, s, 0);

}


static void button_task()
{
	os_timer_disarm(&buttonTimer);
	GPIO_DIS_OUTPUT(15);
	char btnstate = GPIO_INPUT_GET(15);
	if((btnstate != oldbtnstate || btnstate == 1) && processbtn == 0 && finding ==0) {
		if(btnstate == 1) {
			uint32 time =  system_get_time();
			os_timer_disarm(&tickTimer);
			if(lasttime != 0) {
				totaltime += time - lasttime;
			}
			lasttime = time;

			if(totaltime > 1500000 && totaltime < 1600000) {
				os_printf("Vibrate once \r\n");
				GPIO_OUTPUT_SET(13, 1);
				os_delay_us(200000);
				GPIO_OUTPUT_SET(13, 0);

			} else if (totaltime > 4000000 && totaltime < 4100000) {
				os_printf("Vibrate on \r\n");
				GPIO_OUTPUT_SET(13, 1);
				if(connectedap != NULL) {
					static struct softap_config apconf;
					int lwn =os_strlen(connectedap);
					os_memcpy(connectedap + lwn, "-CONFIG", 7);
					connectedap[lwn+7] = 0;
					os_strncpy((char*)apconf.ssid, connectedap, 32);
					//os_strncpy((char*)apconf.password, pass, 64);
					apconf.authmode = AUTH_OPEN;
					apconf.max_connection = 20;
					apconf.ssid_hidden = 0;
					apconf.ssid_len = os_strlen(connectedap);
					apconf.channel = 1;
					wifi_softap_set_config(&apconf);
				}
			} else if (totaltime > 8000000 && totaltime < 8100000) {
				GPIO_OUTPUT_SET(13, 0);
				os_delay_us(200000);
				GPIO_OUTPUT_SET(13, 1);

			}else if (totaltime > 9000000 && totaltime < 9100000) {
				os_printf("Vibrate off \r\n");
				GPIO_OUTPUT_SET(13, 0);
			}
		}

		if(btnstate == 0) {
			os_timer_arm(&tickTimer, 1000, 0);
			if(totaltime < 1500000) {
				GPIO_OUTPUT_SET(14, 1);
				os_delay_us(50000);
				GPIO_OUTPUT_SET(14, 0);
				pwm_start();
			}
			if(totaltime > 1500000 && totaltime < 4000000) {
				blink();
				if(connectedap != NULL)
				{
				static struct softap_config apconf;
				wifi_softap_get_config(&apconf);
				int lwn = os_strlen(connectedap);
				os_memcpy(connectedap + lwn, "-FIND", 5);
				connectedap[lwn+5] = 0;
				os_strncpy((char*)apconf.ssid, connectedap, 32);
				//os_strncpy((char*)apconf.password, pass, 64);
				apconf.authmode = AUTH_OPEN;
				apconf.max_connection = 20;
				apconf.ssid_hidden = 0;
				apconf.ssid_len = os_strlen(connectedap);
				apconf.channel = 1;
				wifi_softap_set_config(&apconf);

				finding = 1;
				os_timer_disarm(&processTimer);
				os_printf("start process timer\r\n");
				os_timer_setfn(&processTimer, (os_timer_func_t *)finding_task, NULL);
				os_timer_disarm(&tickTimer);
				os_timer_arm(&tickTimer, 1, 0); // trigger scan now
				os_timer_arm(&processTimer, 3000, 0);

				os_timer_disarm(&blinkTimer);
				os_timer_setfn(&blinkTimer, blinktimert, NULL);
				os_timer_arm(&blinkTimer, 1, 0);

				lasttime = 0;
				totaltime = 0;
				processbtn = 1;
				oldbtnstate = 0;
				return;
				}
			}
			if(totaltime > 4000000 && totaltime < 7000000) {
				static struct softap_config apconf;
				os_timer_disarm(&processTimer);
				os_printf("start process timer\r\n");
				os_timer_setfn(&processTimer, (os_timer_func_t *)set_friends, NULL);
				os_timer_disarm(&tickTimer);
				os_timer_arm(&tickTimer, 1, 0); // trigger scan now
				os_timer_arm(&processTimer, 5000, 0);

				wifi_softap_get_config(&apconf);
				if(connectedap == NULL) {
					int len = os_strlen(ownap);
					os_memcpy(ownap + len, "-CONFIG", 7);
					ownap[len] = 0;
					os_strncpy((char*)apconf.ssid, ownap, 32);
					//os_strncpy((char*)apconf.password, pass, 64);
					apconf.authmode = AUTH_OPEN;
					apconf.max_connection = 20;
					apconf.ssid_hidden = 0;
					apconf.ssid_len = os_strlen(ownap);
					apconf.channel = 1;
					wifi_softap_set_config(&apconf);
				}
				GPIO_OUTPUT_SET(14, 1);
				lasttime = 0;
				totaltime = 0;
				processbtn = 1;
				oldbtnstate = 0;
				GPIO_OUTPUT_SET(13, 0);
				return;
			} else if(totaltime > 9000000 && totaltime < 25000000) {
				static struct softap_config apconf;
				blink();
				blink();
				connectedap = NULL;
				struct linked_node* ln = friend_list->first;
				while(ln != 0 && friend_list->totalcount > 0) {
					os_printf("looping \r\n");
					struct scan_result* res = (struct scan_result*) ln->data;
					os_printf("Removing friend: %s r\n", res->ssid);
					os_free(res->ssid);
					os_free(res);
					ln = linked_removenode(friend_list, ln);
				}
				os_timer_arm(&buttonTimer, 100, 0);
				wifi_softap_get_config(&apconf);
				os_memset(connectedap + os_strlen(connectedap) - 7, 0, 7);

				os_strncpy((char*)apconf.ssid, connectedap, 32);
				//os_strncpy((char*)apconf.password, pass, 64);
				apconf.authmode = AUTH_OPEN;
				apconf.max_connection = 20;
				apconf.ssid_hidden = 0;
				apconf.ssid_len = os_strlen(connectedap);
				apconf.channel = 1;
				wifi_softap_set_config(&apconf);
				lasttime = 0;
				totaltime = 0;
				oldbtnstate = 0;
				return;
			}
			totaltime = 0;
			lasttime = 0;
		}
		oldbtnstate = btnstate;
	}
	os_timer_arm(&buttonTimer, 100, 0);
}

void tickCb(void * arg) {
	tick_master();
	os_timer_arm(&tickTimer, TICK_SPEED, 0);
}

void init_done() {
	static struct softap_config apconf;
	ownap = os_malloc(80);
	//char * pass = MASTER_PASSWORD;

	scan_list = (struct linked_list*) os_malloc(sizeof(struct linked_list));
	scan_list->first = NULL;
	scan_list->last = NULL;
	scan_list->totalcount = 0;

	friend_list = (struct linked_list*) os_malloc(sizeof(struct linked_list));
	friend_list->first = NULL;
	friend_list->last = NULL;
	friend_list->totalcount = 0;

	gpio_init();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO15);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	GPIO_OUTPUT_SET(15, 0);
	GPIO_OUTPUT_SET(14, 0);
	GPIO_OUTPUT_SET(13, 0);
	GPIO_DIS_OUTPUT(15);

	uint8_t PWM_CH[]= { 1 };
	uint16_t freq = 1;
	ETS_UART_INTR_DISABLE();
	wifi_set_opmode(STATION_MODE);
	wifi_station_disconnect();
	//user_resetclient();
	ETS_UART_INTR_ENABLE();
	pwm_init(freq, PWM_CH);

	GPIO_OUTPUT_SET(13, 0);

	//pwm_start();

//	os_timer_disarm(&blinkTimer);
//	os_timer_setfn(&blinkTimer, blinktimert, NULL);

	uint32 time =  system_get_time();
	os_sprintf(ownap, "FFt%d%d%d%d", time & 0xFF, time >> 8 & 0xFF, time >> 16 & 0xFF, time >> 24 & 0xFF);
	wifi_set_opmode(STATIONAP_MODE);
	wifi_softap_get_config(&apconf);
	os_strncpy((char*)apconf.ssid, ownap, 32);
	//os_strncpy((char*)apconf.password, pass, 64);
	apconf.authmode = AUTH_OPEN;
	apconf.max_connection = 20;
	apconf.ssid_hidden = 0;
	apconf.ssid_len = os_strlen(ownap);
	apconf.channel = 1;
	wifi_softap_set_config(&apconf);

	uart_init(BIT_RATE_115200, ReceiveUART, true); // baudrate, callback, eolchar, printftouart

	//uart_init(BIT_RATE_115200, BIT_RATE_115200); // baudrate, callback, eolchar, printftouart	log_d(LOG_VERBOSE, "Starting up\r\n");

	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	log_d(LOG_VERBOSE, "Main timer running\r\n");
	os_timer_arm(&tickTimer,5000, 0);
	os_timer_disarm(&buttonTimer);
	os_timer_setfn(&buttonTimer, (os_timer_func_t *)button_task, NULL);
	os_timer_arm(&buttonTimer, 5000, 0);

	GPIO_OUTPUT_SET(13, 1);



}

void user_init(void) {
	system_init_done_cb(init_done);
}

