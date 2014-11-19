

#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "io.h"
#include "espmissingincludes.h"
#include "ip/TcpClient.h"
#include "ip/TcpServer.h"

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

//Callback the code calls when a wlan ap scan is done. Basically stores the result in
//the cgiWifiAps struct.
void ICACHE_FLASH_ATTR wifiScanDoneCb(void *arg, STATUS status) {
	int n;
	
	struct bss_info *bss_link = (struct bss_info *)arg;
		os_printf("-%s-%s status: %d\r\n", __FILE__, __func__, status);

	if (status!=OK) {
		cgiWifiAps.scanInProgress=0;
		wifi_station_disconnect(); //test HACK
		return;
	}

//SendData(cmd);
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

	//We're done.
	cgiWifiAps.scanInProgress=0;
}


//Routine to start a WiFi access point scan.
static void ICACHE_FLASH_ATTR wifiStartScan() {
	int x;
	cgiWifiAps.scanInProgress=1;
	x=wifi_station_get_connect_status();
	if (x!=STATION_GOT_IP) {
		//Unit probably is trying to connect to a bogus AP. This messes up scanning. Stop that.
				os_printf("-%s-%s STA status = %d. Disconnecting STA...\r\n", __FILE__, __func__, x);

		wifi_station_disconnect();
	}
	wifi_station_scan(NULL, wifiScanDoneCb);
}




//This CGI is called from the bit of AJAX-code in wifi.tpl. It will initiate a
//scan for access points and if available will return the result of an earlier scan.
//The result is embedded in a bit of JSON parsed by the javascript in wifi.tpl.
int ICACHE_FLASH_ATTR cgiWiFiScan(HttpdConnData *connData) {
	int len;
	int i;
	char buff[1024];
	os_printf("-%s-%s\r\n", __FILE__, __func__);
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/json");
	httpdEndHeaders(connData);

	if (cgiWifiAps.scanInProgress==1) {
		len=os_sprintf(buff, "{\n \"result\": { \n\"inProgress\": \"1\"\n }\n}\n");
		espconn_sent(connData->conn, (uint8 *)buff, len);
	} else {
		len=os_sprintf(buff, "{\n \"result\": { \n\"inProgress\": \"0\", \n\"APs\": [\n");
		espconn_sent(connData->conn, (uint8 *)buff, len);
		if (cgiWifiAps.apData==NULL) cgiWifiAps.noAps=0;
		for (i=0; i<cgiWifiAps.noAps; i++) {
			len=os_sprintf(buff, "{\"essid\": \"%s\", \"rssi\": \"%d\", \"enc\": \"%d\"}%s\n", 
					cgiWifiAps.apData[i]->ssid, cgiWifiAps.apData[i]->rssi, 
					cgiWifiAps.apData[i]->enc, (i==cgiWifiAps.noAps-1)?"":",");
			espconn_sent(connData->conn, (uint8 *)buff, len);
		}
		len=os_sprintf(buff, "]\n}\n}\n");
		espconn_sent(connData->conn, (uint8 *)buff, len);
		wifiStartScan();
	}
	return HTTPD_CGI_DONE;
}

//Temp store for new ap info.
static struct station_config stconf;
static struct softap_config apconf;

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

//This cgi uses the routines above to POST save AP settings
int ICACHE_FLASH_ATTR cgiWiFiApSave(HttpdConnData *connData) {
	char ssid[128];
	char password[128];
	os_printf("-%s-%s  \r\n", __FILE__, __func__);
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	wifi_softap_get_config(&apconf);
	httpdFindArg(connData->postBuff, "ssid", ssid, sizeof(ssid));
	httpdFindArg(connData->postBuff, "password", password, sizeof(password));
	os_strncpy((char*)apconf.ssid, ssid, 32);
	os_strncpy((char*)apconf.password, password, 64);
	wifi_softap_set_config(&apconf);
	httpdRedirect(connData, "/wifi/wifi.tpl");
	return HTTPD_CGI_DONE;
}


//This cgi uses the routines above to connect to a specific access point with the
//given ESSID using the given password.
int ICACHE_FLASH_ATTR cgiWiFiConnect(HttpdConnData *connData) {
	char essid[128];
	char passwd[128];
	static ETSTimer reassTimer;
	os_printf("-%s-%s  \r\n", __FILE__, __func__);
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	httpdFindArg(connData->postBuff, "essid", essid, sizeof(essid));
	httpdFindArg(connData->postBuff, "passwd", passwd, sizeof(passwd));
	os_strncpy((char*)stconf.ssid, essid, 32);
	os_strncpy((char*)stconf.password, passwd, 64);

	//Schedule disconnect/connect
	os_timer_disarm(&reassTimer);
	os_timer_setfn(&reassTimer, reassTimerCb, NULL);
	os_timer_arm(&reassTimer, 1000, 0);
	httpdRedirect(connData, "connecting.html");
	return HTTPD_CGI_DONE;
}
