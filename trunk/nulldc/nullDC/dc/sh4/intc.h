#pragma once

#include "..\..\types.h"
#include "sh4_if.h"


//Enums

int UpdateINTC();
void RaiseInterrupt(InterruptID intr);
bool Do_Exeption(u32 lvl, u32 expEvn, u32 CallVect);

union INTC_ICR_type
{
	u16 reg_data;
	struct
	{
		u32 res:7;
		u32 IRLM:1;
		u32 NMIE:1;
		u32 NMIB:1;
		u32 res_2:4;
		u32 MAI:1;
		u32 NMIL:1;
	};
};

union INTC_IPRA_type
{
	u16 reg_data;
	struct
	{
		u32 RTC:4;
		u32 TMU2:4;
		u32 TMU1:4;
		u32 TMU0:4;
	};
};

union INTC_IPRB_type
{
	u16 reg_data;
	struct
	{
		u32 Reserved:4;
		u32 SCI1:4;
		u32 REF:4;
		u32 WDT:4;
	};
};

union INTC_IPRC_type
{
	u16 reg_data;
	struct
	{
		u32 Hitachi_UDI:4;
		u32 SCIF:4;
		u32 DMAC:4;
		u32 GPIO:4;
	};
};

typedef bool IsPendingFP();
typedef u32 GetPrLvlFP();

struct InterptSourceList_Entry
{
	IsPendingFP* IsPending;
	GetPrLvlFP* GetPrLvl;
	u32 IntEvnCode;
	u32 BranchOffset;
};



extern INTC_ICR_type  INTC_ICR;
extern INTC_IPRA_type INTC_IPRA;
extern INTC_IPRB_type INTC_IPRB;
extern INTC_IPRC_type INTC_IPRC;

void intc_Init();
void intc_Reset(bool Manual);
void intc_Term();

extern bool InterruptsArePending;
