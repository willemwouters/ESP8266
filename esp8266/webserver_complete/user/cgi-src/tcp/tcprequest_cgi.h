#ifndef TCPREQUEST_CGI_H
#define TCPREQUEST_CGI_H

#include "httpd/httpd.h"

void tplTcpRequest(HttpdConnData *connData, char *token, void **arg);

int cgiTcpRequest(HttpdConnData *connData);
int cgiTcpResponse(HttpdConnData *connData);

#endif