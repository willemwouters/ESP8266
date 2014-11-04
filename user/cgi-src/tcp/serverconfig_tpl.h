#ifndef SERVERCONFIG_TPL_H
#define SERVERCONFIG_TPL_H

#include "httpd/httpd.h"


int cgiTcpServerSave(HttpdConnData *connData);
int cgiTcpServerEnable(HttpdConnData *connData);


void tplTcpServerConfig(HttpdConnData *connData, char *token, void **arg);

#endif