#ifndef STATUS_TPL_H
#define STATUS_TPL_H

#include "httpd/httpd.h"


void tplStatus(HttpdConnData *connData, char *token, void **arg);

#endif