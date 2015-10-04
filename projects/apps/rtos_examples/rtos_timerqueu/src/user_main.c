#include "espressif/esp_common.h"
#include "espmissingincludes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/init.h"


#define EVT_TIMER  0x01
#define EVT_DATA 0x02

typedef struct {
	uint8_t event_type;
	char* data;
	int len;
} my_event_t;

static xQueueHandle mainqueue;
static xTimerHandle timerHandle;

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
			printf("%s: Received Timer Event\n", __FUNCTION__);
			break;
		case EVT_DATA:
			printf("%s: Received Event data: -%s- -%d- \n", __FUNCTION__, ev.data, ev.len);
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

void user_init(void) {
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	os_delay_us(500);
    printf("SDK version : %s\n", system_get_sdk_version());
	mainqueue = xQueueCreate(10, sizeof(my_event_t));
	xTaskCreate(simple_task, (signed char * )"simple_task", 256, &mainqueue, 2, NULL);

	timerHandle = xTimerCreate((signed char *) "Trigger", 1000 / portTICK_RATE_MS, pdTRUE, NULL, timer_cb);
	if (timerHandle != NULL) {
		if (xTimerStart(timerHandle, 0) != pdPASS) {
			printf("%s: Unable to start Timer ...\n", __FUNCTION__);
		}
	} else {
		printf("%s: Unable to create Timer ...\n", __FUNCTION__);
	}
}
