#include "types.h"
#include "tmu.h"
#include "intc.h"
#include "dc/mem/sh4_internal_reg.h"


#define tmu_underflow	0x0100
#define tmu_UNIE		0x0020
s32 tmu_cnt[3]={0,0,0};
s32 tmu_cnt_max[3]={4,4,4};
u32 tmu_ch_bit[3]={1,2,4};
u32 tmu_regs_CNT[3];
u32 tmu_regs_COR[3];
u16 tmu_regs_CR[3];
u32 old_mode[3] = {0xFFFF,0xFFFF,0xFFFF};
u8 TMU_TOCR,TMU_TSTR;

const InterruptID tmu_intID[3]={sh4_TMU0_TUNI0,sh4_TMU1_TUNI1,sh4_TMU2_TUNI2};

//Accurate counts for the chanel ch
void UpdateTMU_chan(u32 clc,u32 ch)
{
	//if chanel is on
	if ((TMU_TSTR & tmu_ch_bit[ch])!=0)
	{
		//count :D
		tmu_cnt[ch]+=clc;
		if (tmu_cnt[ch]>tmu_cnt_max[ch])
		{
			//i wonder how is that compiled .. should be just 1 div :p -> it is on vs2k8
			s32 loops=tmu_cnt[ch]/tmu_cnt_max[ch];
			tmu_cnt[ch]=tmu_cnt[ch]%tmu_cnt_max[ch];
			while (loops>tmu_regs_CNT[ch])
			{
				loops-=tmu_regs_CNT[ch];
				tmu_regs_CNT[ch] = tmu_regs_COR[ch];
				tmu_regs_CR[ch] |= tmu_underflow;
				if (tmu_regs_CR[ch] & tmu_UNIE)
					RaiseInterrupt(tmu_intID[ch]);
			}
			tmu_regs_CNT[ch]-=loops;
		}
	}
}

void UpdateTMU(u32 Cycles)
{
	UpdateTMU_chan(Cycles,0);
	UpdateTMU_chan(Cycles,1);
	UpdateTMU_chan(Cycles,2);
}

//Update internal counter registers
void UpdateTMUCounts(u32 reg)
{
	if (old_mode[reg]==(tmu_regs_CR[reg] & 0x7))
		return;
	else
		old_mode[reg]=(tmu_regs_CR[reg] & 0x7);

	switch(tmu_regs_CR[reg] & 0x7)
	{
		case 0:	//4
			tmu_cnt_max[reg]=4;
			break;

		case 1:	//16
			tmu_cnt_max[reg]=16;
			break;

		case 2:	//64
			tmu_cnt_max[reg]=64;
			break;

		case 3:	//256
			tmu_cnt_max[reg]=256;
			break;

		case 4:	//1024
			tmu_cnt_max[reg]=1024;
			break;

		case 5:	//reserved
			printf("TMU ch%d , TCR%d mode is reserved (5)",reg,reg);
			break;

		case 6:	//RTC
			printf("TMU ch%d , TCR%d mode is RTC (6) , can't be used on dreamcast",reg,reg);
			tmu_cnt_max[reg]=3200;//1/(32768/2) second (RTC Clock output)
			break;

		case 7:	//external
			printf("TMU ch%d , TCR%d mode is External (7) , can't be used on dreamcast",reg,reg);
			break;
	}
	tmu_cnt_max[reg]*=4;// because we count in Iö cycles (cpu core cycles) and the tmu is provided w/
					  // the Pö (perhipal clock).. This may or may not be ok , but seems to work well

	/*if (tmu_cnt[reg]>tmu_cnt_max[reg])
		tmu_cnt[reg]=tmu_cnt_max[reg];*/
	tmu_cnt[reg]=0;
}

//A.M.A.N

//Write to status registers
void TMU_TCR0_write(u32 data)
{
	tmu_regs_CR[0]=(u16)data;
	UpdateTMUCounts(0);
}
void TMU_TCR1_write(u32 data)
{
	tmu_regs_CR[1]=(u16)data;
	UpdateTMUCounts(1);
}
void TMU_TCR2_write(u32 data)
{
	tmu_regs_CR[2]=(u16)data;
	UpdateTMUCounts(2);
}

//Chan 2 not used functions
u32 TMU_TCPR2_read()
{
	//this regiser should be not used on dreamcast according to docs
	EMUERROR("Read from TMU_TCPR2  , this regiser should be not used on dreamcast according to docs");
	return 0;
}

void TMU_TCPR2_write(u32 data)
{
	EMUERROR2("Write to TMU_TCPR2  , this regiser should be not used on dreamcast according to docs , data=%d",data);
}



//Init/Res/Term
void tmu_Init()
{
	//TMU TOCR 0xFFD80000 0x1FD80000 8 0x00 0x00 Held Held Pclk
	TMU[(TMU_TOCR_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TOCR_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TOCR_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TOCR_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TOCR_addr&0xFF)>>2].data8=&TMU_TOCR;

	//TMU TSTR 0xFFD80004 0x1FD80004 8 0x00 0x00 Held 0x00 Pclk
	TMU[(TMU_TSTR_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TSTR_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TSTR_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TSTR_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TSTR_addr&0xFF)>>2].data8=&TMU_TSTR;

	//TMU TCOR0 0xFFD80008 0x1FD80008 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCOR0_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCOR0_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TCOR0_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCOR0_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCOR0_addr&0xFF)>>2].data32=&tmu_regs_COR[0];

	//TMU TCNT0 0xFFD8000C 0x1FD8000C 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCNT0_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCNT0_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TCNT0_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCNT0_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCNT0_addr&0xFF)>>2].data32=&tmu_regs_CNT[0];

	//TMU TCR0 0xFFD80010 0x1FD80010 16 0x0000 0x0000 Held Held Pclk
	TMU[(TMU_TCR0_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA;
	TMU[(TMU_TCR0_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TCR0_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCR0_addr&0xFF)>>2].writeFunction=TMU_TCR0_write;
	TMU[(TMU_TCR0_addr&0xFF)>>2].data16=&tmu_regs_CR[0];

	//TMU TCOR1 0xFFD80014 0x1FD80014 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCOR1_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCOR1_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TCOR1_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCOR1_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCOR1_addr&0xFF)>>2].data32=&tmu_regs_COR[1];

	//TMU TCNT1 0xFFD80018 0x1FD80018 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCNT1_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCNT1_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TCNT1_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCNT1_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCNT1_addr&0xFF)>>2].data32=&tmu_regs_CNT[1];

	//TMU TCR1 0xFFD8001C 0x1FD8001C 16 0x0000 0x0000 Held Held Pclk
	TMU[(TMU_TCR1_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA;
	TMU[(TMU_TCR1_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TCR1_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCR1_addr&0xFF)>>2].writeFunction=TMU_TCR1_write;
	TMU[(TMU_TCR1_addr&0xFF)>>2].data16=&tmu_regs_CR[1];

	//TMU TCOR2 0xFFD80020 0x1FD80020 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCOR2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCOR2_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TCOR2_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCOR2_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCOR2_addr&0xFF)>>2].data32=&tmu_regs_COR[2];

	//TMU TCNT2 0xFFD80024 0x1FD80024 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCNT2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCNT2_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TCNT2_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCNT2_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCNT2_addr&0xFF)>>2].data32=&tmu_regs_CNT[2];

	//TMU TCR2 0xFFD80028 0x1FD80028 16 0x0000 0x0000 Held Held Pclk
	TMU[(TMU_TCR2_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA;
	TMU[(TMU_TCR2_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TCR2_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCR2_addr&0xFF)>>2].writeFunction=TMU_TCR2_write;
	TMU[(TMU_TCR2_addr&0xFF)>>2].data16=&tmu_regs_CR[2];

	//TMU TCPR2 0xFFD8002C 0x1FD8002C 32 Held Held Held Held Pclk
	TMU[(TMU_TCPR2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE;
	TMU[(TMU_TCPR2_addr&0xFF)>>2].NextCange=0;
	TMU[(TMU_TCPR2_addr&0xFF)>>2].readFunction=TMU_TCPR2_read;
	TMU[(TMU_TCPR2_addr&0xFF)>>2].writeFunction=TMU_TCPR2_write;
	TMU[(TMU_TCPR2_addr&0xFF)>>2].data32=0;
}

void tmu_Reset(bool Manual)
{
/*
	tmu_cnt[0]=0;
	tmu_cnt[1]=0;
	tmu_cnt[2]=0;

	tmu_cnt_max[0]=16;
	tmu_cnt_max[1]=16;
	tmu_cnt_max[2]=16;
*/
	TMU_TOCR=TMU_TSTR=0;
	tmu_regs_COR[0] = tmu_regs_COR[1] = tmu_regs_COR[2] = 0xffffffff;
	tmu_regs_CNT[0] = tmu_regs_CNT[1] = tmu_regs_CNT[2] = 0xffffffff;	
	UpdateTMUCounts(0);
	UpdateTMUCounts(1);
	UpdateTMUCounts(2);
}

void tmu_Term()
{
}

//INTC sources
bool tmu_CNT0Pending()
{
	return (tmu_regs_CR[0] & (tmu_underflow | tmu_UNIE))==(tmu_underflow | tmu_UNIE);
}
u32 tmu_CNT0Priority()
{
	return INTC_IPRA.TMU0;
}

bool tmu_CNT1Pending()
{
	return (tmu_regs_CR[1] & (tmu_underflow | tmu_UNIE))==(tmu_underflow | tmu_UNIE);
}
u32 tmu_CNT1Priority()
{
	return INTC_IPRA.TMU1;
}

bool tmu_CNT2Pending()
{
	return (tmu_regs_CR[2] & (tmu_underflow | tmu_UNIE))==(tmu_underflow | tmu_UNIE);
}
u32 tmu_CNT2Priority()
{
	return INTC_IPRA.TMU2;
}