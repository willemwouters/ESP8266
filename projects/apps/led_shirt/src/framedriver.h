/*
 * framedrive.h
 *
 *  Created on: Jul 20, 2015
 *      Author: wouters
 */
#include <c_types.h>
#ifndef SRC_FRAMEDRIVER_H_
#define SRC_FRAMEDRIVER_H_
#define FRAME_ERROR 1
#define FRAME_SUCCESS 0
char * ICACHE_FLASH_ATTR get_startbuffer(int buffer);
char * ICACHE_FLASH_ATTR get_buffersaved(int buffer);
void ICACHE_FLASH_ATTR setled(char* data, int len, int dim);
char* ICACHE_FLASH_ATTR getpixel(int buffer, int column, int row);
void ICACHE_FLASH_ATTR setpixel(int buffer, int column, int row, char r, char g, char b);
void ICACHE_FLASH_ATTR writestream(int buffer, char * data, int len);
void ICACHE_FLASH_ATTR freebuffers();
void write_textwall_buffer(int buffer, char * textbuffer, int len);
void write_texttowall(int buffer, int textbuffer, long offset, int fR, int fG, int fB, int fbR, int fbG, int fbB);
void clear_buffer(int buffer);
void rainbow_copybuffer(int buffer, int frame);
void set_buffer(int buffer, int val1, int val2, int val3);
#endif /* SRC_FRAMEDRIVER_H_ */
