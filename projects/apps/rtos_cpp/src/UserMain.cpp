/*
 * UserMain.cpp
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#include "UserMain.h"
#include "Helpers/WifiHelper.h"
#include "Log/Log.h"
#include "TimerThread.h"
#include "UdpServer/UdpServer.h"

extern "C" {
	#include "espressif/esp_common.h"
}


UserMain::UserMain() {

	Log::i((char*) 2, "SDK version", system_get_sdk_version());
	WifiHelper::SetAp("Test");

	TimerThread* t = new TimerThread();
	t->start();

	UdpServer* u = new UdpServer();
	u->start();

}

UserMain::~UserMain() {
	// TODO Auto-generated destructor stub
}

