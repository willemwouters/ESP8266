/*
 * index_cgi.h
 *
 *  Created on: Nov 10, 2014
 *      Author: esp8266
 */
#include "httpd/httpd.h"

#ifndef SRC_PAGES_INDEX_CGI_H_
#define SRC_PAGES_INDEX_CGI_H_

int  cgiRedirect(HttpdConnData *connData);
int  cgiSave(HttpdConnData *connData);
int  cgiIndex(HttpdConnData *connData);


#endif /* SRC_PAGES_INDEX_CGI_H_ */
