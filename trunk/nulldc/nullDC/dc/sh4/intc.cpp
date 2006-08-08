//Sh4 interrupt controller
//needs a rewrite and check varius things all around

#include "types.h"
#include "intc.h"
#include "tmu.h"
#include "ccn.h"
#include "sh4_Registers.h"
#include "sh4_cst.h"
#include "dc/mem/sh4_internal_reg.h"
#include "gdb_stub/gdb_stub.h"
#include "dc/asic/asic.h"

/*
	Now .. here is some text to keep you busy :D

	On sh4
	an Interrupt request is created , by setting the Interrupt request input to hi
	while it is hi , when this Interrupt can be processed , it is 
	the Interrupt code , must set the input to low or after exiting the Interrupt handler the
	Interrupt may be re accepted
	on dc (extra )
	Dc has 2 Interrupt controlers, one on the Holly block (asic) that handles all Interrupts releated to external h/w
	and one on the sh4 that handles all Interrupts that hapen iside the sh4 (from the modules) and , has 3 line input
	from the asic.
	the asic has many Interrupts , 21 normal , 4 external (eg , modem) and 32 error Interrupts.Asic works like a mixer
	and it outputs 3 Interrupt signals from all that.if many Interrupts(asic) occur at once, they can share the same Interrupt(intc)

	intc sorts the Interrupts based on pririty and trys to accept the one w/ the hiest priority first

	how all that crap is implemented :
	The new code is much more accurate, emulating Interrupt request lines (InterruptSourceList).
	This means thast the RaiseInterrupt can't be the olny thing needed for Interrupts but we 
	also need callbacks to get the Interrupt reguest line status.

	On Asic , things are simple , RaiseInterrupt sets the asic Interrupt bits
	and according to the settings on asic , it may signal pedning Interrupts
	withc are checked within CheckInterrupts.For implementing new asic insterupts
	adding em on the Interrupts list is all that is needed

	For internal sh4 Interrupts , exept form adding em on the Interrupts list , 
	you must write seperate Priority and Pending callbacks

	//PERFORMANCE NOTICE\\
	This intc is much more accurate when compared to the old one and much slower.
	Thanks to the detection of any pending Interrupts tho (Flags set at RaiseInterrupt)
	it is much faster when interrupts are not hapening (eg , 99% o the cpu operation).
	Also , the structure of this intc allows us to know much earlier if Interrupts will 
	not be accepted (eg , bl bit ect) so , it is much faster that the previus one when
	Interrupts are disabled using sr.bl or sr.imask

	//conlusion\\
	We have an almost 100% acurate intc while it is faster that the previus one :)
	i'd say that this is a success meh

	missing things :
	More internal Interrupts [olny timers are emulated]...
	Exeptions [both here and on cpu code ...]

	now , let's get to the code
*/

const InterptSourceList_Entry InterruptSourceList[]=
{
	//TMU Interrupts
	{tmu_CNT0Pending,tmu_CNT0Priority,0x400,0x600},
	{tmu_CNT1Pending,tmu_CNT1Priority,0x420,0x600},
	{tmu_CNT2Pending,tmu_CNT2Priority,0x440,0x600},
	//Holly Interrupts
	{asic_RL6Pending,asic_GetRL6Priority,0x320,0x600},
	{asic_RL4Pending,asic_GetRL4Priority,0x360,0x600},
	{asic_RL2Pending,asic_GetRL2Priority,0x3A0,0x600},
	//End of list
	{0,0,0,0}
};

void RaiseInterrupt(InterruptID intr);
bool Do_Interrupt(u32 lvl, u32 intEvn, u32 CallVect);
bool Do_Exeption(u32 lvl, u32 expEvn, u32 CallVect);
int Check_Ints();
bool HandleSH4_exept(InterruptID expt);


#define IPr_LVL6  0x6
#define IPr_LVL4  0x4
#define IPr_LVL2  0x2

extern bool sh4_sleeping;

bool InterruptsArePending=true;

INTC_ICR_type  INTC_ICR;
INTC_IPRA_type INTC_IPRA;
INTC_IPRB_type INTC_IPRB;
INTC_IPRC_type INTC_IPRC;

InterruptID intr_l;

void RaiseExeption(u32 code,u32 vector)
{
	verify(sr.BL == 0);
		
	spc = pc;
	ssr = sr.full;
	sgr = r[15];
	CCN_EXPEVT = code;
	sr.MD = 1;
	sr.RB = 1;
	sr.BL = 1;
	pc = vbr + vector;
	UpdateSR();
}
void RaiseInterrupt(InterruptID intr)
{
	intr_l=intr;
	InterruptsArePending=true;
	//route Interrupt to handler
	InterruptType type = (InterruptType)((u32)intr & (u32)InterruptTypeMask);
	switch (type)
	{
		case sh4_int:
			//whatever..
			//this is no more needed :D
			//may be needed some time later tho .. hmm
			break;

		case sh4_exp:
			//look above ++ no exeption emulation for now ...
			break;

		case holly_nrm:
			RaiseAsicNormal(intr);
			break;

		case holly_ext:
			RaiseAsicExt(intr);
			break;

		case holly_err:
			RaiseAsicErr(intr);
			break;
	}
}



int UpdateINTC()
{
	u32 srlvl=sr.IMASK;


	//Speed optimization , check olny if thre are any Interrupts actualy pending :)
	if (!InterruptsArePending)
		return false;

	//if an Interrupt can be done :
	if (srlvl==0xf)
		return false;

	if ((sr.BL==1) &&(!sh4_sleeping))
		return false;

	InterruptsArePending=false;//guess no until proven wrong
	s32 interID=-1;

	for (int i=0;InterruptSourceList[i].IsPending;i++)
	{
		if (InterruptSourceList[i].IsPending())
		{
			InterruptsArePending=true;
			if (InterruptSourceList[i].GetPrLvl()>srlvl)
			{
				interID=i;
				srlvl=InterruptSourceList[i].GetPrLvl();
				if (srlvl==0xf)
					break;
			}
		}
	}


	if (interID!=-1)
	{
		return Do_Interrupt(srlvl,InterruptSourceList[interID].IntEvnCode,InterruptSourceList[interID].BranchOffset);
	}

	return false;
}

//this is what left from the old intc .. meh .. 
//exeptions are handled here .. .. hmm are they ? :P
bool HandleSH4_exept(InterruptID expt)
{
	switch(expt)
	{

	case sh4_ex_TRAP:
		return Do_Exeption(0,0x160,0x100);

	default:
		return false;
	}
}

bool Do_Interrupt(u32 lvl, u32 intEvn, u32 CallVect)
{
	//printf("Interrupt : %d,0x%x,0x%x\n",lvl,intEvn,CallVect);
	CCN_INTEVT = intEvn;

	ssr = sr.GetFull();
	spc = pc;
	sgr = r[15];
	sr.BL = 1;
	sr.MD = 1;
	sr.RB = 1;
	UpdateSR();
	AddCall(pc,spc,vbr+CallVect,1);
	pc = vbr + CallVect;
	//pc_ptr=(u16*)GetMemPtr(pc);

	return true;
}

bool Do_Exeption(u32 lvl, u32 expEvn, u32 CallVect)
{
	if (GDB_EXEPTION_FILTER(expEvn))
	{
		pc-=2;
		return true;
	}

	CCN_EXPEVT = expEvn;

	ssr = sr.GetFull();
	spc = pc+2;
	sgr = r[15];
	sr.BL = 1;
	sr.MD = 1;
	sr.RB = 1;
	UpdateSR();
	//CallStackTrace.cstAddCall(sh4.pc, sh4.pc, sh4.vbr + 0x600, CallType.Interrupt);
	AddCall(pc,spc,vbr+CallVect,1);
	pc = vbr + CallVect;
	//pc_ptr=(u16*)GetMemPtr(pc);

	pc-=2;//fix up ;)
	return true;
}

//Init/Res/Term
void intc_Init()
{
	//INTC ICR 0xFFD00000 0x1FD00000 16 0x0000 0x0000 Held Held Pclk
	INTC[(INTC_ICR_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	INTC[(INTC_ICR_addr&0xFF)>>2].NextCange=0;
	INTC[(INTC_ICR_addr&0xFF)>>2].readFunction=0;
	INTC[(INTC_ICR_addr&0xFF)>>2].writeFunction=0;
	INTC[(INTC_ICR_addr&0xFF)>>2].data16=&INTC_ICR.reg_data;

	//INTC IPRA 0xFFD00004 0x1FD00004 16 0x0000 0x0000 Held Held Pclk
	INTC[(INTC_IPRA_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	INTC[(INTC_IPRA_addr&0xFF)>>2].NextCange=0;
	INTC[(INTC_IPRA_addr&0xFF)>>2].readFunction=0;
	INTC[(INTC_IPRA_addr&0xFF)>>2].writeFunction=0;
	INTC[(INTC_IPRA_addr&0xFF)>>2].data16=&INTC_IPRA.reg_data;

	//INTC IPRB 0xFFD00008 0x1FD00008 16 0x0000 0x0000 Held Held Pclk
	INTC[(INTC_IPRB_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	INTC[(INTC_IPRB_addr&0xFF)>>2].NextCange=0;
	INTC[(INTC_IPRB_addr&0xFF)>>2].readFunction=0;
	INTC[(INTC_IPRB_addr&0xFF)>>2].writeFunction=0;
	INTC[(INTC_IPRB_addr&0xFF)>>2].data16=&INTC_IPRB.reg_data;

	//INTC IPRC 0xFFD0000C 0x1FD0000C 16 0x0000 0x0000 Held Held Pclk
	INTC[(INTC_IPRC_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	INTC[(INTC_IPRC_addr&0xFF)>>2].NextCange=0;
	INTC[(INTC_IPRC_addr&0xFF)>>2].readFunction=0;
	INTC[(INTC_IPRC_addr&0xFF)>>2].writeFunction=0;
	INTC[(INTC_IPRC_addr&0xFF)>>2].data16=&INTC_IPRC.reg_data;
}

void intc_Reset(bool Manual)
{
}

void intc_Term()
{
}