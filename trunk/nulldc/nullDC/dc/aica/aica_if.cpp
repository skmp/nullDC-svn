#include "types.h"
#include "aica_if.h"
#include "dc/mem/sh4_mem.h"
#include "dc/mem/sb.h"
#include "plugins/plugin_manager.h"

//arm 7 is emulated within the aica implementation
//RTC is emulated here tho xD
//Gota check what to do about the rest regs that are not aica olny .. pfftt [display mode , any other ?]
#include <time.h>

u32 ReadMem_aica_rtc(u32 addr,u32 sz)
{
	//this somehow works :P

	time_t rawtime;
	tm  timeinfo;
	
	//int tm_sec;     /* seconds after the minute - [0,59] */
    //int tm_min;     /* minutes after the hour - [0,59] */
    //int tm_hour;    /* hours since midnight - [0,23] */
    //int tm_mday;    /* day of the month - [1,31] */
    //int tm_mon;     /* months since January - [0,11] */

	timeinfo.tm_year=1998-1900;
	timeinfo.tm_mon=11-1;
	timeinfo.tm_mday=27;
	timeinfo.tm_hour=0;
	timeinfo.tm_min=0;
	timeinfo.tm_sec=0;

	rawtime=mktime( &timeinfo );
	
	rawtime=time (0)-rawtime;//get delta of time since the known dc date
	
	time_t temp=time(0);
	timeinfo=*localtime(&temp);
	if (timeinfo.tm_isdst)
		rawtime+=24*3600;//add an hour if dst (maby rtc has a reg for that ? *watch* and add it if yes :)


	u32 RTC=0x5bfc8900 + (u32)rawtime;// add delta to known dc time

	switch( addr & 0xFF )
	{
	case 0:	
		return RTC>>16;
	case 4:	
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

void Write_SB_ADST(u32 data)
{
	//0x005F7800	SB_ADSTAG	RW	AICA:G2-DMA G2 start address 
	//0x005F7804	SB_ADSTAR	RW	AICA:G2-DMA system memory start address 
	//0x005F7808	SB_ADLEN	RW	AICA:G2-DMA length 
	//0x005F780C	SB_ADDIR	RW	AICA:G2-DMA direction 
	//0x005F7810	SB_ADTSEL	RW	AICA:G2-DMA trigger select 
	//0x005F7814	SB_ADEN	RW	AICA:G2-DMA enable 
	//0x005F7818	SB_ADST	RW	AICA:G2-DMA start 
	//0x005F781C	SB_ADSUSP	RW	AICA:G2-DMA suspend 
	
	if (data&1)
	{
		if (SB_ADEN&1)
		{
			if (SB_ADDIR==1)
				printf("AICA DMA : SB_ADDIR==1 !!!!!!!!\n");

			u32 src=SB_ADSTAR;
			u32 dst=SB_ADSTAG;
			u32 len=SB_ADLEN & 0x7FFFFFFF;

			for (u32 i=0;i<len;i+=4)
			{
				u32 data=ReadMem32(src+i);
				libAICA->aica_info.WriteMem_aica_ram(dst+i,data,4);
			}

			if (SB_ADLEN & 0x80000000)
				SB_ADEN=1;//
			else
				SB_ADEN=0;//

			SB_ADSTAR+=len;
			SB_ADSTAG+=len;
			SB_ADST = 0x00000000;//dma done
			SB_ADLEN = 0x00000000;

			
			RaiseInterrupt(holly_SPU_DMA);
		}
	}
}

void aica_sb_Init()
{
	//NRM
	//6
	sb_regs[((SB_ADST_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_ADST_addr-SB_BASE)>>2)].writeFunction=Write_SB_ADST;
}

void aica_sb_Reset(bool Manual)
{
}

void aica_sb_Term()
{
}