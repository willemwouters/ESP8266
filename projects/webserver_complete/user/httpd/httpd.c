//Esp8266 http server - core routines

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */
#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"

#include "espconn.h"
#include "httpd.h"
#include "io.h"
#include "espfs.h"

//Max length of request head
#define MAX_HEAD_LEN 1024
//Max amount of connections
#define MAX_CONN 8
//Max post buffer len
#define MAX_POST 1024

#define PUSHCONNECTION_TIMEOUT 30

//This gets set at init time.
static HttpdBuiltInUrl *builtInUrls;

//Private data for httpd thing
struct HttpdPriv {
	char head[MAX_HEAD_LEN];
	int headPos;
	int postPos;
};

//Connection pool
static HttpdPriv connPrivData[MAX_CONN];
static HttpdConnData connData[MAX_CONN];

static struct espconn httpdConn;
static esp_tcp httpdTcp;


typedef struct {
	const char *ext;
	const char *mimetype;
} MimeMap;

//The mappings from file extensions to mime types. If you need an extra mime type,
//add it here.
static const MimeMap mimeTypes[]={
	{"htm", "text/htm"},
	{"html", "text/html"},
	{"push", "text/event-stream"},
	{"js", "text/javascript"},
	{"txt", "text/plain"},
	{"jpg", "image/jpeg"},
	{"jpeg", "image/jpeg"},
	{"png", "image/png"},
	{NULL, "text/html"}, //default value
};

//Returns a static char* to a mime type for a given url to a file.
const char ICACHE_FLASH_ATTR *httpdGetMimetype(char *url) {
	int i=0;
	//Go find the extension
	char *ext=url+(strlen(url)-1);
	while (ext!=url && *ext!='.') ext--;
	if (*ext=='.') ext++;

	while (mimeTypes[i].ext!=NULL && os_strcmp(ext, mimeTypes[i].ext)!=0) i++;
	return mimeTypes[i].mimetype;
}

//Looks up the connData info for a specific esp connection
static HttpdConnData ICACHE_FLASH_ATTR *httpdFindConnData(void *arg) {
	int i;
	for (i=0; i<MAX_CONN; i++) {
		if (connData[i].conn==(struct espconn *)arg) return &connData[i];
	}
	os_printf("-%s-%s FindConnData: Huh? Couldn't find connection for %p\r\n", __FILE__, __func__, arg);
	return NULL; //WtF?
}

//Retires a connection for re-use
static void ICACHE_FLASH_ATTR httpdRetireConn(HttpdConnData *conn) {
	if (conn->postBuff!=NULL) os_free(conn->postBuff);
	conn->postBuff=NULL;
	conn->cgi=NULL;
	conn->conn=NULL;
}

//Stupid li'l helper function that returns the value of a hex char.
static int httpdHexVal(char c) {
	if (c>='0' && c<='9') return c-'0';
	if (c>='A' && c<='F') return c-'A'+10;
	if (c>='a' && c<='f') return c-'a'+10;
	return 0;
}

//Decode a percent-encoded value.
//Takes the valLen bytes stored in val, and converts it into at most retLen bytes that
//are stored in the ret buffer. Returns the actual amount of bytes used in ret. Also
//zero-terminates the ret buffer.
int httpdUrlDecode(char *val, int valLen, char *ret, int retLen) {
	int s=0, d=0;
	int esced=0, escVal=0;
	while (s<valLen && d<retLen) {
		if (esced==1)  {
			escVal=httpdHexVal(val[s])<<4;
			esced=2;
		} else if (esced==2) {
			escVal+=httpdHexVal(val[s]);
			ret[d++]=escVal;
			esced=0;
		} else if (val[s]=='%') {
			esced=1;
		} else if (val[s]=='+') {
			ret[d++]=' ';
		} else {
			ret[d++]=val[s];
		}
		s++;
	}
	if (d<retLen) ret[d]=0;
	return d;
}

//Find a specific arg in a string of get- or post-data.
//Line is the string of post/get-data, arg is the name of the value to find. The
//zero-terminated result is written in buff, with at most buffLen bytes used. The
//function returns the length of the result, or -1 if the value wasn't found.
int ICACHE_FLASH_ATTR httpdFindArg(char *line, char *arg, char *buff, int buffLen) {
	char *p, *e;
	if (line==NULL) return 0;
	p=line;
	while(p!=NULL && *p!='\n' && *p!='\r' && *p!=0) {
		os_printf("-%s-%s findArg: %s\r\n", __FILE__, __func__, p);
		if (os_strncmp(p, arg, os_strlen(arg))==0 && p[strlen(arg)]=='=') {
			p+=os_strlen(arg)+1; //move p to start of value
			e=(char*)os_strstr(p, "&");
			if (e==NULL) e=p+os_strlen(p);
			os_printf("-%s-%s findArg: val %s len %d\r\n", __FILE__, __func__,  p, (e-p));			
			return httpdUrlDecode(p, (e-p), buff, buffLen);
		}
		p=(char*)os_strstr(p, "&");
		if (p!=NULL) p+=1;
	}
	os_printf("-%s-%s Finding %s in %s: Not found\r\n", __FILE__, __func__, arg, line);	
	return -1; //not found
}

//Start the response headers.
void ICACHE_FLASH_ATTR httpdStartResponse(HttpdConnData *conn, int code) {
	char buff[128];
	int l;
	l=os_sprintf(buff, "HTTP/1.1 %d OK\r\nServer: esp8266-httpd/1.0\r\n", code);
	espconn_sent(conn->conn, (uint8 *)buff, l);
}

//Send a http header.
void ICACHE_FLASH_ATTR httpdHeader(HttpdConnData *conn, const char *field, const char *val) {
	char buff[256];
	int l;
	l=os_sprintf(buff, "%s: %s\r\n", field, val);
	espconn_sent(conn->conn, (uint8 *)buff, l);
}

//Finish the headers.
void ICACHE_FLASH_ATTR httpdEndHeaders(HttpdConnData *conn) {
	espconn_sent(conn->conn, (uint8 *)"\r\n", 2);
}

//ToDo: sprintf->snprintf everywhere
void ICACHE_FLASH_ATTR httpdRedirect(HttpdConnData *conn, char *newUrl) {
	char buff[1024];
	int l;
	l=os_sprintf(buff, "HTTP/1.1 302 Found\r\nLocation: %s\r\n\r\nMoved to %s\r\n", newUrl, newUrl);
	espconn_sent(conn->conn, (uint8 *)buff, l);
}

int ICACHE_FLASH_ATTR cgiRedirect(HttpdConnData *connData) {
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	httpdRedirect(connData, (char*)connData->cgiArg);
	return HTTPD_CGI_DONE;
}

//This routine is ran some time after a connection attempt to an access point. If
//the connect succeeds, this gets the module in STA-only mode.
static void ICACHE_FLASH_ATTR disconnectPushTimerCb(void *arg) {
	HttpdConnData *conn= (HttpdConnData *) arg;
	os_printf("-%s-%s Disconnecting the Push Tcp Stream\r\n", __FILE__, __func__);	
	os_printf("-%s-%s Set stream timeout to: %d on conn %p\r\n", __FILE__, __func__,  1, conn->conn);	
	espconn_regist_time(conn->conn, 1, 0); // needs to be reset
	espconn_disconnect(conn->conn);
	httpdRetireConn(conn);
}

 void ICACHE_FLASH_ATTR httpdPushMessage(char * url , char * message) {
	//See if the url is somewhere in our internal url table.
	char dataRaw[256];
	os_sprintf(dataRaw,"retry: 10\ndata: %s\n\n", message);
	int i;
	for (i=0; i<MAX_CONN; i++) {
		HttpdConnData * conn = &connData[i];
		if (conn!=NULL && conn->url != NULL) {
			if(os_strcmp(httpdGetMimetype(conn->url), "text/event-stream") == 0 &&  os_strcmp(conn->url, url) == 0) {
			//os_printf("Send push Message to: %s-%s-\r\n", conn->url, dataRaw);
			httpdStartResponse(connData, 200);
			httpdHeader(connData, "Content-Type", httpdGetMimetype(connData->url));
			 if(os_strcmp(url, "/push/console.push") != 0)
			os_printf("-%s-%s Pushing message: %s, to %s\r\n", __FILE__, __func__,  message, url);	

			espconn_sent(conn->conn, (uint8 *)dataRaw, os_strlen(dataRaw));
			}
		}
	}
}
 void ICACHE_FLASH_ATTR httpdSentCb(void *arg) {
	int r;
	HttpdConnData *conn=httpdFindConnData(arg);
	static ETSTimer resetTimerPush;
	if (conn==NULL) return;
	if (conn->cgi==NULL) { //Marked for destruction?
		if(os_strcmp(conn->url, "/push/console.push") != 0) {
				os_printf("-%s-%s \r\n", __FILE__, __func__);		
		}
		if(os_strcmp(httpdGetMimetype(conn->url), "text/event-stream") == 0) {
				os_timer_disarm(&resetTimerPush);
				os_timer_setfn(&resetTimerPush, disconnectPushTimerCb, conn);
				os_timer_arm(&resetTimerPush, PUSHCONNECTION_TIMEOUT * 1000, 0);
				if(os_strcmp(conn->url, "/push/console.push") != 0) {
					os_printf("-%s-%s Set stream timeout to: %d on conn %p\r\n", __FILE__, __func__, PUSHCONNECTION_TIMEOUT, conn->conn);		
				}
				espconn_regist_time(conn->conn, PUSHCONNECTION_TIMEOUT, 0);
		} else {
			os_printf("-%s-%s Set stream timeout to: %d on conn %p\r\n", __FILE__, __func__, 1, conn->conn);		
			espconn_regist_time(conn->conn, 1, 0);  // needs to be reset
			os_printf("-%s-%s Conn %p is done. cgi page Closing: %s.\r\n", __FILE__, __func__,  conn->conn, conn->url);	
			espconn_disconnect(conn->conn);
			httpdRetireConn(conn);
		}
		return;
	}

	r=conn->cgi(conn); //Execute cgi fn.
	if (r==HTTPD_CGI_DONE) {
		conn->cgi=NULL; //mark for destruction. BUT ONLY IF IT ALREADY SEND SOMETHING
	}
}

static const char *httpNotFoundHeader="HTTP/1.0 404 Not Found\r\nServer: esp8266-httpd/0.1\r\nContent-Type: text/plain\r\n\r\nNot Found.\r\n";

static void ICACHE_FLASH_ATTR httpdSendResp(HttpdConnData *conn) {
	int i=0;
	int r;
	//See if the url is somewhere in our internal url table.
	while (builtInUrls[i].url!=NULL && conn->url!=NULL) {
		if (os_strcmp(builtInUrls[i].url, conn->url)==0 || builtInUrls[i].url[0]=='*') {
			os_printf("-%s-%s Is url index %d\r\n", __FILE__, __func__, i);	
			conn->cgiData=NULL;
			conn->cgi=builtInUrls[i].cgiCb;
			conn->cgiArg=builtInUrls[i].cgiArg;
			r=conn->cgi(conn);
			if (r!=HTTPD_CGI_NOTFOUND) {
				if (r==HTTPD_CGI_DONE) conn->cgi=NULL;  //If cgi finishes immediately: mark conn for destruction.
				return;
			}
		}
		i++;
	}
	//Can't find :/
	os_printf("-%s-%s %s not found. 404!\r\n", __FILE__, __func__, conn->url);		
	espconn_sent(conn->conn, (uint8 *)httpNotFoundHeader, os_strlen(httpNotFoundHeader));
	conn->cgi=NULL; //mark for destruction
}

static void ICACHE_FLASH_ATTR httpdParseHeader(char *h, HttpdConnData *conn) {
	int i;
//	os_printf("Got header %s\n", h);
	if (os_strncmp(h, "GET ", 4)==0 || os_strncmp(h, "POST ", 5)==0) {
		char *e;
		//Skip past the space after POST/GET
		i=0;
		while (h[i]!=' ') i++;
		conn->url=h+i+1;
		//Figure out end of url.
		e=(char*)os_strstr(conn->url, " ");
		if (e==NULL) return; //wtf?
		*e=0; //terminate url part
		os_printf("-%s-%s URL = %s\r\n", __FILE__, __func__, conn->url);		
		conn->getArgs=(char*)os_strstr(conn->url, "?");
		if (conn->getArgs!=0) {
			*conn->getArgs=0;
			conn->getArgs++;
			os_printf("-%s-%s GET args = %s\r\n", __FILE__, __func__, conn->getArgs);		
		} else {
			conn->getArgs=NULL;
		}
	} else if (os_strncmp(h, "Content-Length: ", 16)==0) {
		i=0;
		while (h[i]!=' ') i++;
		conn->postLen=atoi(h+i+1);
		if (conn->postLen>MAX_POST) conn->postLen=MAX_POST;
		os_printf("-%s-%s Mallocced buffer for %d bytes of post data.\r\n", __FILE__, __func__, conn->postLen);		
		conn->postBuff=(char*)os_malloc(conn->postLen+1);
		conn->priv->postPos=0;
	}
}

static void ICACHE_FLASH_ATTR httpdRecvCb(void *arg, char *data, unsigned short len) {
	int x;
	char *p, *e;
	HttpdConnData *conn=httpdFindConnData(arg);
	if (conn==NULL) return;

	for (x=0; x<len; x++) {
		if (conn->priv->headPos!=-1) {
			//This byte is a header byte.
			if (conn->priv->headPos!=MAX_HEAD_LEN) conn->priv->head[conn->priv->headPos++]=data[x];
			conn->priv->head[conn->priv->headPos]=0;
			//Scan for /r/n/r/n
			if (data[x]=='\n' && (char *)os_strstr(conn->priv->head, "\r\n\r\n")!=NULL) {
				//Reset url data
				conn->url=NULL;
				//Find end of next header line
				p=conn->priv->head;
				while(p<(&conn->priv->head[conn->priv->headPos-4])) {
					e=(char *)os_strstr(p, "\r\n");
					if (e==NULL) break;
					e[0]=0;
					httpdParseHeader(p, conn);
					p=e+2;
				}
				//If we don't need to receive post data, we can send the response now.
				if (conn->postLen==0) {
					httpdSendResp(conn);
				}
				conn->priv->headPos=-1; //Indicate we're done with the headers.
			}
		} else if (conn->priv->postPos!=-1 && conn->postLen!=0 && conn->priv->postPos <= conn->postLen) {
			//This byte is a POST byte.
			conn->postBuff[conn->priv->postPos++]=data[x];
			if (conn->priv->postPos>=conn->postLen) {
				//Received post stuff.
				conn->postBuff[conn->priv->postPos]=0; //zero-terminate
				conn->priv->postPos=-1;
				os_printf("-%s-%s Post args = %s\r\n", __FILE__, __func__, conn->postBuff);		
				//Send the response.
				httpdSendResp(conn);
				return;
			}
		}
	}
}

static void ICACHE_FLASH_ATTR httpdReconCb(void *arg, sint8 err) {
	HttpdConnData *conn=httpdFindConnData(arg);
	os_printf("-%s-%s \r\n", __FILE__, __func__);		
	if (conn==NULL) return;
}

static void ICACHE_FLASH_ATTR httpdDisconCb(void *arg) {
	int i;
	for (i=0; i<MAX_CONN; i++) {
		if (connData[i].conn!=NULL) {
			if (connData[i].conn->state==ESPCONN_NONE || connData[i].conn->state==ESPCONN_CLOSE) {
				connData[i].conn=NULL;
				if (connData[i].cgi!=NULL) connData[i].cgi(&connData[i]); //flush cgi data
				httpdRetireConn(&connData[i]);
			}
		}
	}
}


static void ICACHE_FLASH_ATTR httpdConnectCb(void *arg) {
	struct espconn *conn=arg;
	int i;
	//Find empty conndata in pool
	for (i=0; i<MAX_CONN; i++) if (connData[i].conn==NULL) break;
	os_printf("espconn %s-%s Con req, conn=%p, pool slot %d\r\n", __FILE__, __func__, conn, i);		
	connData[i].priv=&connPrivData[i];
	if (i==MAX_CONN) {
		os_printf("-%s-%s Conn pool overflow\r\n", __FILE__, __func__);
		espconn_disconnect(conn);
		return;
	}
	connData[i].conn=conn;
	connData[i].priv->headPos=0;
	connData[i].postBuff=NULL;
	connData[i].priv->postPos=0;
	connData[i].postLen=0;

	espconn_regist_recvcb(conn, httpdRecvCb);
	espconn_regist_reconcb(conn, httpdReconCb);
	espconn_regist_disconcb(conn, httpdDisconCb);
	espconn_regist_sentcb(conn, httpdSentCb);
}


void ICACHE_FLASH_ATTR httpdInit(HttpdBuiltInUrl *fixedUrls, int port) {
	int i;

	for (i=0; i<MAX_CONN; i++) {
		connData[i].conn=NULL;
	}
	httpdConn.type=ESPCONN_TCP;
	httpdConn.state=ESPCONN_NONE;
	httpdTcp.local_port=port;
	httpdConn.proto.tcp=&httpdTcp;
	builtInUrls=fixedUrls;
	os_printf("-%s-%s \r\n", __FILE__, __func__);
	espconn_regist_connectcb(&httpdConn, httpdConnectCb);
	espconn_accept(&httpdConn);
}
