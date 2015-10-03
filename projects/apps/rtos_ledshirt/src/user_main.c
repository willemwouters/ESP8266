#include "espressif/esp_common.h"
#include "espmissingincludes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/init.h"
#include "led.h"
#include "gpio.h"

#include "framedriver.h"
#include "ws2812.h"

#define EVT_TIMER  0x01
#define EVT_DATA 0x02

typedef struct {
	uint8_t event_type;
	char* data;
	int len;
} my_event_t;

static xQueueHandle mainqueue;
static xTimerHandle timerHandle;

	char buffer[1024];

	static void receive_udp(void *pvParameters) {
		  int lSocket;
		   struct sockaddr_in sLocalAddr, sDestAddr;

		   int nbytes;
		   int i;
		   lSocket = lwip_socket(AF_INET, SOCK_DGRAM, 0);
		   if(lSocket != 0) {
			   printf("ERROR \r\n");
		   }

		   memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
		   memset((char *)&sDestAddr, 0, sizeof(sDestAddr));

		   /*Destination*/
		   sDestAddr.sin_family = AF_INET;
		   sDestAddr.sin_len = sizeof(sDestAddr);
		   sDestAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
		   sDestAddr.sin_port = htons(8080);

		   /*Source*/
		   sLocalAddr.sin_family = AF_INET;
		   sLocalAddr.sin_len = sizeof(sLocalAddr);
		   sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("192.168.4.1");
		   sLocalAddr.sin_port = htons(8080);

		   int err = lwip_bind(lSocket, (struct sockaddr *)&sLocalAddr, sizeof(sLocalAddr));
		   if(err != 0) {
			   printf("ERROR \r\n");
		   }
		   while (1) {
				 nbytes=lwip_recv(lSocket, buffer, sizeof(buffer),8);
				 if (nbytes>0){
					//lwip_sendto(lSocket, buffer, nbytes, 0, (struct sockaddr *)&sDestAddr, sizeof(sDestAddr));
					char * dat = malloc(nbytes);
					memcpy(dat, buffer, nbytes);
					dat[nbytes] = 0;
					my_event_t ev;
					ev.event_type = EVT_DATA;
					ev.data = dat;
					ev.len = nbytes;
					xQueueSend(mainqueue, &ev, 0);
				 }
		   }
		   lwip_close(lSocket);
	}


static void timer_cb(xTimerHandle xTimer) {
	my_event_t ev;
	ev.event_type = EVT_TIMER;
	xQueueSend(mainqueue, &ev, 0);
}

void simple_task(void *pvParameters) {
	// Received pvParameters is communication queue
	xQueueHandle *com_queue = (xQueueHandle *) pvParameters;
	printf("%s: Started user interface task\n", __FUNCTION__);
	while (1) {
		my_event_t ev;
		xQueueReceive(*com_queue, &ev, portMAX_DELAY);

		switch (ev.event_type) {
		case EVT_TIMER:
			//printf("%s: Received Timer Event\n", __FUNCTION__);
			refreshFrameCb();
			break;
		case EVT_DATA:
			//printf("%s: Received Timer Event\n", __FUNCTION__);
			handle_led_command(ev.data, ev.len);
			free(ev.data);
			break;
		default:
			break;
		}
	}
}


void setap(char * ssid, int len) {
	static struct softap_config apconf;
	wifi_set_opmode(STATIONAP_MODE);
	wifi_softap_get_config(&apconf);
	memset(apconf.ssid, 0, 32);
	strncpy((char*) apconf.ssid, ssid, 32);
	apconf.authmode = AUTH_OPEN;
	apconf.max_connection = 20;
	apconf.ssid_hidden = 0;
	apconf.ssid_len = strlen(ssid);
	apconf.channel = 1;

	wifi_softap_set_config(&apconf);
}

void ICACHE_FLASH_ATTR connectToAp() {
	char * ap = "LedAccess";
	char * pass = "test12345";
	wifi_set_phy_mode( PHY_MODE_11N );

	struct station_config apconf;
	wifi_station_set_auto_connect(true);
	wifi_set_opmode(STATION_MODE);
	wifi_station_get_config(&apconf);
	strncpy((char*)apconf.ssid, ap, 32);
	printf("connecting to: %s", apconf.ssid);
	strncpy((char*)apconf.password, pass, 64);
	wifi_station_set_config(&apconf);
	wifi_promiscuous_enable(1);
	//wifi_set_event_handler_cb(wifi_event_cb);
}
void user_init(void) {
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	os_delay_us(500);
    printf("SDK version : %s\n", system_get_sdk_version());
	mainqueue = xQueueCreate(10, sizeof(my_event_t));

	connectToAp();
	//setap("test", 4);

	init_led();

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO5_U); // disable pullodwn
	GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT5);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 1);

	char outbuffer[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	WS2812OutBuffer( outbuffer, 6 , 0); //Initialize the output.
	write_textwall_buffer(0, "BIER ", 5);


	xTaskCreate(simple_task, (signed char * )"simple_task", 256, &mainqueue, tskIDLE_PRIORITY, NULL);
	xTaskCreate(receive_udp, (signed char * )"test", 256, &mainqueue, tskIDLE_PRIORITY, NULL);

	timerHandle = xTimerCreate((signed char *) "Trigger", 50 / portTICK_RATE_MS, pdTRUE, NULL, timer_cb);
	if (timerHandle != NULL) {
		if (xTimerStart(timerHandle, 0) != pdPASS) {
			printf("%s: Unable to start Timer ...\n", __FUNCTION__);
		}
	} else {
		printf("%s: Unable to create Timer ...\n", __FUNCTION__);
	}
}
