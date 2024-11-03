#include "sdio_sdcard.h"
#include "clock.h"
#include "delay.h"
#include "stdio.h"
#include "led.h"
#include "int.h"

#define MAX_BLOCK  65535


#define		GPG0CON		(*(volatile unsigned long *) 0xE02001A0)
#define		GPG0PUD		(*(volatile unsigned long *) 0xE02001A8)
#define		GPG0DRV		(*(volatile unsigned long *) 0xE02001AC)


static uint8_t card_type; 

static uint32_t RCA; 

///////////////////////////////////////////////////////////////////////////////////////////////////
void hsmmc0_isr(void)
{
	printf("hsmmc_isr\r\n");
	
	if (PRNSTS0 & (1 << INSCARD))
	{
		printf("sdcard inserted..\r\n");
		NORINTSTS0 |= (1 << STACARDINS);
		//led_set(LED3, ON);
	}
	else
	{
		printf("sdcard removed..\r\n");
		NORINTSTS0 |= (1 << STACARDREM);
		//led_set(LED3, OFF);
	}
	
	// clear VIC1ADDR
	intc_clearvectaddr();					
	// clear pending bit	
					
}

static void sd_clock_on(bool_t on_off)
{
	uint32_t timeout;
	if (on_off)
	{
		CLKCON0 |= (1 << 2); // sd时钟使能
		timeout = 1000; // Wait max 10 ms
		while (!(CLKCON0 & (1 << 3)))
		{
			// 等待SD输出时钟稳定
			if (timeout == 0)
			{
				return;
			}
			timeout--;
			delay_us(10);
		}
	} 
	else 
	{
		CLKCON0 &= ~(1 << 2); // sd时钟禁止
	}
}

static void sd_clock_set(uint32_t clock)
{
	uint32_t temp;
	uint32_t timeout;
	uint32_t i;
	sd_clock_on(0); // 关闭时钟	
	temp = CONTROL2_0;
	// Set SCLK_MMC(48M) from SYSCON as a clock source	
	temp = (temp & (~(3 << 4))) | (2 << 4);
	temp |= (1u << 31) | (1u << 30) | (1 << 8);
	if (clock <= 500000) 
	{
		temp &= ~((1 << 14) | (1 << 15));
		CONTROL3_0 = 0;
	} 
	else 
	{
		temp |= ((1 << 14) | (1 << 15));
		CONTROL3_0 = (1u << 31) | (1 << 23);
	}
	CONTROL2_0 = temp;
	
	for (i=0; i<=8; i++) 
	{
		if (clock >= (48000000 / (1 << i))) 
		{
			break;
		}
	}
	temp = ((1 << i) / 2) << 8; // clock div
	temp |= (1 << 0); // Internal Clock Enable
	CLKCON0 = temp;
	timeout = 1000; // Wait max 10 ms

	while (!(CLKCON0 & (1 << 1))) 
	{
		// 等待内部时钟振荡稳定
		if (timeout == 0) 
		{
			return;
		}
		timeout--;
		delay_us(10);
	}
	
	sd_clock_on(1); // 使能时钟
}

//SD CLOCK SUPPLY SEQUENCE
static void sd_clock_supply(bool_t on_off)
{
	if (on_off)
	{
		sd_clock_set(400000);//400k
	}
	else
	{
		sd_clock_on(0);
	}	
}

static int32_t sd_wait_for_buffer_write_ready(void)
{
	int32_t error_state;
	while (1) {
		if (NORINTSTS0 & (1<<15)) { // 出现错误
			break;
		}
		if (NORINTSTS0 & (1<<4)) { // 写缓存准备好
			NORINTSTS0 |= (1<<4); // 清除准备好标志
			return 0;
		}				
	}
			
	error_state = ERRINTSTS0 & 0x1ff; // 可能通信错误,CRC检验错误,超时等
	NORINTSTS0 = NORINTSTS0; // 清除中断标志
	ERRINTSTS0 = ERRINTSTS0; // 清除错误中断标志
	printf("Write buffer error, NORINTSTS: %04x\r\n", error_state);
	return error_state;
}

static int32_t sd_wait_for_command_done(void)
{
	uint32_t i;	
	int32_t error_state;
	// 等待命令发送完成
	for (i=0; i<0x20000000; i++) 
	{
		if (NORINTSTS0 & (1 << 15)) 
		{ // 出现错误
			break;
		}
		if (NORINTSTS0 & (1 << 0)) 
		{
			do 
			{
				NORINTSTS0 = ( 1<< 0); // 清除命令完成位
			} while (NORINTSTS0 & (1<<0));				
			return 0; // 命令发送成功
		}
	}
	
	error_state = ERRINTSTS0 & 0x1ff; // 可能通信错误,CRC检验错误,超时等
	NORINTSTS0 = NORINTSTS0; // 清除中断标志
	ERRINTSTS0 = ERRINTSTS0; // 清除错误中断标志	
	do 
	{
		NORINTSTS0 = (1 << 0); // 清除命令完成位
	} while (NORINTSTS0 & (1 << 0));
	
	printf("Command error, ERRINTSTS = 0x%x ", error_state);	
	return error_state; // 命令发送出错	
}

static int32_t sd_issue_command(uint8_t cmd, uint32_t arg, uint8_t data, uint8_t response)
{
	uint32_t i;
	uint32_t value;
	uint32_t error_state;

	// 检查cmd线是否准备好发送命令
	for (i=0; i<0x1000000; i++) 
	{
		if (!(PRNSTS0 & (1 <<0))) 
		{
			break;
		}
	}
	if (i == 0x1000000) 
	{
		printf("cmd line time out, PRNSTS: %04x\r\n", PRNSTS0);
		return -1; // 命令超时
	}
	
	// 检查DAT线是否准备好
	if (response == CMD_RESP_R1B) 
	{ // R1b回复通过DAT0反馈忙信号
		for (i=0; i<0x10000000; i++) 
		{
			if (!(PRNSTS0 & (1 << 1))) 
			{
				break;
			}		
		}
		if (i == 0x10000000) 
		{
			printf("data line time out, PRNSTS: %04x\r\n", PRNSTS0);			
			return -2;
		}		
	}
	
	ARGUMENT0 = arg; // 写入命令参数
	
	value = (cmd << 8); // command index
	// CMD12可终止传输，？？？是0x12？ 而不是 12？？？
	if (cmd == 12) //0x12
	{
		value |= (0x3 << 6); // command type
	}
	if (data) 
	{
		value |= (1 << 5); // 需使用DAT线作为传输等
	}	
	
	switch (response) 
	{
		case CMD_RESP_NONE:
			value |= (0 << 4) | (0 << 3) | 0x0; // 没有回复,不检查命令及CRC
			break;
		case CMD_RESP_R1:
		case CMD_RESP_R5:
		case CMD_RESP_R6:
		case CMD_RESP_R7:		
			value |= (1 << 4) | (1 << 3) | 0x2; // 检查回复中的命令,CRC
			break;
		case CMD_RESP_R2:
			value |= (0 << 4) | (1 << 3) | 0x1; // 回复长度为136位,包含CRC
			break;
		case CMD_RESP_R3:
		case CMD_RESP_R4:
			value |= (0 << 4) | (0 << 3) | 0x2; // 回复长度48位,不包含命令及CRC
			break;
		case CMD_RESP_R1B:
			value |= (1 << 4) | (1 << 3) | 0x3; // 回复带忙信号,会占用Data[0]线
			break;
		default:
			break;	
	}
	
	CMDREG0 = value;
	
	error_state = sd_wait_for_command_done();
	if (error_state) 
	{
		printf("Command = %d\r\n", cmd);
	}	
	return error_state; // 命令发送出错
}

static int32_t sd_wait_for_buffer_read_ready(void)
{	
	int32_t error_state;
	while (1) 
	{
		if (NORINTSTS0 & (1<<15)) 
		{ // 出现错误
			break;
		}
		if (NORINTSTS0 & (1<<5)) 
		{ // 读缓存准备好
			NORINTSTS0 |= (1<<5); // 清除准备好标志
			return 0;
		}				
	}
			
	error_state = ERRINTSTS0 & 0x1ff; // 可能通信错误,CRC检验错误,超时等
	NORINTSTS0 = NORINTSTS0; // 清除中断标志	
	ERRINTSTS0 = ERRINTSTS0; // 清除错误中断标志
	printf("Read buffer error, NORINTSTS: %04x\r\n", error_state);
	return error_state;
}

static int32_t sd_wait_for_transfer_done(void)
{
	int32_t error_state;
	uint32_t i;
	// 等待数据传输完成
	for (i=0; i<0x20000000; i++) 
	{
		if (NORINTSTS0 & (1<<15)) 
		{ // 出现错误
			break;
		}											
		if (NORINTSTS0 & (1<<1)) 
		{ // 数据传输完								
			do 
			{
				NORINTSTS0 |= (1<<1); // 清除传输完成位
			} while (NORINTSTS0 & (1<<1));									
			return 0;
		}
	}

	error_state = ERRINTSTS0 & 0x1ff; // 可能通信错误,CRC检验错误,超时等
	NORINTSTS0 = NORINTSTS0; // 清除中断标志
	ERRINTSTS0 = ERRINTSTS0; // 清除错误中断标志
	printf("Transfer error, rHM1_ERRINTSTS = 0x%04x\r\n", error_state);	
	do {
		NORINTSTS0 = (1<<1); // 出错后清除数据完成位
	} while (NORINTSTS0 & (1<<1));
	
	return error_state; // 数据传输出错		
}

// Reads the SD Configuration Register (SCR).
int32_t sd_get_scr(SD_SCR *pSCR)
{
	uint8_t *pBuffer;
	int32_t error_state;
	uint32_t temp;
	uint32_t i;
	sd_issue_command(CMD55, RCA<<16, 0, CMD_RESP_R1); 
	BLKSIZE0 = (7 << 12) | (8 << 0); // 最大DMA缓存大小,block为64位8字节			
	BLKCNT0 = 1; // 写入这次读1 block的sd状态数据
	ARGUMENT0 = 0; // 写入命令参数	

	// DMA禁能,读单块		
	TRNMOD0 = (0 << 5) | (1 << 4) | (0 << 2) | (1 << 1) | (0 << 0);	
	// 设置命令寄存器,read SD Configuration CMD51,R1回复
	CMDREG0 = (CMD51<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;
	error_state = sd_wait_for_command_done();
	if (error_state) 
	{
		printf("CMD51 error\r\n");
		return error_state;
	}

	error_state = sd_wait_for_buffer_read_ready();
	if (error_state) 
	{
		return error_state;
	}
	
	// Wide width data (SD Memory Register)
	pBuffer = (uint8_t *)pSCR + sizeof(SD_SCR) - 1;
	for (i=0; i<8/4; i++) 
	{
		temp = BDATA0;
		*pBuffer-- = (uint8_t)temp;
		*pBuffer-- = (uint8_t)(temp>>8);
		*pBuffer-- = (uint8_t)(temp>>16);
		*pBuffer-- = (uint8_t)(temp>>24);		
	}
	
	error_state = sd_wait_for_transfer_done();
	if (error_state) 
	{
		printf("Get SCR register error\r\n");
		return error_state;
	}

	return 0;
}

int32_t sd_switch(uint8_t mode, int32_t group, int32_t function, uint8_t *pStatus)
{
	int32_t error_state;
	int32_t temp;
	uint32_t i;
	
	BLKSIZE0 = (7<<12) | (64<<0); // 最大DMA缓存大小,block为512位64字节			
	BLKCNT0 = 1; // 写入这次读1 block的sd状态数据
	temp = (mode << 31U) | 0xffffff;
	temp &= ~(0xf<<(group * 4));
	temp |= function << (group * 4);
	ARGUMENT0 = temp; // 写入命令参数	

	// DMA禁能,读单块		
	TRNMOD0 = (0<<5) | (1<<4) | (0<<2) | (1<<1) | (0<<0);	
	// 设置命令寄存器,SWITCH_FUNC CMD6,R1回复
	CMDREG0 = (CMD6<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;
	error_state = sd_wait_for_command_done();
	if (error_state) 
	{
		printf("CMD6 error\r\n");
		return error_state;
	}

	error_state = sd_wait_for_buffer_read_ready();
	if (error_state) 
	{
		return error_state;
	}
	
	pStatus += 64 - 1;
	for (i=0; i<64/4; i++) 
	{
		temp = BDATA0;
		*pStatus-- = (uint8_t)temp;
		*pStatus-- = (uint8_t)(temp>>8);
		*pStatus-- = (uint8_t)(temp>>16);
		*pStatus-- = (uint8_t)(temp>>24);			
	}
		
	error_state = sd_wait_for_transfer_done();
	if (error_state) 
	{
		printf("Get sd status error\r\n");
		return error_state;
	}	
	return 0;	
}

static int32_t sd_set_bus_width(uint8_t width)
{
	int32_t state;
	if ((width != 1) || (width != 4)) 
	{
		return -1;
	}
	state = -1; // 设置初始为未成功
	NORINTSTSEN0 &= ~(1<<8); // 关闭卡中断
	sd_issue_command(CMD55, RCA<<16, 0, CMD_RESP_R1);
	if (width == 1) 
	{
		if (!sd_issue_command(CMD6, 0, 0, CMD_RESP_R1)) { // 1位宽
			HOSTCTL0 &= ~(1<<1);
			state = 0; // 命令成功
		}
	} else {
		if (!sd_issue_command(CMD6, 2, 0, CMD_RESP_R1)) { // 4位宽
			HOSTCTL0 |= (1<<1);
			state = 0; // 命令成功
		}
	}
	NORINTSTSEN0 |= (1<<8); // 打开卡中断	
	return state; // 返回0为成功
}

int32_t sd_get_card_state(void)
{
	if (sd_issue_command(CMD13, RCA<<16, 0, CMD_RESP_R1)) {
		return -1; // 卡出错
	} else {
		return ((RSPREG0_0>>9) & 0xf); // 返回R1回复中的[12:9]卡状态
	}
}

// Asks the selected card to send its cardspecific data
int32_t sd_get_csd(uint8_t *pCSD)
{
	uint32_t i;
	uint32_t response[4];
	int32_t state = -1;

	if (card_type != SD_HC && card_type != SD_V1 && card_type != SD_V2) 
	{
		return state; // 未识别的卡
	}
	// 取消卡选择,任何卡均不回复,已选择的卡通过RCA=0取消选择,
	// 卡回到stand-by状态
	sd_issue_command(CMD7, 0, 0, CMD_RESP_NONE);
	for (i=0; i<1000; i++) 
	{
		if (sd_get_card_state() == CARD_STBY) 
		{ // CMD9命令需在standy-by status				
			break; // 状态正确
		}
		delay_us(100);
	}
	if (i == 1000) 
	{
		return state; // 状态错误
	}	
	
	// 请求已标记卡发送卡特定数据(CSD),获得卡信息
	if (!sd_issue_command(CMD9, RCA<<16, 0, CMD_RESP_R2)) {
		pCSD++; // 跳过第一字节,CSD中[127:8]位对位寄存器中的[119:0]
		response[0] = RSPREG0_0;
		response[1] = RSPREG1_0;
		response[2] = RSPREG2_0;
		response[3] = RSPREG3_0;	
		for (i=0; i<15; i++) 
		{ // 拷贝回复寄存器中的[119:0]到pCSD中
			*pCSD++ = ((uint8_t *)response)[i];
		}	
		state = 0; // CSD获取成功
	}

	sd_issue_command(CMD7, RCA<<16, 0, CMD_RESP_R1); // 选择卡,卡回到transfer状态NONE
	return state;
}

//初始化SD卡
//返回值:错误代码;(0,无错误)
int sd_init(void)
{
	int errorstatus = 0;
	uint32_t timeout;
	uint32_t capacity;
	uint32_t i;
	uint32_t OCR;
	uint32_t temp;
	uint8_t switch_status[64];
	SD_SCR SCR;
	uint8_t CSD[16];
	uint32_t c_size, c_size_multi, read_bl_len;
	
	//IO管脚配置
	// channel 0,GPG0[0:6] = CLK, CMD, CDn, DAT[0:3]
	GPG0CON = 0x2222222;
	// pull up enable
	GPG0PUD = 0x2aaa;
	GPG0DRV= 0x3fff;
	
	//配置时钟源
	// channel 0 clock src = SCLKEPLL = 96M
	CLK_SRC4 = (CLK_SRC4 & (~(0xf << 0))) | (0x7 << 0);
	// channel 0 clock = SCLKEPLL/2 = 48M
	CLK_DIV4 = (CLK_DIV4 & (~(0xf << 0))) | (0x1 << 0);
	// software reset for all
	SWRST0 = 0x1;
	
	timeout = 1000; // Wait max 10 ms
	while (SWRST0 & (1 << 0)) 
	{
		if (timeout == 0) 
		{
			return -1; // reset timeout
		}
		timeout--;
		delay_us(10);
	}

	printf("set clock to 400k...\r\n");

	sd_clock_set(400000); // 400k
	TIMEOUTCON0 = 0xe; // 最大超时时间
	HOSTCTL0 &= ~(1 << 2); // 正常速度模式
	// 清除正常中断状态标志
	NORINTSTS0 = NORINTSTS0;
	// 清除错误中断状态标志
	ERRINTSTS0 = ERRINTSTS0;
	NORINTSTSEN0 = 0x7fff; // [14:0]中断状态使能
	ERRINTSTSEN0 = 0x3ff; // [9:0]错误中断状态使能
	NORINTSIGEN0 = 0x7fff; // [14:0]中断信号使能	
	ERRINTSIGEN0 = 0x3ff; // [9:0]错误中断信号使能
	
	printf("reset card to idle...\r\n");
	sd_issue_command(CMD0, 0, 0, CMD_RESP_NONE); // 复位所有卡到空闲状态	
		
	card_type = UNUSABLE; // 卡类型初始化不可用
	if (sd_issue_command(CMD8, 0x1aa, 0, CMD_RESP_R7)) 
	{ // 没有回复,MMC/sd v1.x/not card
			for (i=0; i<100; i++) 
			{
				sd_issue_command(CMD55, 0, 0, CMD_RESP_R1);
				if (!sd_issue_command(CMD41, 0, 0, CMD_RESP_R3)) 
				{ // CMD41有回复说明为sd卡
					OCR = RSPREG0_0; // 获得回复的OCR(操作条件寄存器)值
					if (OCR & 0x80000000) 
					{ // 卡上电是否完成上电流程,是否busy
						card_type = SD_V1; // 正确识别出sd v1.x卡
						printf("SD card version 1.x is detected\r\n");
						break;
					}
				} 
				else 
				{
					// MMC卡识别
					printf("MMC card is not supported\r\n");
					return -1;
				}
				delay_us(1000);				
			}
	} 
	else 
	{ // sd v2.0
		temp = RSPREG0_0;
		if (((temp & 0xff) == 0xaa) && (((temp >> 8) & 0xf) == 0x1)) 
		{ // 判断卡是否支持2.7~3.3v电压
			OCR = 0;
			for (i=0; i<100; i++) 
			{
				OCR |= (1 << 30);
				sd_issue_command(CMD55, 0, 0, CMD_RESP_R1);
				sd_issue_command(CMD41, OCR, 0, CMD_RESP_R3); // reday态
				OCR = RSPREG0_0;
				if (OCR & 0x80000000) 
				{ // 卡上电是否完成上电流程,是否busy
					if (OCR & (1 << 30)) 
					{ // 判断卡为标准卡还是高容量卡
						card_type = SD_HC; // 高容量卡
						printf("SDHC card is detected\r\n");
					} 
					else 
					{
						card_type = SD_V2; // 标准卡
						printf("SD version 2.0 standard card is detected\r\n");
					}
					break;
				}
				delay_us(1000);
			}
		}
	}
	if (card_type == SD_HC || card_type == SD_V1 || card_type == SD_V2) 
	{
		sd_issue_command(CMD2, 0, 0, CMD_RESP_R2); // 请求卡发送CID(卡ID寄存器)号,进入ident
		sd_issue_command(CMD3, 0, 0, CMD_RESP_R6); // 请求卡发布新的RCA(卡相对地址),Stand-by状态
		RCA = (RSPREG0_0 >> 16) & 0xffff; // 从卡回复中得到卡相对地址
		sd_issue_command(CMD7, RCA<<16, 0, CMD_RESP_R1); // 选择已标记的卡,transfer状态
		
		sd_get_scr(&SCR);
		if (SCR.SD_SPEC == 0) 
		{ // sd 1.0 - sd 1.01
			// Version 1.0 doesn't support switching
			sd_clock_set(24000000); // 设置SDCLK = 48M/2 = 24M			
		} 
		else 
		{ // sd 1.10 / sd 2.0
			temp = 0;
			for (i=0; i<4; i++) 
			{
				if (sd_switch(0, 0, 1, switch_status) == 0) 
				{ // switch check
					if (!(switch_status[34] & (1<<1))) 
					{ // group 1, function 1 high-speed bit 273					
						// The high-speed function is ready
						if (switch_status[50] & (1<<1)) 
						{ // group, function 1 high-speed support bit 401
							// high-speed is supported
							if (sd_switch(1, 0, 1, switch_status) == 0) 
							{ // switch
								if ((switch_status[47] & 0xf) == 1) 
								{ // function switch in group 1 is ok?
									// result of the switch high-speed in function group 1
									printf("Switch to high speed mode: CLK @ 50M\r\n");
									sd_clock_set(48000000); // 设置SDCLK = 48M	  会报错，原因还没找到
									//sd_clock_set(25000000); // 设置SDCLK = 48M	
									temp = 1;									
								}
							}
						}
						break;
					}
				} 
			}
			if (temp == 0) 
			{
				sd_clock_set(24000000); // 设置SDCLK = 48M/2 = 24M
			}
		}
			
		if (!sd_set_bus_width(4)) 
		{
			printf("Set bus width error\r\n");
			return -1; // 位宽设置出错
		}
		if (sd_get_card_state() == CARD_TRAN) 
		{ // 此时卡应在transfer态
			if (!sd_issue_command(CMD16, 512, 0, CMD_RESP_R1)) 
			{ // 设置块长度为512字节
				NORINTSTS0 = 0xffff; // 清除中断标志
				sd_get_csd(CSD);
				if ((CSD[15]>>6) == 0) 
				{ // CSD v1.0->sd V1.x, sd v2.00 standar
					read_bl_len = CSD[10] & 0xf; // [83:80]
					c_size_multi = ((CSD[6] & 0x3) << 1) + ((CSD[5] & 0x80) >> 7); // [49:47]
					c_size = ((int32_t)(CSD[9]&0x3) << 10) + ((int32_t)CSD[8]<<2) + (CSD[7]>>6); // [73:62]				
					capacity = (c_size + 1) << ((c_size_multi + 2) + (read_bl_len-9)); // block(512 byte)
				} 
				else 
				{
					c_size = ((CSD[8]&0x3f) << 16) + (CSD[7]<<8) + CSD[6]; // [69:48]
					// 卡容量为字节(c_size+1)*512K byte,以1扇区512 byte字,卡的扇区数为		
					capacity = (c_size+1) << 10;// block (512 byte)
				}
				printf("Card Initialization succeed\r\n");				
				printf("capacity: %ldMB\r\n", capacity / (1024*1024 / 512));
				return 0; // 初始化成功							
			}
		}
	}
	printf("Card Initialization failed\r\n");
	return -1; // 卡工作异常	

#if 0	
	//SD CARD DETECTION SEQUENCE
	NORINTSTSEN0 |= (1 << ENSTACARDREM) | (1 << ENSTACARDNS);
	NORINTSIGEN0 |= (1 << ENSIGCARDREM) | (1 << ENSIGCARDNS);
	
	//NORINTSTS0 |= (1 << STACARDREM);
	//NORINTSTS0 |= (1 << STACARDINS);

	// 设置中断HSMMC0的处理函数
	intc_setvectaddr(NUM_HSMMC0, hsmmc0_isr);	
	// 使能中断HSMMC0
	intc_enable(NUM_HSMMC0);
	 

	return errorstatus;
#endif	
}

int32_t sd_erase_block(uint32_t start_block, uint32_t end_block)
{
	uint32_t i;
	uint32_t ret;

	if (card_type == SD_V1 || card_type == SD_V2) 
	{
		start_block <<= 9; // 标准卡为字节地址
		end_block <<= 9;
	} 
	else if (card_type != SD_HC) 
	{
		return -1; // 未识别的卡
	}	
	ret = sd_issue_command(CMD32, start_block, 0, CMD_RESP_R1);
//	printf("ret: %d\r\n", ret);	
	ret = sd_issue_command(CMD33, end_block, 0, CMD_RESP_R1);
//	printf("ret: %d\r\n", ret);	
	if (!sd_issue_command(CMD38, 0, 0, CMD_RESP_R1B)) {
		for (i=0; i<0x10000; i++) 
		{
			if (sd_get_card_state() == CARD_TRAN) 
			{ // 擦除完成后返回到transfer状态
				printf("Erasing complete!\r\n");
				return 0; // 擦除成功				
			}
			delay_us(1000);			
		}		
	}		

	printf("Erase block failed\r\n");
	return -1; // 擦除失败
}

int32_t sd_read_block(uint8_t *pBuffer, uint32_t block_addr, uint32_t block_number)
{
	uint32_t address = 0;
	uint32_t read_block;
	uint32_t i;
	uint32_t j;
	int32_t error_state;
	uint32_t temp;
	
	if (pBuffer == 0 || block_number == 0) {
		return -1;
	}

	NORINTSTS0 = NORINTSTS0; // 清除中断标志
	ERRINTSTS0 = ERRINTSTS0; // 清除错误中断标志	
	
	while (block_number > 0) 
	{
		if (block_number <= MAX_BLOCK) 
		{
			read_block = block_number; // 读取的块数小于65536 Block
			block_number = 0; // 剩余读取块数为0
		} 
		else 
		{
			read_block = MAX_BLOCK; // 读取的块数大于65536 Block,分多次读
			block_number -= read_block;
		}
		// 根据sd主机控制器标准,按顺序写入主机控制器相应的寄存器		
		BLKSIZE0 = (7<<12) | (512<<0); // 最大DMA缓存大小,block为512字节			
		BLKCNT0 = read_block; // 写入这次读block数目
		
		if (card_type == SD_HC) 
		{
			address = block_addr; // SDHC卡写入地址为block地址
		} 
		else if (card_type == SD_V1 || card_type == SD_V2) 
		{
			address = block_addr << 9; // 标准卡写入地址为字节地址
		}	
		block_addr += read_block; // 下一次读块的地址
		ARGUMENT0 = address; // 写入命令参数		
		
		if (read_block == 1) 
		{
			// 设置传输模式,DMA禁能,读单块
			TRNMOD0 = (0<<5) | (1<<4) | (0<<2) | (1<<1) | (0<<0);
			// 设置命令寄存器,单块读CMD17,R1回复
			CMDREG0 = (CMD17<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;
		} else {
			// 设置传输模式,DMA禁能,读多块			
			TRNMOD0 = (1<<5) | (1<<4) | (1<<2) | (1<<1) | (0<<0);
			// 设置命令寄存器,多块读CMD18,R1回复	
			CMDREG0 = (CMD18<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;			
		}	
		error_state = sd_wait_for_command_done();
		if (error_state) {
			printf("Read Command error\r\n");
			return error_state;
		}	
		for (i=0; i<read_block; i++) {
			error_state = sd_wait_for_buffer_read_ready();
			if (error_state) {
				return error_state;
			}
			if (((uint32_t)pBuffer & 0x3) == 0) {	
				for (j=0; j<512/4; j++) {
					*(uint32_t *)pBuffer = BDATA0;
					pBuffer += 4;
				}
			} else {
				for (j=0; j<512/4; j++) {
					temp = BDATA0;
					*pBuffer++ = (uint8_t)temp;
					*pBuffer++ = (uint8_t)(temp>>8);
					*pBuffer++ = (uint8_t)(temp>>16);
					*pBuffer++ = (uint8_t)(temp>>24);
				}
			}						
		}
		
		error_state = sd_wait_for_transfer_done();
		if (error_state) {
			printf("Read block error\r\n");
			return error_state;
		}	
	}
	return 0; // 所有块读完
}

int32_t sd_write_block(uint8_t *pBuffer, uint32_t block_addr, uint32_t block_number)
{
	uint32_t address = 0;
	uint32_t write_block;
	uint32_t i;
	uint32_t j;
	int32_t error_state;
	
	if (pBuffer == 0 || block_number == 0) 
	{
		return -1; // 参数错误
	}	
	
	NORINTSTS0 = NORINTSTS0; // 清除中断标志
	ERRINTSTS0 = ERRINTSTS0; // 清除错误中断标志
	
	while (block_number > 0) 
	{	
		if (block_number <= MAX_BLOCK) 
		{
			write_block = block_number;// 写入的块数小于65536 Block
			block_number = 0; // 剩余写入块数为0
		} 
		else 
		{
			write_block = MAX_BLOCK; // 写入的块数大于65536 Block,分多次写
			block_number -= write_block;
		}
		if (write_block > 1) 
		{ // 多块写,发送ACMD23先设置预擦除块数
			sd_issue_command(CMD55, RCA<<16, 0, CMD_RESP_R1);
			sd_issue_command(CMD23, write_block, 0, CMD_RESP_R1);
		}
		
		// 根据sd主机控制器标准,按顺序写入主机控制器相应的寄存器			
		BLKSIZE0 = (7<<12) | (512<<0); // 最大DMA缓存大小,block为512字节		
		BLKCNT0 = write_block; // 写入block数目	
		
		if (card_type == SD_HC) 
		{
			address = block_addr; // SDHC卡写入地址为block地址
		} 
		else if (card_type == SD_V1 || card_type == SD_V2) 
		{
			address = block_addr << 9; // 标准卡写入地址为字节地址
		}
		block_addr += write_block; // 下一次写地址
		ARGUMENT0 = address; // 写入命令参数			
		
		if (write_block == 1) 
		{
			// 设置传输模式,DMA禁能写单块
			TRNMOD0 = (0<<5) | (0<<4) | (0<<2) | (1<<1) | (0<<0);
			// 设置命令寄存器,单块写CMD24,R1回复
			CMDREG0 = (CMD24<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;			
		}
		 else
		 {
			// 设置传输模式,DMA禁能写多块
			TRNMOD0 = (1<<5) | (0<<4) | (1<<2) | (1<<1) | (0<<0);
			// 设置命令寄存器,多块写CMD25,R1回复
			CMDREG0 = (CMD25<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;					
		}
		error_state = sd_wait_for_command_done();
		if (error_state) 
		{
			printf("Write Command error\r\n");
			return error_state;
		}	
		
		for (i=0; i<write_block; i++) 
		{
			error_state = sd_wait_for_buffer_write_ready();
			if (error_state) 
			{
				return error_state;
			}
			if (((uint32_t)pBuffer & 0x3) == 0) 
			{
				for (j=0; j<512/4; j++) 
				{
					BDATA0 = *(uint32_t *)pBuffer;
					pBuffer += 4;
				}
			} 
			else
			 {
				for (j=0; j<512/4; j++) 
				{
					BDATA0 = pBuffer[0] + ((uint32_t)pBuffer[1]<<8) +
					((uint32_t)pBuffer[2]<<16) + ((uint32_t)pBuffer[3]<<24);
					pBuffer += 4;
				}
			}	
		}
		
		error_state = sd_wait_for_transfer_done();
		if (error_state) 
		{
			printf("Write block error\r\n");
			return error_state;
		}
		for (i=0; i<0x10000000; i++) 
		{
			if (sd_get_card_state() == CARD_TRAN) 
			{ // 需在transfer status
				break; // 状态正确
			}
		}
		if (i == 0x10000000) 
		{
			return -3; // 状态错误或Programming超时
		}		
	}
	return 0; // 写完所有数据
}

