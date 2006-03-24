#include "types.h"
#include "aica_if.h"
#include "dc/mem/sh4_mem.h"
#include "plugins/plugin_manager.h"

//arm 7 is emulated within the aica implementation
//RTC is emulated here tho xD
//Gota check what to do about the rest regs that are not aica olny .. pfftt [display mode , any other ?]
#include <time.h>

u32 ReadMem_aica_rtc(u32 addr,u32 sz)
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

void WriteMem_aica_rtc(u32 addr,u32 data,u32 sz)
{
	
}

//Init/res/term
void aica_Init()
{
	//mmnnn ? gota fill it w/ something
}

void aica_Reset(bool Manual)
{
	if (!Manual)
	{

	}
}

void aica_Term()
{

}

//Cycles are sh4 cpu cycles
void UpdateAica(u32 cycles)
{
	libAICA->aica_info.UpdateAICA(cycles);
}