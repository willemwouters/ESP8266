/*
 * reg_actions.h
 *
 *  Created on: Mar 25, 2015
 *      Author: wouters
 */

#ifndef REG_ACTIONS_H_
#define REG_ACTIONS_H_
#include "reg_handler.h"


enum REG_ACTION reg_getaction(char * action);

void reg_readaction(struct reg_str *  reg);
void reg_writeaction(struct reg_str *  reg);

#endif /* REG_ACTIONS_H_ */
