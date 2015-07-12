#ifndef WIFI_TPL_H
#define WIFI_TPL_H

#include "httpd/httpd.h"


void tplWlan(HttpdConnData *connData, char *token, void **arg);

#endif