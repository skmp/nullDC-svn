#pragma once
#include "types.h"


enum InterruptType
{
	sh4_int   = 0x00000000,
	sh4_exp   = 0x01000000,
	InterruptTypeMask = 0x7F000000,
	InterruptIntEVNTMask=0x00FFFF00,
	InterruptPIIDMask=0x000000FF,
	//InterruptIDMask=0x00FFFFFF,
};
#define KMIID(type,ID,PIID) ( (type) | ((ID)<<8) | (PIID))
enum InterruptID
{
		//internal interups
		//IRL*
		//sh4_IRL_0			= KMIID(sh4_int,0x200,0),	-> these are not connected on dc
		//sh4_IRL_1			= KMIID(sh4_int,0x220,1),
		//sh4_IRL_2			= KMIID(sh4_int,0x240,2),
		//sh4_IRL_3			= KMIID(sh4_int,0x260,3),
		//sh4_IRL_4			= KMIID(sh4_int,0x280,4),
		//sh4_IRL_5			= KMIID(sh4_int,0x2A0,5),
		//sh4_IRL_6			= KMIID(sh4_int,0x2C0,6),
		//sh4_IRL_7			= KMIID(sh4_int,0x2E0,7),
		//sh4_IRL_8			= KMIID(sh4_int,0x300,8),
		sh4_IRL_9			= KMIID(sh4_int,0x320,0),
		//sh4_IRL_10			= KMIID(sh4_int,0x340,10),-> these are not connected on dc
		sh4_IRL_11			= KMIID(sh4_int,0x360,1),
		//sh4_IRL_12			= KMIID(sh4_int,0x380,12),-> these are not connected on dc
		sh4_IRL_13			= KMIID(sh4_int,0x3A0,2),
		//sh4_IRL_14			= KMIID(sh4_int,0x3C0,14),-> these are not connected on dc
		//sh4_IRL_15			= KMIID(sh4_int,0x340,0), -> no interrupt (masked)

		sh4_HUDI_HUDI		= KMIID(sh4_int,0x600,3),  /* H-UDI underflow */

		sh4_GPIO_GPIOI		= KMIID(sh4_int,0x620,4),

		//DMAC
		sh4_DMAC_DMTE0		= KMIID(sh4_int,0x640,5),
		sh4_DMAC_DMTE1		= KMIID(sh4_int,0x660,6),
		sh4_DMAC_DMTE2		= KMIID(sh4_int,0x680,7),
		sh4_DMAC_DMTE3		= KMIID(sh4_int,0x6A0,8),
		sh4_DMAC_DMAE		= KMIID(sh4_int,0x6C0,9),
		
		//TMU
		sh4_TMU0_TUNI0		=  KMIID(sh4_int,0x400,10), /* TMU0 underflow */
		sh4_TMU1_TUNI1		=  KMIID(sh4_int,0x420,11), /* TMU1 underflow */
		sh4_TMU2_TUNI2		=  KMIID(sh4_int,0x440,12), /* TMU2 underflow */
		sh4_TMU2_TICPI2		=  KMIID(sh4_int,0x460,13),

		//RTC
		sh4_RTC_ATI			= KMIID(sh4_int,0x480,14),
		sh4_RTC_PRI			= KMIID(sh4_int,0x4A0,15),
		sh4_RTC_CUI			= KMIID(sh4_int,0x4C0,16),

		//SCI
		sh4_SCI1_ERI		= KMIID(sh4_int,0x4E0,17),
		sh4_SCI1_RXI		= KMIID(sh4_int,0x500,18),
		sh4_SCI1_TXI		= KMIID(sh4_int,0x520,19),
		sh4_SCI1_TEI		= KMIID(sh4_int,0x540,29),
		
		//SCIF
		sh4_SCIF_ERI		= KMIID(sh4_int,0x700,21),
		sh4_SCIF_RXI		= KMIID(sh4_int,0x720,22),
		sh4_SCIF_BRI		= KMIID(sh4_int,0x740,23),
		sh4_SCIF_TXI		= KMIID(sh4_int,0x760,24),

		//WDT
		sh4_WDT_ITI			= KMIID(sh4_int,0x560,25),
		
		//REF
		sh4_REF_RCMI		= KMIID(sh4_int,0x580,26),
		sh4_REF_ROVI		= KMIID(sh4_int,0x5A0,27),

		//sh4 exeptions 
		sh4_ex_USER_BREAK_BEFORE_INSTRUCTION_EXECUTION = sh4_exp | 0x1e0,
		sh4_ex_INSTRUCTION_ADDRESS_ERROR =sh4_exp | 0x0e0,
		sh4_ex_INSTRUCTION_TLB_MISS =sh4_exp | 0x040,
		sh4_ex_INSTRUCTION_TLB_PROTECTION_VIOLATION = sh4_exp |0x0a0,
		sh4_ex_GENERAL_ILLEGAL_INSTRUCTION = sh4_exp |0x180,
		sh4_ex_SLOT_ILLEGAL_INSTRUCTION = sh4_exp |0x1a0,
		sh4_ex_GENERAL_FPU_DISABLE = sh4_exp |0x800,
		sh4_ex_SLOT_FPU_DISABLE = sh4_exp |0x820,
		sh4_ex_DATA_ADDRESS_ERROR_READ =sh4_exp |0x0e0,
		sh4_ex_DATA_ADDRESS_ERROR_WRITE = sh4_exp | 0x100,
		sh4_ex_DATA_TLB_MISS_READ = sh4_exp | 0x040,
		sh4_ex_DATA_TLB_MISS_WRITE = sh4_exp | 0x060,
		sh4_ex_DATA_TLB_PROTECTION_VIOLATION_READ = sh4_exp | 0x0a0,
		sh4_ex_DATA_TLB_PROTECTION_VIOLATION_WRITE = sh4_exp | 0x0c0,
		sh4_ex_FPU = sh4_exp | 0x120,
		sh4_ex_TRAP = sh4_exp | 0x160,
		sh4_ex_INITAL_PAGE_WRITE = sh4_exp | 0x080,
};



//void FASTCALL RaiseInterrupt_(InterruptID intr);
typedef void FASTCALL Sh4RaiseInterruptFP(InterruptID intr);

enum HollyInterruptType
{
	holly_nrm = 0x0000,
	holly_ext = 0x0100,
	holly_err = 0x0200,
};

enum HollyInterruptID
{		
		// asic9a /sh4 external holly normal [internal]
		holly_RENDER_DONE_vd = holly_nrm | 0,	//bit 0 = End of Render interrupt : Video
		holly_RENDER_DONE_isp = holly_nrm | 1,	//bit 1 = End of Render interrupt : ISP
		holly_RENDER_DONE = holly_nrm | 2,		//bit 2 = End of Render interrupt : TSP

		holly_SCANINT1 = holly_nrm | 3,			//bit 3 = V Blank-in interrupt
		holly_SCANINT2 = holly_nrm | 4,			//bit 4 = V Blank-out interrupt
		holly_HBLank = holly_nrm | 5,			//bit 5 = H Blank-in interrupt
												
		holly_YUV_DMA = holly_nrm | 6,			//bit 6 = End of Transferring interrupt : YUV
		holly_OPAQUE = holly_nrm | 7,			//bit 7 = End of Transferring interrupt : Opaque List
		holly_OPAQUEMOD = holly_nrm | 8,		//bit 8 = End of Transferring interrupt : Opaque Modifier Volume List
		
		holly_TRANS = holly_nrm | 9,			//bit 9 = End of Transferring interrupt : Translucent List
		holly_TRANSMOD = holly_nrm | 10,		//bit 10 = End of Transferring interrupt : Translucent Modifier Volume List
		holly_PVR_DMA = holly_nrm | 11,			//bit 11 = End of DMA interrupt : PVR-DMA
		holly_MAPLE_DMA = holly_nrm | 12,		//bit 12 = End of DMA interrupt : Maple-DMA

		holly_MAPLE_VBOI = holly_nrm | 13,		//bit 13 = Maple V blank over interrupt
		holly_GDROM_DMA = holly_nrm | 14,		//bit 14 = End of DMA interrupt : GD-DMA
		holly_SPU_DMA = holly_nrm | 15,			//bit 15 = End of DMA interrupt : AICA-DMA
		
		holly_EXT_DMA1 = holly_nrm | 16,		//bit 16 = End of DMA interrupt : Ext-DMA1(External 1)
		holly_EXT_DMA2 = holly_nrm | 17,		//bit 17 = End of DMA interrupt : Ext-DMA2(External 2)
		holly_DEV_DMA = holly_nrm | 18,			//bit 18 = End of DMA interrupt : Dev-DMA(Development tool DMA)
		
		holly_CH2_DMA = holly_nrm | 19,			//bit 19 = End of DMA interrupt : ch2-DMA 
		holly_PVR_SortDMA = holly_nrm | 20,		//bit 20 = End of DMA interrupt : Sort-DMA (Transferring for alpha sorting)
		holly_PUNCHTHRU = holly_nrm | 21,		//bit 21 = End of Transferring interrupt : Punch Through List

		// asic9c/sh4 external holly external [EXTERNAL]
		holly_GDROM_CMD = holly_ext | 0x00,	//bit 0 = GD-ROM interrupt
		holly_SPU_IRQ = holly_ext | 0x01,	//bit 1 = AICA interrupt
		holly_EXP_8BIT = holly_ext | 0x02,	//bit 2 = Modem interrupt
		holly_EXP_PCI = holly_ext | 0x03,	//bit 3 = External Device interrupt

		// asic9b/sh4 external holly err only error [error]
		//missing quite a few ehh ?
		//bit 0 = RENDER : ISP out of Cache(Buffer over flow)
		//bit 1 = RENDER : Hazard Processing of Strip Buffer
		holly_PRIM_NOMEM = holly_err | 0x02,	//bit 2 = TA : ISP/TSP Parameter Overflow
		holly_MATR_NOMEM = holly_err | 0x03		//bit 3 = TA : Object List Pointer Overflow
		//bit 4 = TA : Illegal Parameter
		//bit 5 = TA : FIFO Overflow
		//bit 6 = PVRIF : Illegal Address set
		//bit 7 = PVRIF : DMA over run
		//bit 8 = MAPLE : Illegal Address set
		//bit 9 = MAPLE : DMA over run
		//bit 10 = MAPLE : Write FIFO over flow
		//bit 11 = MAPLE : Illegal command
		//bit 12 = G1 : Illegal Address set
		//bit 13 = G1 : GD-DMA over run
		//bit 14 = G1 : ROM/FLASH access at GD-DMA
		//bit 15 = G2 : AICA-DMA Illegal Address set
		//bit 16 = G2 : Ext-DMA1 Illegal Address set
		//bit 17 = G2 : Ext-DMA2 Illegal Address set
		//bit 18 = G2 : Dev-DMA Illegal Address set
		//bit 19 = G2 : AICA-DMA over run
		//bit 20 = G2 : Ext-DMA1 over run
		//bit 21 = G2 : Ext-DMA2 over run
		//bit 22 = G2 : Dev-DMA over run
		//bit 23 = G2 : AICA-DMA Time out
		//bit 24 = G2 : Ext-DMA1 Time out
		//bit 25 = G2 : Ext-DMA2 Time out
		//bit 26 = G2 : Dev-DMA Time out 
		//bit 27 = G2 : Time out in CPU accessing 	
};



typedef void FASTCALL HollyRaiseInterruptFP(HollyInterruptID intr);
typedef void FASTCALL HollyCancelInterruptFP(HollyInterruptID intr);
