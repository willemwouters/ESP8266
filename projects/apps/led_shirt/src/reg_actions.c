/*
 * reg_actions.c
 *
 *  Created on: Mar 25, 2015
 *      Author: wouters
 */
#include "reg_actions.h"
#include "reg_handler.h"
#include "espmissingincludes.h"
#include "osapi.h"
#include "c_types.h"

uint32_t buffer[0x20];

void reset_register() {
	int i;
	for(i = 0; i < 0x20; i++) {
		buffer[i] = 0xFFFFFFFF;
	}
}
enum REG_ACTION reg_getaction(char * action) {
	if(os_strcmp(action, "READ") == 0) {
		return READ;
	} else if(os_strcmp(action, "WRITE") == 0) {
		return WRITE;
	}
	return UNKNOWN;
}
void reg_writeaction(struct reg_str *  reg) {
	struct reg_str * reg_action;
	if(reg->reg >= 0x20) {
		return;
	}
	os_memcpy(buffer + reg->reg, reg->val, 4);
	reg_action = (struct reg_str *) reg_alloc();
	reg_action->action = WRITEOUT;
	reg_action->reg = reg->reg;
	os_memcpy(reg_action->val, buffer + reg->reg, 4);
	reg_handler(reg_action);
	reg_free(reg_action);

}
void reg_readaction(struct reg_str *  reg) {
	struct reg_str * reg_action;
	if(reg->reg >= 0x20) {
		return;
	}
	reg_action = (struct reg_str *) reg_alloc();
	reg_action->action = READOUT;
	reg_action->reg = reg->reg;
	os_memcpy(reg_action->val, buffer + reg->reg, 4);
	reg_handler(reg_action);
	reg_free(reg_action);
}
