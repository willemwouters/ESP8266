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
	pbuf_free(pBuffer);
	os_timer_disarm(&waitTimer);
}


void handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	os_printf("%d-%s \r\n", p->len, p->payload);
	uart0_tx_buffer(p->payload, p->len);
	uart0_tx_buffer("\n", 1);
	pbuf_free(p);
}

void uart_receive(char * line) {
	send_to_all(line, port);
	//SendBroadcast(line);
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
	char * ap = "SKUNKWILLEM";
	char * pass = "00000000";
	uart_init(BIT_RATE_115200, uart_receive, false);
	system_set_os_print(1);
	os_printf("\nUart init done... \n");


	wifi_softap_get_config(&apconf);
	memset(apconf.ssid, 0, 32);
	memset(apconf.password, 0, 64);

	os_strncpy((char*)apconf.ssid, ap, 32);
	os_strncpy((char*)apconf.password, pass, 64);
	apconf.authmode = AUTH_WPA_WPA2_PSK;
	apconf.max_connection = 5;
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

