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

static ETSTimer testTimer;
static struct udp_pcb * pUdpConnection = NULL;

struct ip_info IpInfo;


char * data = "HOOOI";

struct ip_addr ip_addr_udp_multicast;


void TestCb(void* arg) {
	static struct pbuf* pBuffer;
	struct ip_addr ip_addr_udp_server;


	struct netif * Wlan0 = get_first_netif();
	if(Wlan0 != NULL) {
		igmp_start(Wlan0);
		os_printf(" netif != null!!!!! \r\n");
	} else {
		os_printf("SHIT netif == null!!!!! \r\n");
	}

	IP4_ADDR(&ip_addr_udp_multicast, 239, 0, 0, 3);
	IP4_ADDR(&ip_addr_udp_server, 192, 168, 10, 1);

	err_t err = igmp_joingroup(&ip_addr_udp_server, &ip_addr_udp_multicast);
	os_printf("Igmp join: %d", err);


	pBuffer = pbuf_alloc(PBUF_TRANSPORT, sizeof(char) * 20, PBUF_RAM);
	os_memcpy(pBuffer->payload, data, 5);
	os_printf("\nSending data... \n");
	err_t errr = udp_sendto(pUdpConnection, pBuffer, &ip_addr_udp_multicast, 2080);
	os_printf("\nUdp send done...  %d \n", errr);
}


void handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	os_printf(p->payload);
	os_printf("\nReceived UDP... \n");


	os_timer_arm(&testTimer, 5000, 0);
}

static struct softap_config apconf;

void user_init(void) {
	char * ap = "Willem";
	char * pass = "00000000";

	wifi_softap_get_config(&apconf);
	os_strncpy((char*)apconf.ssid, ap, 32);
	os_strncpy((char*)apconf.password, pass, 64);
	apconf.authmode = AUTH_WPA_WPA2_PSK;
	apconf.max_connection = 2;
	apconf.ssid_hidden = 0;
	wifi_softap_set_config(&apconf);


	IP4_ADDR(&IpInfo.gw, 192, 168, 10, 1);
	IP4_ADDR(&IpInfo.ip, 192, 168, 10, 1);
	IP4_ADDR(&IpInfo.netmask, 255, 255, 255, 0);

	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_printf("\nUart init done... \n");


	lwip_init();

	os_printf("\nUdp new... \n");
	pUdpConnection = udp_new();
	if(pUdpConnection == NULL) {
		os_printf("\nCould not create new udp socket... \n");
	}
	os_printf("\nUdp new done... \n");
	os_printf("\nUdp bind... \n");


	udp_bind(pUdpConnection, IP_ADDR_ANY, 2080);
	udp_recv(pUdpConnection, handle_udp_recv, NULL);
	os_printf("\nUdp bind done... \n");


	wifi_softap_dhcps_start();
	wifi_softap_dhcps_stop();

	dhcps_start(&IpInfo);
	dhcps_stop();
	wifi_set_ip_info(0x01, &IpInfo);

	os_timer_disarm(&testTimer);
	os_timer_setfn(&testTimer, TestCb, NULL);

	os_printf("\nReady \n");

}

