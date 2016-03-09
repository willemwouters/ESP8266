#ifndef __CONNECTION_LIST_H
#define __CONNECTION_LIST_H

#define HTTP_POOL 20

typedef struct {
	struct tcp_pcb * connection;
	int timeout;
	const char * data;
	int dataleft;
	int filepos;
	int websocket;
	int pos;
} connections;

connections* findWebsocketConnection();
void deleteConnection(struct tcp_pcb* pcb);
connections* newConnection(struct tcp_pcb* pcb);
connections* getConnectionOffset(struct tcp_pcb * pcb, int off);
connections* getConnection(struct tcp_pcb * pcb);
connections** getConnectionsBegin();
#endif
