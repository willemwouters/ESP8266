/*
Connector to let httpd use the espfs filesystem to serve the files in that.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include "espmissingincludes.h"
#include <string.h>
#include <osapi.h>
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"

#include "httpd.h"
#include "espfs.h"
#include "httpdespfs.h"


//This is a catch-all cgi function. It takes the url passed to it, looks up the corresponding
//path in the filesystem and if it exists, passes the file through. This simulates what a normal
//webserver would do with static files.
int ICACHE_FLASH_ATTR cgiEspFsHook(HttpdConnData *connData) {
	EspFsFile *file=connData->cgiData;
	int len;
	char buff[1024];
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		espFsClose(file);
		return HTTPD_CGI_DONE;
	}

	if (file==NULL) {
		//First call to this cgi. Open the file so we can read it.
		file=espFsOpen(connData->url);
		if (file==NULL) {
			return HTTPD_CGI_NOTFOUND;
		}
		connData->cgiData=file;
		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", httpdGetMimetype(connData->url));
		httpdHeader(connData, "Cache-Control:", "max-age=3600");

		httpdEndHeaders(connData);
		return HTTPD_CGI_MORE;
	}

	len=espFsRead(file, buff, 1024);
	if (len>0) espconn_sent(connData->conn, (uint8 *)buff, len);
	if (len!=1024) {
		//We're done.
		espFsClose(file);
		return HTTPD_CGI_DONE;
	} else {
		//Ok, till next time.
		return HTTPD_CGI_MORE;
	}
}


//cgiEspFsTemplate can be used as a template.

typedef struct {
	EspFsFile *file;
	void *tplArg;
	char tokenTpl[64];
	char tokenInclude[64];
	int tokenTplPos;
	int tokenIncludePos;

} TplData;

typedef void (* TplCallback)(HttpdConnData *connData, char *token, void **arg);


int ICACHE_FLASH_ATTR cgiEspIncludeFsTemplate(HttpdConnData *connData, char * fileName) {
	
	//TplData *tpd=connData->cgiData;
	int len;
	// int x, sp=0;
	//int sp=0;
	//char *e=NULL;
	char buff[1025];
	EspFsFile *file = espFsOpen(fileName);
	len=espFsRead(file, buff, 1024);
	os_printf("-%s-%s reading: fileName: %s\r\n", __FILE__, __func__, fileName);

	if (len!=0) espconn_sent(connData->conn, (uint8 *)buff, len);
	//if (len!=1024) {
		//We're done.
		espFsClose(file);
		return HTTPD_CGI_DONE;
	//} else {
		//Ok, till next time.
	//	return HTTPD_CGI_MORE;
	//}
}


int ICACHE_FLASH_ATTR cgiEspFsTemplate(HttpdConnData *connData) {
	TplData *tpd=connData->cgiData;
	int len;
	int x, sp=0;
	char *e=NULL;
	char buff[1025];

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		((TplCallback)(connData->cgiArg))(connData, NULL, &tpd->tplArg);
		espFsClose(tpd->file);
		os_free(tpd);
		return HTTPD_CGI_DONE;
	}

	if (tpd==NULL) {
		//First call to this cgi. Open the file so we can read it.
		tpd=(TplData *)os_malloc(sizeof(TplData));
		os_printf("-%s-%s Opening file %s\r\n", __FILE__, __func__, connData->url);

		tpd->file=espFsOpen(connData->url);
		tpd->tplArg=NULL;
		tpd->tokenTplPos=-1;
		tpd->tokenIncludePos=-1;

		if (tpd->file==NULL) {
			return HTTPD_CGI_NOTFOUND;
		}
		connData->cgiData=tpd;
		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", httpdGetMimetype(connData->url));
		httpdEndHeaders(connData);
		return HTTPD_CGI_MORE;
	}

	len=espFsRead(tpd->file, buff, 1024);
	if (len>0) {
		sp=0;
		e=buff;
		for (x=0; x<len; x++) {
			if (tpd->tokenTplPos==-1 && tpd->tokenIncludePos==-1) {
				//Inside ordinary text.
				if (buff[x]=='%') {
					//Send raw data up to now
					if (sp!=0) espconn_sent(connData->conn, (uint8 *)e, sp);
					sp=0;
					//Go collect token chars.
					tpd->tokenTplPos=0;
				} else 	if (buff[x]=='@') {
					//Send raw data up to now
					if (sp!=0) espconn_sent(connData->conn, (uint8 *)e, sp);
					sp=0;
						//os_printf("found token \r\n");

					//Go collect token chars.
					tpd->tokenIncludePos=0;
				} else {
					sp++;
				}
			} else {
				if (buff[x]=='@') {
					tpd->tokenInclude[tpd->tokenIncludePos++]=0; //zero-terminate token
					os_printf("%s-%s found include:  %s \r\n", __FILE__, __func__, tpd->tokenInclude);
					 cgiEspIncludeFsTemplate(connData, tpd->tokenInclude);
					//Go collect normal chars again.
					e=&buff[x+1];
					tpd->tokenIncludePos=-1;
				} else if(buff[x]=='%') {
					tpd->tokenTpl[tpd->tokenTplPos++]=0; //zero-terminate token
					os_printf("%s-%s found token:  %s \r\n", __FILE__, __func__, tpd->tokenTpl);

					((TplCallback)(connData->cgiArg))(connData, tpd->tokenTpl, &tpd->tplArg);
					//Go collect normal chars again.
					e=&buff[x+1];
					tpd->tokenTplPos=-1;
				} else {
					if (tpd->tokenIncludePos<(sizeof(tpd->tokenInclude)-1)) tpd->tokenInclude[tpd->tokenIncludePos++]=buff[x];
					if (tpd->tokenTplPos<(sizeof(tpd->tokenTpl)-1)) tpd->tokenTpl[tpd->tokenTplPos++]=buff[x];
				}
			}
		}
	}
	///os_printf("Send remaining bit.:  %s %d \r\n", e, sp);

	//Send remaining bit.
	if (sp!=0) espconn_sent(connData->conn, (uint8 *)e, sp);
	if (len!=1024) {
		//We're done.
		if(connData->cgiArg != NULL)
		((TplCallback)(connData->cgiArg))(connData, NULL, &tpd->tplArg);
		espFsClose(tpd->file);
		return HTTPD_CGI_DONE;
	} else {
		//Ok, till next time.
		return HTTPD_CGI_MORE;
	}
}

