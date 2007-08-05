#include "modem.h"

#define MODEM_COUNTRY_RES 0
#define MODEM_COUNTRY_JAP 1
#define MODEM_COUNTRY_USA 2

#define MODEM_MAKER_SEGA 0
#define MODEM_MAKER_ROCKWELL 1

#define MODEM_DEVICE_TYPE_336K 0


u32 MODEM_ID[2] = 
{
	MODEM_COUNTRY_RES,
	(MODEM_MAKER_ROCKWELL<<4) | (MODEM_DEVICE_TYPE_336K),
};

u32 modem_regs[0x21];

enum ModemStates
{
	MS_INVALID,				//needs reset
	MS_RESET,				//reset is low
	MS_RESETING,			//reset is hi
	MS_ST_CONTROLER,		//Controller self test
	MS_ST_DSP,				//DSP self test
	MS_NORMAL,				//Normal operation

};
void SetBits(u32 reg,u32 v)
{
	modem_regs[reg]|=v;
}
void ResetBits(u32 reg,u32 v)
{
	modem_regs[reg]&=v;
}
void SetMask(u32 reg,u32 v,u32 m)
{
	ResetBits(reg,m);
	SetBits(reg,v);
}
ModemStates state=MS_INVALID;

void DSPTestEnd()
{
}
void SelfTestEnd()
{
	verify(state==MS_ST_CONTROLER);
	state=MS_ST_DSP;

	SetUpdateCallback(DSPTestEnd,5);
}

void ExpireReset()
{
	verify(state==MS_RESETING);
	//Set Self test values :)
	state=MS_ST_CONTROLER;
	//k, lets set values
	SetBits(0x1E,4);
	//modem_regs[
	SetUpdateCallback(SelfTestEnd,5);
}

void modem_reset(u32 v)
{
	if (v==0)
	{
		memset(modem_regs,0,sizeof(modem_regs));
		state=MS_RESET;
		modem_regs[0x20]=0;
		printf("Modem reset start ...\n");
	}
	else
	{
		state=MS_RESETING;
		modem_regs[0x20]=1;
		SetUpdateCallback(ExpireReset,390);
		printf("Modem reset end ...\n");

	}
}
u32 FASTCALL ModemReadMem_A0_006(u32 addr,u32 size)
{
	u32 reg=addr&0x7FF;
	verify((reg&3)==0);
	reg>>=2;

	printf("modem reg %03X read -- wtf is it ?\n",reg);

	if (reg<0x100)
	{
		verify(reg<=1);
		return MODEM_ID[reg];
	}
	else
	{
		reg-=0x100;
		if (reg<0x21)
			return modem_regs[reg];
		else
		{
			printf("modem reg %03X read -- wtf is it ?\n",reg);
			return 0;
		}
	}

}
void FASTCALL ModemWriteMem_A0_006(u32 addr,u32 data,u32 size)
{
	u32 reg=addr&0x7FF;
	verify((reg&3)==0);
	reg>>=2;

	printf("modem reg %03X write -- wtf is it ?\n",reg);

	if (reg<0x100)
	{
		verify(reg<=1);
	}
	else
	{
		reg-=0x100;
		if (reg<0x20)
		{
			modem_regs[reg]=data;
			return;
		}
		else if (reg==0x20)
		{
			modem_reset(data);
		}
		else
		{
			printf("modem reg %03X write -- wtf is it ?\n",reg);
			return;
		}
	}
} 