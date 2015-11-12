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
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "espconn.h"
#include "debug.h"
#include "GDBStub.h"

#include "log.h"
#include "ws2812_i2s.h"

#define os_intr_lock ets_intr_lock
#define os_intr_unlock ets_intr_unlock

static struct udp_pcb * pUdpConnection = NULL;
static ETSTimer framerefreshTimer;


int brightness = 20;
int fontbackR = 0;
int fontbackG = 0;
int fontbackB = 0;
int fontR = 255;
int fontG = 255;
int fontB = 255;
int MODE = NORMAL;
int MODEFLASH = NORMAL;
int MODEFLASHPULSE = NORMAL;
int scroll = 1;

int activebuffer = 0;
int initCommand = 0;
int flashspeed = 2;
int flashspeedtext = 2;
int flashspeedicons = 2;
int flashspeedfade = 2;
int multicastlock = 0;
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
float pulsecount = 0;
int pulsedir = 0;

void ICACHE_FLASH_ATTR refreshFrameCb() {
	os_timer_disarm(&framerefreshTimer);


	if(!GetMutex(&refreshMutext)) {
		LOG_E(LOG_USER,  LOG_USER_TAG, "read - Could not get mutex \r\n");
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

	unsigned char tmpbright = brightness;
	if(MODEFLASHPULSE == FLICKER_PULSE) {
				if(pulsecount <= (brightness) && pulsedir == 0) {
					pulsecount += (flashspeedfade);
					if(pulsecount >= (brightness)) {
						pulsedir = 1;
					}
				}

				if(pulsecount >= 0 && pulsedir == 1) {
					pulsecount -= (flashspeedfade);
					if(pulsecount <= 0) {
						pulsedir = 0;
					}
				}
				if(tmpbright - pulsecount > 0 && tmpbright - pulsecount < 255) {
					tmpbright -= (pulsecount);
				}

	}

	if(MODE == FLICKER_BUFFER && (framecount % (flashspeedicons * 2 )) == 0) {
		if(activebuffer == 1) {
			activebuffer = 2;
		} else if(activebuffer == 2) {
			activebuffer = 1;
		}
	}

	if(framecount % flashspeedtext == 0) {
		textframeoffset++;
	}

	framecount++;


	//rainbow_copybuffer(1, framecount);
	//activebuffer = 1;

	WS2812CopyBuffer(get_startbuffer(activebuffer), (COLUMNS * ROWS * COLORS), flicker, tmpbright);
	system_soft_wdt_feed();

	ReleaseMutex(&refreshMutext);
	os_timer_setfn(&framerefreshTimer, refreshFrameCb2, NULL);
	os_timer_arm(&framerefreshTimer, FRAME_REFRESH_SPEED/2, 0);
}


void ICACHE_FLASH_ATTR refreshFrameCb2() {

	os_timer_disarm(&framerefreshTimer);
	if(!GetMutex(&refreshMutext)) {
		LOG_E(LOG_USER, LOG_USER_TAG,"ERROR  Could not get mutex");
		os_timer_arm(&framerefreshTimer, FRAME_REFRESH_RETRYSPEED, 0);
		return;
	}

	//os_intr_lock();
	ets_wdt_disable();
	setled(get_startbuffer(activebuffer), (COLUMNS * ROWS * COLORS), brightness);
	ets_wdt_enable();
	//os_intr_unlock();

	ReleaseMutex(&refreshMutext);
	os_timer_setfn(&framerefreshTimer, refreshFrameCb, NULL);
	os_timer_arm(&framerefreshTimer, FRAME_REFRESH_SPEED/2, 0);
}



void ICACHE_FLASH_ATTR sendchipid_package(struct udp_pcb *pcb) {
	struct ip_addr ipSend;

	char data[6] = {0xFE, i1 , i2, i3, i4, initCommand };
	struct pbuf* b = pbuf_alloc(PBUF_TRANSPORT, 6, PBUF_RAM);
	os_memcpy (b->payload, data, 6);


	IP4_ADDR(&ipSend, 255, 255, 255, 255);
	pcb->multicast_ip = ipSend;
	pcb->remote_ip = ipSend;
	pcb->remote_port = 9090;
	udp_sendto(pcb, b, IP_ADDR_BROADCAST, 9090);
	pbuf_free(b);
}


void ICACHE_FLASH_ATTR udp_receiver(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port, char forward) {
	int length = p->len;
	char * pusrdata = p->payload;
	int i = 0;
	system_soft_wdt_feed();
	static int precommand = 0;


	if(length > 2) {
			if(!GetMutex(&refreshMutext)) {
				LOG_E(LOG_UDP, LOG_UDP_TAG, "write - Could not get mutex ");
				os_timer_arm(&framerefreshTimer, FRAME_REFRESH_RETRYSPEED, 0);
				pbuf_free(p);
				return;
			}




			if(multicastlock == 1 && pusrdata[0] != 0xFE && pusrdata[0] != 0xFF &&   pusrdata[0] != 0x0E && pusrdata[0] != 0xFD && forward != 1) {
				ReleaseMutex(&refreshMutext);
				pbuf_free(p);
				return;
			}

			if(pusrdata[0] == 0xFE) {
					initCommand++;
					LOG_I(LOG_UDP, LOG_UDP_TAG, "Got init command, processing:");
					pusrdata = &pusrdata[1];
					length = length - 1;
			}

			switch (pusrdata[0]) {
				case 0x00:
					LOG_I(LOG_UDP, LOG_UDP_TAG, "Writing text data to buffer: %s", &pusrdata[2]);
					write_textwall_buffer(0, &pusrdata[2], length-2);
					textframeoffset = 0;
					activebuffer = 0;
					scroll = 1;
					if(pusrdata[1] == 0) {
						scroll = 0;
					}
					break;
				case 0x01:
					if(precommand != pusrdata[0])
					LOG_T(LOG_UDP, LOG_UDP_TAG, "Setting stream of data");
					writestream(pusrdata[1], &pusrdata[2], length-2);
					activebuffer = pusrdata[1];
					break;
				case 0x02:
					if(pusrdata[1] == 0x00) {
						MODEFLASH = FLICKER;
						LOG_I(LOG_UDP, LOG_UDP_TAG, "Setting mode to flash");
					} else {
						MODEFLASH = NORMAL;
						LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting mode to normal");
					}
					break;
				case 0x03:
					if(pusrdata[1] == 0x00) {
						MODE = FLICKER_BUFFER;
						LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting mode to flicker");
					} else {
						MODE = NORMAL;
						LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting mode to normal");
					}
					break;
				case 0x04:
					if(pusrdata[1] > 19) {
						pusrdata[1] = 19;
					}
					if(pusrdata[1] < 0) {
						pusrdata[1] = 0;
					}
					LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting speed: %d", (20 - pusrdata[1]));
					flashspeed = 20 - pusrdata[1];
					break;

				case 0x05:
					LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting dim level: %d", pusrdata[1]);
					brightness = pusrdata[1];
					break;
				case 0x06:
					if(pusrdata[1] > 19) {
						pusrdata[1] = 19;
					}
					if(pusrdata[1] < 0) {
						pusrdata[1] = 0;
					}
					LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting textspeed: %d", pusrdata[1]);
					textspeed = 20 - pusrdata[1];
					break;
				case 0x07:
					set_buffer(pusrdata[1], pusrdata[2], pusrdata[3], pusrdata[4]);
					//os_printf("Setting to val \r\n");
					activebuffer = pusrdata[1];
					break;
				case 0x08:
					fontR = pusrdata[1];
					fontG = pusrdata[2];
					fontB = pusrdata[3];
					LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting font front color");
					break;
				case 0x09:
					fontbackR = pusrdata[1];
					fontbackG = pusrdata[2];
					fontbackB = pusrdata[3];
					LOG_I(LOG_UDP,  LOG_UDP_TAG,  "Setting font background color");
					break;
				case 0x0A:
					if(pusrdata[1] == 0x00) {
						MODEFLASHPULSE = FLICKER_PULSE;
						LOG_I(LOG_UDP, LOG_UDP_TAG, "Setting mode to pulse");
					} else {
						MODEFLASHPULSE = NORMAL;
						LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting mode to normal");
					}
					break;
				case 0x0B:
					if(pusrdata[1] > 19) {
						pusrdata[1] = 19;
					}
					if(pusrdata[1] < 0) {
						pusrdata[1] = 0;
					}
					LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting text speed: %d", (20 - pusrdata[1]));
					flashspeedtext = 20 - pusrdata[1];
					break;
				case 0x0C:
					if(pusrdata[1] > 20) {
						pusrdata[1] = 20;
					}
					if(pusrdata[1] < 2) {
						pusrdata[1] = 2;
					}
					LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting fade speed: %d", ( pusrdata[1]));
					flashspeedfade = pusrdata[1];
					break;
				case 0x0D:
					if(pusrdata[1] > 19) {
						pusrdata[1] = 19;
					}
					if(pusrdata[1] < 0) {
						pusrdata[1] = 0;
					}
					LOG_I(LOG_UDP,  LOG_UDP_TAG, "Setting text speed: %d", (20 - pusrdata[1]));
					flashspeedicons = 20 - pusrdata[1];
					break;
				case 0x0E:
					multicastlock = pusrdata[1];
					LOG_I(LOG_UDP, LOG_UDP_TAG, "Settings multicast lock to: %d", multicastlock);
					break;
				case 0xFD:
					LOG_T(LOG_UDP, LOG_UDP_TAG, "Got forward package");
					p->payload = &pusrdata[1];
					p->len = p->len - 1;
					ReleaseMutex(&refreshMutext);
					udp_receiver(arg,pcb, p, addr, port, 1);
					break;

				case 0xFF:
					LOG_T(LOG_UDP, LOG_UDP_TAG, "Ask for ID package");
					sendchipid_package(pcb);
					break;
				default:
					break;
			}
			if(precommand != pusrdata[0]) {
				precommand = pusrdata[0];
			}
			ReleaseMutex(&refreshMutext);
		}
	if(forward == 1) {
		p->len = p->len - 1;
		p->payload = &pusrdata[0]-1;
	}
	pbuf_free(p);
}

void ICACHE_FLASH_ATTR handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	udp_receiver(arg, pcb, p, addr, port, 0);
}



void ICACHE_FLASH_ATTR
shell_init(void)
{
	err_t err;
	struct ip_addr ipSend;
	lwip_init();
	pUdpConnection = udp_new();

	if(pUdpConnection == NULL) {
		LOG_E(LOG_USER, LOG_USER_TAG, "Could not create new udp socket");
	}

	IP4_ADDR(&ipSend, 224, 0, 0, 1);
	int iret = igmp_joingroup(IP_ADDR_ANY,(struct ip_addr *)(&ipSend));
	if(iret != 0) {
		LOG_E(LOG_UDP, LOG_UDP_TAG,"ERROR  igmp_joingroup" );
	}
//	pUdpConnection->multicast_ip = ipSend;
//	pUdpConnection->remote_ip = ipSend;
//	pUdpConnection->remote_port = 8080;
//	pUdpConnection->ttl = 1;
//	pUdpConnection->so_options |= SOF_BROADCAST;
//	pUdpConnection->so_options |= SOF_ACCEPTCONN;
//	pUdpConnection->so_options |= SOF_REUSEADDR;


	err = udp_bind(pUdpConnection, IP_ADDR_ANY, 8080);
	if(err != 0) {
		LOG_E(LOG_UDP, LOG_UDP_TAG,"ERROR  udp_bind");
	}
	udp_recv(pUdpConnection, handle_udp_recv, pUdpConnection);


}



void ICACHE_FLASH_ATTR wifi_event_cb(System_Event_t *evt) {
	if(evt->event == EVENT_STAMODE_GOT_IP) {
		LOG_I(LOG_USER,  LOG_USER_TAG,"Connected to AP, got ip");
		shell_init();
	}
	if(evt->event == EVENT_STAMODE_DISCONNECTED) {
		if(pUdpConnection != NULL) {
			LOG_W(LOG_USER,  LOG_USER_TAG, "Disconnected from AP");
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
	//wifi_set_phy_mode( PHY_MODE_11N );
	wifi_set_phy_mode(PHY_MODE_11B);
	LOG_I(LOG_USER,  LOG_USER_TAG,"Connecting to AP: %s", ap);

	struct station_config apconf;
	wifi_station_set_auto_connect(true);
	wifi_set_opmode(STATION_MODE);
	wifi_station_get_config(&apconf);
	os_strncpy((char*)apconf.ssid, ap, 32);
	os_strncpy((char*)apconf.password, pass, 64);
	wifi_station_set_config(&apconf);
	wifi_set_event_handler_cb(wifi_event_cb);
}


void ICACHE_FLASH_ATTR user_done(void) {
	os_printf("\r\n"); // clear output

	LOG_I(LOG_USER,  LOG_USER_TAG, "Starting up");
	bool t = system_update_cpu_freq(SYS_CPU_160MHZ);
	if(!t) {
		LOG_E(LOG_USER,  LOG_USER_TAG, "Could not set to 160mhz");
	}
	connectToAp();
	chip_id = system_get_chip_id();
	i1 = (chip_id & 0xff << 24);
	i2= (chip_id & 0xff << 16);
	i3= (chip_id & 0xff << 8);
	i4= (chip_id & 0xff);


//	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
//	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDI_U); // disable pullodwn
//	 GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT12);
//	 GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
//

//	char outbuffer[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//	WS2812OutBuffer( outbuffer, 6 , 0); //Initialize the output.
	ws2812_init();

	CreateMutux(&refreshMutext);

	write_textwall_buffer(0, "BIER ", 5); // initial text while no commands

	os_timer_disarm(&framerefreshTimer);
	os_timer_setfn(&framerefreshTimer, refreshFrameCb, NULL);
	os_timer_arm(&framerefreshTimer, FRAME_REFRESH_SPEED, 0);

}

void user_init() {
	uart_init(BIT_RATE_115200, BIT_RATE_115200); //ReceiveUART, true); // baudrate, callback, eolchar, printftouart

	gdb_init();

	system_init_done_cb(user_done);
}
