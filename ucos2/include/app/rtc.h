#ifndef _RTC_H_
#define _RTC_H_

typedef struct Date
{
    unsigned short year;
    unsigned char month;
    unsigned char day;
    unsigned char week_day;
    unsigned char hour;
    unsigned char mintue;
    unsigned char second;
}date;

void RTC_Read(date *p_date);

#endif 
