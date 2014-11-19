#ifndef WIFI_CGI_H
#define WIFI_CGI_H

#include "httpd/httpd.h"

int cgiWiFiScan(HttpdConnData *connData);
int cgiWiFi(HttpdConnData *connData);
int cgiWiFiConnect(HttpdConnData *connData);
int cgiWiFiApSave(HttpdConnData *connData);

#endif