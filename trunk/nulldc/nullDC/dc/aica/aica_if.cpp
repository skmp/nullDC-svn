#include "types.h"
#include "aica_if.h"
#include "dc\mem\sh4_mem.h"
#include "plugins\plugin_manager.h"

//*FIXME* Actualy use aica plugin

//arm 7 is emulated within the aica implementation
//for rtc
#include <time.h>

Array<u8> aica_ram;
Array<u8> aica_reg;

u32 aica_readreg(u32 addr,u32 sz)
{
	ReadMemArrRet(aica_reg,addr&0x7FFF,sz);

	//should never come here
	return 0;
}

void aica_writereg(u32 addr,u32 data,u32 sz)
{
	WriteMemArrRet(aica_reg,addr&0x7FFF,data,sz);

	//should never come here
}


u32 aica_readram(u32 addr,u32 sz)
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

	if (sz==1)
		return aica_ram[addr&AICA_MEM_MASK];
	else if (sz==2)
		return *(u16*)&aica_ram[addr&AICA_MEM_MASK];
	else if (sz==4)
		return *(u32*)&aica_ram[addr&AICA_MEM_MASK];

	//TODO : Add Warn
	return 0;
}

void aica_writeram(u32 addr,u32 data,u32 sz)
{
	if (sz==1)
		aica_ram[addr&AICA_MEM_MASK]=(u8)data;
	else if (sz==2)
		*(u16*)&aica_ram[addr&AICA_MEM_MASK]=(u16)data;
	else if (sz==4)
		*(u32*)&aica_ram[addr&AICA_MEM_MASK]=data;
}

u32 aica_readrtc(u32 addr,u32 sz)
{
	u32 RTC=0x5bfc8900;//somehow it does not work ....
	switch( addr & 0xFF )
	{
	case 0:	
		//RTC=0x5bfc8900;//(u32)time(NULL)+220752000;
		return RTC>>16;
	case 4:	
		//RTC=0x5bfc8900;//(u32)time(NULL)+220752000;
		return RTC &0xFFFF;
	case 8:	
		return 0;
	}

	//TODO : Add Warn
	return 0;
}

void aica_writertc(u32 addr,u32 data,u32 sz)
{
	
}

void aica_Init()
{
	aica_ram.Resize(AICA_MEM_SIZE,false);
	aica_reg.Resize(0x8000,false);
}

void aica_Reset(bool Manual)
{
	if (!Manual)
	{
		aica_ram.Zero();
		aica_reg.Zero();
	}
}

void aica_Term()
{
	aica_reg.Free();
	aica_ram.Free();
}

//Cycles are sh4 cpu cycles
void UpdateAica(u32 cycles)
{
}