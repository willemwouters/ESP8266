#include "espmissingincludes.h"
#include "connection_list.h"
#include "lwip/mem.h"

static connections * connection_list[HTTP_POOL] = { 0 };

connections* findWebsocketConnection() {
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->websocket == 1) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}

void deleteConnection(struct tcp_pcb* pcb) {
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			os_printf("deleteconn %p \r\n", connection_list[i]);
			os_free(connection_list[i]);
			connection_list[i] = 0;
		}
	}
}
connections* newConnection(struct tcp_pcb* pcb) {
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] == 0) {
			os_printf("newConnection %d - %p - %d \r\n", i, connection_list[i], sizeof(connections));
			connection_list[i] = os_malloc(sizeof(connections));
			os_printf("malloc done \r\n");
			connection_list[i]->websocket = 0;
			connection_list[i]->timeout = 0;
			connection_list[i]->dataleft = 0;
			connection_list[i]->filepos = 0;
			connection_list[i]->connection = pcb;
			os_printf("newConnection done \r\n");
			return connection_list[i];
		}
	}
	return 0;
}

connections* getConnectionOffset(struct tcp_pcb * pcb, int off) {
	//os_printf("find connection: %p \r\n", pcb);
	for (int i = off + 1; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}

connections** getConnectionsBegin() {
	return connection_list;
}

connections* getConnection(struct tcp_pcb * pcb) {
	//os_printf("find connection: %p \r\n", pcb);
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}

