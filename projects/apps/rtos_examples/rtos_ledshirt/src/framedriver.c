/*
 * framedriver.c
 *
 *  Created on: Jul 20, 2015
 *      Author: wouters
 */
#include "framedriver.h"
#include "config.h"
#include "ws2812.h"
#include "font8x8_basic.h"
#include "espressif/esp_common.h"


static char framebuffer[BUFFERS * (COLUMNS *ROWS) * (COLORS)];
static char bitmaptextbuffer[BUFFERS][(COLUMNS * TEXTROWS)][ROWS];

int mTextLength = 0;
void ICACHE_FLASH_ATTR writestream(int buffer, char * data, int len) {
	if(len > (COLUMNS * ROWS * COLORS)) {
		printf("length = %d \r\n", len);
		len = COLUMNS * ROWS * COLORS;
	}
	memcpy(&framebuffer[(buffer * COLUMNS * ROWS * COLORS)], data, len);

}


void ICACHE_FLASH_ATTR set_textpixel(int buffer, int column, int row, char pix) {
	if(DEBUG_LEVEL_WRITE)
		printf("settextpix - col:%d row:%d val:%d \n",column, row, pix);
	bitmaptextbuffer[buffer][column][row] = pix;
}

int tryout = 0;
char ICACHE_FLASH_ATTR get_textpixel(int buffer, int column, int row, long offset) {
	long c = column + offset;
	tryout = c % mTextLength;
	if(DEBUG_LEVEL_READ)
		printf("get_textpix: col:%d row:%d val:%d \n",c, row, bitmaptextbuffer[c][row]);
    return bitmaptextbuffer[buffer][tryout][row];
}


void ICACHE_FLASH_ATTR write_textwall_buffer(int textbuffer, char * text, int len) {
	int offset = 0;
	int bfstart = 0;
	int txtpix = 0;

	for(int i = 0; i < (COLUMNS * TEXTROWS); i++) {
			if(txtpix % TEXTHEIGHT == 0 && i != 0) {
					bfstart++;
					offset += 0;

			}
			if(text[bfstart] == '~') { // when special char;
				for(int z = 0; z < (COLUMNS); z++) {
					for(int y = 0; y < ROWS; y++) {
						set_textpixel(textbuffer, i + offset + z,  y, false);
					}
				}
				offset = offset + ((ROWS * 2)-2);	//JUMP EXACTLY 1 FRAME, for loop does next ++
				bfstart++;
			}

			for(int x = 0; x < ROWS; x++) {
				int a = (1 << (i % ROWS));
				if(bfstart >= len) {
					mTextLength = i + offset;
					printf("textlength in c pix:%d", mTextLength);
					return;
				}
				bool pix = ((font8x8_basic[text[bfstart]][(ROWS-1)-x]) & a);
				set_textpixel(textbuffer, i + offset, x, pix);
			}
			txtpix++;

	}
}

void ICACHE_FLASH_ATTR write_texttowall(int buffer, int textbuffer, long offset, int fR, int fG, int fB, int fbR, int fbG, int fbB) {
	 for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {
			char pix = get_textpixel(textbuffer, i, x, offset);
			if(pix > 0) {
				setpixel(buffer, i, x,  fR, fG, fB);
			} else {
				setpixel(buffer, i, x,  fbR, fbG, fbB);
			}
		}
	 }
}


void setled(char* data, int len ,int dim) {
	WS2812OutBuffer(data, len, dim);
}

char * ICACHE_FLASH_ATTR get_startbuffer(int buffer) {
	return &framebuffer[buffer * COLUMNS * ROWS * COLORS];
}
void ICACHE_FLASH_ATTR set_buffer(int buffer, int val1, int val2, int val3) {
	for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {
			setpixel(buffer, i, x, val1, val2, val3);
		}
	}
}
void ICACHE_FLASH_ATTR clear_buffer(int buffer) {
	for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {
			setpixel(buffer, i, x, 0x00, 0x00, 0x00);
		}
	}
}

void ICACHE_FLASH_ATTR setpixel(int buffer, int column, int row, char g, char r, char b) {
	int p = (buffer * COLUMNS * ROWS * COLORS) + (column * ROWS * COLORS) + (row * COLORS);
	if(DEBUG_LEVEL_WRITE)
		printf("setpixel - led:%d col:%d row:%d val[0]:%d val[1]:%d val[2]:%d \n",p , column, row, r, g ,b);
	framebuffer[p] = g;
	framebuffer[p +1] = r;
	framebuffer[p +2] = b;
}

char* ICACHE_FLASH_ATTR getpixel(int buffer, int column, int row) {
	int p = (buffer * COLUMNS * ROWS * COLORS) + (column * ROWS * COLORS) + (row * COLORS);
    return &framebuffer[p];
}


