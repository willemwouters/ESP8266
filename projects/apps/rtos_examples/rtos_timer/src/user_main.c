/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/12/1, v1.0 create this file.
*******************************************************************************/
#include "espressif/esp_common.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "smartconfig.h"
#include "espmissingincludes.h"

static xTimerHandle timerHandle;

void ICACHE_FLASH_ATTR
UART_SetBaudrate(uint8 uart_no, uint32 baud_rate)
{
	uart_div_modify(uart_no, UART_CLK_FREQ / baud_rate);
}


void ICACHE_FLASH_ATTR
task(void *pvParameters)
{
	printf("SDK version:%s\n", system_get_sdk_version());
}



/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
	UART_SetBaudrate(0,115200);
    printf("SDK version:%s\n", system_get_sdk_version());
    xTaskHandle xHandle = NULL;
    // Create Timer (Trigger a measurement every second)
	timerHandle = xTimerCreate((signed char *)"BMP180 Trigger", 1000/portTICK_RATE_MS, pdTRUE, NULL, task);

	if (timerHandle != NULL)
	{
		if (xTimerStart(timerHandle, 0) != pdPASS)
		{
			printf("%s: Unable to start Timer ...\n", __FUNCTION__);
		}
	}
	else
	{
		printf("%s: Unable to create Timer ...\n", __FUNCTION__);
	}
}
