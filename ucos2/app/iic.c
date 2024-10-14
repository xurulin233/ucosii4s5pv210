/*s5pv210 I2C相关寄存器配置*/
/* GPIO */
#include "exception.h"
#include "stdio.h"



#define GPD1CON  (*(volatile unsigned long*) 0xe02000c0)
#define GPD1PUD  (*(volatile unsigned long*) 0xe02000c8)

/* IIC */
#define I2CCON0  (*(volatile unsigned long*) 0xe1800000)
#define I2CSTAT0 (*(volatile unsigned long*) 0xe1800004)
#define I2CDS0   (*(volatile unsigned char*) 0xe180000c)



#define WRITE    1     /*Tx Mode*/
#define READ     2		/*Rx Mode*/

typedef struct I2C
{

	volatile int mode;				/*读/写*/
	volatile int pt;				/*缓冲区的位置*/
	unsigned char *data;         /*数据缓冲区*/
	volatile int datacount;		/*数据长度*/
	char data_addr;          //读写位置（相对于AT24C08）
	volatile int w_addr;              //写读写位置的标志
}t210_I2C;

static t210_I2C   my_t210_i2C;



static void delay( unsigned int time)
{
	volatile unsigned int i = time;		// volatile 让编译器不要优化，这样才能真正的减
	while (i--);							// 才能消耗时间，实现delay
}




/**
*Master Tx mode
*slvAddr: 从机地址，buf: 发送缓存，len: 发送长度
*/
void i2c0_write(unsigned char slvAddr,unsigned char *buf,int len)
{
		my_t210_i2C.mode = WRITE;
		my_t210_i2C.pt = 0;
		my_t210_i2C.data = buf;
		my_t210_i2C.datacount = len;
		my_t210_i2C.w_addr = 0;
		my_t210_i2C.data_addr = slvAddr;

		I2CSTAT0 = 0xd0;

		/*write slave address to I2CDS*/
		I2CDS0 = 0xa0;
		/*write 0xf0 to I2CSTAT*/
		I2CSTAT0 = 0xf0;

		I2CCON0 &= ~(1<<4);

		/* 等待ACK应答 */
		while(I2CSTAT0 & 0x20);
}


/**
*Master Rx Mode
*
*/
void i2c0_read(unsigned char slvAddr,unsigned char *buf,int len)
{
	my_t210_i2C.mode = READ;
	my_t210_i2C.pt = -1;   /*表示第一个中断不接受数据，为地址数据*/
	my_t210_i2C.data = buf;
	my_t210_i2C.datacount = len-1;
	my_t210_i2C.w_addr = 0;
	my_t210_i2C.data_addr = slvAddr;

	I2CSTAT0 = 0xd0;

	/*write slave address to I2CDS*/
	I2CDS0 = 0xa0;
	/*write 0xb0 to I2CSTAT*/
	I2CSTAT0 = 0xf0;


	while(I2CSTAT0 & 0x20);
			// 写入器件的地址（读操作）
		I2CDS0 = 0xa1;
			// s信号
		I2CSTAT0 = 0xb0;
		while (I2CSTAT0 & 0x20);

}

/**
*I2C ACK period interrupt pending
*/
void isr_i2c0()
{

	unsigned int i;

	switch(my_t210_i2C.mode)
	{
		case WRITE :							//写中断
			{
				if (my_t210_i2C.w_addr == 0)
				{
					I2CDS0 = my_t210_i2C.data_addr;  // 1.先发送要写数据的位置
					delay(10000);
					I2CCON0 = 0xaf;
					my_t210_i2C.w_addr++;
					break;
				}
				if((my_t210_i2C.datacount--) == 0)
					{
						/*Write 0xD0 (M/T Stop) to I2CSTAT.*/
						I2CSTAT0 = 0xd0;
						/*Clear pending bit .*/
						I2CCON0 = 0xaf;
						/*Wait until the stop condition takes effect.*/
						delay(10000);
						break;
					}
				/*N*/
						/*Write new data transmitted to I2CDS.*/
						I2CDS0 = my_t210_i2C.data[my_t210_i2C.pt++];

						/*Clear pending bit to resume.*/
						/*The data of the I2CDS is shifted to SDA.*/
						delay(10000);
						I2CCON0 = 0xaf;
						break;
			}

		case READ :                           /*读中断*/
			{
				/*判断符号应为==*/

				if (my_t210_i2C.w_addr == 0)		   // 1.先发送要读取数据的位置
				{
					I2CDS0 = my_t210_i2C.data_addr;
					delay(1000);
					I2CCON0 = 0xaf;
					my_t210_i2C.w_addr++;
					break;
				}


				if (my_t210_i2C.w_addr == 1)           // 2.当位置发送完毕之后
				{                              //   发送P信号停止传输
					I2CSTAT0 = 0xd0;
					I2CCON0  = 0xaf;
					delay(1000);
					my_t210_i2C.w_addr++;
					break;
				}

				if(my_t210_i2C.pt == -1)	           /*第一次不接受中断*/
					{
						my_t210_i2C.pt = 0;
						if(my_t210_i2C.datacount ==0)
							I2CCON0 = 0x2f;                  /*由at24cxx手册知道，读数据时，最后一个数据不需要ACK*/
						else
							I2CCON0 = 0xaf;
						break;
					}

				/*Read a new data from I2CDS.*/
				if ((my_t210_i2C.datacount--) == 0)
				{
					my_t210_i2C.data[my_t210_i2C.pt++] = I2CDS0; // 5.接收最后一个数据
					I2CSTAT0 = 0x90;            // 发送P信号
					I2CCON0 = 0xaf;
					delay(1000);
					break;
				}
				my_t210_i2C.data[my_t210_i2C.pt++] = I2CDS0;  // 4.接收数据
				if (my_t210_i2C.datacount == 0)
				  I2CCON0 = 0x2f;
				else
				  I2CCON0 = 0xaf;
				break;
			}
		default:
			break;
	}
	/*清除中断*/
	//I2CCON0 &= ~(1<<4);

	/*清除中断函数地址*/
	intc_clearvectaddr();//清除ADDRESS寄存器


}



void i2c0_init()
{
	/*引脚配置,初始化为下拉*/
	GPD1CON |= 0x22;
	GPD1PUD |= 0x5;

	/*I2CCON配置,bit[3:0]=0xf,PCLK=66.7MHz,PCLK=I2CLK/(bit[3:0]+1),I2CLK=4.1MHz*/
	I2CCON0 = (0x1<<7)|(0x0<<6)|(0x1<<5)|(0xf);


	/*I2CSTAT配置,串行输出使能*/
	I2CSTAT0 = 0x10;


	// IIC 中断清除
	I2CCON0 &= ~(1<<4);


	// 设置iic中断的中断处理函数
	intc_setvectaddr(NUM_I2C,isr_i2c0);

	// 使能iic中断
	intc_enable(NUM_I2C);
}


/* 需要根据AT24Cxx芯片手册的读字节协议 */
/*
unsigned char at24cxx_read(unsigned char address)
{
	unsigned char val;
	i2c0_write(0xA0, &address, 1);	//AT24C02的设备地址为0xa0
	i2c0_read(0xA0, (unsigned char *)&val, 1);
	return val;
}

*/

/* 需要根据AT24Cxx芯片手册的写字节协议 */
/*
void at24cxx_write(unsigned char address, unsigned char data)
{
	unsigned char val[2];
	val[0] = address;
	val[1] = data;
	i2c0_write(0xA0, val, 2);
}
*/
