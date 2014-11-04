#ifndef SERVERSTATUS_TPL_H
#define SERVERSTATUS_TPL_H

#include "httpd/httpd.h"


void tplTcpServerStatus(HttpdConnData *connData, char *token, void **arg);

#endif