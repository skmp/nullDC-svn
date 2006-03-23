#include "types.h"
#include "asic.h"

#include "dc\sh4\intc.h"
#include "dc\mem\sb.h"

/*
	asic Interrupt controler
	part of the holly block on dc
*/

//returns true if any RL6 Interrupts are pending
bool asic_RL6Pending()
{
	bool t1=(SB_ISTNRM & SB_IML6NRM)!=0;
	bool t2=(SB_ISTERR & SB_IML6ERR)!=0;
	bool t3=(SB_ISTEXT & SB_IML6EXT)!=0;

	return t1|t2|t3;
}

//return true if any RL4 iterupts are pending
bool asic_RL4Pending()
{
	bool t1=(SB_ISTNRM & SB_IML4NRM)!=0;
	bool t2=(SB_ISTERR & SB_IML4ERR)!=0;
	bool t3=(SB_ISTEXT & SB_IML4EXT)!=0;

	return t1|t2|t3;
}

//return true if any RL2 iterupts are pending
bool asic_RL2Pending()
{
	bool t1=(SB_ISTNRM & SB_IML2NRM)!=0;
	bool t2=(SB_ISTERR & SB_IML2ERR)!=0;
	bool t3=(SB_ISTEXT & SB_IML2EXT)!=0;

	return t1|t2|t3;
}

//Return interrupt pririty level
u32 asic_GetRL6Priority()
{
	return 0x6;
}
//Return interrupt pririty level
u32 asic_GetRL4Priority()
{
	return 0x4;
}
//Return interrupt pririty level
u32 asic_GetRL2Priority()
{
	return 0x2;
}

void RaiseAsicNormal(InterruptID inter)
{
	u32 Interrupt = (u32)(1 << ((((u32)inter & (u32)InterruptIDMask))));
	SB_ISTNRM |= Interrupt;
}

void RaiseAsicErr(InterruptID inter)
{
	u32 Interrupt = (u32)(1 << ((((u32)inter & (u32)InterruptIDMask))));
	SB_ISTERR |= Interrupt;
}

void RaiseAsicExt(InterruptID inter)
{
	u32 Interrupt = (u32)(1 << ((((u32)inter & (u32)InterruptIDMask))));
	SB_ISTEXT |= Interrupt;
}

u32 Read_SB_ISTNRM()
{
	u32 tmp = SB_ISTNRM & 0x3FFFFFFF;
	if (SB_ISTEXT)
		tmp|=0x40000000;
	if (SB_ISTERR)
		tmp|=0x80000000;
	return tmp;
}

void Write_SB_ISTNRM(u32 data)
{
	SB_ISTNRM &= ~data;
}

void Write_SB_ISTEXT(u32 data)
{
	//SB_ISTEXT &= ~data;
}

void Write_SB_ISTERR(u32 data)
{
	SB_ISTERR &= ~data;
}

void Write_SB_SB_IML6NRM(u32 data)
{
	SB_IML6NRM=data;

	if (asic_RL6Pending())
		InterruptsArePending=true;
}
void Write_SB_SB_IML4NRM(u32 data)
{
	SB_IML4NRM=data;

	if (asic_RL4Pending())
		InterruptsArePending=true;
}
void Write_SB_SB_IML2NRM(u32 data)
{
	SB_IML2NRM=data;

	if (asic_RL2Pending())
		InterruptsArePending=true;
}

void Write_SB_SB_IML6EXT(u32 data)
{
	SB_IML6EXT=data;

	if (asic_RL6Pending())
		InterruptsArePending=true;
}
void Write_SB_SB_IML4EXT(u32 data)
{
	SB_IML4EXT=data;

	if (asic_RL4Pending())
		InterruptsArePending=true;
}
void Write_SB_SB_IML2EXT(u32 data)
{
	SB_IML2EXT=data;

	if (asic_RL2Pending())
		InterruptsArePending=true;
}

void Write_SB_SB_IML6ERR(u32 data)
{
	SB_IML6ERR=data;
	
	if (asic_RL6Pending())
		InterruptsArePending=true;
}
void Write_SB_SB_IML4ERR(u32 data)
{
	SB_IML4ERR=data;
	
	if (asic_RL4Pending())
		InterruptsArePending=true;
}
void Write_SB_SB_IML2ERR(u32 data)
{
	SB_IML2ERR=data;
	
	if (asic_RL2Pending())
		InterruptsArePending=true;
}

void asic_reg_Init()
{
	sb_regs[((SB_ISTNRM_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE;
	sb_regs[((SB_ISTNRM_addr-SB_BASE)>>2)].readFunction=Read_SB_ISTNRM;
	sb_regs[((SB_ISTNRM_addr-SB_BASE)>>2)].writeFunction=Write_SB_ISTNRM;

	sb_regs[((SB_ISTEXT_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_ISTEXT_addr-SB_BASE)>>2)].writeFunction=Write_SB_ISTEXT;

	sb_regs[((SB_ISTERR_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_ISTERR_addr-SB_BASE)>>2)].writeFunction=Write_SB_ISTERR;

	//NRM
	//6
	sb_regs[((SB_IML6NRM_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_IML6NRM_addr-SB_BASE)>>2)].writeFunction=Write_SB_SB_IML6NRM;
	//4
	sb_regs[((SB_IML4NRM_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_IML4NRM_addr-SB_BASE)>>2)].writeFunction=Write_SB_SB_IML4NRM;
	//2
	sb_regs[((SB_IML2NRM_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_IML2NRM_addr-SB_BASE)>>2)].writeFunction=Write_SB_SB_IML2NRM;
	//EXT
	//6
	sb_regs[((SB_IML6EXT_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_IML6EXT_addr-SB_BASE)>>2)].writeFunction=Write_SB_SB_IML6EXT;
	//4
	sb_regs[((SB_IML4EXT_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_IML4EXT_addr-SB_BASE)>>2)].writeFunction=Write_SB_SB_IML4EXT;
	//2
	sb_regs[((SB_IML2EXT_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_IML2EXT_addr-SB_BASE)>>2)].writeFunction=Write_SB_SB_IML2EXT;
	//ERR
	//6
	sb_regs[((SB_IML6ERR_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_IML6ERR_addr-SB_BASE)>>2)].writeFunction=Write_SB_SB_IML6ERR;
	//4
	sb_regs[((SB_IML4ERR_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_IML4ERR_addr-SB_BASE)>>2)].writeFunction=Write_SB_SB_IML4ERR;
	//2
	sb_regs[((SB_IML2ERR_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_IML2ERR_addr-SB_BASE)>>2)].writeFunction=Write_SB_SB_IML2ERR;
	
}

void asic_reg_Term()
{

}
//Reset -> Reset - Initialise to defualt values
void asic_reg_Reset(bool Manual)
{

}

