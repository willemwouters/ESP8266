#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
//#include <sys/mman.h>
//#include <arpa/inet.h>
#include <string.h>


Timestamp Time_Convert_TO2TS(Time * time) {
	if (time) {
		int mth[12] = {0, 31, 59, 90, 120, 151, 181, 212, 273, 304, 334};
		int mthb[12] = {0, 31, 60, 91, 121, 152, 182, 213, 274, 305, 335};
		Timestamp timestamp =
				( ( (!(time->year % 4)) && (time->year % 100) ) || (!(time->year % 400)) )?
						((((unsigned long int)( time->year - 1970) / 4)) + (time->year - 1970) * 365 + mthb[time->month-1] + (time->day - 1)) * 86400 + time->hour * 3600 + time -> minute * 60 + time -> second:
						((((unsigned long int)( time->year - 1970) / 4)) + (time->year - 1970) * 365 + mth [time->month-1] + (time->day - 1)) * 86400 + time->hour * 3600 + time -> minute * 60 + time -> second;
		return timestamp;
	} else {
		return 0;
	}
}

Time * Time_Convert_TS2TO(Timestamp _time) {
	int i,tmpyear = 0; unsigned long int tmp;
	Time * time = (Time *) malloc(sizeof(Time));
	unsigned long int mth[12] = {0, 31, 59, 90, 120, 151, 181, 212, 273, 304, 334, 365};

	time->second = _time%60; _time -= time->second;
	time->minute = (_time%3600)/60; _time -= (time->minute*60);
	time->hour = (unsigned int) (_time%((unsigned long int) 24*3600))/3600; _time -= (unsigned long int) time->hour*3600;
	time->year = (unsigned int) (1970 + _time/((unsigned long int) 365*24*60*60));

	for (i=0, tmp=1970;tmpyear;( ( (!(tmp % 4)) && (tmp % 100) ) || (!(tmp % 400)) )?i++:0,tmp++);
	_time -= (unsigned long int) i*24*60*60; _time -= (unsigned long int) (time->year-1970)*365*24*60*60;
	for (i=1;i<=12;i++) if (mth[i]>(_time/((unsigned long int) 24*60*60))) break;
	time->month = i;
	_time -= mth[time->month-1]*24*60*60;
	time->day = _time/((unsigned long int) 24*60*60);

	return time;
}