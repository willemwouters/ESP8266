/*
 * ff.c
 *
 *  Created on: Jun 6, 2015
 *      Author: wouters
 */
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "config.h"
#include "list.h"
#include "mem.h"
#include "stdlib.h"
#include "driver/pwm.h"
#include "gpio.h"


extern  ETSTimer tickTimer;
extern  ETSTimer buttonTimer;
extern  ETSTimer processTimer;
extern  ETSTimer blinkTimer;

extern struct linked_list* scan_list;
extern struct linked_list* friend_list;

extern char link_lock;
extern char * ownap;
extern char * connectedap;
extern char processbtn;

//int blinkcount = 0;
extern int strobespeed;
extern char finding;
extern int blinky;

void blink() {
	blinky = 1;
	int i;
	for(i = 0; i < 4; i++) {
		GPIO_OUTPUT_SET(14, 1);
		os_delay_us(100000);
		GPIO_OUTPUT_SET(14, 0);
		os_delay_us(100000);
	}
	blinky = 0;
}

void set_friends() {
	static struct softap_config apconf;
	os_timer_disarm(&processTimer);
	struct linked_node* ln = friend_list->first;
	while(ln != NULL) {
		struct scan_result* r = (struct scan_result* ) ln->data;
		os_free(r->ssid);
		os_free(r);
		ln = linked_removenode(friend_list, ln);
	}

	char connecttoap = 0;
	while(link_lock);
	link_lock = 1;
	ln = scan_list->first;
	while(ln != NULL) {
		struct scan_result* res = (struct scan_result*) ln->data;
		os_printf("Found: %s %d\r\n", res->ssid, res->strenght);
		if(connectedap == NULL) {
			if(os_strstr(res->ssid, AP_PREFIX) != NULL)
				if(res->strenght >= MIN_STRENGTH) {
					os_printf("Setting group to friend: %s %d\r\n", res->ssid, res->strenght);
					struct scan_result* r = os_malloc(sizeof(struct scan_result));
					r->ssid = os_malloc(80);
					os_memcpy(r->ssid, res->ssid, 80);
					connectedap = r->ssid;
					linked_insertnode(friend_list, r);
					connecttoap = 1;
					break;

			}
		}
		ln = ln->next;
	  }

	link_lock = 0;
	blink();
	processbtn = 0;
	if(connectedap == 0) {
			int len = 0;
			uint8 * mac = os_malloc(10);
			wifi_get_macaddr(0, mac);
			len = os_sprintf(ownap, "FF%d%d%d%d-01", mac[0], mac[1], mac[2], mac[3]);
			ownap[len] = 0;
			connectedap = ownap;
			os_printf("I am the groups name\r\n");
			wifi_softap_get_config(&apconf);
			os_strncpy((char*)apconf.ssid, connectedap, len);
			//os_strncpy((char*)apconf.password, pass, 64);
			apconf.authmode = AUTH_OPEN;
			apconf.max_connection = 20;
			apconf.ssid_hidden = 0;
			apconf.ssid_len = len; //os_strlen(connectedap);
			apconf.channel = 1;
			wifi_softap_set_config(&apconf);
		} else {
			int strlen = os_strlen(connectedap);
			wifi_softap_get_config(&apconf);

			if(connecttoap == 0) {
				os_memset(connectedap + strlen - 7, 0, 7);
				strlen = strlen - 7;
			} else {
				os_memcpy(connectedap + strlen, "-01", 3);
				strlen = strlen + 3;
			}
			connectedap[strlen] = 0;
			os_strncpy((char*)apconf.ssid, connectedap, 32);
			//os_strncpy((char*)apconf.password, pass, 64);
			apconf.authmode = AUTH_OPEN;
			apconf.max_connection = 20;
			apconf.ssid_hidden = 0;
			apconf.ssid_len = strlen;
			apconf.channel = 1;
			wifi_softap_set_config(&apconf);
		}
	os_printf("Vibrate off \r\n");
	os_timer_arm(&buttonTimer, 1000, 0);
}




void finding_task() {
	os_timer_disarm(&processTimer);
	os_printf("tsk once \r\n");

	while(link_lock);
	link_lock = 1;
	struct linked_node* ln = scan_list->first;
	while(ln != NULL && scan_list->totalcount > 0) {
		struct scan_result* res = (struct scan_result*) ln->data;
		os_printf("Found: %s %d\r\n", res->ssid, res->strenght);
		if(connectedap != NULL) {
			if(os_strstr(res->ssid, AP_PREFIX) != NULL) {
				os_printf("FOUND THEM: %s %d\r\n", res->ssid, res->strenght);
				strobespeed = ((res->strenght * -1) - 27) * 15;
				if(strobespeed <= 0) {
					strobespeed = 2;
				}
				if(res->strenght >= -27) {
					os_printf("Closing down find mode, look next to you: %s %d\r\n", res->ssid, res->strenght);
					finding = 0;
					strobespeed = 1000;
				}
			}
		}
		ln = ln->next;
	  }
	link_lock = 0;
	os_printf("next round \r\n");

	if(finding == 1) {
		os_timer_disarm(&tickTimer);
		os_timer_arm(&tickTimer, 1, 0);
		os_timer_arm(&processTimer, 500, 0);
	} else {
		processbtn = 0;
		os_timer_disarm(&blinkTimer);
		os_timer_arm(&buttonTimer, 1, 0);
		blink();
	}
}
