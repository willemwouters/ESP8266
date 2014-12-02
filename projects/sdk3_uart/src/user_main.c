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
#include "lwip/app/dhcpserver.h"
//#include "user_interface.h"
//#include "netif/wlan_lwip_if.h"

static ETSTimer resetTimer;

static ETSTimer testTimer;
struct udp_pcb * pUdpConnection;
struct udp_pcb * pudp;

char * data = "HOOOI";
void TestCb(void* arg) {
	static struct pbuf pBuffer;
	struct ip_addr ip_addr_udp;

	pBuffer.payload = data;
	IP4_ADDR(&ip_addr_udp, 224, 0, 0, 1);
	pudp = udp_new();
	os_printf("\nSending data... \n");
	udp_sendto(pUdpConnection, &pBuffer, &ip_addr_udp, 2080);
	//udp_sendto(pudp, &pBuffer, &ip_addr_udp, 2080);
	os_printf("\nUdp send done... \n");
	os_timer_arm(&testTimer, 20000, 0);

}


////err_t tcp_recv_null(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
err_t handle_tcp_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	os_printf(p->payload);
	os_printf("\nReceived TCP... \n");
	TestCb(NULL);
	 return ERR_OK;
}

err_t handle_tcp_accept(void * arg, struct tcp_pcb * newpcb, err_t err) {
	os_printf("Handle_tcp_accept ... \n");
	 return ERR_OK;
                                  }

err_t handle_poll(void *arg, struct tcp_pcb *tpcb) {

	os_printf("Handle_poll ... \n");

	return ERR_OK;
}

void handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	os_printf(p->payload);
		os_printf("\nReceived UDP... \n");

}


void WifiConnectCb(void* arg) {
	struct ip_addr ip_addr_bind;


	IP4_ADDR(&ip_addr_bind, 192, 168, 10, 1);




			os_timer_disarm(&testTimer);
			os_timer_setfn(&testTimer, TestCb, NULL);
			//os_printf("-%s-%s sedule udp send loop connect \r\n", __FILE__, __func__);
			os_timer_arm(&testTimer, 20000, 0);

		os_printf("\nTcp new... \n");
		struct tcp_pcb * pTcpConnection = tcp_new();
		os_printf("\nTcp new done... \n");

		tcp_bind(pTcpConnection, &ip_addr_bind, 90);
		pTcpConnection = tcp_listen(pTcpConnection);

		tcp_poll(pTcpConnection, handle_poll, 10);
		tcp_accept(pTcpConnection, handle_tcp_accept);
		tcp_recv(pTcpConnection, handle_tcp_recv);
		os_printf("\nTcp receiver setup done... \n");



}


static struct softap_config apconf;


void user_init(void) {
	char * ap = "ESPAp";
		char * pass = "00000000";
		wifi_softap_get_config(&apconf);
		os_strncpy((char*)apconf.ssid, ap, 32);
		os_strncpy((char*)apconf.password, pass, 64);
		apconf.authmode = AUTH_WPA_WPA2_PSK;
		apconf.max_connection = 2;
		//apconf.ssid_hidden = 0;
		wifi_softap_set_config(&apconf);


	struct ip_info IpInfo;
	IP4_ADDR(&IpInfo.gw, 192, 168, 10, 1);
	IP4_ADDR(&IpInfo.ip, 192, 168, 10, 1);
	IP4_ADDR(&IpInfo.netmask, 255, 255, 255, 0);

	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_printf("\nUart init done... \n");

	os_printf("\nStopping old dhcp server.. \n");
	wifi_softap_dhcps_stop();

	lwip_init();
	dhcps_start(&IpInfo);
	wifi_set_ip_info(0x01, &IpInfo);
	os_printf("\nNew dhcp server running.. \n");

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


	os_timer_disarm(&resetTimer);
	os_timer_setfn(&resetTimer, WifiConnectCb, NULL);
	os_printf("-%s-%s Schedule main loop after Wifi connect \r\n", __FILE__, __func__);
	os_timer_arm(&resetTimer, 5000, 0);

	os_printf("\nReady \n");

}

