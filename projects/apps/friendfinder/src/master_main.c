/*
 * master_main.c
 *
 *  Created on: Mar 14, 2015
 *      Author: wouters
 */

#include "config.h"
#include "user_interface.h"
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "driver/uart.h"
#include "gpio.h"
#include "osapi.h"
#include "mem.h"
#include "log.h"
#include "list.h"

extern struct linked_list* scan_list;
extern char link_lock;

void scan_master();

static char scan_started = 0;
void tick_master() {
	os_printf("Going to scan \r\n");
	scan_master();
}
LOCAL scaninfo *pscaninfo;
static void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status)
{


  log_d(LOG_VERBOSE, "-\r");
  if (status == OK)
  {
	  while(link_lock);
	    link_lock = 1;
	    struct linked_node* ln = scan_list->first;
	    while(ln != NULL && scan_list->totalcount > 0) {
	  	struct scan_result* r = (struct scan_result* ) ln->data;
	  	if(r->ssid !=  NULL) {
	  	  os_free(r->ssid);
	  	  os_free(r);
	  	}
	      ln = linked_removenode(scan_list, ln);
	    }



    struct bss_info *bss_link = (struct bss_info *)arg;
    bss_link = bss_link->next.stqe_next;//ignore first

    while (bss_link != 0) 
    { 
      char* ssid = os_malloc(33);
      struct scan_result* res = os_malloc(sizeof(struct scan_result));
      os_memset(ssid, 0, 33);
      if (os_strlen((const char *) bss_link->ssid) <= 32)
      { 
        os_memcpy((char *) ssid, (const char *) bss_link->ssid, os_strlen((const char *) bss_link->ssid));
      }
      else
      {
        os_memcpy(ssid, bss_link->ssid, 32);
      }
      res->ssid = ssid;
      res->strenght = bss_link->rssi;
      linked_insertnode(scan_list, res);

      os_printf(" #%s-%d\r\n", bss_link->ssid,  res->strenght);
//      if(os_strcmp((const char *) bss_link->ssid, MASTER_AP) == 0) {
//    	  log_d(LOG_INFO, "FOUND MASTER AP\r\n");
//    	  reset_master();
//    	  return;
//      }
      bss_link = bss_link->next.stqe_next;
    }
  }
  link_lock = 0;
  scan_started = 0;
}


void scan_master() {
	//log_d(LOG_VERBOSE, "Starting scan\r\n");
	if(scan_started)
		return;

	scan_started = 1;
	wifi_station_scan(NULL, scan_done);
}
