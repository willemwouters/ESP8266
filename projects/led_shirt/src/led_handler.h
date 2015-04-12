/*
 * led_handler.h
 *
 *  Created on: Apr 12, 2015
 *      Author: wouters
 */

#ifndef LED_HANDLER_H_
#define LED_HANDLER_H_
#include "reg_handler.h"

void led_init();
void led_regcallback(struct reg_str *  reg);

#endif /* LED_HANDLER_H_ */
