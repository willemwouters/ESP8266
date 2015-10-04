#include "httpd/httpd.h"

#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"

#include "espconn.h"

//WiFi access point data
typedef struct {
	char ssid[32];
	char rssi;
	char enc;
} ApData;

//Scan resolt
typedef struct {
	char scanInProgress;
	ApData **apData;
	int noAps;
} ScanResultData;

//Static scan status storage.
ScanResultData cgiWifiAps;

static struct station_config stconf;



void ICACHE_FLASH_ATTR wifiScanDoneCb(void *arg, STATUS status) {
	int n;
	struct bss_info *bss_link = (struct bss_info *)arg;
	os_printf("-%s-%s status: %d\r\n", __FILE__, __func__, status);
	if (status!=OK) {
		cgiWifiAps.scanInProgress=0;
		wifi_station_disconnect(); //test HACK
		return;
	}
	//Clear prev ap data if needed.
	if (cgiWifiAps.apData!=NULL) {
		for (n=0; n<cgiWifiAps.noAps; n++) os_free(cgiWifiAps.apData[n]);
		os_free(cgiWifiAps.apData);
	}
	//Count amount of access points found.
	n=0;
	while (bss_link != NULL) {
		bss_link = bss_link->next.stqe_next;
		n++;
	}
	//Allocate memory for access point data
	cgiWifiAps.apData=(ApData **)os_malloc(sizeof(ApData *)*n);
	cgiWifiAps.noAps=n;
	//Copy access point data to the static struct
	n=0;
	bss_link = (struct bss_info *)arg;
	while (bss_link != NULL) {
		cgiWifiAps.apData[n]=(ApData *)os_malloc(sizeof(ApData));
		cgiWifiAps.apData[n]->rssi=bss_link->rssi;
		cgiWifiAps.apData[n]->enc=bss_link->authmode;
		strncpy(cgiWifiAps.apData[n]->ssid, (char*)bss_link->ssid, 32);

		bss_link = bss_link->next.stqe_next;
		n++;
	}
	os_printf("-%s-%s Scan done: found: %d APs\r\n", __FILE__, __func__, n);
	cgiWifiAps.scanInProgress=0;
}



int ICACHE_FLASH_ATTR cgiRedirect(HttpdConnData *connData) {
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	httpdRedirect(connData, "/");
	return HTTPD_CGI_DONE;
}

//This routine is ran some time after a connection attempt to an access point. If
//the connect succeeds, this gets the module in STA-only mode.
static void ICACHE_FLASH_ATTR resetTimerCb(void *arg) {
	int x=wifi_station_get_connect_status();
	if (x==STATION_GOT_IP) {
		//Go to STA mode. This needs a reset, so do that.
		os_printf("-%s-%s Got an IP will set mode to AP and reboot\r\n", __FILE__, __func__);
		//wifi_set_opmode(1);
		system_restart();
	} else {
		os_printf("-%s-%s Connect fail. Not going into STA-only mode\r\n", __FILE__, __func__);
	}
}

//Actually connect to a station. This routine is timed because I had problems
//with immediate connections earlier. It probably was something else that caused it,
//but I can't be arsed to put the code back :P
static void ICACHE_FLASH_ATTR reassTimerCb(void *arg) {
	int x;
	static ETSTimer resetTimer;
	os_printf("-%s-%s Trying to connect to Ap\r\n", __FILE__, __func__);
	wifi_station_disconnect();
	wifi_station_set_config(&stconf);
	wifi_station_connect();
	x=wifi_get_opmode();
	if (x!=1) {
		//Schedule disconnect/connect
		os_timer_disarm(&resetTimer);
		os_timer_setfn(&resetTimer, resetTimerCb, NULL);
		os_printf("-%s-%s  Schedules a reset \r\n", __FILE__, __func__);
		os_timer_arm(&resetTimer, 4000, 0);
	}
}



int ICACHE_FLASH_ATTR cgiSave(HttpdConnData *connData) {
	char essid[128];
		char passwd[128];
		static ETSTimer reassTimer;
		os_printf("-%s-%s  \r\n", __FILE__, __func__);
		if (connData->conn==NULL) {
			//Connection aborted. Clean up.
			return HTTPD_CGI_DONE;
		}
		httpdFindArg(connData->postBuff, "ap", essid, sizeof(essid));
		httpdFindArg(connData->postBuff, "pass", passwd, sizeof(passwd));
		os_strncpy((char*)stconf.ssid, essid, 32);
		os_strncpy((char*)stconf.password, passwd, 64);

		//Schedule disconnect/connect
		os_timer_disarm(&reassTimer);
		os_timer_setfn(&reassTimer, reassTimerCb, NULL);
		os_timer_arm(&reassTimer, 1000, 0);
		httpdRedirect(connData, "/");
		return HTTPD_CGI_DONE;

}
int ICACHE_FLASH_ATTR cgiIndex(HttpdConnData *connData) {
	int len;
	int i;
	char buff[1024];

	if (connData->conn==NULL) {
		return HTTPD_CGI_DONE;
	}

	os_printf("-%s-%s\r\n", __FILE__, __func__);
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/html");
	httpdEndHeaders(connData);
	httpdSendHtmlHeaders(connData);
	if (cgiWifiAps.scanInProgress==1) {

		len=os_sprintf(buff, "<script type=\"text/JavaScript\">\r\nsetTimeout(\"location.href='/';\",1500);</script>");
		espconn_sent(connData->conn, (uint8 *)buff, len);
		len=os_sprintf(buff, "Scanning WiFi Ap's <br />");
		espconn_sent(connData->conn, (uint8 *)buff, len);
	} else {
		len=os_sprintf(buff, "<a href=/>refresh</a><br />\r\n<form action=\"/save.cgi\" method=\"POST\"><br />\r\nNumber of Ap's found: %d <br />\r\n", cgiWifiAps.noAps);
		espconn_sent(connData->conn, (uint8 *)buff, len);
		if (cgiWifiAps.apData==NULL) cgiWifiAps.noAps=0;
		for (i=0; i<cgiWifiAps.noAps; i++) {
			len=os_sprintf(buff, "<input type=\"radio\" name=\"ap\" value=\"%s\">%s<br />",	cgiWifiAps.apData[i]->ssid, cgiWifiAps.apData[i]->ssid);
			espconn_sent(connData->conn, (uint8 *)buff, len);
		}

		len=os_sprintf(buff, "Passphrase: <input name=\"pass\" type=\"text\"><br />\r\n<input type=\"submit\" value=\"Save\"> <br />");
		espconn_sent(connData->conn, (uint8 *)buff, len);

		cgiWifiAps.scanInProgress=1;
		i=wifi_station_get_connect_status();
		if (i!=STATION_GOT_IP) {
			os_printf("-%s-%s STA status = %d. Disconnecting STA...\r\n", __FILE__, __func__, i);
			wifi_station_disconnect();
		}
		wifi_station_scan(NULL, wifiScanDoneCb);
	}
	httpdSendHtmlEndHeaders(connData);

	return HTTPD_CGI_DONE;
}
