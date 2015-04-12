/*
 * led_handler.c
 *
 *  Created on: Apr 12, 2015
 *      Author: wouters
 */


#include "config.h"
#include "user_interface.h"
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "driver/uart.h"
#include "reg_handler.h"
#include "driver/pwm.h"

static u8 ledvalr = 0;
static u8 ledvalg = 0;
static u8 ledvalb = 0;
static int brightness = 0;
static char brightnesspulse = 0;
static char brightnesspulsespeed = 0;
static int counter = 0;
static int blinkspeed = 0;
static char colorrandom = 0;
static char colorrandomspeed = 1;


#define COLOR_REG 0x01
#define COLORRANDOM_REG 0x02
#define COLORRANDOMSPEED_REG 0x03
#define BLINKSPEED_REG 0x04
#define BRIGHTNESS_REG 0x05
#define BRIGHTNESSPULSE_REG 0x06
#define BRIGHTNESSPULSESPEED_REG 0x07
#define COLOR_REG 0x01
#define COLOR_REG 0x01


static u8 colorr = 0;
static u8 colorg = 0;
static u8 colorb = 0;
static u8 colorrandomrdir = 0;
static int colorrandomr = 0;

static u8 brightnessspeeddir = 0;
static int brightnessspeed = 0;

static u8 colorrandomgdir = 0;
static int colorrandomg = 100;

static u8 colorrandombdir = 0;
static int colorrandomb = 250;

static ETSTimer blinkTimer;
char name[2];



void led_blinker() {
	if(colorrandom == 1) {

		if(colorrandomrdir == 0) { colorrandomr=colorrandomr+(1+colorrandomspeed); } else { colorrandomr=colorrandomr-(1+colorrandomspeed); }
		if(colorrandomr > 250)  { colorrandomrdir = 1; colorrandomr = 250;}
		if(colorrandomr <= 0) { colorrandomrdir = 0; colorrandomr = 0;}

		if(colorrandomgdir == 0) { colorrandomg=colorrandomg+(1+colorrandomspeed); } else { colorrandomg=colorrandomg-(1+colorrandomspeed); }
		if(colorrandomg > 250) { colorrandomgdir = 1; colorrandomg = 250;}
		if(colorrandomg <= 0) { colorrandomgdir = 0; colorrandomg = 0;}

		if(colorrandombdir == 0) { colorrandomb=colorrandomb+(1+colorrandomspeed); } else { colorrandomb=colorrandomb-(1+colorrandomspeed); }
		if(colorrandomb > 250) { colorrandombdir = 1; colorrandomb = 250; }
		if(colorrandomb <= 0) { colorrandombdir = 0; colorrandomb = 0; }


		ledvalr = colorrandomr;
		ledvalg = colorrandomg;
		ledvalb = colorrandomb;
	}

	colorr = ledvalr;
	colorg = ledvalg;
	colorb = ledvalb;
	counter++;
	if(counter % 4 < 2 && blinkspeed != 0) {
		colorr = 0;
		colorg = 0;
		colorb = 0;
	}
	if(counter == 255) {
		counter = 0;
	}
	if(brightness == 0) {
		brightness = 1;
	}

	if(brightnesspulse == 1) {
		if(brightnessspeeddir == 0) { brightness=brightness+(1+brightnesspulsespeed); } else { brightness=brightness-(1+brightnesspulsespeed); }
		if(brightness > 250) { brightnessspeeddir = 1; brightness = 250; }
		if(brightness <= 0) { brightnessspeeddir = 0; brightness = 0; }
	}

	pwm_set_duty(colorr*brightness, 2);
	pwm_set_duty(colorb*brightness, 1);
	pwm_set_duty(colorg*brightness, 0);
	pwm_start();
	if(blinkspeed < 10 && blinkspeed != 0) {
		blinkspeed = 10;
	}
	int timer = blinkspeed;
	if(timer == 0) timer = 10;
	os_timer_arm(&blinkTimer, timer, 0);
}


void led_regcallback(struct reg_str *  reg) {
	if(reg->destination[0] != name[0] || reg->destination[1] != name[1]) {
		return;
	}

	if(reg->reg == COLOR_REG && reg->action == WRITE) {
		ledvalr = reg->val[0];
		ledvalg = reg->val[1];
		ledvalb = reg->val[2];
	}
	if(reg->reg == COLORRANDOM_REG && reg->action == WRITE) {
		colorrandom = reg->val[0];
	}
	if(reg->reg == COLORRANDOMSPEED_REG && reg->action == WRITE) {
		colorrandomspeed = reg->val[0];
	}
	if(reg->reg == BLINKSPEED_REG && reg->action == WRITE) {
		blinkspeed = reg->val[0];
	}
	if(reg->reg == BRIGHTNESS_REG && reg->action == WRITE) {
		brightness = reg->val[0];
	}
	if(reg->reg == BRIGHTNESSPULSE_REG && reg->action == WRITE) {
		brightnesspulse = reg->val[0];
	}
	if(reg->reg == BRIGHTNESSPULSESPEED_REG && reg->action == WRITE) {
		brightnesspulsespeed = reg->val[0];
	}
}

void led_init(char * n) {
		reg_addlistener(led_regcallback, 0x32);
		name[0] = n[0];
		name[1] = n[1];
		os_timer_disarm(&blinkTimer);
		os_timer_setfn(&blinkTimer, led_blinker, NULL);
		os_timer_arm(&blinkTimer, 1, 0);
}


