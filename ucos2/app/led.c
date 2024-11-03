
#include "delay.h"

#define GPJ2CON		0xE0200280
#define GPJ2DAT		0xE0200284

#define rGPJ2CON	*((volatile unsigned int *)GPJ2CON)
#define rGPJ2DAT	*((volatile unsigned int *)GPJ2DAT)


// 该函数要实现led闪烁效果
void led_blink(void)
{
	// led初始化，也就是把GPJ2CON中设置为输出模式
	//volatile unsigned int *p = (unsigned int *)GPJ2CON;
	//volatile unsigned int *p1 = (unsigned int *)GPJ2DAT;
	rGPJ2CON = 0x11111111;
	
	while (1)
	{
		// led亮
		rGPJ2DAT = ((0) | (0<<1) | (0<<2) | (0<<3));
		// 延时
		delay(900000);
		// led灭
		rGPJ2DAT = ((1) | (1<<1) | (1<<2) | (1<<3));
		// 延时
		delay(900000);
	}
}

// 亮1个led
void led1(void)
{
	rGPJ2CON = 0x11111111;
	rGPJ2DAT = ((0) | (1<<1) | (1<<2) | (1<<3));
}


// 亮2个led
void led2(void)
{
	rGPJ2CON = 0x11111111;
	rGPJ2DAT = ((1) | (0<<1) | (1<<2) | (1<<3));
}

// 亮3个led
void led3(void)
{
	rGPJ2CON = 0x11111111;
	rGPJ2DAT = ((1) | (1<<1) | (0<<2) | (1<<3));
}

// 亮4个led
void led4(void)
{
	rGPJ2CON = 0x11111111;
	rGPJ2DAT = ((1) | (1<<1) | (1<<2) | (0<<3));
}

void led_off(void)
{
	rGPJ2CON = 0x11111111;
	rGPJ2DAT = ((1) | (1<<1) | (1<<2)| (1<<3));
}



	
