#ifndef PUSH_INDEX_TPL_H
#define PUSH_INDEX_TPL_H

#include "httpd/httpd.h"
int  cgiPushIndexRequest(HttpdConnData *connData);


void tplPushIndex(HttpdConnData *connData, char *token, void **arg);

#endif