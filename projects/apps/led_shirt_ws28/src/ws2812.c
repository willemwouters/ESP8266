#include "ws2812.h"
#include "ets_sys.h"
#include "eagle_soc.h"
#include "osapi.h"
#include "gpio.h"
//I just used a scope to figure out the right time periods.

int fl;

static void ICACHE_FLASH_ATTR __attribute__((optimize("O2"))) send_ws_0(uint8_t gpio){
  uint8_t i;
  i = 4; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << gpio);
  i = 9; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << gpio);
}

static void ICACHE_FLASH_ATTR __attribute__((optimize("O2"))) send_ws_1(uint8_t gpio){
  uint8_t i;
  i = 8; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << gpio);
  i = 6; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << gpio);
}


void WS2812OutBuffer( uint8_t * buffer, uint16_t length )
{
	uint16_t i;

	GPIO_OUTPUT_SET(GPIO_ID_PIN(WSGPIO), 0);

	for( i = 0; i < length; i++ )
	{
		uint8_t byte = buffer[i];

		if( byte & 0x80 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
		if( byte & 0x40 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
		if( byte & 0x20 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
		if( byte & 0x10 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
		if( byte & 0x08 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
		if( byte & 0x04 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
		if( byte & 0x02 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
		if( byte & 0x01 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);

	}
	//reset will happen when it's low long enough.
	//(don't call this function twice within 10us)
}



