#include "led.h"
#include "stdio.h"
#include "ucos_ii.h"
#include "app_cfg.h"
#include "exception.h"
#include "target.h"
#include "lcd.h"
#include "ugui.h"



// 设置栈
OS_STK  MainTaskStk[MainTaskStkLengh];
OS_STK	Task0Stk[Task0StkLengh];
OS_STK	Task1Stk[Task1StkLengh];

//信号量
OS_EVENT *sem;

#define		lcdtype					0x3  //屏幕型号S70
extern unsigned char *gImage_bmp;


/* GUI structure */
UG_GUI gui;


/* Some defines */
#define MAX_OBJECTS        10


/* Window 1 */
UG_WINDOW window_1;
UG_OBJECT obj_buff_wnd_1[MAX_OBJECTS];



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
    OSStatInit();
    // 创建用户应用程序任务

	//lcd
	lcd_init(lcdtype);
	lcd_clear_screen(0x0);

	UG_Init(&gui,(void(*)(UG_S16,UG_S16,UG_COLOR))lcd_draw_pixel,480,800);

	UG_SelectGUI(&gui);
	printf("@@@@@ UG_Init\n");



	/* Register hardware acceleration */
	UG_DriverRegister( DRIVER_DRAW_LINE, (void*)lcd_draw_line );
	UG_DriverRegister( DRIVER_FILL_FRAME, (void*)lcd_fill_frame );
	UG_DriverEnable( DRIVER_DRAW_LINE );
	UG_DriverEnable( DRIVER_FILL_FRAME );

	UG_FillFrame(0,0,200,400,C_YELLOW);
	printf("@@@@@ UG_FillFrame C_YELLOW\n");
	//UG_DrawCircle(200,400,90,C_LIGHT_GREEN);

	/* Create Window 1 */
	UG_WindowCreate( &window_1, obj_buff_wnd_1, MAX_OBJECTS, NULL );
	UG_WindowSetTitleText( &window_1, "GUI" );
	UG_WindowSetTitleTextFont( &window_1, &FONT_12X20 );

	UG_WindowShow( &window_1 );


	UG_Update();

	//lcd_draw_bmp(gImage_bmp,lcdtype);

	//创建信号量
	sem = OSSemCreate(1);

    OSTaskCreate(Task0,(void *)0, &Task0Stk[Task0StkLengh - 1], Task0Prio);
    OSTaskCreate(Task1,(void *)0, &Task1Stk[Task1StkLengh - 1], Task1Prio);

    while(1)
    {
	    printf("Enter Maintask\r\n");
		OSTaskSuspend(MainTaskPrio);
        //OSTimeDly(OS_TICKS_PER_SEC);
    }
}


void delay1(void)
{
	volatile unsigned int i = 9000000;		// volatile 让编译器不要优化，这样才能真正的减
	while (i--);							// 才能消耗时间，实现delay
}


void Task0(void *pdata)
{
	INT8U err;
	while (1)
	{

		OSSemPend(sem,0,&err);
		if (err == OS_NO_ERR) {
			printf("led on\r\n");
			led1();
			delay1();
			printf("led onn\r\n");
			OSSemPost(sem);
			}
		OSTimeDly(10);

/*
		printf("led on\r\n");
		led1();
		delay1();
		printf("led on\r\n");
*/

	    //printf("Enter Task1\r\n");
		//OSTaskSuspend(Task0Prio);
	    //OSTimeDly(OS_TICKS_PER_SEC/5);
	}
}



void Task1(void *pdata)
{
	INT8U err;
	while (1)
	{

		OSSemPend(sem, 0, &err); // 等待获取信号量
		if (err == OS_NO_ERR) {
			printf("led off \r\n");
			led_off();
			delay1();
			printf("led offf \r\n");
			OSSemPost(sem); // 释放信号量
		}
		OSTimeDly(10);

/*
		printf("led off \r\n");
		led_off();
		delay1();
		printf("led offf \r\n");
		OSTimeDly(10);
*/
    //printf("Enter Task2\r\n");
	//OSTaskResume(Task0Prio);
	//printf("OSTaskResume Task1 end\r\n");
    //OSTimeDly(OS_TICKS_PER_SEC/5);
    //printf("OSCPUUsage = %d % \r\n",OSCPUUsage);
    //OSTimeDly(OS_TICKS_PER_SEC/200);

	}
}




int raise(int param)
{
    return 0;
}

