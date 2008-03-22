#include "aica.h"
#include "arm7.h"
#include "sgc_if.h"
#include "mem.h"
#include <math.h>


#define SAMPLE_CYCLES (DCclock/(44100/441))

s32 rem_sh4_cycles=0;


#define SH4_IRQ_BIT (1<<(u8)holly_SPU_IRQ)

CommonData_struct* CommonData;
DSPData_struct* DSPData;
InterruptInfo* MCIEB;
InterruptInfo* MCIPD;
InterruptInfo* MCIRE;
InterruptInfo* SCIEB;
InterruptInfo* SCIPD;
InterruptInfo* SCIRE;

//Interrupts
//arm side
//This is an intc that replaces the m68k cpu's intc .. i name it e68k :p
void SetL(u32 witch)
{
	if (witch>7)
		witch=7;	//higher bits share bit 7

	u32 bit=1<<witch;

	if (CommonData->SCILV0 & bit)
		CommonData->L0=1;
	else
		CommonData->L0=0;

	if (CommonData->SCILV1 & bit)
		CommonData->L1=1;
	else
		CommonData->L1=0;
	
	if (CommonData->SCILV2 & bit)
		CommonData->L2=1;
	else
		CommonData->L2=0;
}

//Set to true when the out of the intc is 1
bool e68k_out = false;
void update_e68k()
{
	if (!e68k_out)
	{
		//if no pending int
		//look for one !
		u32 p_ints = SCIEB->full & SCIPD->full;

		//if there is one pending
		if (p_ints !=0)
		{
			u32 bit_value=1;//first bit
			//scan all interrupts , lo to hi bit.I assume low bit ints have higher priority over others
			for (u32 i=0;i<11;i++)
			{
				if (p_ints & bit_value)
				{
					//for the first one , Set the L reg & exit
					SetL(i);
					break;
				}
				bit_value<<=1;	//next bit
			}
			//Set the pending signal
			e68k_out=1;
		}
	}
}

//sh4 side
void UpdateSh4Ints()
{
	u32 p_ints = MCIEB->full & MCIPD->full;
	if (p_ints)
	{
		if ((*aica_params.SB_ISTEXT & SH4_IRQ_BIT )==0)
		{
			//if no interrupt is allready pending then raise one :)
			aica_params.RaiseInterrupt(holly_SPU_IRQ);
		}
	}
	else
	{
		if (*aica_params.SB_ISTEXT&SH4_IRQ_BIT)
		{
			aica_params.CancelInterrupt(holly_SPU_IRQ);
		}
	}

}

////
//Timers :)
struct AicaTimerData
{
	union
	{
		struct 
		{
			u32 count:8;
			u32 md:3;
			u32 nil:5;
			u32 pad:16;
		};
		u32 data;
	};
};
class AicaTimer
{
public:
	AicaTimerData* data;
	s32 c_step;
	u32 m_step;
	u32 id;
	void Init(u8* regbase,u32 timer)
	{
		data=(AicaTimerData*)&regbase[0x2890 + timer*4];
		id=timer;
		m_step=1<<(data->md);
		c_step=m_step;
	}
	void StepTimer()
	{
		c_step--;
		if (c_step==0)
		{
			c_step=m_step;
			data->count++;
			if (data->count==0)
			{
				if (id==0)
				{
					SCIPD->TimerA=1;
					MCIPD->TimerA=1;
				}
				else if (id==1)
				{
					SCIPD->TimerB=1;
					MCIPD->TimerB=1;
				}
				else
				{
					SCIPD->TimerC=1;
					MCIPD->TimerC=1;
				}
			}
		}
	}

	void RegisterWrite()
	{
		u32 n_step=1<<(data->md);
		if (n_step!=m_step)
		{
			m_step=n_step;
			c_step=m_step;
		}
	}
};

AicaTimer timers[3];

//Mainloop
void FASTCALL UpdateAICA(u32 Cycles)
{
	rem_sh4_cycles-=Cycles*441;

	while(rem_sh4_cycles<=0)
	{
		//note that this trades off some sh4/arm sync for simplicity of code :)
		//run arm *first*. Register reads should reflect the current aica state.
		//Note that writes should affect the next state .. meh :p cant have everything :)
		//Interrupts are (idealy) raised right after the sample is done, so they go after aica_sample
		arm_Run(512/arm_sh4_bias);
		AICA_Sample();

		SCIPD->SAMPLE_DONE=1;

		for (int i=0;i<3;i++)
			timers[i].StepTimer();

		update_e68k();

		rem_sh4_cycles+=SAMPLE_CYCLES;
	}

	//Make sure sh4 interrupt system is up to date :)
	UpdateSh4Ints();	
}

//Memory i/o
template<u32 sz>
void WriteAicaReg(u32 reg,u32 data)
{
	switch (reg)
	{
	case REG_L:
		//Read olny
		return;
	case REG_M:
		if (data&1)
		{
			e68k_out=0;//clear the pending interrupt from e68k , i guess it emulates the pending interrupt mask
			update_e68k();
		}
		//Write olny -- reads 0
		break;
	case SCIPD_addr:
		verify(sz!=1);
		if (data & (1<<5))
		{
			SCIPD->SCPU=1;
			update_e68k();
		}
		//Read olny
		return;

	case SCIRE_addr:
		{
			verify(sz!=1);
			SCIPD->full&=~(data /*& SCIEB->full*/ );	//is the & SCIEB->full  correct ? acordint to saturn docs it is :)
			data=0;//Write olny
			//no need to call update_e68k , M must (?) be writen to accept interrupt .. hopefully
		}
		break;

	case MCIPD_addr:
		if (data & (1<<5))
		{
			verify(sz!=1);
			MCIPD->SCPU=1;
			UpdateSh4Ints();
		}
		//Read olny
		return;

	case MCIRE_addr:
		{
			verify(sz!=1);
			MCIPD->full&=~data;
			UpdateSh4Ints();
			//Write olny
		}
		break;

	case TIMER_A:
		WriteMemArr(aica_reg,reg,data,sz);
		timers[0].RegisterWrite();
		break;

	case TIMER_B:
		WriteMemArr(aica_reg,reg,data,sz);
		timers[1].RegisterWrite();
		break;

	case TIMER_C:
		WriteMemArr(aica_reg,reg,data,sz);
		timers[2].RegisterWrite();
		break;

	default:
		WriteMemArr(aica_reg,reg,data,sz);
		break;
	}
}



template void WriteAicaReg<1>(u32 reg,u32 data);
template void WriteAicaReg<2>(u32 reg,u32 data);

//misc :p
void AICA_Init()
{
	verify(sizeof(*CommonData)==0x508);
	verify(sizeof(*DSPData)==0x15C8);

	CommonData=(CommonData_struct*)&aica_reg[0x2800];
	DSPData=(DSPData_struct*)&aica_reg[0x3000];
	//slave cpu (arm7)

	SCIEB=(InterruptInfo*)&aica_reg[0x289C];
	SCIPD=(InterruptInfo*)&aica_reg[0x289C+4];
	SCIRE=(InterruptInfo*)&aica_reg[0x289C+8];
	//Main cpu (sh4)
	MCIEB=(InterruptInfo*)&aica_reg[0x28B4];
	MCIPD=(InterruptInfo*)&aica_reg[0x28B4+4];
	MCIRE=(InterruptInfo*)&aica_reg[0x28B4+8];

	sgc_Init();
	for (int i=0;i<3;i++)
		timers[i].Init(aica_reg,i);
	rem_sh4_cycles=0;
}


void AICA_Term()
{
	sgc_Term();
}
