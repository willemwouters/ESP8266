/*
 * reg_handler.h
 *
 *  Created on: Mar 19, 2015
 *      Author: wouters
 */

#ifndef REG_HANDLER_H_
#define REG_HANDLER_H_


enum REG_ACTION {
	READ,
	WRITE,
	READOUT,
	WRITEOUT,
	UNKNOWN
};

struct reg_str {
	char * destination;
	enum REG_ACTION action;
	char reg;
	char *val;
};

struct reg_callback {
	void * callback;
	char reg;
};

void reg_tochar(struct reg_str * reg, char * data);
void network_request(char * data);


#endif /* REG_HANDLER_H_ */
