/*
 * reg_handler.h
 *
 *  Created on: Mar 19, 2015
 *      Author: wouters
 */

#ifndef REG_HANDLER_H_
#define REG_HANDLER_H_
#include "c_types.h"

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0)


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
	uint32 addr;
	char *val;
};

struct reg_callback {
	void * callback;
	char reg;
};

void reg_tochar(struct reg_str * reg, char * data);
void network_request(char * data, uint32 ip);


#endif /* REG_HANDLER_H_ */
