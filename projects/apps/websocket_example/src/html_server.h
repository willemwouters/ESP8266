#ifndef __HTML_SERVER_H
#define __HTML_SERVER_H

#include "lwip/err.h"

void server_init(void);

static err_t server_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t server_poll(void *arg, struct tcp_pcb *pcb);
static void server_err(void *arg, err_t err);
static err_t server_sent(void *arg, struct tcp_pcb *pcb, u16_t len);

#endif
