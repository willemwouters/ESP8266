#ifndef SERVERSTATUS_TPL_H
#define SERVERSTATUS_TPL_H

#include "../../../modules/httpd/httpd.h"


void tplTcpServerStatus(HttpdConnData *connData, char *token, void **arg);

#endif