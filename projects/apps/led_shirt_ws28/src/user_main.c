#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "ws2812.h"
#include "driver/uart.h"
#include "config.h"
#include "framedriver.h"
#include "mutex.h"
#include "lwipopts.h"
#include "lwip/sockets.h"
#include "mem.h"
#include "lwip/stats.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"
#include "lwip/udp.h"
#include "espconn.h"


#define os_intr_lock ets_intr_lock
#define os_intr_unlock ets_intr_unlock

static struct udp_pcb * pUdpConnection = NULL;
static ETSTimer framerefreshTimer;
static ETSTimer writeudpTimer;

int brightness = 5;
int fontbackR = 0;
int fontbackG = 0;
int fontbackB = 0;
int fontR = 255;
int fontG = 255;
int fontB = 255;
int MODE = NORMAL;
int MODEFLASH = NORMAL;
int scroll = 1;

int activebuffer = 0;
int initCommand = 0;
int flashspeed = 2;
int textspeed = 1;

int32 chip_id = 0;
int i1= 0;
int i2= 0;
int i3 = 0;
int i4 = 0;

int32 refreshMutext = 0;

int FRAME_REFRESH_SPEED =  (1000 / FRAMESPEED); // lets assume 5us for framereset

long framecount = 0;
long textframeoffset = 0;

int BrightnessE[256] = { 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3,
						3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5,
					5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7,
					7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 11, 11,
					11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
					19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
					29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
					40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
					55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
					71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
					90, 91, 93, 94, 95, 96, 98, 99,100,102,103,104,106,107,109,110,
					111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,
					135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,
					162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,
					191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
					222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255};

void refreshFrameCb2();
void refreshFrameCb() {
	os_timer_disarm(&framerefreshTimer);


	if(!GetMutex(&refreshMutext)) {
		os_printf("read - Could not get mutex \r\n");
		os_timer_arm(&framerefreshTimer, FRAME_REFRESH_RETRYSPEED, 0);
		return;
	}

	int flicker = 0;

	if(activebuffer == 0) {
		if(scroll != 1) {
			textframeoffset = 0;
		}
		write_texttowall(0, 0, textframeoffset, fontR, fontG, fontB, fontbackR, fontbackG, fontbackB);
	}
	system_soft_wdt_feed();
	if(MODEFLASH == FLICKER && (framecount % (flashspeed + 1)) < ((flashspeed+2) / 2)) {
		flicker = 1;
	}

	if(MODE == FLICKER_BUFFER && (framecount % (flashspeed * 2 )) == 0) {
		if(activebuffer == 1) {
			activebuffer = 2;
		} else if(activebuffer == 2) {
			activebuffer = 1;
		}
	}

	if(framecount % flashspeed == 0) {
		textframeoffset++;
	}

	framecount++;
	WS2812CopyBuffer(get_startbuffer(activebuffer), (COLUMNS * ROWS * COLORS), flicker, brightness);
	system_soft_wdt_feed();

	ReleaseMutex(&refreshMutext);
	os_timer_setfn(&framerefreshTimer, refreshFrameCb2, NULL);
	os_timer_arm(&framerefreshTimer, FRAME_REFRESH_SPEED/2, 0);
}


void refreshFrameCb2() {

	os_timer_disarm(&framerefreshTimer);
	if(!GetMutex(&refreshMutext)) {
		os_printf("read - Could not get mutex \r\n");
		os_timer_arm(&framerefreshTimer, FRAME_REFRESH_RETRYSPEED, 0);
		return;
	}

	os_intr_lock();
	ets_wdt_disable();
	setled(get_startbuffer(activebuffer), (COLUMNS * ROWS * COLORS), brightness);
	ets_wdt_enable();
	os_intr_unlock();

	ReleaseMutex(&refreshMutext);
	os_timer_setfn(&framerefreshTimer, refreshFrameCb, NULL);
	os_timer_arm(&framerefreshTimer, FRAME_REFRESH_SPEED/2, 0);
}



void sendchipid_package(struct udp_pcb *pcb) {
	char data[6] = {0xFE, i1 , i2, i3, i4, initCommand };
	struct pbuf* b = pbuf_alloc(PBUF_TRANSPORT, 6, PBUF_RAM);
	os_memcpy (b->payload, data, 6);
	udp_sendto(pcb, b, IP_ADDR_BROADCAST, 9090);
	pbuf_free(b);
	os_timer_disarm(&writeudpTimer);
}
void ICACHE_FLASH_ATTR handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	int length = p->len;
	char * pusrdata = p->payload;
	int i = 0;

	if(length > 2) {
			if(!GetMutex(&refreshMutext)) {
				os_printf("write - Could not get mutex \r\n");
				os_timer_arm(&framerefreshTimer, FRAME_REFRESH_RETRYSPEED, 0);
				pbuf_free(p);
				return;
			}

			switch (pusrdata[0]) {
				case 0x00:
					os_printf("Writing text data to buffer: 0 len%d\r\n", length-2);
					write_textwall_buffer(0, &pusrdata[2], length-2);
					textframeoffset = 0;
					activebuffer = 0;
					scroll = 1;
					if(pusrdata[1] == 0) {
						scroll = 0;
					}
					os_printf("Text:");
					for(i = 2; i < length; i++) {
						os_printf("%c", pusrdata[i]);
					}
					os_printf("\n");
					break;
				case 0x01:
					writestream(pusrdata[1], &pusrdata[2], length-2);
					activebuffer = pusrdata[1];
					break;
				case 0x02:
					os_printf("Setting mode to flicker \r\n");
					if(pusrdata[1] == 0x00) {
						MODEFLASH = FLICKER;
						os_printf("Setting mode to flicker buffer\r\n");
					} else {
						MODEFLASH = NORMAL;
						os_printf("Setting mode to normal \r\n");
					}
					break;
				case 0x03:
					if(pusrdata[1] == 0x00) {
						MODE = FLICKER_BUFFER;
						os_printf("Setting mode to flicker buffer\r\n");
					} else {
						MODE = NORMAL;
						os_printf("Setting mode to normal \r\n");
					}
					break;
				case 0x04:
					if(pusrdata[1] > 19) {
						pusrdata[1] = 19;
					}
					if(pusrdata[1] < 0) {
						pusrdata[1] = 0;
					}
					os_printf("Setting framespeed: %d \r\n", pusrdata[1]);
					flashspeed = 20 - pusrdata[1];
					break;
				case 0x05:
					os_printf("Setting dim: %d \r\n", pusrdata[1]);
					brightness = pusrdata[1];
					break;
				case 0x06:
					if(pusrdata[1] > 19) {
						pusrdata[1] = 19;
					}
					if(pusrdata[1] < 0) {
						pusrdata[1] = 0;
					}
					os_printf("Setting textspeed: %d \r\n", pusrdata[1]);
					textspeed = 20 - pusrdata[1];
					break;
				case 0x07:
					set_buffer(pusrdata[1], pusrdata[2], pusrdata[3], pusrdata[4]);
					os_printf("Setting to val \r\n");
					activebuffer = pusrdata[1];
					break;
				case 0x08:
					fontR = pusrdata[1];
					fontG = pusrdata[2];
					fontB = pusrdata[3];
					os_printf("Setting fontcolor \r\n");
					break;
				case 0x09:
					fontbackR = pusrdata[1];
					fontbackG = pusrdata[2];
					fontbackB = pusrdata[3];
					os_printf("Setting font back \r\n");
					break;
				case 0xFE:
					initCommand++;
					ReleaseMutex(&refreshMutext);
					p->payload = &pusrdata[1];
					p->len = p->len - 1;
					handle_udp_recv(arg, pcb, p,  addr, port);
					break;
				case 0xFF:
					os_timer_disarm(&writeudpTimer);
					os_timer_setfn(&writeudpTimer,(os_timer_func_t *) sendchipid_package, pUdpConnection);
					os_timer_arm(&writeudpTimer, 5, 0);
					break;
				default:
					break;
			}
			ReleaseMutex(&refreshMutext);
		}
	pbuf_free(p);
}

void ICACHE_FLASH_ATTR
shell_init(void)
{
	err_t err;
	struct ip_addr ipSend;
	lwip_init();
	pUdpConnection = udp_new();
	IP4_ADDR(&ipSend, 255, 255, 255, 255);
	pUdpConnection->multicast_ip = ipSend;
	pUdpConnection->remote_ip = ipSend;
	pUdpConnection->remote_port = 8080;
	if(pUdpConnection == NULL) {
		os_printf("\nCould not create new udp socket... \n");
	}
	err = udp_bind(pUdpConnection, IP_ADDR_ANY, 8080);
	udp_recv(pUdpConnection, handle_udp_recv, pUdpConnection);
}



void ICACHE_FLASH_ATTR wifi_event_cb(System_Event_t *evt) {
	if(evt->event == EVENT_STAMODE_GOT_IP) {
		shell_init();
	}
	if(evt->event == EVENT_STAMODE_DISCONNECTED) {
		if(pUdpConnection != NULL) {
			udp_disconnect(pUdpConnection);
			udp_remove(pUdpConnection);
			pUdpConnection = NULL;
			initCommand = 0;
		}
	}

}
void ICACHE_FLASH_ATTR connectToAp() {
	char * ap = "LedAccess";
	char * pass = "test12345";
	wifi_set_phy_mode( PHY_MODE_11N );

	struct station_config apconf;
	wifi_station_set_auto_connect(true);
	wifi_set_opmode(STATION_MODE);
	wifi_station_get_config(&apconf);
	os_strncpy((char*)apconf.ssid, ap, 32);
	os_printf("connecting to: %s", apconf.ssid);
	os_strncpy((char*)apconf.password, pass, 64);
	wifi_station_set_config(&apconf);

	wifi_set_event_handler_cb(wifi_event_cb);
}

void user_done(void) {

	connectToAp();

	bool t = system_update_cpu_freq(SYS_CPU_160MHZ);
	if(!t) {
		os_printf("Could not set high cpu freq \r\n");
	}
	chip_id = system_get_chip_id();
	i1 = (chip_id & 0xff << 24);
	i2= (chip_id & 0xff << 16);
	i3= (chip_id & 0xff << 8);
	i4= (chip_id & 0xff);


	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO5_U); // disable pullodwn
	 GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT5);
	 GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 1);


	char outbuffer[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	WS2812OutBuffer( outbuffer, 6 , 0); //Initialize the output.


	CreateMutux(&refreshMutext);

	write_textwall_buffer(0, "BIER ", 5);

	os_timer_disarm(&framerefreshTimer);
	os_timer_setfn(&framerefreshTimer, refreshFrameCb, NULL);
	os_timer_arm(&framerefreshTimer, FRAME_REFRESH_SPEED, 0);
}

void user_init() {
	uart_init(BIT_RATE_115200, BIT_RATE_115200); //ReceiveUART, true); // baudrate, callback, eolchar, printftouart

	system_init_done_cb(user_done);
}
