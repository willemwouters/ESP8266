#include "ws2812.h"

#include "gpio.h"
#include "config.h"
#include "espressif/esp_common.h"
#include "espmissingincludes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//I just used a scope to figure out the right time periods.

int fl;


int GammaE[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
					2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
					6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11,
					11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
					19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
					29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
					40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
					55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
					71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
					90, 91, 93, 94, 95, 96, 98, 99,100,102,103,104,106,107,109,110,
					111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,
					135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,
					162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,
					191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
					222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255};

float  dimscale[20] = { 0.05, 0.1, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50, 0.55,  0.60, 0.65, 0.70, 0.75, 0.80, 0.85, 0.90, 0.95, 1};
static char lTmlBuf[COLUMNS * ROWS * COLORS];
void  __attribute__((optimize("O2"))) send_ws_0(uint8_t gpio){
  uint8_t i;
  i = 4; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << gpio);
  i = 9; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << gpio);
}

void __attribute__((optimize("O2"))) send_ws_1(uint8_t gpio){
  uint8_t i;
  i = 8; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << gpio);
  i = 6; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << gpio);
}



void WS2812CopyBuffer( uint8_t * buffer, uint16_t length, int flicker, int dim)
{
	uint16_t i;

	for( i = 0; i < length; i++ )
			{
				int val = i;
				if(i % 3 == 0) {
					val = val + 1;
				}
				if(i % 3 == 1) {
					val = val - 1;
				}

				int tot = i / 3;
				int r = 0;

				int vr = 7;
				int t = tot / ROWS;

				if(t % 2 == 1) {
					int add = 0;
					add = ((ROWS-1) - ((tot % ROWS) * 2) ); //   (6 - (0 * 2) )  		7 mod 7 = 0   =
					val = val + (add * 3);	// 21 + 21 = 42		24 + 18 = 42
				}


				uint8_t byte = buffer[val];
				if(dim > 19) {
					dim = 19;
				}
				byte = byte * dimscale[dim];
				if(flicker) {
					byte = 0x00;
				}

				lTmlBuf[i] = GammaE[byte];
			}
}
#define IO_PIN 4
#define IO_MUX PERIPHS_IO_MUX_GPIO4_U
#define IO_FUNC FUNC_GPIO4


void WS2812OutBuffer( uint8_t * buffer, uint16_t length, int dim)
{
	uint16_t i;

	GPIO_OUTPUT_SET(GPIO_ID_PIN(WSGPIO), 0);

//taskDISABLE_INTERRUPTS();
	for( i = 0; i < length; i++ )
	{

			int tot = i / 3;
			//tot = tot;
			if(REALROWS == 7) {
				if(tot == 0 || tot == 15 || tot == 31 || tot == 47 || tot == 63 || tot == 79 || tot == 95  ) {
					if(tot == 0) {
						i = i + 3;
					} else {
						i = i + 6;
					}
				}
			}
			taskYIELD();

			uint8_t byte = lTmlBuf[i];
			//printf("%d", byte);
			if( byte & 0x80 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
			if( byte & 0x40 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
			if( byte & 0x20 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
			if( byte & 0x10 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
			if( byte & 0x08 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
			if( byte & 0x04 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
			if( byte & 0x02 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
			if( byte & 0x01 ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);


	}
	//taskENABLE_INTERRUPTS();
	//os_printf("\r\n");
	//reset will happen when it's low long enough.
	//(don't call this function twice within 10us)
}



