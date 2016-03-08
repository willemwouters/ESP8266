#ifndef __ESP_WEBSOCKET_H
#define __ESP_WEBSOCKET_H

int writeToWebsocket(char * data);
void my_server_init(void);

static const char *header_key = "Sec-WebSocket-Key: ";
static const char  *ws_uuid ="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

typedef struct {
	struct tcp_pcb * connection;
	int timeout;
	char * data;
	int dataleft;
	int filepos;
	int websocket;
	int websocket_done;
	int pos;
} connections;

static char * HEADER_OK = "HTTP/1.x 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: max-age=3600, public \r\n\
Content-Type: text/html \r\n\
Content-Encoding: gzip \r\n\r\n";

static char * HEADER_WEBSOCKET = "\
HTTP/1.1 101 WebSocket Protocol Handshake\r\n\
Connection: Upgrade\r\n\
Upgrade: WebSocket\r\n\
Access-Control-Allow-Origin: http://192.168.179.14\r\n\
Access-Control-Allow-Credentials: true\r\n\
Access-Control-Allow-Headers: content-type \r\n\
Sec-WebSocket-Accept: ";


#endif
