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
#include "reg_actions.h"
#include "gpio.h"
#include "osapi.h"
#include "led_handler.h"
#include "queue.h"
#include "mem.h"

#include "log.h"
//static queue retry_queue;
static queue udp_queue;

static struct udp_pcb * pUdpConnection = NULL;
struct ip_info IpInfo;

void scan_master();
static u8 watchdog_running = false;
static ETSTimer scanTimer;

//static ETSTimer instructTimer;
static uint32 clients[20];
//static uint32 smartphone;

static struct station_info * ipleases;

struct client_data {
	uint32 addr;
	int missedack;
};

static struct client_data* clientlist[10];

struct udp_data {
	struct pbuf *p ;
	uint32 addr;
};

void reset_master() {
	os_timer_disarm(&scanTimer);
	watchdog_running = 0;
}

struct udpreg {
	uint32 addr;
	char data[20];
	int port;
	bool received;
	int delay;
	int retries;
};


struct linked_node {
	struct udpreg * data;
	struct linked_node * prev;
	struct linked_node * next;
	void * list;
};

struct linked_list {
	int totalcount;
	struct linked_node * first;
	struct linked_node * last;
};

static struct linked_list * list;
static bool linked_lock = false;
void linked_insertnode(void * d) {
	struct linked_node* n = (struct linked_node*) os_malloc(sizeof(struct linked_node));
	while(linked_lock);
	linked_lock = true;
	if(list->totalcount == 0) {
		list->first = n;
		list->last = n;
		list->totalcount = 1;
		n->list = list;
		n->prev = NULL;
	} else {
		n->list = n;
		n->prev = list->last;
		list->last->next = n;
		list->last = n;
		list->totalcount++;
	}
	linked_lock = false;
	n->next = NULL;
	n->data = d;

}

struct linked_node * linked_removenode(struct linked_node * n) {
	if(list->totalcount <= 0) {
		log_d(LOG_ERROR, "Error removing node \r\n");
		return NULL;
	}
	while(linked_lock);
		linked_lock = true;
	list->totalcount--;
	struct linked_node * ntmp;
	os_free(n->data);
	if(n->prev != NULL) {
		n->prev->next = n->next;
	}
	if(n->next != NULL) {
		n->next->prev = n->prev;
	}
	ntmp = n->next;
	if(list->totalcount == 1) {
		if(n->prev != NULL) {
			list->first = n->prev;
			list->last = n->prev;
		} else if(n->next != NULL) {
			list->first = n->next;
			list->last = n->next;
		}
	}
	linked_lock = false;
	//os_printf("5 totalcount = %d \r\n", list->totalcount);
	os_free(n);
	//os_printf("6 \r\n");
	return ntmp;
}
void handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	//os_printf("UDP Receive: %s \r\n", p->payload);
	int i;
	if (p != NULL) {
		os_printf( "RECEIVED FROM UDP: %s \r\n", p->payload);
		struct udp_data * r = (struct udp_data *) os_malloc(sizeof(struct udp_data));
		r->addr = addr->addr;
		r->p = p;
		enqueue(&udp_queue, r);
	}
}

void SendBroadcast(char * data, uint32 ip, int port) {
    struct udp_pcb * pCon = NULL;
	struct pbuf* pBuffer;
	struct ip_addr ipSend;
	ipSend.addr = ip;
	err_t err;
	pCon = udp_new();
	pBuffer = pbuf_alloc(PBUF_TRANSPORT, os_strlen(data), PBUF_RAM);
	os_memcpy(pBuffer->payload, (char*) data, os_strlen(data));
	err = udp_sendto(pCon, pBuffer, &ipSend, 8000);
	pbuf_free(pBuffer);
	udp_remove(pCon);
}

void sendtoall(char * data, int port, bool requestack) {
	int i = 0;
	log_d(LOG_VERBOSE, "sendtoall \r\n");
	wifi_softap_free_station_info();
	ipleases = wifi_softap_get_station_info();
	for(i = 0; i < 10; i++) {
		if(requestack && clientlist[i] != NULL && clientlist[i]->addr != clients[0]) {
			SendBroadcast(data, ipleases->ip.addr, 8000);
			os_printf("Udp send to: %d, smartphone is: %d \r\n", ipleases->ip.addr, clients[0]);
			log_d(LOG_INFO, "Going to forward to nodes, required ack \r\n");
			struct udpreg * p = (struct udpreg *) os_malloc(sizeof(struct udpreg));
			p->addr = ipleases->ip.addr;
			p->port = port;
			os_memcpy(p->data, data, os_strlen(data));
			p->retries = 0;
			p->received = false;
			p->delay = 0;
			linked_insertnode(p);
		}
	}

}


void uart_receive(char * line) {
	sendtoall(line, 8000, false);
}

void callback_master(struct reg_str *  reg) {
	char data[20];
	if(reg->action == WRITE && reg->destination[0] == 'A'  && reg->destination[1] == 'P') {
		clients[0] = reg->addr; // assume the one who writes is the smartphone
		log_d(LOG_INFO, "We got a valid network register request: %s:%d:%x:"BYTETOBINARYPATTERN" "BYTETOBINARYPATTERN"\r\n", reg->destination, reg->action, reg->reg, BYTETOBINARY(reg->val[0]), BYTETOBINARY(reg->val[1]));
		reg_writeaction(reg);
		reg_tochar(reg, data);
		data[0] = '0';
		data[1] = '1';
		sendtoall(data, 8000, true);
	}
	if(reg->action == WRITEOUT) {
		log_d(LOG_INFO, "Sending ACK to smartphone \r\n");
		SendBroadcast("ACK", clients[0], 8000);
	}
}

void scanWatchdog(void * arg) {
	int i = 0;
	log_d(LOG_VERBOSE, "MASTER WATCHDOG TICK \r\n");
	scan_master();
	for(i = 0; i < 10; i++) {
		if(clientlist[i] != NULL) {
			if(clientlist[i]->missedack > 20) {
				os_free(clientlist[i]);
				clientlist[i] = NULL;
			}
		}
	}
	os_timer_arm(&scanTimer, 10000, 0);
}
void init() {
	err_t err;
	int i = 0;
	struct ip_addr ipSend;
	static struct softap_config apconf;
	char * ap = MASTER_AP;
	char * pass = MASTER_PASSWORD;
	uart_init(BIT_RATE_115200, BIT_RATE_115200); // uart_receive, false);
	log_d(LOG_INFO, "Initializing master\r\n");
	list = (struct linked_list*) os_malloc(sizeof(struct linked_list));
	list->first = NULL;
	list->last = NULL;
	list->totalcount = 0;
	wifi_softap_free_station_info();

	for(i = 0; i < 10; i++) {
		clientlist[i] = NULL;
	}

	os_printf("Settings Station AP mode \r\n");
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
	log_d(LOG_INFO, "Starting master watchdog \r\n");
	os_timer_disarm(&scanTimer);
	os_timer_setfn(&scanTimer, scanWatchdog, NULL);
	os_timer_arm(&scanTimer, 50, 0);
	init_queue(&udp_queue);
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
	udp_recv(pUdpConnection, handle_udp_recv, NULL);
	led_init("AP");
	reg_addlistener(callback_master, 0x32);
}

static void handleack(char* data, uint32 addr) {
//	log_d(LOG_INFO, "Got data:");
//	log_d(LOG_INFO, (const char*) data);
//	log_d(LOG_INFO, "\r\n");
	if(os_strstr(data, "ACK") != NULL) {
		log_d(LOG_INFO, "Received ACK \r\n");
		struct linked_node * n;
		n = list->first;
		while(n != NULL && list->totalcount > 0) {
			if(n->data->addr == addr) {
				n = linked_removenode(n);
			} else {
				n = n->next;
			}
		}
	} else if(os_strstr(data, "ALIVE") != NULL) {
		int i = 0;
		char found = 0;

		for(i = 0; i < 10; i++) {
			if(clientlist[i] != NULL) {
				if(clientlist[i]->addr == addr) {
					clientlist[i]->missedack = 0;
					found = 1;
				}
			}
		}

		if(found == 0) {
			for(i = 0; i < 10; i++) {
				if(clientlist[i] == NULL) {
					struct client_data* client = (struct client_data*) os_malloc(sizeof(struct client_data));
					clientlist[i] = client;
				}
			}
		}

	}




	static struct client_data clientlist[10];



}
void tick_master() {
	int i = 0;
	struct udp_data* d = dequeue(&udp_queue);
	log_d(LOG_VERBOSE, "MASTER TICK \r\n");

	if(!watchdog_running) {
		init();
	}

	for(i = 0; i < 10; i++) {
		if(clientlist[i] != NULL) {
			clientlist[i]->missedack++;
		}
	}

	if(d != NULL) {
		log_d(LOG_INFO, "Grabbing latest udp package for processing\r\n");
		handleack(d->p->payload, d->addr);
		network_request(d->p->payload, d->addr);
		pbuf_free(d->p);
	}

	struct linked_node * n;
	n = list->first;
	while(n != NULL && list->totalcount > 0) {
		if(n->data->received == false) {
			n->data->delay++;
			if(n->data->delay > 10) {
				log_d(LOG_INFO, "Did not receive ACK, resending \r\n");
				SendBroadcast(n->data->data, n->data->addr, n->data->port);
				n->data->retries++;
				n->data->delay = 0;
			}
		}

		if(n->data->retries > 1) {
			log_d(LOG_WARNING, "We tried 2 extra times, no ACK, give up \r\n");
			n = linked_removenode(n);
		} else {
			n = n->next;
		}
	}

}

static void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status)
{
  uint8 ssid[33];
  log_d(LOG_VERBOSE, "\r\nScan done");
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
    	  log_d(LOG_INFO, "FOUND MASTER AP\r\n");
    	  config_mode = MODE_SLAVE;
    	  reset_master();
    	  return;
      }
      bss_link = bss_link->next.stqe_next;
    }
  }
}


void scan_master() {
	wifi_station_scan(NULL, scan_done);
}
