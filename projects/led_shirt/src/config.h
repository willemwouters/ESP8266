/*
 * config.h
 *
 *  Created on: Mar 14, 2015
 *      Author: wouters
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define MODE_CONNECTING 0
#define MODE_SLAVE 1
#define MODE_MASTER_S 2
#define MODE_MASTER_C 3
#define CLIENT_AP "CLIENT"
#define MASTER_AP "WILLEM"
#define MASTER_PASSWORD "00000000"

#define TICK_SPEED_CONNECTING 5000
#define TICK_SPEED_MASTER 500
#define TICK_SPEED_SLAVE 500

int config_mode;

#endif /* CONFIG_H_ */
