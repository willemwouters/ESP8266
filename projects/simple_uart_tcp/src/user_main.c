#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "tcp/TcpServer.h"


#include "tty/uart.h"

static ETSTimer startupTimer;

static struct softap_config apconf;

void ReceiveTcpUdp(void * data) {
	os_printf("\r\nTCP CALLBACK====\r\n");
	uart0_sendStr(data); // will data back to uart
	os_printf("\r\n====END_DATA\r\n");
}

void ReceiveUART(void * data) {
	os_printf("\r\nUART CALLBACK====\r\n");
	os_printf(data);
	err_t err = SendUdpData(data);
	os_printf("\r\n====%dEND_DATA\r\n", err);
}

void SetupWifiAp() {
	char * ap = "WWWWW";
	char * pass = "00000000";

	wifi_set_opmode(2);
	wifi_softap_get_config(&apconf);
	os_strncpy((char*)apconf.ssid, ap, 32);
	os_strncpy((char*)apconf.password, pass, 64);
	apconf.authmode = AUTH_WPA_WPA2_PSK;
	apconf.max_connection = 20;
	apconf.ssid_hidden = 0;
	wifi_softap_set_config(&apconf);
}

void StartupCb(void * arg) {
	SetupTcpUdpServer(1, 60000, 2, ReceiveTcpUdp); // enabled, port, mode, callback
	err_t err = SendUdpData("TEST");
	os_printf("Send out data: %d", err);
}

void user_init(void) {
	SetupUart(BIT_RATE_115200, ReceiveUART, '\r', true); // baudrate, callback, eolchar, printftouart
	SetupWifiAp();

	os_timer_disarm(&startupTimer);
	os_timer_setfn(&startupTimer, StartupCb, NULL);
	os_timer_arm(&startupTimer, 5000, 0);
}

