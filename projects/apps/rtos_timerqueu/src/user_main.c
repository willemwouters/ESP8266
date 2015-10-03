#include "espressif/esp_common.h"

#include "espmissingincludes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

#define MY_EVT_TIMER  0x01
#define MY_EVT_DATA 0x02

typedef struct {
	uint8_t event_type;
	int data;
} my_event_t;

static xQueueHandle mainqueue;
static xTimerHandle timerHandle;

bool informUser(const xQueueHandle* resultQueue, uint8_t cmd) {
	my_event_t ev;
	ev.event_type = MY_EVT_DATA;
	ev.data = cmd;
	return (xQueueSend(*resultQueue, &ev, 0) == pdTRUE);
}

static void timer_cb(xTimerHandle xTimer) {
	my_event_t ev;
	ev.event_type = MY_EVT_TIMER;
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
		case MY_EVT_TIMER:
			printf("%s: Received Timer Event\n", __FUNCTION__);
			informUser(com_queue, 1);
			break;
		case MY_EVT_DATA:
			printf("%s: Received Event data: %d\n", __FUNCTION__, ev.data);
			break;
		default:
			break;
		}
	}
}

void user_setup(void) {
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	os_delay_us(500);
}

void user_init(void) {
	user_setup();
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
