#include "stdio.h"
void target_init()
{
	// 初始化时钟
	clock_init();	
	// 初始化串口
    uart_init();
    // 测试串口
	printf("main init\r\n");															
}
