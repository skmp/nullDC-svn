#include "types.h"
#include "dc/mem/sh4_internal_reg.h"
#include "rtc.h"




u32 RTC_R64CNT=0;


//Init term res
void rtc_Init()
{
	// NAOMI reads from at least RTC_R64CNT
#ifdef BUILD_NAOMI
	//RTC R64CNT 0xFFC80000 0x1FC80000 8 Held Held Held Held Pclk
	RTC[(RTC_R64CNT_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_R64CNT_addr&0xFF)>>2].NextCange=0;
	RTC[(RTC_R64CNT_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_R64CNT_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_R64CNT_addr&0xFF)>>2].data32=&RTC_R64CNT;
#endif
}

void rtc_Reset(bool Manual)
{
}
void rtc_Term()
{
}