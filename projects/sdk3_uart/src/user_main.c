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

static ETSTimer waitTimer;
static struct udp_pcb * pUdpConnection = NULL;

struct ip_info IpInfo;
int port = 9000;


void SendUnicast(void * data) {
	struct ip_addr ipSend;
	struct pbuf* pBuffer;
    struct udp_pcb * pCon = NULL;
	err_t err;
	IP4_ADDR(&ipSend, 192, 168, 10, 101);
	pCon = udp_new();
	pBuffer = pbuf_alloc(PBUF_TRANSPORT, os_strlen(data), PBUF_RAM);
	os_memcpy(pBuffer->payload, (char*) data, os_strlen(data));
	err = udp_sendto(pCon, pBuffer, &ipSend, port);
	if(err != 0) {
		uart0_tx_buffer("ERROR SENDING");
		uart0_tx_buffer("\r\n");
	}
	pbuf_free(pBuffer);
	udp_remove(pCon);
}

void SendBroadcast(void * data) {
	struct ip_addr ipSend;
    struct udp_pcb * pCon = NULL;
	struct pbuf* pBuffer;
	err_t err;
	IP4_ADDR(&ipSend, 255, 255, 255, 255);
	pCon = udp_new();
	pBuffer = pbuf_alloc(PBUF_TRANSPORT, os_strlen(data), PBUF_RAM);
	os_memcpy(pBuffer->payload, (char*) data, os_strlen(data));
	err = udp_sendto(pCon, pBuffer, &ipSend, port);
	if(err != 0) {
		uart0_tx_buffer("ERROR SENDING");
		uart0_tx_buffer("\r\n");
	}
	pbuf_free(pBuffer);
	os_timer_disarm(&waitTimer);
}


void handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	uart0_tx_buffer("Udp:");
	uart0_tx_buffer(p->payload);
	uart0_tx_buffer("\r");
	//SendUnicast(p->payload);
	send_to_all(p->payload, port);
	SendBroadcast(p->payload);
	pbuf_free(p);
}

void uart_receive(char * line) {
	uart0_tx_buffer("Uart:");
    uart0_tx_buffer(line);
    uart0_tx_buffer("\r\n");
	//SendUnicast(line);
	send_to_all(line, port);
	SendBroadcast(line);
}


void init_udp() {
	err_t err;
	struct ip_addr ipSend;

	lwip_init();
	pUdpConnection = udp_new();
	IP4_ADDR(&ipSend, 255, 255, 255, 255);

	pUdpConnection->multicast_ip = ipSend;
	pUdpConnection->remote_ip = ipSend;
	pUdpConnection->remote_port = port;

	if(pUdpConnection == NULL) {
		os_printf("\nCould not create new udp socket... \n");
	}
	err = udp_bind(pUdpConnection, IP_ADDR_ANY, port);
	udp_recv(pUdpConnection, handle_udp_recv, pUdpConnection);
	os_timer_disarm(&waitTimer);
}


static struct softap_config apconf;

void user_init(void) {
	char * ap = "Willem";
	char * pass = "00000000";
	uart_init(BIT_RATE_115200, uart_receive, true);
	os_printf("\nUart init done... \n");


	wifi_softap_get_config(&apconf);
	os_strncpy((char*)apconf.ssid, ap, strlen(ap));
	os_strncpy((char*)apconf.password, pass, strlen(pass));
	apconf.authmode = AUTH_WPA_WPA2_PSK;
	apconf.max_connection = 2;
	apconf.ssid_hidden = 0;
	wifi_softap_set_config(&apconf);


	IP4_ADDR(&IpInfo.gw, 192, 168, 10, 1);
	IP4_ADDR(&IpInfo.ip, 192, 168, 10, 1);
	IP4_ADDR(&IpInfo.netmask, 255, 255, 255, 0);
	wifi_softap_dhcps_stop();
	dhcps_start(&IpInfo);
	wifi_set_ip_info(0x01, &IpInfo);


	os_timer_disarm(&waitTimer);
	os_timer_setfn(&waitTimer, init_udp, NULL);
	os_timer_arm(&waitTimer, 2000, 0);

}

