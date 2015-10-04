/*
 * UdpServer.h
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#ifndef SRC_UDPSERVER_H_
#define SRC_UDPSERVER_H_
#include "Thread/Thread.h"

class UdpServer : public Thread{
public:
	UdpServer();
	virtual ~UdpServer();
	static void run(void * p);
};

#endif /* SRC_UDPSERVER_H_ */
