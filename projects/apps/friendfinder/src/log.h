/*
 * log.h
 *
 *  Created on: Apr 18, 2015
 *      Author: wouters
 */

#ifndef LOG_H_
#define LOG_H_
#include "config.h"

#define LOG_TICK 0
#define LOG_VERBOSE 1
#define LOG_INFO 2
#define LOG_WARNING 3
#define LOG_ERROR 4

#define log_d(lvl, fmt, ...) do {	\
	if(lvl >= LOG_LEVEL) \
	uart0_tx_buffer(fmt, os_strlen(fmt)); \
} while(0)


#endif /* LOG_H_ */
