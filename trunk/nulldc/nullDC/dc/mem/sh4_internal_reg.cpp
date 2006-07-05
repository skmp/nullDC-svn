#include "types.h"
#include "sh4_internal_reg.h"

#include "dc/sh4/bsc.h"
#include "dc/sh4/ccn.h"
#include "dc/sh4/cpg.h"
#include "dc/sh4/dmac.h"
#include "dc/sh4/intc.h"
#include "dc/sh4/rtc.h"
#include "dc/sh4/sci.h"
#include "dc/sh4/scif.h"
#include "dc/sh4/tmu.h"
#include "dc/sh4/ubc.h"

u32 sq0_dw[8];
u32 sq1_dw[8];
Array<u8> OnChipRAM;

//All registers are 4 byte alligned

Array<RegisterStruct> CCN(16,true);			//CCN  : 14 registers
Array<RegisterStruct> UBC(9,true);			//UBC  : 9 registers
Array<RegisterStruct> BSC(19,true);			//BSC  : 18 registers
Array<RegisterStruct> DMAC(17,true);		//DMAC : 17 registers
Array<RegisterStruct> CPG(5,true);			//CPG  : 5 registers
Array<RegisterStruct> RTC(16,true);			//RTC  : 16 registers
Array<RegisterStruct> INTC(4,true);			//INTC : 4 registers
Array<RegisterStruct> TMU(12,true);			//TMU  : 12 registers
Array<RegisterStruct> SCI(8,true);			//SCI  : 8 registers
Array<RegisterStruct> SCIF(10,true);		//SCIF : 10 registers


//helper functions
INLINE u32 RegSRead(Array<RegisterStruct>& reg,u32 offset,u32 size)
{
#ifdef TRACE
	if (offset & 3/*(size-1)*/) //4 is min allign size
	{
		EMUERROR("unallinged register read");
	}
#endif

	offset>>=2;

#ifdef TRACE
	if (reg[offset].flags& size)
	{
#endif
		if (reg[offset].flags & REG_READ_DATA )
		{
			if (size==4)
				return  *reg[offset].data32;
			else if (size==2)
				return  *reg[offset].data16;
			else 
				return  *reg[offset].data8;
		}
		else
		{
			if (reg[offset].readFunction)
				return reg[offset].readFunction();
			else
			{
				if (!(reg[offset].flags& REG_NOT_IMPL))
					EMUERROR("ERROR [readed write olny register]");
			}
		}
#ifdef TRACE
	}
	else
	{
		if (!(reg[offset].flags& REG_NOT_IMPL))
			EMUERROR("ERROR [wrong size read on register]");
	}
#endif
	if (reg[offset].flags& REG_NOT_IMPL)
		EMUERROR2("Read from internal Regs , not  implemented , offset=%x",offset);
	return 0;
}

INLINE void RegSWrite(Array<RegisterStruct>& reg,u32 offset,u32 data,u32 size)
{
#ifdef TRACE
	if (offset & 3/*(size-1)*/) //4 is min allign size
	{
		EMUERROR("unallinged register write");
	}
#endif
offset>>=2;
#ifdef TRACE
	if (reg[offset].flags & size)
	{
#endif
		if (reg[offset].flags & REG_WRITE_DATA)
		{
			if (size==4)
				*reg[offset].data32=data;
			else if (size==2)
				*reg[offset].data16=(u16)data;
			else
				*reg[offset].data8=(u8)data;

			return;
		}
		else
		{
			if (reg[offset].flags & REG_CONST)
				EMUERROR("Error [Write to read olny register , const]");
			else
			{
				if (reg[offset].writeFunction)
				{
					reg[offset].writeFunction(data);
					return;
				}
				else
				{
					if (!(reg[offset].flags& REG_NOT_IMPL))
						EMUERROR("ERROR [Write to read olny register]");
				}
			}
		}
#ifdef TRACE
	}
	else
	{
		if (!(reg[offset].flags& REG_NOT_IMPL))
			EMUERROR4("ERROR :wrong size write on register ; offset=%x , data=%x,sz=%d",offset,data,size);
	}
#endif
	if ((reg[offset].flags& REG_NOT_IMPL))
		EMUERROR3("Write to internal Regs , not  implemented , offset=%x,data=%x",offset,data);
}

//Region P4
u32 ReadMem_P4(u32 addr,u32 sz)
{
	if (((addr>>26)&0x7)==7)
	{
		return ReadMem_area7(addr,sz);	
	}

	switch((addr>>24)&0xFF)
	{

	case 0xE0:
	case 0xE1:
	case 0xE2:
	case 0xE3:
		printf("Unhandled p4 read [Store queue] 0x%x\n",addr);
		break;

	case 0xF0:
		printf("Unhandled p4 read [Instruction cache address array] 0x%x\n",addr);
		break;

	case 0xF1:
		printf("Unhandled p4 read [Instruction cache data array] 0x%x\n",addr);
		break;

	case 0xF2:
		printf("Unhandled p4 read [Instruction TLB address array] 0x%x\n",addr);
		break;

	case 0xF3:
		printf("Unhandled p4 read [Instruction TLB data arrays 1 and 2] 0x%x\n",addr);
		break;

	case 0xF4:
		{
			int W,Set,A;
			W=(addr>>14)&1;
			A=(addr>>3)&1;
			Set=(addr>>5)&0xFF;
			//printf("Unhandled p4 read [Operand cache address array] %d:%d,%d  0x%x\n",Set,W,A,addr);
			return 0;
		}
		break;

	case 0xF5:
		printf("Unhandled p4 read [Operand cache data array] 0x%x",addr);
		break;

	case 0xF6:
		printf("Unhandled p4 read [Unified TLB address array] 0x%x\n",addr);
		break;

	case 0xF7:
		printf("Unhandled p4 read [Unified TLB data arrays 1 and 2] 0x%x\n",addr);
		break;

	case 0xFF:
		return ReadMem_area7(addr,sz);

	default:
		printf("Unhandled p4 read [Reserved] 0x%x\n",addr);
		break;
	}
	
	EMUERROR2("Read from P4 not implemented , addr=%x",addr);
	return 0;
	
}


void __fastcall WriteMem_sq_32(u32 addr,u32 data)
{
	u32 offset = (addr >> 2) & 7; // 3 bits

	if ((addr & 0x20)) // 0: SQ0, 1: SQ1
	{
		sq1_dw[offset] = data;
	}
	else
	{
		sq0_dw[offset] = data;
	}
	return;
}
void WriteMem_P4(u32 addr,u32 data,u32 sz)
{
	if (((addr>>26)&0x7)==7)
	{
		WriteMem_area7(addr,data,sz);	
		return;
	}

	switch((addr>>24)&0xFF)
	{

	case 0xE0:
	case 0xE1:
	case 0xE2:
	case 0xE3:
		printf("Unhandled p4 Write [Store queue] 0x%x",addr);
		break;

	case 0xF0:
		printf("Unhandled p4 Write [Instruction cache address array] 0x%x = %x\n",addr,data);
		break;

	case 0xF1:
		printf("Unhandled p4 Write [Instruction cache data array] 0x%x = %x\n",addr,data);
		break;

	case 0xF2:
		printf("Unhandled p4 Write [Instruction TLB address array] 0x%x = %x\n",addr,data);
		break;

	case 0xF3:
		printf("Unhandled p4 Write [Instruction TLB data arrays 1 and 2] 0x%x = %x\n",addr,data);
		break;

	case 0xF4:
		{
			int W,Set,A;
			W=(addr>>14)&1;
			A=(addr>>3)&1;
			Set=(addr>>5)&0xFF;
			//printf("Unhandled p4 Write [Operand cache address array] %d:%d,%d  0x%x = %x\n",Set,W,A,addr,data);
			return;
		}
		break;

	case 0xF5:
		printf("Unhandled p4 Write [Operand cache data array] 0x%x = %x\n",addr,data);
		break;

	case 0xF6:
		printf("Unhandled p4 Write [Unified TLB address array] 0x%x = %x\n",addr,data);
		break;

	case 0xF7:
		printf("Unhandled p4 Write [Unified TLB data arrays 1 and 2] 0x%x = %x\n",addr,data);
		break;

	case 0xFF:
		WriteMem_area7(addr,data,sz);
		break;

	default:
		printf("Unhandled p4 Write [Reserved] 0x%x\n",addr);
		break;
	}

	EMUERROR3("Write to P4 not implemented , addr=%x,data=%x",addr,data);
}


//Area 7
u32 ReadMem_area7(u32 addr,u32 sz)
{
	if (CCN_CCR.ORA)
	{
		if ((addr>=0x7c000000) && (addr<=0x7FFFFFFF))
		{
			if (sz==1)
				return OnChipRAM[addr&OnChipRAM_MASK];
			else if (sz==2)
				return *(u16*)&OnChipRAM[addr&OnChipRAM_MASK];
			else if (sz==4)
				return *(u32*)&OnChipRAM[addr&OnChipRAM_MASK];

			return 0;
		}
	}

	addr&=0x1FFFFFFF;
	switch (A7_REG_HASH(addr))
	{
	case A7_REG_HASH(CCN_BASE_addr):
		if (addr<=0x1F00003C)
		{
			return RegSRead(CCN,addr & 0xFF,sz);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(UBC_BASE_addr):
		if (addr<=0x1F200020)
		{
			return RegSRead(UBC,addr & 0xFF,sz);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(BSC_BASE_addr):
		if (addr<=0x1F800048)
		{
			return RegSRead(BSC,addr & 0xFF,sz);
		}
		else if ((addr>=BSC_SDMR2_addr) && (addr<= 0x1F90FFFF))
		{
			//dram settings 2 / write olny
			EMUERROR("Read from write-olny registers [dram settings 2]");
		}
		else if ((addr>=BSC_SDMR3_addr) && (addr<= 0x1F94FFFF))
		{
			//dram settings 3 / write olny
			EMUERROR("Read from write-olny registers [dram settings 3]");
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;



	case A7_REG_HASH(DMAC_BASE_addr):
		if (addr<=0x1FA00040)
		{
			return RegSRead(DMAC,addr & 0xFF,sz);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(CPG_BASE_addr):
		if (addr<=0x1FC00010)
		{
			return RegSRead(CPG,addr & 0xFF,sz);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(RTC_BASE_addr):
		if (addr<=0x1FC8003C)
		{
			return RegSRead(RTC,addr & 0xFF,sz);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(INTC_BASE_addr):
		if (addr<=0x1FD0000C)
		{
			return RegSRead(INTC,addr & 0xFF,sz);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(TMU_BASE_addr):
		if (addr<=0x1FD8002C)
		{
			return RegSRead(TMU,addr & 0xFF,sz);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(SCI_BASE_addr):
		if (addr<=0x1FE0001C)
		{
			return RegSRead(SCI,addr & 0xFF,sz);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(SCIF_BASE_addr):
		if (addr<=0x1FE80024)
		{
			return RegSRead(SCIF,addr & 0xFF,sz);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	//who realy cares about ht-udi ? it's not existant on dc iirc ..
	case A7_REG_HASH(UDI_BASE_addr):
		switch(addr)
		{
		//UDI SDIR 0x1FF00000 0x1FF00000 16 0xFFFF Held Held Held Pclk
		case UDI_SDIR_addr :
			break;


		//UDI SDDR 0x1FF00008 0x1FF00008 32 Held Held Held Held Pclk
		case UDI_SDDR_addr :
			break;
		}
		break;
	}


	EMUERROR2("Unkown Read from Area7 , addr=%x",addr);
	return 0;
}

void WriteMem_area7(u32 addr,u32 data,u32 sz)
{
	if (CCN_CCR.ORA)
	{
		if ((addr>=0x7c000000) && (addr<=0x7FFFFFFF))
		{
			//ReadMemFromPtrRet(OnChipRAM,(adr&0x1FFF),sz);
			if (sz==1)
				OnChipRAM[addr&OnChipRAM_MASK]=(u8)data;
			else if (sz==2)
				*(u16*)&OnChipRAM[addr&OnChipRAM_MASK]=(u16)data;
			else if (sz==4)
				*(u32*)&OnChipRAM[addr&OnChipRAM_MASK]=data;

			return;
		}
	}
	addr&=0x1FFFFFFF;
	switch (A7_REG_HASH(addr))
	{

	case A7_REG_HASH(CCN_BASE_addr):
		if (addr<=0x1F00003C)
		{
			RegSWrite(CCN,addr & 0xFF,data,sz);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(UBC_BASE_addr):
		if (addr<=0x1F200020)
		{
			RegSWrite(UBC,addr & 0xFF,data,sz);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(BSC_BASE_addr):
		if (addr<=0x1F800048)
		{
			RegSWrite(BSC,addr & 0xFF,data,sz);
			return;
		}
		else if ((addr>=BSC_SDMR2_addr) && (addr<= 0x1F90FFFF))
		{
			//dram settings 2 / write olny
			return;//no need ?
		}
		else if ((addr>=BSC_SDMR3_addr) && (addr<= 0x1F94FFFF))
		{
			//dram settings 3 / write olny
			return;//no need ?
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;



	case A7_REG_HASH(DMAC_BASE_addr):
		if (addr<=0x1FA00040)
		{
			RegSWrite(DMAC,addr & 0xFF,data,sz);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(CPG_BASE_addr):
		if (addr<=0x1FC00010)
		{
			RegSWrite(CPG,addr & 0xFF,data,sz);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(RTC_BASE_addr):
		if (addr<=0x1FC8003C)
		{
			RegSWrite(RTC,addr & 0xFF,data,sz);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(INTC_BASE_addr):
		if (addr<=0x1FD0000C)
		{
			RegSWrite(INTC,addr & 0xFF,data,sz);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(TMU_BASE_addr):
		if (addr<=0x1FD8002C)
		{
			RegSWrite(TMU,addr & 0xFF,data,sz);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(SCI_BASE_addr):
		if (addr<=0x1FE0001C)
		{
			RegSWrite(SCI,addr & 0xFF,data,sz);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(SCIF_BASE_addr):
		if (addr<=0x1FE80024)
		{
			RegSWrite(SCIF,addr & 0xFF,data,sz);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	//who realy cares about ht-udi ? it's not existant on dc iirc ..
	case A7_REG_HASH(UDI_BASE_addr):
		switch(addr)
		{
			//UDI SDIR 0xFFF00000 0x1FF00000 16 0xFFFF Held Held Held Pclk
		case UDI_SDIR_addr :
			break;


			//UDI SDDR 0xFFF00008 0x1FF00008 32 Held Held Held Held Pclk
		case UDI_SDDR_addr :
			break;
		}
		break;
	}

	EMUERROR3("Write to Area7 not implemented , addr=%x,data=%x",addr,data);
}

//Init/Res/Term
void sh4_internal_reg_Init()
{
	OnChipRAM.Resize(OnChipRAM_SIZE,false);

	for (u32 i=0;i<30;i++)
	{
	if (i<CCN.Size)	CCN[i].flags=REG_NOT_IMPL;	//(16,true);	//CCN  : 14 registers
	if (i<UBC.Size)	UBC[i].flags=REG_NOT_IMPL;	//(9,true);		//UBC  : 9 registers
	if (i<BSC.Size)	BSC[i].flags=REG_NOT_IMPL;	//(19,true);	//BSC  : 18 registers
	if (i<DMAC.Size)DMAC[i].flags=REG_NOT_IMPL;	//(17,true);	//DMAC : 17 registers
	if (i<CPG.Size)	CPG[i].flags=REG_NOT_IMPL;	//(5,true);		//CPG  : 5 registers
	if (i<RTC.Size)	RTC[i].flags=REG_NOT_IMPL;	//(16,true);	//RTC  : 16 registers
	if (i<INTC.Size)INTC[i].flags=REG_NOT_IMPL;	//(4,true);		//INTC : 4 registers
	if (i<TMU.Size)	TMU[i].flags=REG_NOT_IMPL;	//(12,true);	//TMU  : 12 registers
	if (i<SCI.Size)	SCI[i].flags=REG_NOT_IMPL;	//(8,true);		//SCI  : 8 registers
	if (i<SCIF.Size)SCIF[i].flags=REG_NOT_IMPL;	//(10,true);	//SCIF : 10 registers
	}

	//initialise Register structs
	bsc_Init();
	ccn_Init();
	cpg_Init();
	dmac_Init();
	intc_Init();
	rtc_Init();
	sci_Init();
	scif_Init();
	tmu_Init();
	ubc_Init();
}

void sh4_internal_reg_Reset(bool Manual)
{
	OnChipRAM.Zero();
	//Reset register values
	bsc_Reset(Manual);
	ccn_Reset(Manual);
	cpg_Reset(Manual);
	dmac_Reset(Manual);
	intc_Reset(Manual);
	rtc_Reset(Manual);
	sci_Reset(Manual);
	scif_Reset(Manual);
	tmu_Reset(Manual);
	ubc_Reset(Manual);
}

void sh4_internal_reg_Term()
{
	//free any alloc'd resources [if any]
	ubc_Term();
	tmu_Term();
	scif_Term();
	sci_Term();
	rtc_Term();
	intc_Term();
	dmac_Term();
	cpg_Term();
	ccn_Term();
	bsc_Term();
	OnChipRAM.Free();
}