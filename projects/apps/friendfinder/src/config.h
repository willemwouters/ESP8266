/*
 * config.h
 *
 *  Created on: Mar 14, 2015
 *      Author: wouters
 */

#ifndef CONFIG_H_
#define CONFIG_H_

struct scan_result {
	char * ssid;
	int strenght;
};
#define AP_PREFIX "SMC"
#define MIN_STRENGTH -70
#define TICK_SPEED 5000

#define LOG_LEVEL 0

#endif /* CONFIG_H_ */
