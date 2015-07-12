//UDPClient.c
 
/*
 * gcc -o client UDPClient.c
 * ./client <server-ip>
 */
 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h> 
#include <string.h>
#define BUFLEN 512
#define PORT 8000
 
void err(char *s)
{
    perror(s);
    exit(1);
}

 struct sockaddr_in serv_addr;
 struct sockaddr_in my_addr, cli_addr;

 char bufrv[BUFLEN];
int slen=sizeof(serv_addr);

void sender(int sockfd, char * buf) {

int go = 0;
  while(go == 0) {
	if (sendto(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&serv_addr, slen)==-1)
            err("sendto()");

  for(int i = 0; i < 4; i++) {
	if (recvfrom(sockfd, bufrv, BUFLEN, 0, (struct sockaddr*)&cli_addr, (socklen_t*) &slen)==-1)
        {   printf("Nothing received \r\n"); 
	
	} else {
		printf("Received packet from %s:%d\nData: %s\n\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), bufrv);

	if(strstr(bufrv, "ACKWRITE") != NULL) {
		go = 1;
		i = 10;
	}
	}


        
  }
  }
}
 
int main(int argc, char** argv)
{
   
    int sockfd, i;

    char buf[BUFLEN];
   
 char bufr[BUFLEN];
 char bufg[BUFLEN];
 char bufb[BUFLEN];
struct timeval tv;
tv.tv_sec  = 0;
tv.tv_usec = 20000;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        err("socket");

if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
 err("sockopt");
}

bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(8000);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     
 if (bind(sockfd, (struct sockaddr* ) &my_addr, sizeof(my_addr))==-1)
      err("bind");
    else
      printf("Server : bind() successful\n");

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_aton("224.0.0.1", &serv_addr.sin_addr)==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
 
    while(1)
    {

	


        usleep(100000);
	
	buf[0] = 'A';
	buf[1] = 'P';
	buf[2] = ':';
	buf[3] = 'W';
	buf[4] = 'R';
	buf[5] = 'I';
	buf[6] = 'T';
	buf[7] = 'E';
	buf[8] = ':';
	buf[9] = 0x01;
	buf[10] = ':';

	printf("\nRed: ");
        scanf("%[^\n]",bufr);
        getchar();
       printf("Green: ");
        scanf("%[^\n]",bufg);
        getchar();
       printf("Blue: ");
        scanf("%[^\n]",bufb);
        getchar();
       
	
 	printf("Sending \r\n");
	buf[11] = atoi(bufr);
	buf[12] = atoi(bufg);
	buf[13] = atoi(bufb);
	sender(sockfd, buf);

buf[9] = 0x02;
 printf("Speed: ");
        scanf("%[^\n]",bufb);
        getchar();

buf[11] = atoi(bufb);
buf[12] = 0;
buf[13] = 0;
	sender(sockfd, buf);

buf[9] = 0x03;
 printf("Brightness: ");
        scanf("%[^\n]",bufb);
        getchar();

buf[11] = atoi(bufb);;
buf[12] = 0;
buf[13] = 0;
	sender(sockfd, buf);


/*
usleep(100000);
	
	BUF[0] = 'A';
	BUF[1] = 'P';
	BUF[2] = ':';
	BUF[3] = 'W';
	BUF[4] = 'R';
	BUF[5] = 'I';
	BUF[6] = 'T';
	BUF[7] = 'E';
	BUF[8] = ':';
	BUF[9] = 0x01;
	BUF[10] = ':';
	BUF[11] = 255;
	BUF[12] = 0;

        if (sendto(sockfd, BUF, BUFLEN, 0, (struct sockaddr*)&serv_addr, slen)==-1)
            err("sendto()");
*/

	
    }
 
    close(sockfd);
    return 0;
}
