#ifndef __CLOCK_H
#define __CLOCK_H

// 时钟相关寄存器
#define APLL_LOCK 			( *((volatile unsigned long *)0xE0100000) )		
#define MPLL_LOCK 			( *((volatile unsigned long *)0xE0100008) )
#define EPLL_LOCK           ( *((volatile unsigned long *)0xE0100010) )

#define APLL_CON0 			( *((volatile unsigned long *)0xE0100100) )
#define APLL_CON1 			( *((volatile unsigned long *)0xE0100104) )
#define MPLL_CON 			( *((volatile unsigned long *)0xE0100108) )
#define EPLL_CON0           ( *((volatile unsigned long *)0xE0100110) )
#define EPLL_CON1           ( *((volatile unsigned long *)0xE0100114) )

#define CLK_SRC0 			( *((volatile unsigned long *)0xE0100200) )
#define CLK_SRC1 			( *((volatile unsigned long *)0xE0100204) )
#define CLK_SRC2 			( *((volatile unsigned long *)0xE0100208) )
#define CLK_SRC3 			( *((volatile unsigned long *)0xE010020c) )
#define CLK_SRC4 			( *((volatile unsigned long *)0xE0100210) )
#define CLK_SRC5 			( *((volatile unsigned long *)0xE0100214) )
#define CLK_SRC6 			( *((volatile unsigned long *)0xE0100218) )
#define CLK_SRC_MASK0 		( *((volatile unsigned long *)0xE0100280) )
#define CLK_SRC_MASK1 		( *((volatile unsigned long *)0xE0100284) )

#define CLK_DIV0 			( *((volatile unsigned long *)0xE0100300) )
#define CLK_DIV1 			( *((volatile unsigned long *)0xE0100304) )
#define CLK_DIV2 			( *((volatile unsigned long *)0xE0100308) )
#define CLK_DIV3 			( *((volatile unsigned long *)0xE010030c) )
#define CLK_DIV4 			( *((volatile unsigned long *)0xE0100310) )
#define CLK_DIV5 			( *((volatile unsigned long *)0xE0100314) )
#define CLK_DIV6 			( *((volatile unsigned long *)0xE0100318) )
#define CLK_DIV7 			( *((volatile unsigned long *)0xE010031c) )

#endif
