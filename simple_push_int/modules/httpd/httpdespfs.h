#ifndef HTTPDESPFS_H
#define HTTPDESPFS_H

#include "../../modules/fs/espfs.h"
#include "../../modules/httpd/httpd.h"

int cgiEspFsHook(HttpdConnData *connData);
int ICACHE_FLASH_ATTR cgiEspFsTemplate(HttpdConnData *connData);

#endif