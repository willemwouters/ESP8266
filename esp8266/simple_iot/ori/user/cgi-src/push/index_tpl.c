#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd/httpd.h"
#include "espmissingincludes.h"

#include "ip/TcpClient.h"
#include "ip/TcpServer.h"
#include "wifi/WifiUtility.h"
//#include "io/io.h"
#include "gpio.h"
#include "io/key.h"


#define PLUG_KEY_NUM            1

LOCAL struct keys_param keys;
LOCAL struct single_key_param *single_key[PLUG_KEY_NUM];


#define PLUG_KEY_2_IO_MUX     PERIPHS_IO_MUX_MTMS_U
#define PLUG_KEY_2_IO_NUM     2
#define PLUG_KEY_2_IO_FUNC    FUNC_GPIO2

#define PLUG_KEY_0_IO_MUX     PERIPHS_IO_MUX_MTMS_U
#define PLUG_KEY_0_IO_NUM     0
#define PLUG_KEY_0_IO_FUNC    FUNC_GPIO0


//This cgi is when opening push index
int ICACHE_FLASH_ATTR cgiPushIndexRequest(HttpdConnData *connData) {
	char text[128];
	os_printf("-%s-%s \r\n", __FILE__, __func__);
	httpdFindArg(connData->postBuff, "text", text, sizeof(text));
	httpdPushMessage("/push/listen.push", text);
	espconn_sent(connData->conn, (uint8 *) " ", 1);
	return HTTPD_CGI_DONE;
}

LOCAL void ICACHE_FLASH_ATTR user_plug_short_press(void)
{
	char * msg = "{ \"GPIO0\": \"SHORT PRESS\" }" ;
   	os_printf("-%s-%s \r\n", __FILE__, __func__);
	httpdPushMessage("/push/io.push", msg);
}

LOCAL void ICACHE_FLASH_ATTR user_plug_long_press(void)
{
	char * msg = "{ \"GPIO0\": \"LONG PRESS\" }" ;
   	os_printf("-%s-%s \r\n", __FILE__, __func__);
	httpdPushMessage("/push/io.push", msg);
}

void initIntGpio() {
   	os_printf("-%s-%s \r\n", __FILE__, __func__);

	single_key[0] = key_init_single(PLUG_KEY_0_IO_NUM, PLUG_KEY_0_IO_MUX, PLUG_KEY_0_IO_FUNC,
                                    user_plug_long_press, user_plug_short_press, 50, 1000);

// single_key[1] = key_init_single(PLUG_KEY_2_IO_NUM, PLUG_KEY_2_IO_MUX, PLUG_KEY_2_IO_FUNC,
//                                     user_plug_long_press, user_plug_short_press, 50, 1000);
    keys.key_num = PLUG_KEY_NUM;
    keys.single_key = single_key;

    key_init(&keys);
}

//Template code for the WLAN page.
void ICACHE_FLASH_ATTR tplPushIndex(HttpdConnData *connData, char *token, void **arg) {
	char buff[1024];
   	os_printf("-%s-%s \r\n", __FILE__, __func__);

	initIntGpio();
	if (token==NULL) return;
	int i = GPIO_INPUT_GET(BIT0);
	int b = GPIO_INPUT_GET(BIT1);
	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "GPIO0")==0) {
			if(i == 1) {
				os_strcpy(buff, "1");
			} else {
				os_strcpy(buff, "2");
			}
	}
	if (os_strcmp(token, "GPIO1")==0) {
			if(b == 1) {
				os_strcpy(buff, "1");
			} else {
				os_strcpy(buff, "2");
			}
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}

