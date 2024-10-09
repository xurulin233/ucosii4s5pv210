#include "s5pv210.h"
#include "rtc.h"
#include "stdio.h"
#include "exception.h"

// 函数功能：把十进制num转成bcd码，譬如把56转成0x56
static unsigned int num_2_bcd(unsigned int num)
{
	// 第一步，把56拆分成5和6
	// 第二步，把5和6组合成0x56
	return (((num / 10)<<4) | (num % 10));
}

// 函数功能：把bcd码bcd转成十进制，譬如把0x56转成56
static unsigned int bcd_2_num(unsigned int bcd)
{
	// 第一步，把0x56拆分成5和6
	// 第二步，把5和6组合成56
	return (((bcd & 0xf0)>>4)*10 + (bcd & (0x0f)));
}




/*
读取RTC的值
*/

void RTC_Read(rtc_time *p_date)
{
  	// 第一步，打开RTC读写开关
	rRTCCON |= (1<<0);

	// 第二步，读RTC时间寄存器
	p_date->year = bcd_2_num(rBCDYEAR) + 2000;
	p_date->month = bcd_2_num(rBCDMON);
	p_date->date = bcd_2_num(rBCDDATE);
	p_date->hour = bcd_2_num(rBCDHOUR);
	p_date->minute = bcd_2_num(rBCDMIN);
	p_date->second = bcd_2_num(rBCDSEC);
	p_date->week_day = bcd_2_num(rBCDDAY);

   	// 最后一步，关上RTC的读写开关
	rRTCCON &= ~(1<<0);
}


/*
闹钟处理函数
*/

void isr_alarm(void)
{
	rtc_time now_time;
	// 第一，中断处理代码，就是真正干活的代码
	printf("alarm \r\n");
	RTC_Read(&now_time);
	printf("now time is: %d:%d:%d:%d:%d:%d:%d.\r\n",now_time.year,
	now_time.month,now_time.date,now_time.hour,now_time.minute,
	now_time.second,now_time.week_day);

	// 第二，清除中断挂起
	rINTP |= (0x1<<1);
	////清除ADDRESS寄存器
	intc_clearvectaddr();
}


/*
闹钟使能
*/
void rtc_init()
{

	//每分钟58秒产生一次中断
	rALMSEC = num_2_bcd(23);

	//全局闹钟使能，秒闹钟使能
	rRTCALM |= 1<<0;
	rRTCALM |= 1<<6;

	// 设置alarm中断的中断处理函数
	intc_setvectaddr(NUM_RTC_ALARM,isr_alarm);

	intc_enable(NUM_RTC_ALARM);

}





