/*
 * led.c
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#include "espressif/esp_common.h"
#include "espmissingincludes.h"
#include "framedriver.h"
#include "config.h"
#include "ws2812.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/udp.h"

#define NORMAL 1
#define FLICKER 2
#define FLICKER_BUFFER 3

static int brightness = 5;
static int fontbackR = 0;
static int fontbackG = 0;
static int fontbackB = 0;
static int fontR = 255;
static int fontG = 255;
static int fontB = 255;
static int MODE = NORMAL;
static int MODEFLASH = NORMAL;
static int scroll = 1;

static int activebuffer = 0;
static int initCommand = 0;
static int flashspeed = 2;
static int textspeed = 1;
static long textframeoffset = 0;


int32 chip_id = 0;
int i1= 0;
int i2= 0;
int i3 = 0;
int i4 = 0;

int FRAME_REFRESH_SPEED =  (1000 / FRAMESPEED); // lets assume 5us for framereset

long framecount = 0;

void init_led() {
	chip_id = system_get_chip_id();
	i1 = (chip_id & 0xff << 24);
	i2= (chip_id & 0xff << 16);
	i3= (chip_id & 0xff << 8);
	i4= (chip_id & 0xff);
}
void refreshFrameCb() {

	int flicker = 0;

	if(activebuffer == 0) {
		if(scroll != 1) {
			textframeoffset = 0;
		}
		write_texttowall(0, 0, textframeoffset, fontR, fontG, fontB, fontbackR, fontbackG, fontbackB);
	}

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
	//ets_intr_lock();
	taskYIELD();

	taskENTER_CRITICAL();
	//taskDISABLE_INTERRUPTS();
	setled(get_startbuffer(activebuffer), (COLUMNS * ROWS * COLORS), brightness);
	//taskENABLE_INTERRUPTS();
	taskEXIT_CRITICAL();

	//ets_intr_unlock();
}

void sendid() {
	char data[6] = {0xFE, i1 , i2, i3, i4, initCommand };
	struct pbuf* b = pbuf_alloc(PBUF_TRANSPORT, 6, PBUF_RAM);
	memcpy (b->payload, data, 6);
	struct udp_pcb * pUdpConnection = udp_new();
	udp_sendto(pUdpConnection, b, IP_ADDR_BROADCAST, 9090);
	pbuf_free(b);
	udp_disconnect(pUdpConnection);
}



void  handle_led_command(char * pusrdata, int length) {

	int i = 0;

	if(length > 2) {

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
					handle_led_command(&pusrdata[1], length-1);
					break;
				case 0xFF:
					//os_printf("Send id \r\n");
					//sendid();
//					os_timer_disarm(&writeudpTimer);
//					os_timer_setfn(&writeudpTimer,(os_timer_func_t *) sendchipid_package, pUdpConnection);
//					os_timer_arm(&writeudpTimer, 5, 0);
					break;
				default:
					break;
			}

		}
	//pbuf_free(p);
}
