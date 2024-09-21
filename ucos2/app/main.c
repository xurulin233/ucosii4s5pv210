#include "led.h"
#include "stdio.h"
#include "ucos_ii.h"
#include "app_cfg.h"
#include "exception.h"
#include "target.h"

// 设置栈
OS_STK  MainTaskStk[MainTaskStkLengh];
OS_STK	Task0Stk[Task0StkLengh];       															
OS_STK	Task1Stk[Task1StkLengh];  

int main(void)
{
	
	// 初始化单板，主要就是初始化时钟和串口
	target_init();

	// 初始化内核，主要就是初始化所有的变量和数据结构
    OSInit();
    // 初始化tick time
    OSTimeSet(0);	
    // 将MainTask由睡眠态变成就绪态
    OSTaskCreate(MainTask,(void *)0, &MainTaskStk[MainTaskStkLengh - 1], MainTaskPrio);
    // 开始多任务调度，OSStart之前用户至少创建一个任务。
    // 此时有三个任务:空闲任务idle、统计任务stat、maintask(优先级最高)
    OSStart();																					
    for(;;)
    	;

	return 0;
}


void MainTask(void *pdata) 
{
#if OS_CRITICAL_METHOD == 3                                										
    OS_CPU_SR  cpu_sr;
#endif
	// 关闭中断
	OS_ENTER_CRITICAL();
	// 初始化中断
	system_initexception();																		
	// 初始化定时器,应该在开始多任务之后再初始化时钟节拍中断
    timer_init();		
    // 打开中断
	OS_EXIT_CRITICAL();
		
    // 初始化统计任务。
    // 如果要使用统计任务，则在用户的第一个任务里需调用OSStatInit函数
    //OSStatInit();																				
    // 创建用户应用程序任务
    OSTaskCreate(Task0,(void *)0, &Task0Stk[Task0StkLengh - 1], Task0Prio);			
    OSTaskCreate(Task1,(void *)0, &Task1Stk[Task1StkLengh - 1], Task1Prio);
 
    while(1)
    {
	    printf("Enter Maintask\r\n");	
        OSTimeDly(OS_TICKS_PER_SEC);
    }
}

void Task0(void *pdata)
{		
	while (1)
	{
	    printf("Enter Task1\r\n");   	
	    OSTimeDly(OS_TICKS_PER_SEC/5);
	}
}



void Task1(void *pdata)
{		
	while (1)
	{
	    printf("Enter Task2\r\n");	
	    OSTimeDly(OS_TICKS_PER_SEC/5);
	}
}

