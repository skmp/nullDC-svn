#include "types.h"
#include "aica_if.h"
#include "dc/mem/sh4_mem.h"
#include "dc/mem/sb.h"
#include "plugins/plugin_manager.h"

//arm 7 is emulated within the aica implementation
//RTC is emulated here tho xD
//Gota check what to do about the rest regs that are not aica olny .. pfftt [display mode , any other ?]
#include <time.h>
//u8 aica_ram[2*1024*1024];
VArray2 aica_ram;
u32 VREG;//video reg =P
u32 ReadMem_aica_rtc(u32 addr,u32 sz)
{
	
	//this somehow works :P

	time_t rawtime=0;
	tm  timeinfo;
	
	//int tm_sec;     /* seconds after the minute - [0,59] */
    //int tm_min;     /* minutes after the hour - [0,59] */
    //int tm_hour;    /* hours since midnight - [0,23] */
    //int tm_mday;    /* day of the month - [1,31] */
    //int tm_mon;     /* months since January - [0,11] */
/*
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

*/
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
u32 FASTCALL ReadMem_aica_reg(u32 addr,u32 sz)
{
	if (sz==1)
	{
		if ((addr & 0x7FFF)==0x2C01)
		{
			return VREG;
		}
		else
			return libAICA.ReadMem_aica_reg(addr,sz);
	}
	else
	{
		if ((addr & 0x7FFF)==0x2C00)
		{
			return libAICA.ReadMem_aica_reg(addr,sz) | (VREG<<8);
		}
		else
			return libAICA.ReadMem_aica_reg(addr,sz);
	}
}
void FASTCALL WriteMem_aica_reg(u32 addr,u32 data,u32 sz)
{
	if (sz==1)
	{
		if ((addr & 0x7FFF)==0x2C01)
		{
			VREG=data;
			printf("VREG = %02X\n",VREG);
		}
		else
		{
			libAICA.WriteMem_aica_reg(addr,data,sz);
		}
	}
	else
	{
		if ((addr & 0x7FFF)==0x2C00)
		{
			VREG=(data>>8)&0xFF;
			printf("VREG = %02X\n",VREG);
			libAICA.WriteMem_aica_reg(addr,data&(~0xFF00),sz);
		}
		else
		{
			libAICA.WriteMem_aica_reg(addr,data,sz);
		}
	}
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
		aica_ram.Zero();
	}
}

void aica_Term()
{

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
				msgboxf("AICA DMA : SB_ADDIR==1 !!!!!!!!",MBX_OK | MBX_ICONERROR);

			u32 src=SB_ADSTAR;
			u32 dst=SB_ADSTAG;
			u32 len=SB_ADLEN & 0x7FFFFFFF;

			for (u32 i=0;i<len;i+=4)
			{
				u32 data=ReadMem32_nommu(src+i);
				WriteMem32_nommu(dst+i,data);
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