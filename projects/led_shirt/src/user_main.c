#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
//#include "TcpServer.h"
#include "reg_handler.h"
#include "reg_actions.h"
#include "config.h"
#include "driver/uart.h"
#include "driver/pwm.h"

static ETSTimer tickTimer;

static struct softap_config apconf;
//uart0_tx_buffer(data, strlen(data)); // will data back to uart
//err_t err = SendUdpData(data);;

void ReceiveUART(char * val) {
	os_printf("Received stuff: %s \r\n", val);
}

void tickCb(void * arg) {
	//SetupTcpUdpServer(1, 60000, 2, ReceiveTcpUdp); // enabled, port, mode, callback
	//err_t err = SendUdpData("TEST");
	switch (config_mode) {
		case MODE_CONNECTING:
			tick_connecting();
			os_timer_arm(&tickTimer, TICK_SPEED_CONNECTING, 0);
			break;
		case MODE_MASTER_C:
		case MODE_MASTER_S:
			tick_master();
			os_timer_arm(&tickTimer, TICK_SPEED_MASTER, 0);
			break;
		case MODE_SLAVE:
			tick_slave();
			os_timer_arm(&tickTimer, TICK_SPEED_SLAVE, 0);
			break;
		default:
			break;
	}
	//os_printf("Send out data: %d", err);
	//os_printf("Timer tick\r\n");

}

void StartupCb(void * arg) {
	config_mode = MODE_CONNECTING;
	wifi_set_opmode(STATION_MODE);
	os_printf("WiFi AP up\r\n");
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_printf("Main timer running\r\n");
	os_timer_arm(&tickTimer, 1000, 0);
}
extern void sendtoall(char * data, int port);
void callback(struct reg_str *  reg) {
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
		sendtoall("ACK", 8000);
		//os_printf("Wrote to reg:%d, %d %d \r\n", reg->reg, reg->val[0], reg->val[0]);
	}
}


void user_init(void) {
	uint8_t PWM_CH[]= {0, 0, 0};
	uint16_t freq = 100;
	pwm_init(freq, PWM_CH);
	pwm_start();
	static struct softap_config apconf;
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

	reset_register();
	uart_init(BIT_RATE_115200, ReceiveUART, true); // baudrate, callback, eolchar, printftouart
	os_printf("Starting up....\r\n");
	reg_init();

	reg_addlistener(callback, 0x32);
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, StartupCb, NULL);
	os_timer_arm(&tickTimer, 5000, 0);
}

