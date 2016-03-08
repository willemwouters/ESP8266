#ifndef __ESP_WEBSOCKET_H
#define __ESP_WEBSOCKET_H
#include "connection_list.h"
#include "html_server.h"
#include "lwip/err.h"

void websocket_init(websocket_gotdata call);
int ICACHE_FLASH_ATTR websocket_writedata(char * data);
void ICACHE_FLASH_ATTR websocket_recv(char * string,char*url, connections* con, struct tcp_pcb *pcb, struct pbuf *p);
void ICACHE_FLASH_ATTR websocket_parse(char * data, size_t dataLen, struct tcp_pcb *pcb);



static const char *header_key = "Sec-WebSocket-Key: ";
static const char  *ws_uuid ="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";


#endif
