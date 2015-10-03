/*
 * WifiHelper.cpp
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#include "WifiHelper.h"
#include "Log.h"

extern "C" {
	#include "espressif/esp_common.h"

}

WifiHelper::WifiHelper() {
	Log::e("Test");
}


WifiHelper::~WifiHelper() {
	// TODO Auto-generated destructor stub
}

void WifiHelper::SetAp(char* ap) {
	static struct softap_config apconf;
	Log::i((char*) 1, __FUNCTION__);
	wifi_set_opmode(STATIONAP_MODE);
	wifi_softap_get_config(&apconf);
	memset(apconf.ssid, 0, 32);
	strncpy((char*) apconf.ssid, ap, 32);
	apconf.authmode = AUTH_OPEN;
	apconf.max_connection = 20;
	apconf.ssid_hidden = 0;
	apconf.ssid_len = strlen(ap);
	apconf.channel = 1;
	wifi_softap_set_config(&apconf);
	Log::i((char*) 3, __FUNCTION__, "Done, ", ap);
}
