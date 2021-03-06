////////////////////////////////////////////////////////////////////////////////////////
/// @file  aica.cpp
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "base.h"
#include "aica.h"
#include "dc_globals.h"
#include "arm7.h"
//#include "sh4.h"
//#include "sh4_asic.h"
//#include "SH4Memory.h"

#define SEMI_AICA

#ifdef SEMI_AICA
#include "AicaSemi/CAica.h"
#ifdef BAKMODE
#include "AICA/AICASndDriverWaveOut.h"
#include "AICA/AICASndDriverDSound.h"
#endif
#else
#include "AICA/AICAMain.h"
#include "AICA/AICASndDriverWaveOut.h"
#include "AICA/AICASndDriverDSound.h"
#endif
//#include "sh4HWRegisters.h"


////////////////////////////////////////////////////////////////////////////////////////
//
#define INTELLINGENT_MIX_EMULATION

#ifdef SEMI_AICA
const int MIN_PROCESS_SAMPLES_SIZE = 44100/(60*2);
#else
const int MIN_PROCESS_SAMPLES_SIZE = 128;
#endif

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
		COMD_CURRENT_SAMPLE_POS         = 0x2814,
		ARM_TIMER_A_RW_32							  = 0x2890,
		ARM_TIMER_B_RW_32							  = 0x2894,
		ARM_TIMER_C_RW_32							  = 0x2898,
		ARM_SCIEB_RW_32								  = 0x289C,			
		ARM_SCIPD_RW_32								  = 0x28a0,		  
		ARM_SCIRE_W_32								  = 0x28A4,					  
		ARM_INT_REQUEST_RW_32           = 0x2d00,		
		ARM_INT_INT_SH4_Enable_RW_32    = 0x28b4,
		ARM_INTReset_RW_32              = 0x28bc,		
		ARM_RESET_RW_32                 = 0x2c00,

		ARM_MCIEB_RW_32								  = 0x28B4,			
		ARM_MCIPD_RW_32								  = 0x28B8,		  
		ARM_MCIRE_W_32								  = 0x28BC,					  

		// DSP Data

	};

	static TError Init          ();
	static void   ReadTimer     ();
	static DWORD  UpdateTimer   (DWORD uCycles, const DWORD TIMER_RW_32 = TAICA::ARM_TIMER_A_RW_32, const DWORD BIT_TIMER_ON = 0x40 );

	static unsigned char    m_aRegisters[MAX_REGISTERS];
	static DWORD            m_uTimeStamp;
#ifdef SEMI_AICA
	static CAICA m_AICA;
#else
	static CAICAMain<float> m_AICA;
#endif

	static void End();
};

CAICASndDriverDSound* pSndDriver;

#define HW_REG_LONG(a) (*((DWORD*)&TAICA::m_aRegisters[(a)]))
#define HW_REG_WORD(a) (*((WORD*)&TAICA::m_aRegisters[(a)]))
#define HW_REG_BYTE(a) (*((BYTE*)&TAICA::m_aRegisters[(a)]))

// AICA Statics
unsigned char     TAICA::m_aRegisters[TAICA::MAX_REGISTERS];
DWORD             TAICA::m_uTimeStamp;
#ifdef SEMI_AICA
CAICA TAICA::m_AICA;
#else
CAICAMain<float>  TAICA::m_AICA;
#endif

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

		if (HW_REG_LONG(ARM_SCIEB_RW_32) & BIT_TIMER_ON)
		{		
			g_pArm7->RaiseInterrupt();
			HW_REG_LONG(ARM_INT_REQUEST_RW_32) = 2; // Timer Interrupt
		}

		if (HW_REG_LONG(ARM_MCIEB_RW_32) & BIT_TIMER_ON)
		{		
			//TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_SPU_IRQ);			
			params.RaiseInterrupt(holly_SPU_IRQ);
		}
	}

	HW_REG_LONG(TIMER_RW_32) &= 0xffff00;
	HW_REG_LONG(TIMER_RW_32) |= uAuxTimer;

	return uResto;
}

void TAICA::ReadTimer()
{
	// Cycles from last call
	DWORD uCyclesSH4Diff;
	if (sh4_cycles >= m_uTimeStamp)
		uCyclesSH4Diff = sh4_cycles - m_uTimeStamp;
	else
		uCyclesSH4Diff = NUM_CYCLES_PER_BLOCK;
	//uCyclesSH4Diff = (SH4_CYCLES_PER_FRAME-m_uTimeStamp) + g_pSH4->GetCycles();	

	// There's a bug, prevent it so..
	if ( uCyclesSH4Diff > (NUM_CYCLES_PER_BLOCK+300) )
		uCyclesSH4Diff = NUM_CYCLES_PER_BLOCK;

	// Sound Update (sh4=200mhz, sound=44100hz)
	double dIncSamples = (double)uCyclesSH4Diff * m_AICA.GetMixVelocity() * 0.0002205;//((double)uCyclesSH4Diff * 44100.0)/**2.2*/ / 200000000.0;
	static double s_dSamples = 0.0;  
	s_dSamples += dIncSamples;
	int iSamplesToUpdate = (int)s_dSamples;
#ifdef SEMI_AICA
	if ( iSamplesToUpdate >= MIN_PROCESS_SAMPLES_SIZE )
	{
		//    m_AICA.ProcessSync(MIN_PROCESS_SAMPLES_SIZE);
		//  s_dSamples -= (double)MIN_PROCESS_SAMPLES_SIZE;
		m_AICA.ProcessSync(iSamplesToUpdate);
		s_dSamples -= (double)iSamplesToUpdate;
	}
#else

	if ( iSamplesToUpdate >= MIN_PROCESS_SAMPLES_SIZE )
	{
		m_AICA.ProcessSync(iSamplesToUpdate);
		s_dSamples -= (double)iSamplesToUpdate;
	}
#endif

	// UpdateTimers
	static double s_adSamplesTimers[3] = { 0.0, 0.0, 0.0 }; 
	for ( int i = 0 ; i < 3 ; ++i )
	{
		s_adSamplesTimers[i] += dIncSamples;
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

	// Temporal, to debug
	/*static double s_dCyclesPerMixSecond = 0.0;
	static DWORD s_uCyclesTemp = 0;
	s_dCyclesPerMixSecond += dIncSamples;
	s_uCyclesTemp += uCyclesSH4Diff;
	if ( (int)s_dCyclesPerMixSecond >= 44100 )
	{
	char szTemp[256];
	sprintf(szTemp,"Cycles per mix second(%i) (%i) (%f) (%i) (%i) (%i) (%i)\n",s_uCyclesTemp,iTemp,(float)s_dCyclesPerMixSecond,iTemp1,iTemp2,iTemp3,iTemp2+iTemp3);
	OutputDebugString(szTemp);
	s_uCyclesTemp = 0;
	s_dCyclesPerMixSecond -= (double)((int)s_dCyclesPerMixSecond);
	iTemp = 0;
	iTemp1 = 0;
	iTemp2 = 0;
	iTemp3 = 0;
	}*/

	// New TimeStamp
	m_uTimeStamp += uCyclesSH4Diff;
	if (m_uTimeStamp>=SH4_CYCLES_PER_FRAME)
		m_uTimeStamp = NUM_CYCLES_PER_BLOCK;
	//m_uTimeStamp = m_uTimeStamp - SH4_CYCLES_PER_FRAME;
}


void AICARefresh()
{
	/*
	{
	static int iAux = 0;

	iAux++;
	if (iAux&0x200)
	DirectSoundDoWork();
	}*/
	TAICA::ReadTimer();

	AICAWriteDword(0x2808,0x100);
}

#ifdef SEMI_AICA
#ifdef BAKMODE
class CCallbackSound : public IAICASndDriver::ICallback
{
public:
	void      GetNewBlockData   (signed short* paOutput)  { TAICA::m_AICA.GetSamples(BlockSize,paOutput); }
	void      SetBlockDataSize  (int iSamples)            { BlockSize=iSamples; }

private:
	int BlockSize;
} g_ProcessSound;
#endif
#else
class CCallbackSound : public IAICASndDriver::ICallback
{
public:
	void      GetNewBlockData   (signed short* paOutput)  { TAICA::m_AICA.GetNewBlockData(paOutput); }
	void      SetBlockDataSize  (int iSamples)            { TAICA::m_AICA.SetBlockDataSize(iSamples); }

private:
} g_ProcessSound;
#endif

extern HWND g_hWnd;
TError TAICA::Init()
{
	ZeroMemory(m_aRegisters,sizeof(m_aRegisters));


	HW_REG_LONG(TAICA::ARM_SCIPD_RW_32) = 0x0;					    
	HW_REG_LONG(TAICA::ARM_TIMER_A_RW_32) = 0x0;
	HW_REG_LONG(TAICA::ARM_TIMER_B_RW_32) = 0x0;
	HW_REG_LONG(TAICA::ARM_TIMER_C_RW_32) = 0x0;
	HW_REG_LONG(TAICA::ARM_SCIEB_RW_32) = 0x40;
#ifdef SEMI_AICA
#ifdef BAKMODE
	pSndDriver = new CAICASndDriverDSound;
#ifdef XBOX
	pSndDriver->Init(NULL);
#else
	pSndDriver->Init(g_hWnd);
#endif
	TError Error = m_AICA.Init(m_aRegisters,g_pSH4SoundRAM);
	if ( Error == RET_OK )
		pSndDriver->SetCallback(&g_ProcessSound);
#else
	TError Error = m_AICA.Init(m_aRegisters,g_pSH4SoundRAM);
#endif
#else
	//CAICASndDriverWaveOut* pSndDriver = new CAICASndDriverWaveOut;
	pSndDriver = new CAICASndDriverDSound;
#ifdef XBOX
	pSndDriver->Init(NULL);
#else
	pSndDriver->Init(g_hWnd);
#endif
	EAICAMixMode eMixMode = E_AICA_MIXMODE_REAL_EMULATION;
#ifdef INTELLINGENT_MIX_EMULATION
	eMixMode = E_AICA_MIXMODE_INTELLIGENT_EMULATION;
#endif
	TError Error = m_AICA.Init(pSndDriver,eMixMode);
	if ( Error == RET_OK )
		pSndDriver->SetCallback(&g_ProcessSound);
#endif
	return Error;
}

void TAICA::End()
{
	m_AICA.End();
	pSndDriver->End();
}

TError AicaInit()
{
	return TAICA::Init();
}

void AicaEnd()
{
	TAICA::End();

}

DWORD AICAReadDword(const DWORD uAddress)
{
	DWORD uData = 0;

	// Powerful CA
	if ( uAddress == TAICA::COMD_CURRENT_SAMPLE_POS )
	{
		int iChannel = (HW_REG_LONG(TAICA::COMD_CURRENT_CHANNEL_TO_MONITOR)>>8)&0x3f;
		uData = TAICA::m_AICA.GetCurrentPosChannel(iChannel)&0xffff;
	}
	else if (uAddress == (TAICA::ARM_RESET_RW_32))
	{
		uData = READ32LE(&TAICA::m_aRegisters[uAddress]);
		uData&=0xff;

		uData |= DWORD(g_videoCableType)<<8;
	}
	else if (uAddress<TAICA::MAX_REGISTERS)
		uData = READ32LE(&TAICA::m_aRegisters[uAddress]);

	//ODS(("WARNING_L1::AICA::ReadDword %04X at %08X",uData,uAddress));

	return uData;
}
WORD AICAReadWord(const DWORD uAddress)
{
	WORD uData = 0;

	// Powerful CA
	if ( uAddress == TAICA::COMD_CURRENT_SAMPLE_POS )
	{
		int iChannel = (HW_REG_LONG(TAICA::COMD_CURRENT_CHANNEL_TO_MONITOR)>>8)&0x3f;
		uData = TAICA::m_AICA.GetCurrentPosChannel(iChannel)&0xffff;
	}
	else if (uAddress == (TAICA::ARM_RESET_RW_32))
	{
		uData = READ16LE(&TAICA::m_aRegisters[uAddress]);
		uData&=0xff;

		uData |= WORD(g_videoCableType)<<8;
	}
	else if (uAddress<TAICA::MAX_REGISTERS)
		uData = READ16LE(&TAICA::m_aRegisters[uAddress]);

	return uData;
}
BYTE AICAReadByte(const DWORD uAddress)
{
	BYTE uData = 0;

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
#ifndef SEMI_AICA
float Log2(float x)
{
	const float inv_log_of_2 = 1.4426950f;
	return log(x) * inv_log_of_2;
}

inline float CalculatePitchPercent(BYTE cOCT, float fFNS)
{
	int iOCTTable[16] = { 0,1,2,3,4,5,6,7,-8,-7,-6,-5,-4,-3,-2,-1 };
	int iOCT = iOCTTable[cOCT];

	float fPitchPercent = 1200.f*Log2((1024.f+fFNS)/1024.f);
	fPitchPercent = fPitchPercent + 1200.f*(float)iOCT;
	fPitchPercent /= 1200.f;

	return fPitchPercent;
}

inline void AICAUpdateChannelData(DWORD uAddress)
{
	// Channel Data
	if ( uAddress >= TAICA::CHND_RANGE_START && uAddress <= TAICA::CHND_RANGE_END )
	{
		DWORD uChannel = uAddress / TAICA::CHND_SIZE;
		DWORD uStartAdressChannel = uChannel*TAICA::CHND_SIZE;
		DWORD uChanAddress = uAddress - uStartAdressChannel;

		//if ( uChannel == 49 )
		switch(uChanAddress)
		{
			// PlayControl
		case TAICA::CHND_PLAYCONTROL:
		case TAICA::CHND_SAMPLEADDRLOW:
			{
				DWORD uAddress = (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_SAMPLEADDRLOW)&0xffff) | (((HW_REG_LONG(uStartAdressChannel)&0x3f)<<16));
				bool bValid = true;//(uData&(1<<15)) != 0;

				/*if ( uChannel == 48 || uChannel == 49)
				{
				DWORD uAddressChanAnterior = (HW_REG_LONG(((uChannel-1)*TAICA::CHND_SIZE)+TAICA::CHND_SAMPLEADDRLOW)&0xffff) | (((HW_REG_LONG(((uChannel-1)*TAICA::CHND_SIZE))&0x3f)<<16));
				int a = 0;
				}*/

				if ( bValid )
				{
					bool bKeyOn = (HW_REG_LONG(uStartAdressChannel)&(1<<14)) != 0;

					// KeyOn
					if ( bKeyOn )
					{
						void* pData = (void*) &g_pSH4SoundRAM[/*((HW_REG_LONG(uStartAdressChannel+TAICA::CHND_SAMPLEADDRLOW)&0xffff) | ((uData&0x3f)<<16))*/uAddress];
						int iType = (HW_REG_LONG(uStartAdressChannel)&0x180)>>7;
						int iSamplesSize = HW_REG_LONG(uStartAdressChannel+TAICA::CHND_SAMPLE_LOOP_END);
						int iLoopIni = -1; // No loop by now
						if ( (HW_REG_LONG(uStartAdressChannel)&(1<<9)) != 0)
							iLoopIni = HW_REG_LONG(uStartAdressChannel+TAICA::CHND_SAMPLE_LOOP_START); // Loop on

						// PitchPercent
						BYTE cOCT = (BYTE)((HW_REG_LONG(uStartAdressChannel+TAICA::CHND_SAMPLE_PITCH)>>11)&0xF);
						float fFNS = (float)(HW_REG_LONG(uStartAdressChannel+TAICA::CHND_SAMPLE_PITCH)&0x3FF);
						float fPitchPercent = CalculatePitchPercent(cOCT,fFNS);
						if ( iType >= 2 && cOCT > 1 ) // adpcm octave is limited to 1
							cOCT = 1;

						int iVolume = (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_SAMPLE_VOL)&0xFF00)>>8;

						int iVolSendDSP = (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_DSP_SEND_VALUES)>>4)&0xf;
						int iVolSendDirect = (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_DIRECT_SEND_VALUES)>>8)&0xf;
						int iPanSendDirect = (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_DIRECT_SEND_VALUES))&0x1f;

						/*char szTemp[256];
						sprintf(szTemp,"PlayChannel(%i) Address(0x%x) Vol(%i) VolDAC(%i) VolDSP(%i) Pan(%i)\n",uChannel,uAddress,iVolume,iVolSendDirect,iVolSendDSP,iPanSendDirect);
						OutputDebugString(szTemp);*/

						// Play channel
						if (iSamplesSize)
							TAICA::m_AICA.PlayChannel(uChannel,pData,(ESampleDataType)iType,iSamplesSize,iLoopIni,fPitchPercent,iVolume,iPanSendDirect);
					}
					// KeyOff, is on??
					else 
					{
						/*char szTemp[256];
						sprintf(szTemp,"Stop(%i)\n",uChannel);
						OutputDebugString(szTemp);*/
						//if ( (HW_REG_LONG(uAddress)&(1<<14)) != 0 )
						{
							// Stop channel
							TAICA::m_AICA.StopChannel(uChannel);
						}
					}
				}
			}
			break;
		case TAICA::CHND_SAMPLE_PITCH:
			// Is On?
			//if ( (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_PLAYCONTROL)&(1<<14)) != 0 )
			{
				// Pitch            
				int iType = (HW_REG_LONG(uStartAdressChannel)&0x180)>>7;
				BYTE cOCT = (BYTE)((HW_REG_LONG(uStartAdressChannel+TAICA::CHND_SAMPLE_PITCH)>>11)&0xF);
				float fFNS = (float)(HW_REG_LONG(uStartAdressChannel+TAICA::CHND_SAMPLE_PITCH)&0x3FF);
				float fPitchPercent = CalculatePitchPercent(cOCT,fFNS);
				if ( iType >= 2 && cOCT > 1 ) // adpcm octave is limited to 1
					cOCT = 1;

				TAICA::m_AICA.SetPitchChannel(uChannel,fPitchPercent);
			}
			break;
		case TAICA::CHND_SAMPLE_VOL:
			{
				// Is On?
				//if ( (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_PLAYCONTROL)&(1<<14)) != 0 )
				{
					int iVolume = (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_SAMPLE_VOL)&0xFF00)>>8;
					TAICA::m_AICA.SetVolumeChannel(uChannel,iVolume);
				}
			}
			break;
		case TAICA::CHND_DIRECT_SEND_VALUES:
			{
				int iVolSendDirect = (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_DIRECT_SEND_VALUES)>>8)&0xf;
				int iPanSendDirect = (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_DIRECT_SEND_VALUES))&0x1f;

				TAICA::m_AICA.SetPanChannel(uChannel,iPanSendDirect);

				/*char szTemp[256];
				sprintf(szTemp,"VolDAC(%i)(%i) Pan(%i)\n",uChannel,iVolSendDirect,iPanSendDirect);
				OutputDebugString(szTemp);*/
			}
			break;
		case TAICA::CHND_DSP_SEND_VALUES:
			{
				int iVolSendDSP = (HW_REG_LONG(uStartAdressChannel+TAICA::CHND_DSP_SEND_VALUES)>>4)&0xf;

				/*char szTemp[256];
				sprintf(szTemp,"VolDSP(%i)(%i)\n",uChannel,iVolSendDSP);
				OutputDebugString(szTemp);*/
			}
			break;
		}
	}
}
#endif

void TestAddress(const DWORD uAddress,const DWORD uData)
{
	if (uAddress == TAICA::ARM_MCIRE_W_32)
	{
		DWORD uMCIPD = HW_REG_LONG(TAICA::ARM_MCIPD_RW_32);

		uMCIPD&=~uData;

		HW_REG_LONG(TAICA::ARM_MCIPD_RW_32) = uMCIPD;

		uMCIPD = uMCIPD & HW_REG_LONG(TAICA::ARM_MCIEB_RW_32);

		if (!uMCIPD)
		{    
			//DWORD* pAddress = SH4HWRegistersGetPtr(TSH4_ASIC::ASIC_ADR_BASE);

			//ASSERT(pAddress);

			// DWORD uEvent = TSH4_ASIC::ASIC_EVT_SPU_IRQ;
			//pAddress[(uEvent>>8)&0xff] &= ~(1<<(uEvent&0xff));    
			u32 Interrupt = 1<<(u8)holly_SPU_IRQ;
			if (*params.SB_ISTEXT&Interrupt)
				params.CancelInterrupt(holly_SPU_IRQ);
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
					TAICA::m_uTimeStamp = sh4_cycles;
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

	// ChannelData
#ifdef SEMI_AICA
	TAICA::m_AICA.UpdateChannelData(uAddress);
	TAICA::m_AICA.UpdateChannelData(uAddress+1);
#else
	AICAUpdateChannelData(uAddress);
#endif

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
				TAICA::m_uTimeStamp = sh4_cycles;
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
#ifdef SEMI_AICA
		TAICA::m_AICA.UpdateChannelData(uAddress);
		TAICA::m_AICA.UpdateChannelData(uAddress+1);
#else
		AICAUpdateChannelData(uAddress);
#endif
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
				TAICA::m_uTimeStamp = sh4_cycles;
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
#ifdef SEMI_AICA
		TAICA::m_AICA.UpdateChannelData(uAddress);
#else
		AICAUpdateChannelData(uAddress);
#endif
	}
}

