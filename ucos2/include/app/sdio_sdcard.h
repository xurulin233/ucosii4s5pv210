#ifndef __SDIO_SDCARD_H
#define __SDIO_SDCARD_H																			   
													   
#include "nick_types.h"

#define SDMASYSAD0 			(*(volatile unsigned long 	*)0xEB000000)
#define BLKSIZE0 			(*(volatile unsigned short 	*)0xEB000004)
#define BLKCNT0				(*(volatile unsigned short 	*)0xEB000006)
#define ARGUMENT0 			(*(volatile unsigned long 	*)0xEB000008)
#define TRNMOD0 			(*(volatile unsigned short 	*)0xEB00000C)
#define CMDREG0 			(*(volatile unsigned short 	*)0xEB00000E)
#define RSPREG0_0 			(*(volatile unsigned long 	*)0xEB000010)
#define RSPREG1_0 			(*(volatile unsigned long 	*)0xEB000014)
#define RSPREG2_0 			(*(volatile unsigned long 	*)0xEB000018)
#define RSPREG3_0 			(*(volatile unsigned long 	*)0xEB00001C)
#define BDATA0 				(*(volatile unsigned long 	*)0xEB000020)
#define PRNSTS0 			(*(volatile unsigned long 	*)0xEB000024)
#define HOSTCTL0 			(*(volatile unsigned char 	*)0xEB000028)
#define PWRCON0 			(*(volatile unsigned char 	*)0xEB000029)
#define BLKGAP0 			(*(volatile unsigned char 	*)0xEB00002A)
#define WAKCON0 			(*(volatile unsigned char	*)0xEB00002B)
#define CLKCON0 			(*(volatile unsigned short	*)0xEB00002C)
#define TIMEOUTCON0			(*(volatile unsigned char	*)0xEB00002E)
#define SWRST0 				(*(volatile unsigned char	*)0xEB00002F)
#define NORINTSTS0 			(*(volatile unsigned short	*)0xEB000030)
#define ERRINTSTS0 			(*(volatile unsigned short 	*)0xEB000032)
#define NORINTSTSEN0 		(*(volatile unsigned short 	*)0xEB000034)
#define ERRINTSTSEN0 		(*(volatile unsigned short 	*)0xEB000036)
#define NORINTSIGEN0 		(*(volatile unsigned short 	*)0xEB000038)
#define ERRINTSIGEN0 		(*(volatile unsigned short 	*)0xEB00003A)
#define ACMD12ERRSTS0 		(*(volatile unsigned short 	*)0xEB00003C)
#define CAPAREG0 			(*(volatile unsigned long 	*)0xEB000040)
#define MAXCURR0 			(*(volatile unsigned long 	*)0xEB000048)
#define FEAER0 				(*(volatile unsigned short 	*)0xEB000050)
#define FEERR0 				(*(volatile unsigned short 	*)0xEB000052)
#define ADMAERR0 			(*(volatile unsigned long 	*)0xEB000054)
#define ADMASYSADDR0 		(*(volatile unsigned long 	*)0xEB000058)
#define CONTROL2_0 			(*(volatile unsigned long 	*)0xEB000080)
#define CONTROL3_0 			(*(volatile unsigned long 	*)0xEB000084)
#define CONTROL4_0 			(*(volatile unsigned long 	*)0xEB00008C)
#define HCVER0 				(*(volatile unsigned short 	*)0xEB0000FE)


////////////////////////////////////////////////////////////////////////////////
#define ENSTACARDREM		(7)
#define ENSTACARDNS			(6)
#define ENSIGCARDREM		(7)
#define ENSIGCARDNS			(6)
#define STACARDREM			(7)
#define STACARDINS			(6)
#define INSCARD				(16)


////////////////////////////////////////////////////////////////////////////////



#define	CMD0	0
#define	CMD1	1
#define	CMD2	2	
#define	CMD3	3	
#define	CMD6	6
#define	CMD7	7
#define	CMD8	8
#define	CMD9	9
#define	CMD13	13
#define	CMD16	16
#define	CMD17	17
#define	CMD18	18
#define	CMD23	23	
#define	CMD24	24
#define	CMD25	25	
#define	CMD32	32	
#define	CMD33	33	
#define	CMD38	38	
#define	CMD41	41	
#define CMD51	51
#define	CMD55	55	

// 卡类型
#define UNUSABLE		0
#define SD_V1			1	
#define	SD_V2			2	
#define	SD_HC			3
#define	MMC				4
	
// 卡状态
#define CARD_IDLE		0
#define CARD_READY		1
#define CARD_IDENT		2
#define CARD_STBY		3
#define CARD_TRAN		4
#define CARD_DATA		5
#define CARD_RCV		6
#define CARD_PRG		7
#define CARD_DIS		8

// 卡回复类型	
#define CMD_RESP_NONE	0
#define CMD_RESP_R1		1
#define CMD_RESP_R2		2
#define CMD_RESP_R3		3
#define CMD_RESP_R4		4
#define CMD_RESP_R5		5
#define CMD_RESP_R6		6
#define CMD_RESP_R7		7
#define CMD_RESP_R1B	8
	
typedef struct {
	uint32_t RESERVED1;
	uint32_t RESERVED2 : 16;	
	uint32_t SD_BUS_WIDTHS : 4;
	uint32_t SD_SECURITY : 3;	
	uint32_t DATA_STAT_AFTER_ERASE : 1;
	uint32_t SD_SPEC : 4;	
	uint32_t SCR_STRUCTURE : 4;
} SD_SCR;	

int32_t sd_init(void);
int32_t sd_get_card_state(void);
int32_t sd_get_state(uint8_t *pStatus);
int32_t sd_get_scr(SD_SCR *pSCR);
int32_t sd_get_csd(uint8_t *pCSD);
int32_t sd_erase_block(uint32_t StartBlock, uint32_t EndBlock);
int32_t sd_write_block(uint8_t *pBuffer, uint32_t BlockAddr, uint32_t BlockNumber);
int32_t sd_read_block(uint8_t *pBuffer, uint32_t BlockAddr, uint32_t BlockNumber);


#endif 





