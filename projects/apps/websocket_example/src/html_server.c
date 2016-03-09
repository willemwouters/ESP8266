#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "espmissingincludes.h"
#include "lwip_websocket.h"
#include "connection_list.h"
#include "html_server.h"
#include "html_dataparser.h"

static websocket_gotdata data_callback;

#define HTTP_BUFFER_SIZE (1000)

void server_init(websocket_gotdata call) {
	struct tcp_pcb *pcb;
	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 8000); //server port for incoming connection
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, server_accept);
	data_callback = call;
	websocket_init(call);
}

static void server_close(struct tcp_pcb *pcb) {

	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, NULL);
	int i = tcp_close(pcb);
	os_printf("\nserver_close(): Closing...%d \n", i);
	deleteConnection(pcb);
}

static err_t server_accept(void *arg, struct tcp_pcb *pcb, err_t err) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	tcp_setprio(pcb, TCP_PRIO_MIN);
	tcp_arg(pcb, NULL);
	tcp_recv(pcb, server_recv);
	tcp_err(pcb, server_err);
	tcp_sent(pcb, server_sent);
	tcp_poll(pcb, server_poll, 2); //every two seconds of inactivity of the TCP connection
	tcp_accepted(pcb);
	os_printf("\nserver_accept(): Accepting incoming connection on server...\n");
	return ERR_OK;
}


static err_t server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
	connections * con = getConnection(pcb);
	if (con != 0 && con->dataleft > 0) {
		int flag = TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY;
		long filesize = con->dataleft;
		if (con->dataleft > HTTP_BUFFER_SIZE) {
			filesize = HTTP_BUFFER_SIZE;
		} else {
			flag = TCP_WRITE_FLAG_COPY;
		}
		char * d = os_malloc(HTTP_BUFFER_SIZE + 1);
		os_memcpy(d, con->data + con->filepos, filesize);
		d[filesize] = 0;
		os_printf("DATA: from: %d to %d flag %d  size %d - left:%d -   %s\r\n", con->filepos, con->filepos + filesize, flag, filesize, con->dataleft, d);
		int i = tcp_write(pcb, d, filesize, flag);
		tcp_output(pcb);
		os_printf("ERROR CODE: %d \r\n", i);
		if (i == 0) {
			con->filepos += filesize;
			con->dataleft -= filesize;
		}
		if (con->dataleft <= 0) {
			con->dataleft = 0;
			con->filepos = 0;
			if(con->websocket == 0) {
				server_close(pcb);
			}
		}
		os_free(d);
	} else {
		if (con != 0 && con->websocket == 0) {
			server_close(pcb);
		}
	}

	return 0;
}



static err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	char *string;
	int length;
	LWIP_UNUSED_ARG(arg);
	os_printf("pcb: %p \r\n", pcb);

	connections* con = getConnection(pcb);
	if (con == 0) {
		os_printf("create connection: %p \r\n", pcb);
		con = newConnection(pcb);
		if (con == 0) {
			os_printf("ERROR creating new conn \r\n");
			return ERR_MEM;
		}
	} else {
		con->timeout = 0;
	}

	if (err == ERR_OK && p != NULL) {
		pcb->flags |= TF_NODELAY;  //TF_NAGLEMEMERR
		string = p->payload;
		length = strlen(string);
		os_printf("string:%s \r\n", string);
		tcp_recved(pcb, p->tot_len); // if frame is big

		char url[50] = { 0 };
		if (strstr(string, "GET /") != 0) {
			char *begin = strstr(string, "GET /") + 4;
			char *end = strstr(begin, " ");
			os_memcpy(url, begin, end - begin);
			url[end - begin] = 0;
		}
		if (strstr(string, HEADER_WEBSOCKETLINE) != 0) {
			con->websocket = 1;
		}

		if(con->websocket == 1) {
			websocket_recv(string,url, con, pcb, p);
			pbuf_free(p);
		} else {
			os_printf("NORMAL MESSAGE \r\n");
			int err = send_chunk(pcb, 0, 0);
			tcp_output(con->connection);
			pbuf_free(p);
			if (err == 0) {
				server_close(pcb);
			}
		}
	} else {
		os_printf("\nserver_recv(): Errors-> ");
		if (err != ERR_OK)
			os_printf("1) Connection is not on ERR_OK state, but in %d state->\n", err);

		if (p == NULL) {
			os_printf("2) Pbuf pointer p is a NULL pointer->\n ");
			os_printf("2) Remote computer closed connection \n ");
			con->websocket = 0;
		} else {
			pbuf_free(p);
		}

		if (con->websocket != 1) {
			os_printf("server_recv(): Closing server-side connection...");
			server_close(pcb);
		}
		con->websocket = 0;
	}

	return ERR_OK;
}

static err_t server_poll(void *arg, struct tcp_pcb *pcb) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(pcb);
	connections * con;
	connections ** connection_list = getConnectionsBegin();
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0) {
			con = connection_list[i];
			con->timeout++;
			os_printf("\nserver_poll(): Call number %d %p\n", con->timeout, pcb);
			if (con->timeout > 10) {
				if (con->websocket == 1 && con->timeout > 20) {
					server_close(pcb);
				} else if (con->websocket == 0) {
					server_close(pcb);
				}
			}
		}
	}

	return ERR_OK;
}

static void server_err(void *arg, err_t err) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	os_printf("\nserver_err(): Fatal error, exiting...\n");
	server_init(data_callback); // TODO
	return;
}

