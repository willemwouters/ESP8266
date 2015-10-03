/*
 * UdpServer.cpp
 *
 *  Created on: Oct 3, 2015
 *      Author: wouters
 */

#include "UdpServer.h"
#include "Log/Log.h"

extern "C" {
	#include "espressif/esp_common.h"
	#include "lwip/sockets.h"
	#include "lwip/init.h"
}


UdpServer::UdpServer(): Thread(&UdpServer::run) {
	// TODO Auto-generated constructor stub
	lwip_init();
}
char buffer[1024];

void UdpServer::run(void*)
{
		int lSocket;
		struct sockaddr_in sLocalAddr, sDestAddr;
		Log::i("Going to start");

		int nbytes;
		int i;
		lSocket = lwip_socket(AF_INET, SOCK_DGRAM, 0);
		if (lSocket != 0) {
			printf("ERROR \r\n");
		}

		memset((char *) &sLocalAddr, 0, sizeof(sLocalAddr));
		memset((char *) &sDestAddr, 0, sizeof(sDestAddr));

		/*Destination*/
		sDestAddr.sin_family = AF_INET;
		sDestAddr.sin_len = sizeof(sDestAddr);
		sDestAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
		sDestAddr.sin_port = htons(1235);

		/*Source*/
		sLocalAddr.sin_family = AF_INET;
		sLocalAddr.sin_len = sizeof(sLocalAddr);
		sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("192.168.4.1");
		sLocalAddr.sin_port = htons(1235);

		int err = lwip_bind(lSocket, (struct sockaddr *) &sLocalAddr,
				sizeof(sLocalAddr));
		if (err != 0) {
			printf("ERROR \r\n");
		}
		while (1) {
			nbytes = lwip_recv(lSocket, buffer, sizeof(buffer), 8);
			if (nbytes > 0) {
				Log::i((char*) 3, __FUNCTION__, "Received:", buffer);
				//lwip_sendto(lSocket, buffer, nbytes, 0, (struct sockaddr *) &sDestAddr, sizeof(sDestAddr));
			} else {
				Log::i((char*) 2, __FUNCTION__, "Timedout");
			}
			Thread::sleep(200);
		}
		lwip_close(lSocket);

}
UdpServer::~UdpServer() {
	// TODO Auto-generated destructor stub
}

