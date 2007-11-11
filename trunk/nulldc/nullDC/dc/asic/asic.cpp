#include "types.h"
#include "asic.h"

#include "dc\sh4\intc.h"
#include "dc\mem\sb.h"
#include "dc\maple\maple_if.h"

/*
	asic Interrupt controler
	part of the holly block on dc
*/

//returns true if any RL6 Interrupts are pending
void asic_RL6Pending()
{
	bool t1=(SB_ISTNRM & SB_IML6NRM)!=0;
	bool t2=(SB_ISTERR & SB_IML6ERR)!=0;
	bool t3=(SB_ISTEXT & SB_IML6EXT)!=0;

	InterruptPend(sh4_IRL_9,t1|t2|t3);
}

//return true if any RL4 iterupts are pending
void asic_RL4Pending()
{
	bool t1=(SB_ISTNRM & SB_IML4NRM)!=0;
	bool t2=(SB_ISTERR & SB_IML4ERR)!=0;
	bool t3=(SB_ISTEXT & SB_IML4EXT)!=0;

	InterruptPend(sh4_IRL_11,t1|t2|t3);
}

//return true if any RL2 iterupts are pending
void asic_RL2Pending()
{
	bool t1=(SB_ISTNRM & SB_IML2NRM)!=0;
	bool t2=(SB_ISTERR & SB_IML2ERR)!=0;
	bool t3=(SB_ISTEXT & SB_IML2EXT)!=0;

	InterruptPend(sh4_IRL_13,t1|t2|t3);
}

void RaiseAsicNormal(HollyInterruptID inter)
{
	if (inter==holly_SCANINT2)
				maple_vblank();

	u32 Interrupt = 1<<(u8)inter;
	SB_ISTNRM |= Interrupt;

	asic_RL2Pending();
	asic_RL4Pending();
	asic_RL6Pending();
}

void RaiseAsicExt(HollyInterruptID inter)
{
	u32 Interrupt = 1<<(u8)inter;
	SB_ISTEXT |= Interrupt;

	asic_RL2Pending();
	asic_RL4Pending();
	asic_RL6Pending();
}

void RaiseAsicErr(HollyInterruptID inter)
{
	u32 Interrupt = 1<<(u8)inter;
	SB_ISTERR |= Interrupt;

	asic_RL2Pending();
	asic_RL4Pending();
	asic_RL6Pending();
}

void fastcall asic_RaiseInterrupt(HollyInterruptID inter)
{
	u8 m=inter>>8;
	switch(m)
	{
	case 0:
		RaiseAsicNormal(inter);
		break;
	case 1:
		RaiseAsicExt(inter);
		break;
	case 2:
		RaiseAsicErr(inter);
		break;
	}
}
void fastcall asic_CancelInterrupt(HollyInterruptID inter)
{
	SB_ISTEXT&=~(1<<(u8)inter);
	asic_RL2Pending();
	asic_RL4Pending();
	asic_RL6Pending();
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

	asic_RL2Pending();
	asic_RL4Pending();
	asic_RL6Pending();
}

void Write_SB_ISTEXT(u32 data)
{
	//SB_ISTEXT &= ~data;
}

void Write_SB_ISTERR(u32 data)
{
	SB_ISTERR &= ~data;

	asic_RL2Pending();
	asic_RL4Pending();
	asic_RL6Pending();
}

void Write_SB_SB_IML6NRM(u32 data)
{
	SB_IML6NRM=data;

	asic_RL6Pending();
}
void Write_SB_SB_IML4NRM(u32 data)
{
	SB_IML4NRM=data;

	asic_RL4Pending();
}
void Write_SB_SB_IML2NRM(u32 data)
{
	SB_IML2NRM=data;

	asic_RL2Pending();
}

void Write_SB_SB_IML6EXT(u32 data)
{
	SB_IML6EXT=data;

	asic_RL6Pending();
}
void Write_SB_SB_IML4EXT(u32 data)
{
	SB_IML4EXT=data;

	asic_RL4Pending();
}
void Write_SB_SB_IML2EXT(u32 data)
{
	SB_IML2EXT=data;

	asic_RL2Pending();
}

void Write_SB_SB_IML6ERR(u32 data)
{
	SB_IML6ERR=data;
	
	asic_RL6Pending();
}
void Write_SB_SB_IML4ERR(u32 data)
{
	SB_IML4ERR=data;
	
	asic_RL4Pending();
}
void Write_SB_SB_IML2ERR(u32 data)
{
	SB_IML2ERR=data;
	
	asic_RL2Pending();
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

