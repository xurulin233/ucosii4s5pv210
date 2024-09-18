
/*
#define SD_START_BLOCK	45
#define SD_BLOCK_CNT	(2048*2)
#define DDR_START_ADDR	0x23E00000



typedef unsigned int bool;

// 通道号：0，或者2
// 开始扇区号：45
// 读取扇区个数：32
// 读取后放入内存地址：0x23E00000
// with_init：0
typedef bool(*pCopySDMMC2Mem)(int, unsigned int, unsigned short, unsigned int*, bool);

typedef void (*pBL2Type)(void);


// 从SD卡第45扇区开始，复制32个扇区内容到DDR的0x23E00000，然后跳转到23E00000去执行

void copy_bl2_2_ddr(void)
{
	// 第一步，读取SD卡扇区到DDR中
	pCopySDMMC2Mem p1 = (pCopySDMMC2Mem)(*(unsigned int *)0xD0037F98);
	//pCopySDMMC2Mem p1 = (pCopySDMMC2Mem)0xD0037F98);
	
	led2();
	delay();
	p1(0, SD_START_BLOCK, SD_BLOCK_CNT, (unsigned int *)DDR_START_ADDR, 0);		// 读取SD卡到DDR中
	led1();
	delay();
	// 第二步，跳转到DDR中的BL2去执行
	pBL2Type p2 = (pBL2Type)DDR_START_ADDR;
	p2();
	
	led3();
	delay();
}
*/

typedef unsigned int (*copy_sd_mmc_to_mem) (unsigned int  channel, unsigned int  start_block, unsigned char block_size, unsigned int  *trg, unsigned int  init);
void copy_bl2_2_ddr(void)
{
	unsigned long ch;
	void (*BL2)(void);
	ch = *(volatile unsigned int *)(0xD0037488);

	// 函数指针
	copy_sd_mmc_to_mem copy_bl2 = (copy_sd_mmc_to_mem) (*(unsigned int *) (0xD0037F98));

	unsigned int ret;
	led1();
	delay();
	// 通道0
	if (ch == 0xEB000000)
	{
		// 0:channel 0
		// 49:源,代码位于扇区49,1 sector = 512 bytes
		// 32:长度，拷贝32 sector，即16K
		// 0x23E00000:目的,链接地址0x23E00000
		int i;
		for(i = 0;i<8*5;i++)   //2.5M
		{
			ret = copy_bl2(0, 49+128*i, 128,(unsigned int *)(0x23E00000+0x10000*i), 0);
		}
	led2();
	delay();
	
	}  
	// 通道2
	else if (ch == 0xEB200000)
	{
		ret = copy_bl2(2, 49, 1024,(unsigned int *)0x23E00000, 0);
	}
	else
		return;

	// 跳转到DRAM
    BL2 = (void *)0x23E00000;
    (*BL2)();
}























