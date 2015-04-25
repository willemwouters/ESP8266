/*
 * reg_handler.c
 *
 *  Created on: Mar 19, 2015
 *      Author: wouters
 */

#include "user_interface.h"

#include "ets_sys.h"
#include "osapi.h"
#include "reg_handler.h"
#include "mem.h"
#include "queue.h"
#include "espmissingincludes.h"
typedef void (*callback_type)(struct reg_str *);

static queue read_queue;


int check_error(char * dat) {
	if(dat == NULL) {
		return -1;
	}
	return 0;
}
struct reg_str * reg_alloc() {
	struct reg_str * p = (struct reg_str *) os_malloc(sizeof(struct reg_str));
	p->action = UNKNOWN;
	p->destination = (char*) os_malloc(10);
	p->val = (char*) os_malloc(10);
	return p;
}
struct reg_callback * reg_callback_malloc() {
	struct reg_callback * p = (struct reg_callback *) os_malloc(sizeof(struct reg_str));
		p->callback = NULL;
		p->reg = 0x00;
		return p;
}
void reg_addlistener(void* callback, char reg) {
	struct reg_callback * r = reg_callback_malloc();
	r->callback = callback;
	r->reg = reg;
	enqueue(&read_queue, r);
}

void reg_tochar(struct reg_str * reg, char * data) {
	int counter = 0;
	data[counter] = reg->destination[0]; counter++;
	data[counter] = reg->destination[1]; counter++;
	data[counter] = ':'; counter++;
	if(reg->action == WRITE) {
		data[counter] = 'W'; counter++;
		data[counter] = 'R'; counter++;
		data[counter] = 'I'; counter++;
		data[counter] = 'T'; counter++;
		data[counter] = 'E'; counter++;
	}
	data[counter] = ':'; counter++;
	data[counter] = reg->reg; counter++;
	data[counter] = ':'; counter++;
	data[counter] = reg->val[0]; counter++;
	data[counter] = reg->val[1]; counter++;
	data[counter] = reg->val[2]; counter++;
}
void reg_init() {
	init_queue(&read_queue);
}
void reg_free(struct reg_str * p) {
	os_free(p->destination);
	os_free(p->val);
	os_free(p);
}

void reg_callback_free(struct reg_callback * c) {
	os_free(c);
}

void reg_handler(struct reg_str * regaction) {
	int i = 0;
	//led_regcallback(regaction);
	//callback_master(regaction);
	for(i = 0; i < read_queue.count; i++) {
		struct reg_callback* rc = read_queue.q[i];
		if(rc->reg == regaction->reg || rc->reg > 0x20) {
			((callback_type) rc->callback)(regaction);
		}
	}
}

void network_request(char * data,  uint32 ip) {
// AP:READ:0x02:0xFFFF
	struct reg_str * reg_action;
	int i;
	char dest[10];
	char action[10];
	char reg;
	char val[10];
	char * pch;
	char * ech;
	if(os_strlen(data) > 10) {
		os_memcpy(dest, data, 2);
		dest[2] = 0;
		pch=strchr(data,':');
		if(pch == NULL) return;
		ech=strchr(pch+1,':');
		if(ech == NULL) return;
		i = (ech-data+1) - (pch-data+1) - 1;
		os_memcpy(action, pch+1,  i);
		action[i] = 0;
		pch=strchr(pch+1,':');
		if(pch == NULL) return;
		os_memcpy(&reg, pch+1, 1);
		pch=strchr(pch+1,':');
		if(pch == NULL) return;
		os_memcpy(val, pch+1, 4);
		val[4] = 0;
		reg_action = reg_alloc();
		memcpy(reg_action->destination, dest, os_strlen(dest));
		reg_action->destination[os_strlen(dest)] = 0;
		reg_action->action = reg_getaction(action);
		reg_action->reg = reg;
		reg_action->addr = ip;
		memcpy(reg_action->val, val, 4);
		//os_printf("We got a valid network register request: %s:%d:%x:"BYTETOBINARYPATTERN" "BYTETOBINARYPATTERN"\r\n", reg_action->destination, reg_action->action, reg_action->reg, BYTETOBINARY(reg_action->val[0]), BYTETOBINARY(reg_action->val[1]));
		reg_handler(reg_action);
		reg_free(reg_action);
	}

}

