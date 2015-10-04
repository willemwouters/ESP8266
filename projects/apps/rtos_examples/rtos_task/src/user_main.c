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

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "smartconfig.h"
#include "espmissingincludes.h"


void ICACHE_FLASH_ATTR
UART_SetBaudrate(uint8 uart_no, uint32 baud_rate)
{
	uart_div_modify(uart_no, UART_CLK_FREQ / baud_rate);
}


void ICACHE_FLASH_ATTR
task(void *pvParameters)
{
	printf("SDK version:%s\n", system_get_sdk_version());
    vTaskDelete(NULL);
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
    xTaskCreate(task, "smartconfig_task", 256, NULL, 2, &xHandle);

//    if( xHandle != NULL )
//	{
//	 vTaskDelete( xHandle );
//	}
}
