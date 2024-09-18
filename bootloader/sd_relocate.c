
/*
#define SD_START_BLOCK	45
#define SD_BLOCK_CNT	(2048*2)
#define DDR_START_ADDR	0x23E00000



typedef unsigned int bool;

// ͨ���ţ�0������2
// ��ʼ�����ţ�45
// ��ȡ����������32
// ��ȡ������ڴ��ַ��0x23E00000
// with_init��0
typedef bool(*pCopySDMMC2Mem)(int, unsigned int, unsigned short, unsigned int*, bool);

typedef void (*pBL2Type)(void);


// ��SD����45������ʼ������32���������ݵ�DDR��0x23E00000��Ȼ����ת��23E00000ȥִ��

void copy_bl2_2_ddr(void)
{
	// ��һ������ȡSD��������DDR��
	pCopySDMMC2Mem p1 = (pCopySDMMC2Mem)(*(unsigned int *)0xD0037F98);
	//pCopySDMMC2Mem p1 = (pCopySDMMC2Mem)0xD0037F98);
	
	led2();
	delay();
	p1(0, SD_START_BLOCK, SD_BLOCK_CNT, (unsigned int *)DDR_START_ADDR, 0);		// ��ȡSD����DDR��
	led1();
	delay();
	// �ڶ�������ת��DDR�е�BL2ȥִ��
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

	// ����ָ��
	copy_sd_mmc_to_mem copy_bl2 = (copy_sd_mmc_to_mem) (*(unsigned int *) (0xD0037F98));

	unsigned int ret;
	led1();
	delay();
	// ͨ��0
	if (ch == 0xEB000000)
	{
		// 0:channel 0
		// 49:Դ,����λ������49,1 sector = 512 bytes
		// 32:���ȣ�����32 sector����16K
		// 0x23E00000:Ŀ��,���ӵ�ַ0x23E00000
		int i;
		for(i = 0;i<8*5;i++)   //2.5M
		{
			ret = copy_bl2(0, 49+128*i, 128,(unsigned int *)(0x23E00000+0x10000*i), 0);
		}
	led2();
	delay();
	
	}  
	// ͨ��2
	else if (ch == 0xEB200000)
	{
		ret = copy_bl2(2, 49, 1024,(unsigned int *)0x23E00000, 0);
	}
	else
		return;

	// ��ת��DRAM
    BL2 = (void *)0x23E00000;
    (*BL2)();
}























