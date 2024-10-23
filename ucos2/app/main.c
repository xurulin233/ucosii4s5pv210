#include "led.h"
#include "stdio.h"
#include "ucos_ii.h"
#include "app_cfg.h"
#include "exception.h"
#include "target.h"
#include "lcd.h"
#include "ugui.h"
#include "key.h"
#include "rtc.h"
//#include "nand.h"


// 设置栈
OS_STK  MainTaskStk[MainTaskStkLengh];
OS_STK	Task0Stk[Task0StkLengh];
OS_STK	Task1Stk[Task1StkLengh];

//信号量
OS_EVENT *sem;

#define		lcdtype					0x3  //屏幕型号S70
extern unsigned char *gImage_bmp;

//消息队列
#define QUEUE_NUM 32 // 定义信息数量

static OS_EVENT * _msgQueue; //消息队列句柄
static void * _msgQueueCache[QUEUE_NUM]; //消息队列缓存区指针数值，每个元素指向一条消息,每条消息的内存最好是全局变量


/* GUI structure */
UG_GUI gui;
rtc_time now_time;


/* Some defines */
#define MAX_OBJECTS        10


/* Window 1 */
UG_WINDOW window_1;
UG_OBJECT obj_buff_wnd_1[MAX_OBJECTS];

//rtc
char *week_name[7]={ "SUN","MON", "TUES", "WED", "THURS","FRI", "SAT" };


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

	//lcd init
	lcd_init(lcdtype);
	lcd_clear_screen(0x0);

	//key init
	Button_Init();

	//rtc init
	//rtc_init();
	RTC_Read(&now_time);
   //	printf("now time is: %d-%02d-%02d  %02d:%02d:%02d %s\r\n",now_time.year,
	//now_time.month,now_time.date,now_time.hour,now_time.minute,
	//now_time.second,week_name[now_time.week_day - 1]);
	rtc_ticktime_enable(1);

	//iic
	i2c0_init();
	unsigned char dat[256] = "xurulin hello";	 //发送数据域
	unsigned char rev[256] = {0};	 //接收数据域
	i2c0_write(0, dat, 16);  // 从第0页发送数据
	printf("--------------------------\r\n");
	i2c0_read(0, rev, 16);  // 从第0页读数据
	printf("%s\r\n", rev);	  // 打印数据


	//nandflash
	unsigned char buf[5];
	unsigned char data_buf[2048];
	nand_init();
	nand_read_id(buf);
	printf("Maker  Code 0x%x\r\n",buf[0]);
	printf("Device Code 0x%x\r\n",buf[1]);
	printf("3rd    Code θx%x\r\n",buf[2]);
	printf("4th    Code 0x%x\r\n",buf[3]);

	printf("Page   Size %dKB\r\n", (1 <<  ((buf[3] >>  0) & (0x03)) ) );
	printf("Block  Size %dKB\r\n", (64 << ((buf[3] >>  4) & (0x03)) ) );

	printf("5th    Code 0x%x\r\n",buf[4]);
	nand_read_page(0,data_buf);
	int i;
	for(i=0;i<2048;i++)
	{
		printf("%02x",data_buf[i]);
		if(i%16 == 0)
		printf("\r\n");
	}
//	UG_Init(&gui,(void(*)(UG_S16,UG_S16,UG_COLOR))lcd_draw_pixel,480,800);

//	UG_SelectGUI(&gui);



	/* Register hardware acceleration */
//	UG_DriverRegister( DRIVER_DRAW_LINE, (void*)lcd_draw_line );
//	UG_DriverRegister( DRIVER_FILL_FRAME, (void*)lcd_fill_frame );
//	UG_DriverEnable( DRIVER_DRAW_LINE );
//	UG_DriverEnable( DRIVER_FILL_FRAME );

//	UG_FillFrame(0,0,200,400,C_YELLOW);
//	printf("@@@@@ UG_FillFrame C_YELLOW\n");
	//UG_DrawCircle(200,400,90,C_LIGHT_GREEN);

	/* Create Window 1 */
//	UG_WindowCreate( &window_1, obj_buff_wnd_1, MAX_OBJECTS, NULL );
//	UG_WindowSetTitleText( &window_1, "GUI" );
//	UG_WindowSetTitleTextFont( &window_1, &FONT_12X20 );

//	UG_WindowShow( &window_1 );


//UG_Update();


	//lcd_draw_bmp(gImage_bmp,lcdtype);

	//创建信号量
	//sem = OSSemCreate(1);


	//创建消息队列。 第一个参数指定消息队列缓存区的起始地址；第二个参数指定缓存区的大小，即消息的数量
	_msgQueue = OSQCreate(&_msgQueueCache[0], QUEUE_NUM);

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


int isMsgInQueue(OS_EVENT *pevent, void *msg)
{
    if(pevent == NULL || msg == NULL)
    {
        return -1;
    }

    OS_Q *pq = NULL;
    pq = (OS_Q *)pevent->OSEventPtr;

    if(pq->OSQEntries == 0){
        return 0;
    }

    void ** msg_q = pq->OSQOut;
    while(msg_q != pq->OSQIn)
    {
        if(*msg_q == msg){
            return 1;
        }
        msg_q++;
        (msg_q == pq->OSQEnd) ? (msg_q = pq->OSQStart) : 0;
    }

    return 0;
}
/**
 * @brief 发送一条消息到缓存区，去重处理
 * @param[msg] 指向一条消息的地址，这个地址所在内存最好是全局变量
 * @return 1表示成功, 其它失败
 */
int sendMsgQueue(void * msg)
{
    if(isMsgInQueue(_msgQueue, msg))
    {
        return 1;
    }
    return (OSQPost(_msgQueue, msg) == OS_NO_ERR);
}
void Task0(void *pdata)
{
	char msg[256];
	unsigned int msg_num;

		while(1)
		{
			sprintf(msg,"hello task1 %d",msg_num);
			if( sendMsgQueue(msg) == 1)
			{
				/*正常获取到一条消息*/
				//printf("@@@ task0 send msg %s \n",msg);
				msg_num++;
			}
			 OSTimeDlyHMSM (0, 0,1, 0);
		}


/*
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
*/
/*
		printf("led on\r\n");
		led1();
		delay1();
		printf("led on\r\n");
*/

	    //printf("Enter Task1\r\n");
		//OSTaskSuspend(Task0Prio);
	    //OSTimeDly(OS_TICKS_PER_SEC/5);
//	}
}



void Task1(void *pdata)
{
	INT8U err = OS_NO_ERR;
	char * msg = NULL;
		while(1)
		{
			msg = (char *)OSQPend(_msgQueue, 0, &err);
			if(msg != NULL && err == OS_NO_ERR)
			{
				/*正常获取到一条消息*/
				printf("@@@receive msg %s\r\n",msg);
			}

			OSTimeDlyHMSM(0, 0,1, 0);
		}

	/*
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
*/

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

//	}
}




int raise(int param)
{
    return 0;
}

