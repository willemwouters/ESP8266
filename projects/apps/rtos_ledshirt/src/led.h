/*
 * led.h
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#ifndef SRC_LED_H_
#define SRC_LED_H_


void  handle_led_command(char * pusrdata, int length);
void refreshFrameCb();
void init_led();
#endif /* SRC_LED_H_ */
