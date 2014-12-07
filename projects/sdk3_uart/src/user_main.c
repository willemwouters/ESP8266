#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "driver/uart.h"
#include "lwip/ip_addr.h"

#include "lwip/udp.h"
#include "lwip/tcp.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"

#include "lwip/app/dhcpserver.h"
#include "lwip/app/espconn.h"
#include "lwip/app/espconn_udp.h"

//#include "user_interface.h"
//#include "netif/wlan_lwip_if.h"

static ETSTimer resetTimer;

static ETSTimer waitTimer;
static struct udp_pcb * pUdpConnection = NULL;
static struct udp_pcb * pUdpSendConnection = NULL;

struct ip_info IpInfo;


struct netif * Wlan0;
//struct ip_addr ip_addr_udp_multicast;


void handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	os_printf(p->payload);
}

void SendBroadcast(char * data) {
	struct ip_addr ipSend;
	err_t err;
	pUdpSendConnection = udp_new();
	static struct pbuf* pBuffer;
	IP4_ADDR(&ipSend, 255, 255, 255, 255);
	pBuffer = pbuf_alloc(PBUF_TRANSPORT, os_strlen(data), PBUF_RAM);
	os_memcpy(pBuffer->payload, data, os_strlen(data));
	err = udp_sendto_if(pUdpSendConnection, pBuffer, &ipSend, 2080, Wlan0);
}


void uart_receive(char * line) {
    SendBroadcast(line);
}

void init_udp() {
	err_t err;
	lwip_init();
	pUdpConnection = udp_new();
	if(pUdpConnection == NULL) {
		os_printf("\nCould not create new udp socket... \n");
	}
	err = udp_bind(pUdpConnection, &IpInfo.ip, 2080);
	udp_recv(pUdpConnection, handle_udp_recv, pUdpConnection);
	Wlan0 = get_first_netif();
	/*

	if(Wlan0 != NULL) {
		igmp_start(Wlan0);
		os_printf(" netif != null!!!!! \r\n");
	} else {
		os_printf("SHIT netif == null!!!!! \r\n");
	}

	err = igmp_joingroup(&IpInfo.ip, &ip_addr_udp_multicast);
	os_printf("Igmp join: %d", err);
	*/
	os_timer_disarm(&waitTimer);
}

static struct softap_config apconf;

void user_init(void) {
	char * ap = "Willem";
	char * pass = "00000000";
	uart_init(BIT_RATE_115200, uart_receive, false);
	os_printf("\nUart init done... \n");


	wifi_softap_get_config(&apconf);
	os_strncpy((char*)apconf.ssid, ap, 32);
	os_strncpy((char*)apconf.password, pass, 64);
	apconf.authmode = AUTH_WPA_WPA2_PSK;
	apconf.max_connection = 2;
	apconf.ssid_hidden = 0;
	wifi_softap_set_config(&apconf);

	wifi_softap_dhcps_start();
	wifi_softap_dhcps_stop();

	dhcps_start(&IpInfo);
	dhcps_stop();

	IP4_ADDR(&IpInfo.gw, 192, 168, 10, 1);
	IP4_ADDR(&IpInfo.ip, 192, 168, 10, 1);
	IP4_ADDR(&IpInfo.netmask, 255, 255, 255, 0);

	wifi_set_ip_info(0x01, &IpInfo);


	os_timer_disarm(&waitTimer);
	os_timer_setfn(&waitTimer, init_udp, NULL);
	os_timer_arm(&waitTimer, 5000, 0);

}

