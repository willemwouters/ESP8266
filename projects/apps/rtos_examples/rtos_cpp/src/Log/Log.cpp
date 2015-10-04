/*
 * Log.cpp
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#include "Log.h"
extern "C" {
	#include "espressif/esp_common.h"
	#include "stdarg.h"
	#include "stdio.h"
}

Log::Log() {

}

void Log::i(const char * format, ...) {
	va_list ap;
	int size = (int) format;
	va_start(ap, format);

	if(size > 10) {
		size = 1;
		printf("[INFO]-  [%s]", format);
	} else {
		format = va_arg(ap, char *);
		printf("[INFO]  [%s]", format);
	}

	for(int i = 0; i < size-1; i++ ) {
		format = va_arg(ap, char *);
		printf("%s", format);
	}
	printf("\r\n");
	va_end(ap);
}

void Log::e(char * format, ...) {
	va_list ap;
	va_start(ap, format);
	int size = (int) format;

	format = va_arg(ap, char *);
	printf("[ERROR]  [%s]", format);

	for(int i = 0; i < size-1; i++ ) {
		format = va_arg(ap, char *);
		printf("%s", format);
	}
	printf("\r\n");
	va_end(ap);
}


Log::~Log() {
	// TODO Auto-generated destructor stub
}

