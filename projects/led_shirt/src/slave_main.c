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
#include "gpio.h"
#include "led_handler.h"


static uint8_t chackTime = 0;
os_timer_t at_japDelayChack;
char init_slave = 0;
char ip_connected = 0;
int connect_attempts = 0;
static struct udp_pcb * pUdpConnection = NULL;

void scan_slave();
void wifi_connect(char * ssid, char * password, char * bssid);
void reset_slave() {
	init_slave = 0;
	chackTime = 0;
	os_timer_disarm(&at_japDelayChack);
}
static struct softap_config apconf;

void handle_udp_recv_slave(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	//os_printf("UDP Receive: %s \r\n", p->payload);
	network_request(p->payload);
	pbuf_free(p);
}

void initer_slave() {
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


void tick_slave() {

	os_printf("SLAVE TICK \r\n");
	if(!init_slave) {
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
	  	init_slave = 1;
	}
	if(connect_attempts > 5) {
		os_printf("Switching to MASTER \r\n");
		config_mode = MODE_MASTER_S;
		connect_attempts = 0;
		reset_slave();
	}
}


/**
  * @brief  Transparent data through ip.
  * @param  arg: no used
  * @retval None
  */
void ICACHE_FLASH_ATTR
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

}


int8_t ICACHE_FLASH_ATTR
at_dataStrCpy(void *pDest, const void *pSrc, int8_t maxLen)
{
//  assert(pDest!=NULL && pSrc!=NULL);

  char *pTempD = pDest;
  const char *pTempS = pSrc;
  int8_t len;

  if(*pTempS != '\"')
  {
    return -1;
  }
  pTempS++;
  for(len=0; len<maxLen; len++)
  {
    if(*pTempS == '\"')
    {
      *pTempD = '\0';
      break;
    }
    else
    {
      *pTempD++ = *pTempS++;
    }
  }
  if(len == maxLen)
  {
    return -1;
  }
  return len;
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
    	  wifi_connect(bss_link->ssid, MASTER_PASSWORD, bss_link->bssid);
    	  return;
      }
      os_sprintf(temp,"+CWLAP:(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
                 bss_link->authmode, ssid, bss_link->rssi,
                 MAC2STR(bss_link->bssid),bss_link->channel);
      os_printf(temp);
      bss_link = bss_link->next.stqe_next;
    }
  }
  wifi_station_scan(NULL, scan_done_slave);
}


void scan_slave() {
	struct scan_config config;
	wifi_station_scan(NULL, scan_done_slave);
}

/**
  * @brief  Setup commad of join to wifi ap.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR wifi_connect(char * ssid, char * password, char * bssid)
{
	char temp[64];
	struct station_config stationConf;
	int8_t len;
	connect_attempts++;
	//wifi_station_get_config(&stationConf);
	os_bzero(&stationConf, sizeof(struct station_config));
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





