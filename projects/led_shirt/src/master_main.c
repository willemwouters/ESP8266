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
#include "reg_handler.h"
#include "gpio.h"
#include "led_handler.h"

static struct udp_pcb * pUdpConnection = NULL;
struct ip_info IpInfo;

void scan_master();
u8 watchdog_running = false;
static ETSTimer scanTimer;

static ETSTimer instructTimer;

struct station_info * ipleases;

void reset_master() {
	os_timer_disarm(&scanTimer);
	watchdog_running = 0;
}

void handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	//os_printf("UDP Receive: %s \r\n", p->payload);

	network_request(p->payload);
	pbuf_free(p);
}

void SendBroadcast(char * data, struct ip_addr ipSend, int port) {
    struct udp_pcb * pCon = NULL;
	struct pbuf* pBuffer;
	err_t err;
	pCon = udp_new();
	pBuffer = pbuf_alloc(PBUF_TRANSPORT, os_strlen(data), PBUF_RAM);
	os_memcpy(pBuffer->payload, (char*) data, os_strlen(data));
	err = udp_sendto(pCon, pBuffer, &ipSend, 8000);
	pbuf_free(pBuffer);
	udp_remove(pCon);
}

void sendtoall(char * data, int port) {
	struct station_info * ipleases;
	ipleases = wifi_softap_get_station_info();
	while(ipleases) {
		SendBroadcast(data, ipleases->ip, 8000);
		os_printf("Udp send to: %d \r\n", ipleases->ip.addr);
		ipleases = STAILQ_NEXT(ipleases, next);
	}
	wifi_softap_free_station_info();
}


void uart_receive(char * line) {
	sendtoall(line, 8000);
}

void callback_master(struct reg_str *  reg) {
	char data[20];

	if(reg->action == WRITE && reg->destination[0] == 'A') {
		os_printf("Sending \r\n");
		reg_tochar(reg, data);
		data[0] = '0';
		data[1] = '1';
		os_printf("Sending: %s \r\n", data);
		sendtoall(data, 8000);
	}
}

void scanWatchdog(void * arg) {
	os_printf("MASTER WATCHDOG TICK \r\n");
//	sendtoall("AP:READ:2:22", 8000);
//	sendtoall("AAAAAAAA:AAAAAAA\r\n", 8000);
//	sendtoall("AP:READ:3:22", 8000);
//	sendtoall("AAAAAAAAAAAAAAA\r\n", 8000);
//	sendtoall("AP:Rs", 8000);
//	sendtoall("AP:WRITE:2:22", 8000);
	scan_master();
	os_timer_arm(&scanTimer, 10000, 0);
}
void init() {
	err_t err;
	struct ip_addr ipSend;

	static struct softap_config apconf;
	char * ap = MASTER_AP;
	char * pass = MASTER_PASSWORD;
	uart_init(BIT_RATE_115200, uart_receive, true);

	wifi_set_opmode(STATIONAP_MODE);
	wifi_softap_get_config(&apconf);
	os_strncpy((char*)apconf.ssid, ap, 32);
	//os_strncpy((char*)apconf.password, pass, 64);
	apconf.authmode = AUTH_OPEN;
	apconf.max_connection = 20;
	apconf.ssid_hidden = 0;
	apconf.ssid_len = 6;
	apconf.channel = 1;
	wifi_softap_set_config(&apconf);
	os_printf("Starting master watchdog \r\n");
	os_timer_disarm(&scanTimer);
	os_timer_setfn(&scanTimer, scanWatchdog, NULL);
	os_timer_arm(&scanTimer, 50, 0);

	watchdog_running = 1;

	//lwip_init();
	pUdpConnection = udp_new();
	IP4_ADDR(&ipSend, 255, 255, 255, 255);

	pUdpConnection->multicast_ip = ipSend;
	pUdpConnection->remote_ip = ipSend;
	pUdpConnection->remote_port = 8000;

	if(pUdpConnection == NULL) {
		os_printf("\nCould not create new udp socket... \n");
	}
	err = udp_bind(pUdpConnection, IPADDR_ANY, 8000);
	udp_recv(pUdpConnection, handle_udp_recv, pUdpConnection);
	led_init("AP");
	reg_addlistener(callback_master, 0x32);
}


void tick_master() {

	os_printf("MASTER TICK \r\n");
	if(!watchdog_running) {
		init();
	}
}



/**
  * @brief  Wifi ap scan over callback to display.
  * @param  arg: contain the aps information
  * @param  status: scan over status
  * @retval None
  */
static void ICACHE_FLASH_ATTR
scan_done(void *arg, STATUS status)
{
  uint8 ssid[33];
  char temp[128];

  if (status == OK)
  {
    struct bss_info *bss_link = (struct bss_info *)arg;
    bss_link = bss_link->next.stqe_next;//ignore first

    while (bss_link != 0)
    {
      os_memset(ssid, 0, 33);
      if (os_strlen(bss_link->ssid) <= 32)
      {
        os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
      }
      else
      {
        os_memcpy(ssid, bss_link->ssid, 32);
      }
      //os_printf("%s \r\n", bss_link->ssid);
      if(os_strcmp(bss_link->ssid, MASTER_AP) == 0) {
    	  os_printf("FOUND MASTER AP\r\n");
    	  config_mode = MODE_SLAVE;
    	  reset_master();
    	  return;
      }
      os_sprintf(temp,"+CWLAP:(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
                 bss_link->authmode, ssid, bss_link->rssi,
                 MAC2STR(bss_link->bssid),bss_link->channel);
      os_printf(temp);
      bss_link = bss_link->next.stqe_next;
    }
  }
}


void scan_master() {
	struct scan_config config;
	wifi_station_scan(NULL, scan_done);
}
