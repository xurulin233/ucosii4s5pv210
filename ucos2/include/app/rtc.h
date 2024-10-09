#ifndef _RTC_H_
#define _RTC_H_

typedef struct rtc_time
{
	unsigned int year;
	unsigned int month;
	unsigned int date;			// 几号
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
	unsigned int week_day;			// 星期几
}rtc_time;


void RTC_Read(rtc_time *p_date);
void rtc_init();


#endif
