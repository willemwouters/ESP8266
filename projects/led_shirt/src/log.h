/*
 * log.h
 *
 *  Created on: Apr 18, 2015
 *      Author: wouters
 */

#ifndef LOG_H_
#define LOG_H_
#include "config.h"

#define LOG_VERBOSE 0
#define LOG_INFO 1
#define LOG_WARNING 2
#define LOG_ERROR 3

#define log_d(lvl, fmt, ...) do {	\
	static const char flash_str[] = fmt;	\
	if(lvl >= LOG_LEVEL) \
	uart0_tx_buffer(fmt, os_strlen(fmt)); \
} while(0)

//os_printf(flash_str, ##__VA_ARGS__);	\

#endif /* LOG_H_ */
