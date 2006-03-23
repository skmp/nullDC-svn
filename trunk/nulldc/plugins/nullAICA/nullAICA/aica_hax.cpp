#include "aica_hax.h"

u8 *aica_reg;
u8 *aica_ram;

u32 ReadMem_reg(u32 addr,u32 size)
{
	ReadMemArrRet(aica_reg,addr&0x7FFF,size);

	//should never come here
	return 0;
}

void WriteMem_reg(u32 addr,u32 data,u32 size)
{
	WriteMemArrRet(aica_reg,addr&0x7FFF,data,size);

	//should never come here
}

u32 ReadMem_ram(u32 addr,u32 size)
{
	//goto NoHacks;
	if((addr&0xFF)==0x5C)
		return 1;			// hack naomi aica ram check ?

	//kos/katana
	if (addr==0x80FFC0)
		aica_ram[((addr-0x800000)&0x1FFFFF)]=aica_ram[((addr-0x800000)&0x1FFFFF)]?0:3;
	//return 0x3;			//hack snd_dbg

	//the kos command list is 0x810000 to 0x81FFFF
	//here we hack the first and last comands
	//seems to fix everything ^^
	if (addr==0x81000C)
		return 0x1;			//hack kos command que

	//here we hack the first and last comands
	//seems to fix everything ^^
	if (addr==0x81FFFC)
		return 0x1;			//hack kos command que

	//crazy taxi / doa2 /*
	if (addr>0x800100 && addr<0x800300)
		return 0x800000;			//hack katana command que

	//trickstyle 
	//another position for command queue ?
	if (addr>=0x813400 && addr<=0x813900)
	{
		//what to return what to return ??
		//this not works good
		//return 0x1;
	}

	//hack Katana wait
	//Waits for something , then writes to the value returned here
	//Found in Roadsters[also in Toy cmd]
	//it seems it is somehow rlated to a direct mem read after that 
	// im too tired to debug and find the actualy relation but it is
	if (addr==0x8000EC)
		return 0x80000;

	//huh ?
	if (addr==0x8000E8)
		return 0x80000;

	//OMG WTF DIE!!!
	if ((addr>0x8014e0) && (addr<0x8015e0))
	{
		return 0xFFFFFF;
	}
	//mwhahaha arm7 is on mwhahahah more bugs now mwhahaa

	if (size==1)
		return aica_ram[addr&AICA_MEM_MASK];
	else if (size==2)
		return *(u16*)&aica_ram[addr&AICA_MEM_MASK];
	else if (size==4)
		return *(u32*)&aica_ram[addr&AICA_MEM_MASK];

	//TODO : Add Warn
	return 0;
}

void WriteMem_ram(u32 addr,u32 data,u32 size)
{
	if (size==1)
		aica_ram[addr&AICA_MEM_MASK]=(u8)data;
	else if (size==2)
		*(u16*)&aica_ram[addr&AICA_MEM_MASK]=(u16)data;
	else if (size==4)
		*(u32*)&aica_ram[addr&AICA_MEM_MASK]=data;
}

void init_mem()
{
	aica_ram=(u8*)malloc(AICA_MEM_SIZE);
	aica_reg=(u8*)malloc(0x8000);
}

void term_mem()
{
	free(aica_reg);
	free(aica_ram);
}