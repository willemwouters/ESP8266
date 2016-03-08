
#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"
#include "lwip/pbuf.h"
#include "ssl/ssl_crypto.h"
#include "lwip/tcp.h"

#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"

#include "lwip/tcp.h"
#include "lwip_websocket.h"
#include "rofs.h"
#include "gpio.h"

#include "driver/pwm.h"

extern const uint8_t rofs_data[];
extern const RO_FS ro_file_system;
static err_t server_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t server_poll(void *arg, struct tcp_pcb *pcb);
static err_t server_err(void *arg, err_t err);
static err_t server_sent(void *arg, struct tcp_pcb *pcb, u16_t len);
#define HTTP_BUFFER_SIZE (1000)

connections * connection_list[10] = { 0 };

connections* findWebsocketConnection() {
	for (int i = 0; i < 10; i++) {
		if (connection_list[i] != 0 && connection_list[i]->websocket == 1) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}

void deleteConnection(struct tcp_pcb* pcb) {
	for (int i = 0; i < 10; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			os_printf("deleteconn %p \r\n", connection_list[i]);
			os_free(connection_list[i]);
			connection_list[i] = 0;
		}
	}
}
connections* newConnection(struct tcp_pcb* pcb) {
	for (int i = 0; i < 10; i++) {
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
	for (int i = off + 1; i < 10; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}

connections* getConnection(struct tcp_pcb * pcb) {
	//os_printf("find connection: %p \r\n", pcb);
	for (int i = 0; i < 10; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}

void my_server_init(void) {
	struct tcp_pcb *pcb;

	uint8_t PWM_CH[]= {0, 0, 0};   // PIN CONFIG IS IN PWM.h
	uint16_t freq = 100;
	pwm_init(freq, PWM_CH);
	pwm_start();


	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 8000); //server port for incoming connection
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, server_accept);
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
	tcp_poll(pcb, server_poll, 8); //every two seconds of inactivity of the TCP connection
	tcp_accepted(pcb);
	os_printf("\nserver_accept(): Accepting incoming connection on server...\n");
	return ERR_OK;
}

void ICACHE_FLASH_ATTR websocket_write(void *arg, struct tcp_pcb *pcb) {
	uint8_t byte;
	int fsize = os_strlen(arg) + 2;
	char * buff = os_malloc(fsize);
	byte = 0x80; //set first bit
	byte |= 0x01; //frame->TYPE; //set op code
	buff[0] = byte;
	byte = 0;
	int SIZE = os_strlen(arg);
	if (SIZE < 126) {
		byte = os_strlen(arg);

		buff[1] = byte;

	} else {

		os_printf("Too much data \r\n");
	}

	os_memcpy(&buff[2], arg, byte);
	tcp_write(pcb, buff, fsize, TCP_WRITE_FLAG_MORE);
	os_free(buff);
}

void ICACHE_FLASH_ATTR websocket_parse(char * data, size_t dataLen, struct tcp_pcb *pcb) {
	uint8_t byte = data[0];
	int FIN = byte & 0x80;
	int TYPE = byte & 0x0F;
	os_printf("frame type %02X %02X \r\n", TYPE, FIN);
	os_printf("%02X %02X %02X %02X \r\n", data[0], data[1], data[2], data[3]);
	if ((TYPE > 0x03 && TYPE < 0x08) || TYPE > 0x0B) {
		os_printf("Invalid frame type %02X \r\n", TYPE);
		return;
	}

	byte = data[1];
	int MASKED = byte & 0x80;
	int SIZE = byte & 0x7F;

	int offset = 2;
	if (SIZE == 126) {
		SIZE = 0;
		SIZE = data[3];                 //LSB
		SIZE |= (uint64_t) data[2] << 8; //MSB
		offset = 4;
	} else if (SIZE == 127) {
		SIZE = 0;
		SIZE |= (uint64_t) data[2] << 56;
		SIZE |= (uint64_t) data[3] << 48;
		SIZE |= (uint64_t) data[4] << 40;
		SIZE |= (uint64_t) data[5] << 32;
		SIZE |= (uint64_t) data[6] << 24;
		SIZE |= (uint64_t) data[7] << 16;
		SIZE |= (uint64_t) data[8] << 8;
		SIZE |= (uint64_t) data[9];
		offset = 10;
	}

	if (MASKED) {
		//read mask key
		char mask[4];
		mask[0] = data[offset];
		mask[1] = data[offset + 1];
		mask[2] = data[offset + 2];
		mask[3] = data[offset + 3];
		offset += 4;
		uint64_t i;
		for (i = 0; i < SIZE; i++) {
			data[i + offset] ^= mask[i % 4];
		}
		char * DATA = &data[offset];
		DATA[SIZE] = 0;
		os_printf("SIZE: %d  tSIZE: %d, DATA: =%s=  \r\n", SIZE, dataLen, DATA);



		if(strstr(DATA, "R:") != 0) {
			char dat[3] = { 0 };
			os_memcpy(dat, &DATA[2], SIZE);
			os_printf("%s\r\n", dat);
			int i = atoi(dat);
			os_printf("%d\r\n", i);
			pwm_set_duty(i, 1);
			pwm_start();
		} else if(strstr(DATA, "G:") != 0) {
			char dat[3] = { 0 };
			os_memcpy(dat, &DATA[2], SIZE);
			os_printf("%s\r\n", dat);
			int i = atoi(dat);
			os_printf("%d\r\n", i);
			pwm_set_duty(i, 0);
			pwm_start();
		} else if(strstr(DATA, "B:") != 0) {
			char dat[3] = { 0 };
			os_memcpy(dat, &DATA[2], SIZE);
			os_printf("%s\r\n", dat);
			int i = atoi(dat);
			os_printf("%d\r\n", i);
			pwm_set_duty(i, 2);
			pwm_start();
		}


		if (SIZE + offset < dataLen) {
			websocket_parse(&data[SIZE + offset], dataLen - (SIZE + offset), pcb);
		}

	}

}

static err_t server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
	//os_printf("server_sent \r\n");
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

static int send_chunk(struct tcp_pcb * pcb, char * data, int len) {
	int file = 0;
	connections * con = getConnection(pcb);
	if (con != 0) {
		con->filepos = 0;
		long filesize = 0;
		con->dataleft = ro_file_system.files[file].size;
		;
		int flag = TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY;
		char * d = os_malloc(os_strlen(HEADER_OK) + HTTP_BUFFER_SIZE + 1);
		tcp_output(pcb);
		con->data = &rofs_data[ro_file_system.files[file].offset];
		os_memcpy(d, HEADER_OK, os_strlen(HEADER_OK));
		tcp_write(pcb, d, os_strlen(HEADER_OK), flag);
		tcp_output(pcb);
		os_free(d);

		if (con->dataleft == 0) {
			return 0;
		}
	}
	return -1;
	//os_free(d);
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
			return;
		}
	} else {
		con->timeout = 0;
	}

	if (err == ERR_OK && p != NULL) {
		pcb->flags |= TF_NODELAY;  //TF_NAGLEMEMERR
		string = p->payload;
		length = strlen(string);
		tcp_recved(pcb, p->tot_len); // if frame is big

		char url[50] = { 0 };
		if (strstr(string, "GET /") != 0) {
			char *begin = strstr(string, "GET /") + 4;
			char *end = strstr(begin, " ");
			os_memcpy(url, begin, end - begin);
			url[end - begin] = 0;
		}

		if (strstr(url, "/echo") != 0) {
			char * key;
			if (strstr(string, header_key) != 0) {
				char * begin = strstr(string, header_key) + os_strlen(header_key);
				char * end = strstr(begin, "\r");
				key = os_malloc((end - begin) + 1);
				os_memcpy(key, begin, end - begin);
				key[end - begin] = 0;
			}
			uint8_t digest[20]; //sha1 is always 20 byte long
			SHA1_CTX ctx;
			os_printf("key:  -%s-\n", key);
			SHA1_Init(&ctx);
			SHA1_Update(&ctx, key, os_strlen(key));
			SHA1_Update(&ctx, ws_uuid, os_strlen(ws_uuid));
			SHA1_Final(digest, &ctx);
			char base64Digest[31]; //
			Base64encode(base64Digest, (const char*) digest, 20);
			int file = 0;
			char * d = os_malloc(os_strlen(HEADER_WEBSOCKET) + 36);
			os_sprintf(d, "%s%s\r\n\r\n", HEADER_WEBSOCKET, base64Digest);
			os_printf("Handshake completed \r\n");
			tcp_write(pcb, d, os_strlen(d), TCP_WRITE_FLAG_MORE);
			pbuf_free(p);
			os_free(key);
			con->websocket = 1;
		} else if (con->websocket == 1) {
			os_printf("WEBSOCKET MESSAGE \r\n");
			websocket_parse(string, os_strlen(string), pcb);
			pbuf_free(p);
			con->websocket_done = 1;
		} else {
			os_printf("NORMAL MESSAGE \r\n");
			int err = send_chunk(pcb, 0, 0);
			tcp_output(con->connection);
			pbuf_free(p);
			if (err == 0) {
				server_close(pcb);
			}
			con->websocket = 0;
		}
	} else {
		os_printf("\nserver_recv(): Errors-> ");
		if (err != ERR_OK)
			os_printf("1) Connection is not on ERR_OK state, but in %d state->\n", err);
		pbuf_free(p);
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

	for (int i = 0; i < 10; i++) {
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

static err_t server_err(void *arg, err_t err) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	os_printf("\nserver_err(): Fatal error, exiting...\n");
	my_server_init();
	return ERR_OK;
}

int writeToWebsocket(char * data) {
	connections * con = findWebsocketConnection();
	if (con != 0 && con->connection != 0 && con->websocket_done == 1) {
		websocket_write(data, con->connection);
		int ret = tcp_output(con->connection);
		if (ret == 0)
			con->timeout = 0;

		return ret;
	}
	return -1;
}
