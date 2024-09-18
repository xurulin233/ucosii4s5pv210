#include "s5pv210.h"

 
// 串口初始化程序
void uart_init(void)
{
	// 1、初始化Tx Rx对应的GPIO引脚
	GPA0CON &= ~(0xff<<0);			// 把寄存器的bit0～7全部清零
	GPA0CON |= 0x00000022;			// 0b0010, Rx Tx
	
	// 2、几个关键寄存器的设置
	ULCON0 = 0x3;
	UCON0 = 0x5;
	UMCON0 = 0;
	UFCON0 = 0;
	
    // 3、设置波特率
	// 波特率设置公式：DIV_VAL = PCLK / (bps x 16) - 1	
	// 其中PCLK_PSYS = 66.7，bps表示想设置的波特率
	// DIV_VAL =  66700000/(115200*16) - 1 = 35.18
    // 整数部分是35，小数部分是0.18
 
    //整数部分是35
	UBRDIV0 = 35; 
 
	// (rUDIVSLOT中的1的个数)/16=上一步计算的余数=0.18
	// (rUDIVSLOT中的1的个数 = 16*0.18= 2.88 = 3
    // 3个1，查官方推荐表得到这个数字0x0888
	UDIVSLOT0 = 0x0888;		
}
 
 
// 串口发送程序，发送一个字节
void putc(char c)
{                  	
	// 串口发送一个字符，其实就是把一个字节丢到发送缓冲区中去
	// 因为串口控制器发送1个字节的速度远远低于CPU的速度，所以CPU发送1个字节前必须
	// 确认串口控制器当前缓冲区是空的（意思就是串口已经发完了上一个字节）
	// 如果缓冲区非空则位为0，此时应该循环，直到位为1
	while (!(UTRSTAT0 & (1<<1))); //rUTRSTAT0的bit[1]表示是否发完，为1则发完
	UTXH0 = c;
}
 
// 串口接收程序，轮询方式，接收一个字节
char getc(void)
{  
    //rUTRSTAT0的bit[0]表示是否接收完成，为1则接收完
	while (!(UTRSTAT0 & (1<<0)));
	return (URXH0 & 0x0f);//这里应该是 return (rURXH0 & 0xff)？
}
