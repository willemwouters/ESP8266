#ifndef __TIME_H__
#define __TIME_H__




typedef unsigned long int Timestamp;

typedef enum {
	TIME_OBJECT = 0,
	TIME_STAMP = 1
} TimeFormat;

typedef struct Time {
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int weekday;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
}Time;

Timestamp Time_Convert_TO2TS(Time * time);

Time * Time_Convert_TS2TO(Timestamp _time);


#endif