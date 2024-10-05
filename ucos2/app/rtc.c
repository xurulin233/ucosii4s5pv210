#include "s5pv210.h"
#include "rtc.h"


/*
读取RTC的值
*/

void RTC_Read(date *p_date)
{
    rRTCCON = 0x01;
    
    p_date->year = rBCDYEAR +0x1f00 ;
    p_date->month = rBCDMON ;
    p_date->day = rBCDDATE ;
    p_date->week_day = rBCDDAY;
    p_date->hour = rBCDHOUR ;
    p_date->mintue =rBCDMIN  ;
    p_date->second = rBCDSEC ;
    
    rRTCCON = 0x00;
}
