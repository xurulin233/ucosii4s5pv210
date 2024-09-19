#ifndef __boot_config_h__
#define __boot_config_h__

#define Mode_USR      0x10
#define Mode_FIQ      0x11
#define Mode_IRQ      0x12
#define Mode_SVC      0x13
#define Mode_SEC	  0x16
#define Mode_ABT      0x17
#define Mode_UND      0x1B
#define Mode_SYS      0x1F
#define Mode_MASK     0x1F
#define NOINT         0xC0
#define I_Bit         0x80
#define F_Bit         0x40

//------------------------------------------------------------------------------
//    OEM Stack Layout
// EBOOT, STEPLOADER also use this value
//------------------------------------------------------------------------------
#define DRAM_BASE_PA_START 0x23e00000
#define IMAGE_NK_OFFSET 0x1000000
#define TOP_OF_STACKS_PHYSICAL      (DRAM_BASE_PA_START+IMAGE_NK_OFFSET)    // Stack Top is in front of NK Image
#define TOP_OF_STACKS_VIRTUAL       (DRAM_BASE_CA_START+IMAGE_NK_OFFSET)

// Stack Size of Each Mode
#define FIQStackSize     2048
#define IRQStackSize     2048
#define AbortStackSize   2048
#define UndefStackSize   2048
#define SVCStackSize     2048
#define SECStackSize	 2048
#define UserStackSize   10485760
#define SysStackSize     10485760
//; Stack Location of Each Mode (in Physical Address)
#define FIQStack_PA    (TOP_OF_STACKS_PHYSICAL    - 0x0)
#define IRQStack_PA    (FIQStack_PA        - FIQStackSize)
#define AbortStack_PA  (IRQStack_PA        - IRQStackSize)
#define UndefStack_PA  (AbortStack_PA        - AbortStackSize)
#define SVCStack_PA    (UndefStack_PA        - UndefStackSize)
//#define UserStack_PA  (SVCStack_PA        - SVCStackSize)
#define SECStack_PA		(SVCStack_PA        - SVCStackSize)
#define SysStack_PA    (SECStack_PA        - SECStackSize)

#endif