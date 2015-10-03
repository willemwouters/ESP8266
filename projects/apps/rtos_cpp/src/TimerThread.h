/*
 * TimerThread.h
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#ifndef SRC_TIMERTHREAD_H_
#define SRC_TIMERTHREAD_H_
#include "Thread/Thread.h"

class TimerThread : public Thread{
public:
	TimerThread();
	virtual ~TimerThread();
	static void run(void * p);
};

#endif /* SRC_TIMERTHREAD_H_ */
