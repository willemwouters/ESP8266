#include "rofs.h"
#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"
#include "lwip/app/espconn.h"
#include "lwip/pbuf.h"
#include "ssl/ssl_crypto.h"
#include "lwip/tcp.h"




#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"

#include "lwip/tcp.h"
extern const  uint8_t rofs_data[];
extern const RO_FS ro_file_system;
static  esp_tcp server_tcp;
static struct espconn c;

static const char *header_key = "Sec-WebSocket-Key: ";
static const char  *ws_uuid ="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

enum ws_frame_type{

    WS_CONTINUATION=0x00,
    WS_TEXT=0x01,
    WS_BINARY=0x02,
    WS_PING=0x09,
    WS_PONG=0x0A,
    WS_CLOSE=0x08,
    WS_INVALID=0xFF
};
void ICACHE_FLASH_ATTR websocket_write(void *arg){

    uint8_t byte;
    os_printf("SEND TEXT %s\r\n", arg);
    //fin
    int fsize = os_strlen(arg) + 2;
char * buff = os_malloc(fsize);

    byte = 0x80 ; //set first bit
    byte |= WS_TEXT; //frame->TYPE; //set op code
    buff[0] = byte;



    byte=0;
//    if(frame->SIZE < 126)
//    {
        byte = os_strlen(arg);


        buff[1] = byte;

    /*}
    else if(frame->SIZE <  0xFFFF){ //can we fit it into an uint16?
        byte=126;
        w(&byte,1,arg); //transmit extended lenght indicator

        //send lower 2 bytes of the uint64 len
        byte = 0xFF & frame->SIZE >> 8;
        w(&byte,1,arg);

        byte = 0xFF & frame->SIZE;
        w(&byte,1,arg);

    }
    else{
        byte=127;
        w(&byte,1,arg); //transmit extended lenght indicator

        //transmit the whole uint64
        w((char *)&frame->SIZE,8,arg);
    }
*/
        os_memcpy(&buff[2], arg, byte);
        espconn_sent(&c, buff, fsize);
}

void ICACHE_FLASH_ATTR ws_parse_framee(char * data,size_t dataLen){


    uint8_t byte = data[0];

    //is FIN?
    int FIN = byte & 0x80;

    //opcode
    int TYPE = byte & 0x0F;


    if( (TYPE > 0x03 && TYPE < 0x08) || TYPE > 0x0B )
    {
       os_printf("Invalid frame type %02X \r\n",TYPE);
       //TYPE=WS_INVALID;
       return;
    }

    //mask
    byte = data[1];
    int MASKED = byte & 0x80;

    //frame size
    int SIZE = byte & 0x7F;

    int offset = 2;
    if(SIZE == 126)
    {
        SIZE=0;
        SIZE = data[3];                 //LSB
        SIZE |= (uint64_t)data[2] << 8; //MSB
        offset=4;
    }
    else if(SIZE == 127){
        SIZE=0;
        SIZE |= (uint64_t)data[2] << 56;
        SIZE |= (uint64_t)data[3] << 48;
        SIZE |= (uint64_t)data[4] << 40;
        SIZE |= (uint64_t)data[5] << 32;
        SIZE |= (uint64_t)data[6] << 24;
        SIZE |= (uint64_t)data[7] << 16;
        SIZE |= (uint64_t)data[8] <<  8;
        SIZE |= (uint64_t)data[9] ;
        offset=10;
    }

    if(MASKED){

        //read mask key
        char mask[4];
        mask[0]=data[offset];
        mask[1]=data[offset+1];
        mask[2]=data[offset+2];
        mask[3]=data[offset+3];

        offset+=4;

        //unmaks data
        uint64_t i;
        for(i=0;i<SIZE;i++){
            data[i+offset] ^= mask[i % 4];
        }

    }

    char * DATA = &data[offset];
    DATA[SIZE] = 0;
    if(strstr(DATA, "test")) {
    	char * d = " test ";
    	websocket_write(d);
    }
    os_printf("SIZE: %d  tSIZE: %d, DATA: =%s=  \r\n", SIZE, dataLen, DATA);
    if(SIZE + 6 < dataLen) {
    	ws_parse_framee(&data[SIZE+offset], dataLen - (SIZE + 6));
    }
    //ws_parse_frame

}


void onwebsocket(char * dataa) {
	  os_printf("server_recv():  WEBSOCKET %d\n", 1);
char * data = "\
HTTP/1.1 101 WebSocket Protocol Handshake\r\n\
Connection: Upgrade\r\n\
Upgrade: WebSocket\r\n\
Access-Control-Allow-Origin: http://192.168.179.14\r\n\
Access-Control-Allow-Credentials: true\r\n\
Access-Control-Allow-Headers: content-type \r\n\
Sec-WebSocket-Accept: ";

	    	  	  	  	 char * key;
	    	  	  	  	 if(strstr(dataa, header_key) != 0) {
	    	  	  	  		 char * begin = strstr(dataa, header_key) + os_strlen(header_key);
	    	  	  	  		 char * end = strstr(begin, "\r");
		    	  	  	  	 key = os_malloc((end-begin) + 1);
	    	  	  	  		 os_memcpy(key, begin, end-begin );
	    	  	  	  		 key[end-begin] = 0;
	    	  	  	  	 }
	    	  	  	  	 os_printf("key: -%s- \r\n", key);
	    	  	  	    //char * key = "Auxi9Jc2JZ8g95kIdm6FNA==";
					    uint8_t digest[20]; //sha1 is always 20 byte long
					    uint8_t uuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; //sha1 is always 20 byte long
						SHA1_CTX ctx;
						 os_printf("server_recv():  %d\n", 1);

						SHA1_Init(&ctx);
						os_printf("server_recv():  %d\n", 2);
						SHA1_Update(&ctx, key,os_strlen(key));
						os_printf("server_recv():  %d\n", 22);
						SHA1_Update(&ctx,ws_uuid,os_strlen(ws_uuid));
						os_printf("server_recv():  %d\n", 3);
						SHA1_Final(digest,&ctx);
						os_printf("server_recv():  %d\n", 4);

						//rLHCkw/SKsO9GAH/ZSFhBATDKrU=\r\n\

						char base64Digest[31]; //
						Base64encode(base64Digest,(const char*)digest,20);
						os_printf("server_recv():  %d\n", 5);
						//accept the handshake
						//http_SET_HEADER(c,HTTP_UPGRADE,"WebSocket");
						//http_SET_HEADER(c,HTTP_CONNECTION,"Upgrade");
						//http_SET_HEADER(c,HTTP_WEBSOCKET_ACCEPT,base64Digest);


	    	  		  int file = 0;
	    	  		  char * d = os_malloc(os_strlen(data) + 36);
	    	  		  os_sprintf(d, "%s%s\r\n\r\n", data, base64Digest);
	    	  		   os_printf("next: %s \r\n", d);
	    	  		//  tcp_write(pcb, d, os_strlen(d), TCP_WRITE_FLAG_MORE);
	//    	  		  pbuf_free(p);
	//    	  		server_close(pcb);
	    			  espconn_send(&c, d, os_strlen(d));
}
void ondata(char * dataa) {
	char * string = dataa;
	int length = strlen(dataa);
	   char url[50] = { 0 };
	   if(strstr(string, "GET /") != 0) {
		   char *begin = strstr(string, "GET /") + 4;
		   char *end = strstr(begin, " ");
		   os_memcpy(url, begin, end - begin);
		   url[end - begin] = 0;
	   }




		os_printf("\nserver_recv(): Incoming url is %s\n", url);
		os_printf("\nserver_recv(): Incoming string is %s\n", string);



	      os_printf("\nserver_recv(): String length is %d byte\n", length);

char * data = "HTTP/1.x 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: max-age=3600, public \r\n\
Content-Type: text/html \r\n\
Content-Encoding: none \r\n\r\n";

			  int file = 0;
			  char * d = os_malloc(os_strlen(data) + ro_file_system.files[file].size);
			  os_memcpy(d, data, os_strlen(data));
			  os_printf("size: %d \r\n", ro_file_system.files[file].size);
			  os_memcpy(&d[os_strlen(data)], &rofs_data[ro_file_system.files[file].offset], ro_file_system.files[file].size);
			  os_printf("next: %s \r\n", d);
			  espconn_send(&c, d, os_strlen(d));

}




void ICACHE_FLASH_ATTR http_ws_server_init()
{

//	app_callback = received;
//	data_sent_callback=data_sent;
//	client_disconnected_callback = disconnect;
//	client_connected_callback = connect;

	//espconn_delete(&ws_config.server_conn); //just to be sure we are on square 1

	c.type = ESPCONN_TCP;
	c.state = ESPCONN_NONE;
	c.proto.tcp = &server_tcp;
	c.proto.tcp->local_port = 8000;

	//NODE_DBG("Websocket server init, conn=%p", &ws_config.server_conn);

}

typedef struct {
	struct espconn * connection;
	int websocket;
} connections;

static connections * connection_list[10] = { 0 };

static void ICACHE_FLASH_ATTR http_ws_connect_callback(void *arg) {

	struct espconn *conn=arg;

	//http_connection *c = http_new_connection(1,conn); // get a connection from the pool, signal it's an incomming connection
	//c->cgi.execute = http_ws_cgi_execute; 		  // attach our cgi dispatcher

	//attach app callback to cgi function
	//c->cgi.function=app_callback;

	//c->handshake_ok=0;

	os_printf("connect cb \r\n");
	//let's disable NAGLE alg so TCP outputs faster ( in theory )
	espconn_set_opt(conn, ESPCONN_NODELAY | ESPCONN_REUSEADDR );

	//override timeout to 240s
	espconn_regist_time(conn,240,0);

}
static void ICACHE_FLASH_ATTR http_process_disconnect_cb(void *arg) {

	os_printf("Disconnect conn=%p \r\n",arg);
}

//Callback called when there's data available on a socket.
static void ICACHE_FLASH_ATTR http_process_received_cb(void *arg, char *data, unsigned short len) {
	os_printf("http_process_received_cb, len: %d \r\n",len);

	//http_connection *conn = http_process_find_connection(arg);
	int i = 0;
	while(connection_list[i] !=0 && connection_list[i]->connection != arg && i < 10) {
		i++;
	}
	if(connection_list[i] == 0) {
		connection_list[i] = os_malloc(sizeof(connections));

	}
 	connection_list[i]->connection = arg;

	if(&c==NULL){
		espconn_disconnect(arg);
		return;
	}

//	//pass data to http_parser
//	size_t nparsed = http_parser_execute(
//		&(&c->parser),
//		&(conn->parser_settings),
//		data,
//		len);

    int length = strlen(data);
    char url[50] = { 0 };
    if(strstr(data, "GET /") != 0) {
 	   char *begin = strstr(data, "GET /") + 4;
 	   char *end = strstr(begin, " ");
 	   os_memcpy(url, begin, end - begin);
 	   url[end - begin] = 0;
    }


	if (strstr(data, "echo")) {
  		/* handle new protocol */
	    os_printf("WEBSOCK %s\r\n", data);
	    onwebsocket(data);
	    connection_list[i]->websocket = 1;
		//on_websocket_data(&conn->parser,data,len);
	} else if(connection_list[i]->websocket == 1){
		os_printf("WEBSOCK MESSAGE %s\r\n", data);
		ws_parse_framee(data, os_strlen(data));


	} else {

	  /* Handle error. Usually just close the connection. */
	    os_printf("NORMAL %s\r\n", data);
	    ondata(data);

		espconn_disconnect(&c);
		//http_process_free_connection(conn);
	}

}

//esp conn callbacks
static void ICACHE_FLASH_ATTR http_process_sent_cb(void *arg) {

	os_printf("http_process_sent_cb, conn %p \r\n",arg);
}

void ICACHE_FLASH_ATTR http_ws_server_start(){


//	NODE_DBG("Websocket server start, conn=%p", &ws_config.server_conn);

	espconn_regist_recvcb(&c, http_process_received_cb);
//	espconn_regist_reconcb(&c, http_process_reconnect_cb);
	espconn_regist_disconcb(&c, http_process_disconnect_cb);
	espconn_regist_sentcb(&c, http_process_sent_cb);

	espconn_regist_connectcb(&c, http_ws_connect_callback);
	espconn_accept(&c);

}
