/*
 * Thread.cpp
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#include "Thread.h"
#include "TimerThread.h"
extern "C" {
	#include "espressif/esp_common.h"
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "freertos/queue.h"
}


Thread::Thread(pdTASK_CODE ptr) {
	mPtr = ptr;
}

void Thread::sleep(int msec) {
	vTaskDelay(msec / portTICK_RATE_MS);
}

void Thread::start() {
	static xQueueHandle mainqueue;
	xTaskCreate(mPtr, (signed char * )"simple_task", 256, &mainqueue, 2, NULL);
}

Thread::~Thread() {
	// TODO Auto-generated destructor stub
}

