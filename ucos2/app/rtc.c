#include "s5pv210.h"
#include "rtc.h"
#include "stdio.h"
#include "exception.h"
#include "lcd.h"

char *week_num[7]={ "SUN","MON", "TUES", "WED", "THURS","FRI", "SAT" };

//#define RGB(r,g,b)   				(unsigned int)( (r << 16) + (g << 8) + b )



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

//写入time
void rtc_set_time(const struct rtc_time *p)
{
	// 第一步，打开RTC读写开关
	rRTCCON |= (1<<0);

	// 第二步，写RTC时间寄存器
	rBCDYEAR = num_2_bcd(p->year - 2000);
	rBCDMON = num_2_bcd(p->month);
	rBCDDATE = num_2_bcd(p->date);
	rBCDHOUR = num_2_bcd(p->hour);
	rBCDMIN = num_2_bcd(p->minute);
	rBCDSEC = num_2_bcd(p->second);
	rBCDDAY = num_2_bcd(p->week_day);

	// 最后一步，关上RTC的读写开关
	rRTCCON &= ~(1<<0);
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
	printf("now time is: %0d-%02d-%02d  %02d:%02d:%02d %s\r\n",now_time.year,
	now_time.month,now_time.date,now_time.hour,now_time.minute,
	now_time.second,week_num[now_time.week_day - 1]);

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


void isr_rtc_tick(void) //Tick timer 的ISR （800ms）
{
	rtc_time now_time;
	RTC_Read(&now_time);
	//printf("tick: %d-%2d-%2d  %2d:%2d:%2d %s\r\n",now_time.year,
	//now_time.month,now_time.date,now_time.hour,now_time.minute,
	//now_time.second,week_num[now_time.week_day - 1]);

	lcd_printf(10,0 , RGB( 0xFF,0xFF,0xFF), RGB( 0x00,0x00,0x00),0,
	"%d-%02d-%02d  %02d:%02d:%02d %s",now_time.year,
	now_time.month,now_time.date,now_time.hour,now_time.minute,
	now_time.second,week_num[now_time.week_day - 1]);


	rINTP |= (1<<0); //清中断
	intc_clearvectaddr();
}





// 使能/关闭Tick timer
void rtc_ticktime_enable(unsigned char bdata)
{
	rRTCCON &= ~(0xf<<4); //clock = 32768
	rTICCNT = 8192;//32768/(4); //1秒刷新4次
	rRTCCON |= (1<<8); //enable tick timer

	// 设置tick中断的中断处理函数
	intc_setvectaddr(NUM_RTC_TICK,isr_rtc_tick);

	intc_enable(NUM_RTC_TICK);

}




