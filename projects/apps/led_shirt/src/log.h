/*
 * log.h
 *
 *  Created on: Oct 11, 2015
 *      Author: wouters
 */

#ifndef SRC_LOG_H_
#define SRC_LOG_H_

#define LOG_ON 0x01
#define LOG_OFF 0x00

#define LOG_TRACE 0x00
#define LOG_TRACE_TAG "TRACE"
#define LOG_INFO 0x01
#define LOG_INFO_TAG "INFO"
#define LOG_WARNING 0x02
#define LOG_WARNING_TAG "WARNING"
#define LOG_ERROR 0x03
#define LOG_ERROR_TAG "ERROR"

#define LOG_UDP LOG_ON
#define LOG_USER LOG_ON
#define LOG_FRAMEDRIVER LOG_ON
#define LOG_ENABLE LOG_ON

#define LOG_UDP_TAG "UDP"
#define LOG_USER_TAG "USER"
#define LOG_FRAMEDRIVER_TAG "FRAMEDRIVER"


#define LOG_LEVEL LOG_WARNING



#define LOG(debug, tag, message, ...) do { \
									\
		char * t;					\
		if(debug == LOG_TRACE) {	\
			t = LOG_TRACE_TAG;		\
		}							\
									\
		if(debug == LOG_INFO) {		\
			t = LOG_INFO_TAG;		\
		}							\
									\
		if(debug == LOG_WARNING) {	\
			t = LOG_WARNING_TAG;	\
		}							\
									\
		if(debug == LOG_ERROR) {	\
			t = LOG_ERROR_TAG;		\
		}							\
									\
		if((debug >= LOG_LEVEL) && (LOG_ENABLE & LOG_ON)) { \
			os_printf("[%s][%s][%s]\t", tag, t, __func__); os_printf(message, ##__VA_ARGS__); os_printf("\r\n"); \
		}\
		\
} while (0)


#define LOG_W(debug, tag, message, ...) do { \
	if(debug == LOG_ON) { \
	LOG(LOG_WARNING, tag, message, ##__VA_ARGS__); \
	} \
}while(0)

#define LOG_T(debug, tag, message, ...) do { \
	if(debug == LOG_ON) { \
	LOG(LOG_TRACE, tag, message, ##__VA_ARGS__); \
	} \
}while(0)

#define LOG_E(debug, tag, message, ...) do { \
	if(debug == LOG_ON) { \
	LOG(LOG_ERROR, tag, message, ##__VA_ARGS__); \
	} \
}while(0)


#define LOG_I(debug, tag, message, ...) do { \
	if(debug == LOG_ON) { \
		LOG(LOG_INFO, tag, message, ##__VA_ARGS__); \
	} \
}while(0)

#endif /* SRC_LOG_H_ */
