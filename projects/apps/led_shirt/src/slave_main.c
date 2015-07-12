/*
 * slave_main.c
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
#include "led_handler.h"


static uint8_t chackTime = 0;
static os_timer_t at_japDelayChack;
static os_timer_t scanTimer;
static os_timer_t watchdog_slave;
static char init_slave = 0;
static char ip_connected = 0;
static int connect_attempts = 0;
static struct udp_pcb * pUdpConnection = NULL;

static bool lock = false;

void SendBroadcastSlave(char * data, ip_addr_t* ipSend, int port) {
	struct udp_pcb * pCon = NULL;
	struct pbuf* pBuffer;
	err_t err;
	pCon = udp_new();
	pBuffer = pbuf_alloc(PBUF_TRANSPORT, os_strlen(data), PBUF_RAM);
	os_memcpy(pBuffer->payload, (char*) data, os_strlen(data));
	err = udp_sendto(pCon, pBuffer, ipSend, 8000);
	pbuf_free(pBuffer);
	udp_remove(pCon);
	os_printf("d pbuf\r\n");
}


static void scan_slave();
static void wifi_connect(char * ssid, char * password, char * bssid);

static void reset_slave() {
	init_slave = 0;
	chackTime = 0;
	ip_connected = 0;
	wifi_station_disconnect();
	os_timer_disarm(&at_japDelayChack);
}
static struct softap_config apconf;

static void handle_udp_recv_slave(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	os_printf("UDP Receive: %s \r\n", p->payload);
	while(lock) { os_delay_us(100); }
	network_request(p->payload, addr->addr);
	os_printf("all callbacks done\r\n");
	pbuf_free(p);
	os_printf("freeing pbuf \r\n");
}

static void initer_slave() {
	err_t err;
	struct ip_addr ipSend;
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
	udp_recv(pUdpConnection, handle_udp_recv_slave, pUdpConnection);
	led_init("01");
}

void callback_slave(struct reg_str *  reg) {
	os_printf("callback \r\n");
	if(reg->action == READ) {
		reg_readaction(reg);
	}
	if(reg->action == READOUT) {
		//os_printf("Read from reg:%d, %d %d \r\n", reg->reg, reg->val[0], reg->val[1]);
	}
	if(reg->action == WRITE) {
		reg_writeaction(reg);
	}
	if(reg->action == WRITEOUT) {
		struct ip_addr ipSend;
		IP4_ADDR(&ipSend, 255, 255, 255, 255);

		os_printf("Sending out ACK \r\n");
		SendBroadcastSlave("ACK", &ipSend, 8000);
	}
	os_printf("callback done \r\n");
}

void slave_sendalive() {
	struct ip_addr ipSend;
	os_timer_disarm(&watchdog_slave);
	IP4_ADDR(&ipSend, 255, 255, 255, 255);
	os_printf("Sending out ACK \r\n");
	SendBroadcastSlave("ALIVE", &ipSend, 8000);
	os_timer_arm(&watchdog_slave, 2000, 0);
}

void initslave(){
	char * ap = CLIENT_AP;
	char * pass = MASTER_PASSWORD;
	wifi_set_opmode(STATION_MODE);
	wifi_softap_get_config(&apconf);
	os_strncpy((char*)apconf.ssid, ap, 32);
	os_strncpy((char*)apconf.password, pass, 64);
	apconf.authmode = AUTH_WPA_WPA2_PSK;
	apconf.max_connection = 20;
	apconf.ssid_hidden = 0;
	wifi_softap_set_config(&apconf);
	scan_slave();
	reg_addlistener(callback_slave, 0x32);

	os_timer_setfn(&watchdog_slave, (os_timer_func_t *)slave_sendalive, NULL);
	os_timer_arm(&watchdog_slave, 2000, 0);
}


void tick_slave() {

	os_printf("SLAVE TICK \r\n");
	if(!init_slave) {
		initslave();
	  	init_slave = 1;
	}
	if(connect_attempts > 5) {
		os_printf("Switching to MASTER \r\n");
		config_mode = MODE_MASTER_S;
		connect_attempts = 0;
		reset_slave();
	}
	os_printf("slave tick done\r\n");
}

static int scancount = 0;
/**
  * @brief  Transparent data through ip.
  * @param  arg: no used
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_japChack(void *arg)
{

  uint8_t japState;
  char temp[32];
  os_printf("Checking if we have ip...\r\n");
  os_timer_disarm(&at_japDelayChack);
  chackTime++;
  japState = wifi_station_get_connect_status();
  if(japState == STATION_GOT_IP)
  {
	os_printf("YEAHHH we have ip...\r\n");
    chackTime = 0;
    connect_attempts = 0;
    if(ip_connected == 0) {
    	initer_slave();
    	ip_connected = 1;
    }
  }
  else if(chackTime >= 10)
  {
	ip_connected = 0;
    wifi_station_disconnect();
    os_sprintf(temp,"+CWJAP:%d\r\n",japState);
    os_printf(temp);
    os_printf("\r\nFAIL\r\n");
    reset_slave();
    return;
  }
  //scan_slave();
  os_timer_arm(&at_japDelayChack, 2000, 0);
  os_printf("ip check done\r\n");
}



/**
  * @brief  Wifi ap scan over callback to display.
  * @param  arg: contain the aps information
  * @param  status: scan over status
  * @retval None
  */


static void ICACHE_FLASH_ATTR
scan_done_slave(void *arg, STATUS status)
{
  uint8 ssid[33];
  char temp[128];
  os_printf("scan done init\r\n");
  bool found = false;
  os_timer_disarm(&scanTimer);
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
    	  scancount = 0;
    	  found = true;
    	  if(ip_connected == 0)
    		  wifi_connect(bss_link->ssid, MASTER_PASSWORD, bss_link->bssid);
      }
      os_sprintf(temp,"+CWLAP:(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
                 bss_link->authmode, ssid, bss_link->rssi,
                 MAC2STR(bss_link->bssid),bss_link->channel);
      os_printf(temp);
      bss_link = bss_link->next.stqe_next;
    }
  }
  scancount++;
  if(scancount > 3  && found == 0) {
	config_mode = MODE_MASTER_S;
	connect_attempts = 0;
	reset_slave();
	return;
  }

  os_timer_arm(&scanTimer, 2000, 0);
  os_printf("scan done\r\n");
}

static void scan() {
	os_timer_setfn(&scanTimer, (os_timer_func_t *)scan, NULL);
	wifi_station_scan(NULL, scan_done_slave);
}


static void scan_slave() {
	struct scan_config config;
	scan();
	scancount = 0;
}

/** 
  * @brief  Setup commad of join to wifi ap.
  * @param  id: commad id number  
  * @param  pPara: AT input param
  * @retval None
  */
static void ICACHE_FLASH_ATTR wifi_connect(char * ssid, char * password, char * bssid)
{
	char temp[64];
	struct station_config stationConf;
	int8_t len;
	connect_attempts++;
	//wifi_station_get_config(&stationConf);
	ets_bzero(&stationConf, sizeof(struct station_config));
	os_memcpy(&stationConf.ssid, ssid, os_strlen(ssid));
	//os_memcpy(&stationConf.password, password, os_strlen(password));
	os_memcpy(&stationConf.bssid, bssid, 6);
	stationConf.bssid_set = 1;
    wifi_station_disconnect();
    os_printf("stationConf.ssid: -%s-\r\n", stationConf.ssid);
    os_printf("stationConf.password: -%s-\r\n", stationConf.password);

    ETS_UART_INTR_DISABLE();
    wifi_station_set_config(&stationConf);
    ETS_UART_INTR_ENABLE();
    wifi_station_connect();
    os_timer_disarm(&at_japDelayChack);
    os_timer_setfn(&at_japDelayChack, (os_timer_func_t *)at_japChack, NULL);
    os_timer_arm(&at_japDelayChack, 3000, 0);

}





