/*
 * TimerThread.cpp
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#include "TimerThread.h"
#include "Log.h"

TimerThread::TimerThread() : Thread(&TimerThread::run) {

}

void TimerThread::run(void * p) {
	while(1) {
		Log::i(__FUNCTION__);
		Thread::sleep(100);
	}
}
TimerThread::~TimerThread() {}

