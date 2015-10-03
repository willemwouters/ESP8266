/*
 * Log.h
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#ifndef SRC_LOG_H_
#define SRC_LOG_H_
#include "bits/stringfwd.h"

class Log {
public:
	Log();
	virtual ~Log();
	static void e(char * format, ...);
	static void i(const char * format, ...);

};

#endif /* SRC_LOG_H_ */
