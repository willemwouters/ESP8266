/*
 * WifiHelper.h
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#ifndef SRC_WIFIHELPER_H_
#define SRC_WIFIHELPER_H_


class WifiHelper {
public:
	WifiHelper();
	static void SetAp(char * ap);
	virtual ~WifiHelper();
};

#endif /* SRC_WIFIHELPER_H_ */
