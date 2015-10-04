/*
 * Thread.h
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#ifndef SRC_THREAD_THREAD_H_
#define SRC_THREAD_THREAD_H_

extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
}
typedef void (*fptr)(void*);

class Thread {
public:
	Thread(pdTASK_CODE p);
	virtual ~Thread();
	void start();
	static void sleep(int msec);

private:
	pdTASK_CODE mPtr;
};

#endif /* SRC_THREAD_THREAD_H_ */
