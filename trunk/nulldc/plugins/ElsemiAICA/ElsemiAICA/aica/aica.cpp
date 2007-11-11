////////////////////////////////////////////////////////////////////////////////////////
/// @file  aica.cpp
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////



#include "stdafx.h"
#include "base.h"
#include "aica.h"
#include "dc_globals.h"
#include "arm7.h"
#include "../chanka_aica.h"



#include "AicaSemi/CAICA.h"


////////////////////////////////////////////////////////////////////////////////////////
//
#define INTELLINGENT_MIX_EMULATION

int MIN_PROCESS_SAMPLES_SIZE = 44100/(60*2);

struct TAICA
{
	enum
	{
		MAX_REGISTERS = 0x10000,

    // Channel Data
      CHND_RANGE_START              = 0x0000,
      CHND_RANGE_END                = 0x1FFC,
      CHND_SIZE                     = 0x80,
      CHND_PLAYCONTROL              = 0x0000,
      CHND_SAMPLEADDRLOW            = 0x0004,
      CHND_SAMPLE_LOOP_START        = 0x0008,
      CHND_SAMPLE_LOOP_END          = 0x000C,
      CHND_SAMPLE_PITCH             = 0x0018,
      CHND_DSP_SEND_VALUES          = 0x0020,
      CHND_DIRECT_SEND_VALUES       = 0x0024,
      CHND_SAMPLE_VOL               = 0x0028,

    // Common Data
      COMD_CURRENT_CHANNEL_TO_MONITOR = 0x280C,
	  COMD_CURRENT_ENV_POS         = 0x2810,
      COMD_CURRENT_SAMPLE_POS         = 0x2814,
		  ARM_TIMER_A_RW_32							  = 0x2890,
			ARM_TIMER_B_RW_32							  = 0x2894,
			ARM_TIMER_C_RW_32							  = 0x2898,
			ARM_SCIEB_RW_32								  = 0x289C,			
			ARM_SCIPD_RW_32								  = 0x28a0,		  
			ARM_SCILV0_RW_32								  = 0x28a8,		  
			ARM_SCILV1_RW_32								  = 0x28ac,		  
			ARM_SCILV2_RW_32								  = 0x28b0,		  
			ARM_SCIRE_W_32								  = 0x28A4,					  
		  ARM_INT_REQUEST_RW_32           = 0x2d00,		
		  ARM_INT_CLR_RW_32           = 0x2d04,		
		  ARM_INT_INT_SH4_Enable_RW_32    = 0x28b4,
		  ARM_INTReset_RW_32              = 0x28bc,		
      ARM_RESET_RW_32                 = 0x2c00,

			ARM_MCIEB_RW_32								  = 0x28B4,			
			ARM_MCIPD_RW_32								  = 0x28B8,		  
			ARM_MCIRE_W_32								  = 0x28BC,					  

    // DSP Data

	};

  static TError Init          ();
  static void   ReadTimer     (u32 cycles);
  static DWORD  UpdateTimer   (DWORD uCycles, const DWORD TIMER_RW_32 = TAICA::ARM_TIMER_A_RW_32, const DWORD BIT_TIMER_ON = 0x40 );
  static void CheckIRQ		  ();

	static unsigned char    m_aRegisters[MAX_REGISTERS];
	//static DWORD            m_uTimeStamp;

	static CAICA m_AICA;

  static void End();
};

#define HW_REG_LONG(a) (*((DWORD*)&TAICA::m_aRegisters[(a)]))
#define HW_REG_WORD(a) (*((WORD*)&TAICA::m_aRegisters[(a)]))
#define HW_REG_BYTE(a) (*((BYTE*)&TAICA::m_aRegisters[(a)]))

// AICA Statics
unsigned char     TAICA::m_aRegisters[TAICA::MAX_REGISTERS];
//DWORD             TAICA::m_uTimeStamp;

CAICA TAICA::m_AICA;
#define aica_reg (TAICA::m_aRegisters)

#if 0
DWORD TAICA::UpdateTimer(DWORD uCycles, const DWORD TIMER_RW_32, const DWORD BIT_TIMER_ON )
{
  DWORD uResto = 0;
	DWORD uAuxTimer = HW_REG_LONG(TIMER_RW_32);

  ASSERT((uAuxTimer&0x700) == 0); // only inc 1 sample supported by now

	uAuxTimer&=0xff;
  uAuxTimer+=uCycles;

	if (uAuxTimer>=0x100)
	{
    uResto = uAuxTimer&0xff;
    uAuxTimer = 0;

		HW_REG_LONG(ARM_SCIPD_RW_32) |= BIT_TIMER_ON;		
		HW_REG_LONG(ARM_MCIPD_RW_32) |= BIT_TIMER_ON;		

/*		if(HW_REG_LONG(ARM_SCIEB_RW_32) & (~0x80))
			int a=1;
		if (HW_REG_LONG(ARM_SCIEB_RW_32) & BIT_TIMER_ON)
		{		
			g_pArm7->RaiseInterrupt();
			HW_REG_LONG(ARM_INT_REQUEST_RW_32) = 2; // Timer Interrupt
		}

		if (HW_REG_LONG(ARM_MCIEB_RW_32) & BIT_TIMER_ON)
		{		
	    TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_SPU_IRQ);			
		}
*/
	}

	HW_REG_LONG(TIMER_RW_32) &= 0xffff00;
	HW_REG_LONG(TIMER_RW_32) |= uAuxTimer;

  return uResto;
}

u32 sample_cycles=0;
void TAICA::ReadTimer(u32 cycles)
{
	// Cycles from last call
	sample_cycles+=cycles;
	const u32 min_update_cycles=(u32)((u64)200000000*(u64)MIN_PROCESS_SAMPLES_SIZE/(u64)44100);

	if(sample_cycles>min_update_cycles)
	{
		sample_cycles-=min_update_cycles;

		m_AICA.ProcessSync(MIN_PROCESS_SAMPLES_SIZE);
	}

	// UpdateTimers
	static double s_adSamplesTimers[3] = { 0.0, 0.0, 0.0 }; 
	for ( int i = 0 ; i < 3 ; ++i )
	{
		s_adSamplesTimers[i] += cycles/44100.0;
		int iSamplesTimersToUpdate = (int)s_adSamplesTimers[i];
		if ( iSamplesTimersToUpdate )
		{
			DWORD aTIMER_RW_32[3] = { ARM_TIMER_A_RW_32, ARM_TIMER_B_RW_32, ARM_TIMER_C_RW_32 };
			DWORD aBIT_TIMER_ON[3] = { 0x40, 0x80, 0x100 };

			int iResto = UpdateTimer(iSamplesTimersToUpdate,aTIMER_RW_32[i],aBIT_TIMER_ON[i]);
			s_adSamplesTimers[i] -= (double)iSamplesTimersToUpdate;
			s_adSamplesTimers[i] += (double)iResto;
		}
	}
}


static int DecodeSCI(int mask)
{
	int irq=0;
	if(mask>0x80)
		mask=0x80;
	if(HW_REG_LONG(TAICA::ARM_SCILV0_RW_32)&mask)
		irq|=1;
	if(HW_REG_LONG(TAICA::ARM_SCILV1_RW_32)&mask)
		irq|=2;
	if(HW_REG_LONG(TAICA::ARM_SCILV2_RW_32)&mask)
		irq|=4;
	return irq;
}


void TAICA::CheckIRQ()
{
	if(HW_REG_LONG(ARM_INT_REQUEST_RW_32)==0xFF)
	{
		if(HW_REG_LONG(ARM_SCIPD_RW_32))
		{
			for(int i=0x400;i;i>>=1)
			{
				if (HW_REG_LONG(ARM_SCIEB_RW_32) & i)
				{	
					HW_REG_LONG(ARM_INT_REQUEST_RW_32) = DecodeSCI(i); // Timer Interrupt
					g_pArm7->RaiseInterrupt();
					break;
				}
			}
		}


		if(HW_REG_LONG(ARM_MCIPD_RW_32))
		{
			for(int i=0x400;i;i>>=1)
			{
				if (HW_REG_LONG(ARM_MCIEB_RW_32) & i)
				{		
					//TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_SPU_IRQ);
					params.RaiseInterrupt(holly_SPU_IRQ);
					break;
				}
			}
		}
	}
}

void AICARefresh(u32 cycles)
{
	TAICA::ReadTimer(cycles);
	TAICA::CheckIRQ();

  AICAWriteDword(0x2808,0x100);
}
#endif

TError TAICA::Init()
{
	ZeroMemory(m_aRegisters,sizeof(m_aRegisters));


	HW_REG_LONG(TAICA::ARM_SCIPD_RW_32) = 0x0;					    
	HW_REG_LONG(TAICA::ARM_TIMER_A_RW_32) = 0x0;
	HW_REG_LONG(TAICA::ARM_TIMER_B_RW_32) = 0x0;
	HW_REG_LONG(TAICA::ARM_TIMER_C_RW_32) = 0x0;
	HW_REG_LONG(TAICA::ARM_SCIEB_RW_32) = 0x40;
	HW_REG_LONG(TAICA::ARM_INT_REQUEST_RW_32) = 0xFF;

	TError Error = m_AICA.Init(m_aRegisters,g_pSH4SoundRAM);
	MIN_PROCESS_SAMPLES_SIZE = m_AICA.GetSuggestedUpdateSize();

  return Error;
}

void TAICA::End()
{
  m_AICA.End();
}

unsigned short AICA_r16(unsigned int addr);
unsigned char AICA_r8(unsigned int addr);
void AICA_w16(unsigned int addr,unsigned short val);
void AICA_w8(unsigned int addr,unsigned char val);

BYTE AICAReadByte(const DWORD uAddress)
{
	BYTE uData = 0;
AICA_r8(uAddress);
  // Powerful CA
  if ( uAddress == TAICA::COMD_CURRENT_SAMPLE_POS || uAddress == TAICA::COMD_CURRENT_SAMPLE_POS+1 )
  {
    int iChannel = (HW_REG_LONG(TAICA::COMD_CURRENT_CHANNEL_TO_MONITOR)>>8)&0x3f;
    uData = (TAICA::m_AICA.GetCurrentPosChannel(iChannel)>>(uAddress-TAICA::COMD_CURRENT_SAMPLE_POS)*8)&0xff;
  }
  else if (uAddress == (TAICA::ARM_RESET_RW_32+1))
  {
    uData = g_videoCableType;
  }
	else if (uAddress<TAICA::MAX_REGISTERS)
		uData = READ8LE(&TAICA::m_aRegisters[uAddress]);

	return uData;
}
WORD AICAReadWord(const DWORD uAddress)
{
	WORD uData = 0;
AICA_r16(uAddress);
  // Powerful CA
  if ( uAddress == TAICA::COMD_CURRENT_SAMPLE_POS )
  {
    int iChannel = (HW_REG_LONG(TAICA::COMD_CURRENT_CHANNEL_TO_MONITOR)>>8)&0x3f;
    uData = TAICA::m_AICA.GetCurrentPosChannel(iChannel)&0xffff;
  }
   else if ( uAddress == TAICA::COMD_CURRENT_ENV_POS )
  {
    int iChannel = (HW_REG_LONG(TAICA::COMD_CURRENT_CHANNEL_TO_MONITOR)>>8)&0x3f;
    uData = TAICA::m_AICA.GetCurrentEnvChannel(iChannel)&0xffff;
  }
  else if (uAddress<TAICA::MAX_REGISTERS)
      uData = READ16LE(&TAICA::m_aRegisters[uAddress]);

	return uData;
}
DWORD AICAReadDword(const DWORD uAddress)
{
	return AICAReadWord(uAddress);
}
#if 0



void TestAddress(const DWORD uAddress,const DWORD uData)
{
	if(uAddress==TAICA::ARM_INT_CLR_RW_32)
	{
		if(uData&1)
		{
			HW_REG_LONG(TAICA::ARM_INT_REQUEST_RW_32)=0xFF;
		}
	}
  if (uAddress == TAICA::ARM_MCIRE_W_32)
  {
    DWORD uMCIPD = HW_REG_LONG(TAICA::ARM_MCIPD_RW_32);

    uMCIPD&=~uData;

    HW_REG_LONG(TAICA::ARM_MCIPD_RW_32) = uMCIPD;

    uMCIPD = uMCIPD & HW_REG_LONG(TAICA::ARM_MCIEB_RW_32);

    if (!uMCIPD)
    {    
      /*DWORD* pAddress = SH4HWRegistersGetPtr(TSH4_ASIC::ASIC_ADR_BASE);

      ASSERT(pAddress);

      DWORD uEvent = TSH4_ASIC::ASIC_EVT_SPU_IRQ;

	  pAddress[(uEvent>>8)&0xff] &= ~(1<<(uEvent&0xff));    */
		u32 Interrupt = (u32)(1 << ((((u32)holly_SPU_IRQ & (u32)InterruptIDMask))));
		*params.SB_ISTEXT&=~Interrupt;
    }
  }
  else if (uAddress == TAICA::ARM_SCIRE_W_32)
  {
    DWORD uSCIPD = HW_REG_LONG(TAICA::ARM_SCIPD_RW_32);

    uSCIPD&=~uData;

    HW_REG_LONG(TAICA::ARM_SCIPD_RW_32) = uSCIPD;

  }

}

void AICAWriteDword(const DWORD uAddress, const DWORD _uData)
{
	DWORD uData = _uData;

  TestAddress(uAddress,_uData);
	
	if (uAddress == TAICA::ARM_MCIPD_RW_32)
	{
		DWORD uData = HW_REG_LONG(TAICA::ARM_MCIEB_RW_32);
		uData &= _uData;
		if (uData&0x20)
		{
			//TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_SPU_IRQ);
			params.RaiseInterrupt(holly_SPU_IRQ);
		}						
	}

	/*if (uData == 0xf && uAddress == 0x2800)
	{
		g_pArm7->SetErrorCode(CArm7::E_BREAKPOINT);
	}*/

	switch(uAddress)
	{
    // Common Data
      // Resets ARM7DI
	    case TAICA::ARM_RESET_RW_32:
		    {
			    BYTE uOld = READ8LE(&TAICA::m_aRegisters[uAddress]);
			    if ((uOld&0x1)^(uData&0x1))
			    {
				    g_pArm7->SetEnable((uData&0x1) == 0);

				    if ((uData&0x1) == 0)
				    {					    
						//TAICA::m_uTimeStamp = sh4_cycles;
						sample_cycles=0;
				    }
			    }
		      //g_pArm7->SetEnable((uData&0x1) == 0);		
			    ODS(("WARNING_L1::AICA::WriteDword %04X at %08X",uData,uAddress));
		    }
		    break;
			case TAICA::ARM_SCIRE_W_32:
				{
					DWORD uDataAux = ~uData;
					HW_REG_LONG(TAICA::ARM_SCIPD_RW_32)&=uDataAux;
				}
				break;
	  default:
      //ASSERT(false);
		  //ODS(("WARNING_L1::AICA::WriteDword %04X at %08X",uData,uAddress));
		  break;
	}
	

	if (uAddress<TAICA::MAX_REGISTERS)
		WRITE32LE(&TAICA::m_aRegisters[uAddress],uData);
	if(uAddress>=0x3000 && uAddress<0x4000 && uData)	
		int a=1;
  // ChannelData
  TAICA::m_AICA.UpdateChannelData(uAddress);
  TAICA::m_AICA.UpdateChannelData(uAddress+1);
}
void AICAWriteWord(const DWORD uAddress, const WORD _uData)
{
	WORD uData = _uData;

  TestAddress(uAddress,_uData);
	if (uAddress == TAICA::ARM_MCIPD_RW_32)
	{
		DWORD uData = HW_REG_LONG(TAICA::ARM_MCIEB_RW_32);
		uData &= _uData;
		if (uData&0x20)
		{
			//TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_SPU_IRQ);
			params.RaiseInterrupt(holly_SPU_IRQ);
		}						
	}
  else if(uAddress == TAICA::ARM_RESET_RW_32)
  {
    BYTE uOld = READ8LE(&TAICA::m_aRegisters[uAddress]);
    if ((uOld&0x1)^(uData&0x1))
    {
      g_pArm7->SetEnable((uData&0x1) == 0);

      if ((uData&0x1) == 0)
      {					    
        //TAICA::m_uTimeStamp = sh4_cycles;
		  sample_cycles=0;
      }
    }
  }

  DWORD uAddress32 = uAddress&0xFFFFFFFC;
  DWORD uOldValue = READ32LE(&TAICA::m_aRegisters[uAddress32]);

	if (uAddress == TAICA::ARM_SCIRE_W_32)
	{
		DWORD uDataAux = ~uData;
		HW_REG_LONG(TAICA::ARM_SCIPD_RW_32)&=uDataAux;
	}	

	if (uAddress<TAICA::MAX_REGISTERS)
		WRITE16LE(&TAICA::m_aRegisters[uAddress],uData);

  // Channel Data
  DWORD uNewValue = READ32LE(&TAICA::m_aRegisters[uAddress32]);
  if ( uOldValue != uNewValue )
  {
    TAICA::m_AICA.UpdateChannelData(uAddress);
    TAICA::m_AICA.UpdateChannelData(uAddress+1);
  }
}


void AICAWriteByte(const DWORD uAddress, const BYTE _uData)
{
	BYTE uData = _uData;

  TestAddress(uAddress,_uData);

	if (uAddress == TAICA::ARM_MCIPD_RW_32)
	{
		DWORD uData = HW_REG_LONG(TAICA::ARM_MCIEB_RW_32);
		uData &= _uData;
		if (uData&0x20)
		{
			//TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_SPU_IRQ);
			params.RaiseInterrupt(holly_SPU_IRQ);
		}						
	}
  else if(uAddress == TAICA::ARM_RESET_RW_32)
  {
    BYTE uOld = READ8LE(&TAICA::m_aRegisters[uAddress]);
    if ((uOld&0x1)^(uData&0x1))
    {
      g_pArm7->SetEnable((uData&0x1) == 0);

      if ((uData&0x1) == 0)
      {					    
        //TAICA::m_uTimeStamp = sh4_cycles;
		  sample_cycles=0;
      }
    }
  }

  DWORD uAddress32 = uAddress&0xFFFFFFFC;
  DWORD uOldValue = READ32LE(&TAICA::m_aRegisters[uAddress32]);

	if (uAddress == TAICA::ARM_SCIRE_W_32)
	{
		DWORD uDataAux = ~uData;
		HW_REG_LONG(TAICA::ARM_SCIPD_RW_32)&=uDataAux;
	}	

  if (uAddress<TAICA::MAX_REGISTERS)
		WRITE8LE(&TAICA::m_aRegisters[uAddress],uData);

  // Channel Data
  DWORD uNewValue = READ32LE(&TAICA::m_aRegisters[uAddress32]);
  if ( uOldValue != uNewValue )
  {
    TAICA::m_AICA.UpdateChannelData(uAddress);
  }
}






#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "aica.h"
#include "arm7.h"
//#include "mem.h"
#include <math.h>

#define SH4_IRQ_BIT (1<<((u8)holly_SPU_IRQ))

CommonData_struct* CommonData;
InterruptInfo* MCIEB;
InterruptInfo* MCIPD;
InterruptInfo* MCIRE;
InterruptInfo* SCIEB;
InterruptInfo* SCIPD;
InterruptInfo* SCIRE;

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
		//find for one !
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

////
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
	u32 c_step;
	u32 m_step;
	u32 id;
	void Init(u8* regbase,u32 timer)
	{
		data=(AicaTimerData*)&regbase[0x2890 + timer*4];
		id=timer;
		m_step=1<<(data->md);
		c_step=0;
	}
	void UpdateTimer(u32 samples)
	{
		c_step+=samples;
		while (c_step>=m_step)
		{
			c_step-=m_step;
			data->count++;
			if (data->count==0)
			{
				//wiii
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
				update_e68k();
			}
		}
	}

	void RegisterWrite()
	{
		u32 n_step=1<<(data->md);
		if (n_step!=m_step)
		{
			m_step=n_step;
			c_step=0;
		}
	}
};

AicaTimer timers[3];
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
		if (*aica_params.SB_ISTEXT & SH4_IRQ_BIT)
			aica_params.CancelInterrupt(holly_SPU_IRQ);
	}
}
u32 sample_cycles=0;
u32 sample_cycles_=0;
void FASTCALL UpdateAICA(u32 Cycles)
{
	//run arm
	g_pArm7->BlockStepArm7(Cycles/(8*ARM7BIAS));
	//*arm_Run(Cycles/(arm_sh4_bias*arm_sh4_ratio));

	//Generate sound 
	// Cycles from last call
	sample_cycles+=Cycles;
	const u32 min_update_cycles=(u32)((u64)200000000*(u64)MIN_PROCESS_SAMPLES_SIZE/(u64)44100);

	u32 sc=0;//AICA_GenerateSamples(Cycles);

	if(sample_cycles>=min_update_cycles)
	{
		sample_cycles-=min_update_cycles;

		TAICA::m_AICA.ProcessSync(MIN_PROCESS_SAMPLES_SIZE);
		
		//sc=MIN_PROCESS_SAMPLES_SIZE;
	}
	sample_cycles_+=Cycles;
	if (sample_cycles_>((u64)200000000/(u64)44100))
	{
		sample_cycles_-=(u64)200000000/(u64)44100;
		sc=1;
	}
	
	if (sc==0)
		return;

	SCIPD->SAMPLE_DONE=1;

	for (int i=0;i<3;i++)
		timers[i].UpdateTimer(sc);

	update_e68k();

	UpdateSh4Ints();
	//Interrupts for arm are handled from the arm mainloop directly :) (jeez , gota love the simple arm interr. system ;p)
}


template<u32 sz>
void WriteAicaReg(u32 reg,u32 data)
{
	if (sz==1)
		AICA_w16(reg,data);
	else
		AICA_w16(reg,data);
	if ((reg ) == 0x2c00)
	{
		//printf("Write to ARM reset, value= %x\n",data);
		//arm_SetEnabled((data&1)==0);
		g_pArm7->SetEnable((data&1)==0);
		data&=~1;
	}
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
		if (reg<0x2000)
		{
			TAICA::m_AICA.UpdateChannelData(reg);
			if (sz!=1)
				TAICA::m_AICA.UpdateChannelData(reg+1);
		}
		break;
	}
}



template void WriteAicaReg<1>(u32 reg,u32 data);
template void WriteAicaReg<2>(u32 reg,u32 data);

void AICA_Init()
{
	memset(aica_reg,0,sizeof(aica_reg));
	CommonData=(CommonData_struct*)&aica_reg[0x2800];
	//slave cpu (arm7)

	SCIEB=(InterruptInfo*)&aica_reg[0x289C];
	SCIPD=(InterruptInfo*)&aica_reg[0x289C+4];
	SCIRE=(InterruptInfo*)&aica_reg[0x289C+8];
	//Main cpu (sh4)
	MCIEB=(InterruptInfo*)&aica_reg[0x28B4];
	MCIPD=(InterruptInfo*)&aica_reg[0x28B4+4];
	MCIRE=(InterruptInfo*)&aica_reg[0x28B4+8];


	for (int i=0;i<3;i++)
		timers[i].Init(aica_reg,i);

	#define log2(n) (log((float) n)/log((float) 2))

	TAICA::Init();
}


void AICA_Term()
{
	TAICA::End();
}
