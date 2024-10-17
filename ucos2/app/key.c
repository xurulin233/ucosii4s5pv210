#include "exception.h"
#include "s5pv210.h"
#include "stdio.h"
#include "lcd.h"


#define 	GPH2CON 				(*(volatile unsigned long *) 0xE0200C40)
#define 	GPH2DAT					(*(volatile unsigned long *) 0xE0200C44)
#define 	GPH2_0_EINT16 			(0xf<<(0*4))
#define 	GPH2_1_EINT17 			(0xf<<(1*4))
#define 	GPH2_2_EINT18 			(0xf<<(2*4))
#define 	GPH2_3_EINT19 			(0xf<<(3*4))
#define		EXT_INT_0_CON  			( *((volatile unsigned long *)0xE0200E00) )
#define		EXT_INT_1_CON  			( *((volatile unsigned long *)0xE0200E04) )
#define		EXT_INT_2_CON  			( *((volatile unsigned long *)0xE0200E08) )
#define		EXT_INT_3_CON  			( *((volatile unsigned long *)0xE0200E0C) )
#define		EXT_INT_0_MASK   		( *((volatile unsigned long *)0xE0200F00) )
#define		EXT_INT_1_MASK   		( *((volatile unsigned long *)0xE0200F04) )
#define		EXT_INT_2_MASK   		( *((volatile unsigned long *)0xE0200F08) )
#define		EXT_INT_3_MASK   		( *((volatile unsigned long *)0xE0200F0C) )
#define		EXT_INT_0_PEND   		( *((volatile unsigned long *)0xE0200F40) )
#define		EXT_INT_1_PEND   		( *((volatile unsigned long *)0xE0200F44) )
#define		EXT_INT_2_PEND   		( *((volatile unsigned long *)0xE0200F48) )
#define		EXT_INT_3_PEND   		( *((volatile unsigned long *)0xE0200F4C) )


//#define RGB(r,g,b)   				(unsigned int)( (r << 16) + (g << 8) + b )


void isr_key(void)
{
	char key_name[256];

	// 真正的isr应该做2件事情。
	// 第一，中断处理代码，就是真正干活的代码
	// 因为EINT16～31是共享中断，所以要在这里再次去区分具体是哪个子中断
	if (EXT_INT_2_PEND & (1<<0))
	{
		sprintf(key_name,"key1");
	}
	if (EXT_INT_2_PEND & (1<<1))
	{
		sprintf(key_name,"key2");
	}
	if (EXT_INT_2_PEND & (1<<2))
	{
		sprintf(key_name,"key3");
	}
	if (EXT_INT_2_PEND & (1<<3))
	{
		sprintf(key_name,"key4");
	}

	lcd_printf(24,0 , RGB( 0xFF,0xFF,0xFF), RGB( 0x00,0x00,0x00),0,"key is %s",key_name);

	// 第二，清除中断挂起
	EXT_INT_2_PEND |= (0x0F<<0);
	intc_clearvectaddr();//清除ADDRESS寄存器
}


// 初始化按键,仅在测试中断时使用
int Button_Init(void)
{

	// 1. 外部中断对应的GPIO模式设置
	GPH2CON |= 0xFFFF<<0;	// GPH2_0123共4个引脚设置为外部中断模式

	// 2. 中断触发模式设置
	EXT_INT_2_CON &= ~(0xFFFF<<0);
	EXT_INT_2_CON |= ((2<<0)|(2<<4)|(2<<8)|(2<<12));//设置为下降沿触

	//中断允许
	EXT_INT_2_MASK &= ~(0x0F<<0);

	// 设置eint16中断的中断处理函数
	intc_setvectaddr(NUM_EINT16_31,isr_key);

	// 使能eint16中断
	intc_enable(NUM_EINT16_31);

	return 0;
}
