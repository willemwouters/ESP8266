#ifndef PUSH_CONSOLE_TPL_H
#define PUSH_CONSOLE_TPL_H

#include "../../../modules/httpd/httpd.h"


void tplPushConsole(HttpdConnData *connData, char *token, void **arg);

#endif