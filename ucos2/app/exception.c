#include "stdio.h"
#include "exception.h"

void enable_irq(void)
{
	asm volatile (
		"mrs r4,cpsr\n\t"
		"bic r4,r4,#0x80\n\t"
		"msr cpsr,r4\n\t"
		:::"r4"
	);
}

void disable_irq(void)
{
	asm volatile (
		"mrs r4,cpsr\n\t"
		"orr r4,r4,#0x80\n\t"
		"msr cpsr,r4\n\t"
		:::"r4"
	);
}

void undef_handle(void)
{
	printf("undefined instruction exception.\n");
	while(1);
}

void fiq_handle(void)
{
	printf("fiq exception.\n");
	while(1);
}

void pabort_handle(void)
{
	unsigned int temp;
	printf("pabort exception.\n");
		__asm__ __volatile__(
	   "mov %0, r14\n"
		:"=r"(temp)
		:
		);
		printf("pabort lr=0x%x\r\n",temp);
	while(1);
}

void dabort_handle(void)
{

	printf("dabort exception.\n");
	while(1);
}

void reserved_handle(void)
{

	printf("reserved exception.\n");
	while(1);
}

void swi_handle(void)
{
	printf("swi exception.\n");
	while(1);
}

// 初始化中断控制器  
void intc_init(void)
{
	// 禁止所有中断
	VIC0INTENCLEAR = 0xffffffff;
	VIC1INTENCLEAR = 0xffffffff;
	VIC2INTENCLEAR = 0xffffffff;
	VIC3INTENCLEAR = 0xffffffff;

	// 选择中断类型为IRQ
	VIC0INTSELECT = 0x0;
	VIC1INTSELECT = 0x0;
	VIC2INTSELECT = 0x0;
	VIC3INTSELECT = 0x0;

	// 清VICxADDR
	intc_clearvectaddr();
}

// 保存需要处理的中断的中断处理函数的地址  
void intc_setvectaddr(unsigned long intnum, void (*handler)(void))
{
	//VIC0
	if(intnum<32)			
	{
		*( (volatile unsigned long *)(VIC0VECTADDR + 4*intnum) ) = (unsigned)handler;
	}
	//VIC1
	else if(intnum<64) 		
	{
		*( (volatile unsigned long *)(VIC1VECTADDR + 4*(intnum-32)) ) = (unsigned)handler;
	}
	//VIC2
	else if(intnum<96) 			
	{
		*( (volatile unsigned long *)(VIC2VECTADDR + 4*(intnum-64)) ) = (unsigned)handler;
	}
	//VIC3
	else	
	{
		*( (volatile unsigned long *)(VIC3VECTADDR + 4*(intnum-96)) ) = (unsigned)handler;
	}
	return;
}
// 清除需要处理的中断的中断处理函数的地址  
void intc_clearvectaddr(void)
{
	// VICxADDR:当前正在处理的中断的中断处理函数的地址
	VIC0ADDR = 0;
	VIC1ADDR = 0;
	VIC2ADDR = 0;
	VIC3ADDR = 0;
}
void intc_enable(unsigned long intnum)
{
	unsigned long temp;
	if(intnum<32)
	{
		temp = VIC0INTENABLE;
		temp |= (1<<intnum);
		VIC0INTENABLE = temp;
	}
	else if(intnum<64)
	{
		temp = VIC1INTENABLE;
		temp |= (1<<(intnum-32));
		VIC1INTENABLE = temp;
	}
	else if(intnum<96)
	{
		temp = VIC2INTENABLE;
		temp |= (1<<(intnum-64));
		VIC2INTENABLE = temp;
	}
	else if(intnum<NUM_ALL)
	{
		temp = VIC3INTENABLE;
		temp |= (1<<(intnum-96));
		VIC3INTENABLE = temp;
	}
	// NUM_ALL : enable all interrupt
	else 
	{
		VIC0INTENABLE = 0xFFFFFFFF;
		VIC1INTENABLE = 0xFFFFFFFF;
		VIC2INTENABLE = 0xFFFFFFFF;
		VIC3INTENABLE = 0xFFFFFFFF;
	}

}
void intc_disable(unsigned long intnum)
{
	unsigned long temp;

	if(intnum<32)
	{
		temp = VIC0INTENCLEAR;
		temp |= (1<<intnum);
		VIC0INTENCLEAR = temp;
	}
	else if(intnum<64)
	{
		temp = VIC1INTENCLEAR;
		temp |= (1<<(intnum-32));
		VIC1INTENCLEAR = temp;
	}
	else if(intnum<96)
	{
		temp = VIC2INTENCLEAR;
		temp |= (1<<(intnum-64));
		VIC2INTENCLEAR = temp;
	}
	else if(intnum<NUM_ALL)
	{
		temp = VIC3INTENCLEAR;
		temp |= (1<<(intnum-96));
		VIC3INTENCLEAR = temp;
	}
	// NUM_ALL : disable all interrupt
	else 
	{
		VIC0INTENCLEAR = 0xFFFFFFFF;
		VIC1INTENCLEAR = 0xFFFFFFFF;
		VIC2INTENCLEAR = 0xFFFFFFFF;
		VIC3INTENCLEAR = 0xFFFFFFFF;
	}

	return;
}
unsigned long intc_getvicirqstatus(unsigned long ucontroller)
{
	if(ucontroller == 0)
		return	VIC0IRQSTATUS;
	else if(ucontroller == 1)
		return 	VIC1IRQSTATUS;
	else if(ucontroller == 2)
		return 	VIC2IRQSTATUS;
	else if(ucontroller == 3)
		return 	VIC3IRQSTATUS;
	else
	{}
	return 0;
}
void irq_handler(void)
{
	unsigned long vicaddr[4] = {VIC0ADDR,VIC1ADDR,VIC2ADDR,VIC3ADDR};
	int i=0;
	void (*isr)(void) = NULL;

	for(; i<4; i++)
	{
		if(intc_getvicirqstatus(i) != 0)
		{
			isr = (void (*)(void)) vicaddr[i];
			break;
		}
	}

	(*isr)();
}

void system_initexception( void)
{
	setup_vector_base();
	// 初始化中断控制器
	intc_init();
}

extern unsigned int _interrupt_vector_table;
void setup_vector_base(void)
{
	volatile unsigned int * vector_table_ptr = &_interrupt_vector_table;

	// Update the interrupt vector address
	_set_interrupt_vector(vector_table_ptr);
}