////////////////////////////////////////////////////////////////////////////////////////
/// @file  arm7.cpp
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "base.h"
#include "Arm7.h"
#include "arm7Memory.h"
#include "dc_globals.h"

//typedef DWORD u32;
//typedef BYTE u8;
///typedef WORD u16;

//typedef int s32;
//typedef short s16;
//typedef char s8;

#define CPUReadHalfWordQuick Arm7ReadHalfWordQuick
#define CPUReadMemoryQuick Arm7ReadMemoryQuick
#define CPUWriteMemory Arm7WriteMemory
#define CPUWriteHalfWord Arm7WriteHalfWord
#define CPUWriteByte Arm7WriteByte
#define CPUReadByte Arm7ReadByte
#define CPUReadMemory Arm7ReadMemory
#define CPUReadHalfWord Arm7ReadHalfWord
#define CPUReadHalfWordSigned Arm7ReadHalfWordSigned

#define reg m_aReg
#define armNextPC m_uArmNextPC


#define CPUUpdateTicksAccesint(a) 1
#define CPUUpdateTicksAccessSeq32(a) 1
#define CPUUpdateTicksAccesshort(a) 1
#define CPUUpdateTicksAccess32(a) 1
#define CPUUpdateTicksAccess16(a) 1

static void CPUSwap(u32 *a, u32 *b)
{
	u32 c = *b;
	*b = *a;
	*a = c;
}
static void CPUSwap(DWORD *a, DWORD *b)
{
	u32 c = *b;
	*b = *a;
	*a = c;
}


CArm7::reg_pair CArm7::m_aReg[45];

int CArm7::memoryWait[16] =
{ 0, 0, 2, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0 };
int CArm7::memoryWait32[16] =
{ 0, 0, 6-1, 0, 0, 0, 0, 0, 8-1, 8-1, 8-1, 8-1, 8-1, 8-1, 8-1, 0 };
int CArm7::memoryWaitSeq[16] =
{ 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 4, 4, 8, 8, 4, 0 };
int CArm7::memoryWaitSeq32[16] =
{ 2, 0, 3, 0, 0, 2, 2, 0, 4, 4, 8, 8, 16, 16, 8, 0 };
int CArm7::memoryWaitFetch[16] =
{ 3, 0, 3, 0, 0, 1, 1, 0, 4, 4, 4, 4, 4, 4, 4, 0 };
int CArm7::memoryWaitFetch32[16] =
{ 6, 0, 6, 0, 0, 2, 2, 0, 8, 8, 8, 8, 8, 8, 8, 0 };

const int CArm7::cpuMemoryWait[16] = {
	0, 0, 2, 0, 0, 0, 0, 0,
		2, 2, 2, 2, 2, 2, 0, 0
};
const int CArm7::cpuMemoryWait32[16] = {
	0, 0, 3, 0, 0, 0, 0, 0,
		3, 3, 3, 3, 3, 3, 0, 0
};

const bool CArm7::memory32[16] =
{ true, false, false, true, true, false, false, true, false, false, false, false, false, false, true, false};

const int CArm7::thumbCycles[] = {
	//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 1
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 2
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 3
		1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 4
		2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 5
		2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 6
		2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 7
		2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 8
		2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 9
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // a
		1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 4, 1, 1,  // b
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // c
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3,  // d
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // e
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2   // f
};



bool CArm7::N_FLAG;
bool CArm7::Z_FLAG;
bool CArm7::C_FLAG;
bool CArm7::V_FLAG;
bool CArm7::armIrqEnable;
bool CArm7::armFiqEnable;
bool CArm7::armState;
int CArm7::armMode;
BYTE CArm7::cpuBitsSet[256];

u16 CArm7::IE;
u16 CArm7::IF;
u16 CArm7::IME;

int CArm7::cpuSavedTicks;
int* CArm7::extCpuLoopTicks = NULL;
int* CArm7::extClockTicks = NULL;
int* CArm7::extTicks = NULL;

bool CArm7::intState = false;
bool CArm7::stopState = false;
bool CArm7::holdState = false;

#define CPU_BREAK_LOOP \
	cpuSavedTicks = cpuSavedTicks - *extCpuLoopTicks;\
	*extCpuLoopTicks = *extClockTicks;

#define CPU_BREAK_LOOP_2 \
	cpuSavedTicks = cpuSavedTicks - *extCpuLoopTicks;\
	*extCpuLoopTicks = *extClockTicks;\
	*extTicks = *extClockTicks;


////////////////////////////////////////////////////////////////////////////////////////
/// @fn      TError CArm7::Init()
/// @return  RET_OK if successful.
/// @brief   Initializes the class. Required before any other method.
////////////////////////////////////////////////////////////////////////////////////////
TError CArm7::Init()
{
	TError Error = RET_OK;

	End();

	// Begin Reset
	m_uStop = -1;
	Reset();
	// End Reset

	int i;
	for(i = 0; i < 256; i++) 
	{
		int count = 0;
		int j;
		for(j = 0; j < 8; j++)
			if(i & (1 << j))
				count++;
		cpuBitsSet[i] = count;
	}


	// Alloc Stuff

	m_bInit = true;
	if (Error != RET_OK)
		CArm7::End();

	return Error;
}

////////////////////////////////////////////////////////////////////////////////////////
/// @fn      void CArm7::End()
/// @brief   Frees all class memory.
////////////////////////////////////////////////////////////////////////////////////////
void CArm7::End()
{
	if (IsOk())
	{
		// Dispose stuff

		m_bInit = false;
	}
}

void CArm7::CPUSwitchMode(int mode, bool saveState, bool breakLoop)
{
	CPUUpdateCPSR();

	switch(armMode) {
	case 0x10:
	case 0x1F:
		reg[R13_USR].I = reg[13].I;
		reg[R14_USR].I = reg[14].I;
		reg[17].I = reg[16].I;
		break;
	case 0x11:
		CPUSwap(&reg[R8_FIQ].I, &reg[8].I);
		CPUSwap(&reg[R9_FIQ].I, &reg[9].I);
		CPUSwap(&reg[R10_FIQ].I, &reg[10].I);
		CPUSwap(&reg[R11_FIQ].I, &reg[11].I);
		CPUSwap(&reg[R12_FIQ].I, &reg[12].I);
		reg[R13_FIQ].I = reg[13].I;
		reg[R14_FIQ].I = reg[14].I;
		reg[SPSR_FIQ].I = reg[17].I;
		break;
	case 0x12:
		reg[R13_IRQ].I  = reg[13].I;
		reg[R14_IRQ].I  = reg[14].I;
		reg[SPSR_IRQ].I =  reg[17].I;
		break;
	case 0x13:
		reg[R13_SVC].I  = reg[13].I;
		reg[R14_SVC].I  = reg[14].I;
		reg[SPSR_SVC].I =  reg[17].I;
		break;
	case 0x17:
		reg[R13_ABT].I  = reg[13].I;
		reg[R14_ABT].I  = reg[14].I;
		reg[SPSR_ABT].I =  reg[17].I;
		break;
	case 0x1b:
		reg[R13_UND].I  = reg[13].I;
		reg[R14_UND].I  = reg[14].I;
		reg[SPSR_UND].I =  reg[17].I;
		break;
	}

	u32 CPSR = reg[16].I;
	u32 SPSR = reg[17].I;

	switch(mode) {
	case 0x10:
	case 0x1F:
		reg[13].I = reg[R13_USR].I;
		reg[14].I = reg[R14_USR].I;
		reg[16].I = SPSR;
		break;
	case 0x11:
		CPUSwap(&reg[8].I, &reg[R8_FIQ].I);
		CPUSwap(&reg[9].I, &reg[R9_FIQ].I);
		CPUSwap(&reg[10].I, &reg[R10_FIQ].I);
		CPUSwap(&reg[11].I, &reg[R11_FIQ].I);
		CPUSwap(&reg[12].I, &reg[R12_FIQ].I);
		reg[13].I = reg[R13_FIQ].I;
		reg[14].I = reg[R14_FIQ].I;
		if(saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_FIQ].I;
		break;
	case 0x12:
		reg[13].I = reg[R13_IRQ].I;
		reg[14].I = reg[R14_IRQ].I;
		reg[16].I = SPSR;
		if(saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_IRQ].I;
		break;
	case 0x13:
		reg[13].I = reg[R13_SVC].I;
		reg[14].I = reg[R14_SVC].I;
		reg[16].I = SPSR;
		if(saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_SVC].I;
		break;
	case 0x17:
		reg[13].I = reg[R13_ABT].I;
		reg[14].I = reg[R14_ABT].I;
		reg[16].I = SPSR;
		if(saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_ABT].I;
		break;    
	case 0x1b:
		reg[13].I = reg[R13_UND].I;
		reg[14].I = reg[R14_UND].I;
		reg[16].I = SPSR;
		if(saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_UND].I;
		break;    
	default:
		//systemMessage(MSG_UNSUPPORTED_ARM_MODE,"Unsupported ARM mode %02x", mode);
		ASSERT(false);
		break;
	}
	armMode = mode;
	CPUUpdateFlags(breakLoop);
	CPUUpdateCPSR();
}

void CArm7::CPUUpdateCPSR()
{
	u32 CPSR = reg[16].I & 0x40;
	if(N_FLAG)
		CPSR |= 0x80000000;
	if(Z_FLAG)
		CPSR |= 0x40000000;
	if(C_FLAG)
		CPSR |= 0x20000000;
	if(V_FLAG)
		CPSR |= 0x10000000;
	if(!armState)
		CPSR |= 0x00000020;
	if (!armFiqEnable)
		CPSR |= 0x40;
	if(!armIrqEnable)
		CPSR |= 0x80;
	CPSR |= (armMode & 0x1F);
	reg[16].I = CPSR;
}

void CArm7::CPUUpdateFlags(bool breakLoop)
{
	u32 CPSR = reg[16].I;

	N_FLAG = (CPSR & 0x80000000) ? true: false;
	Z_FLAG = (CPSR & 0x40000000) ? true: false;
	C_FLAG = (CPSR & 0x20000000) ? true: false;
	V_FLAG = (CPSR & 0x10000000) ? true: false;
	armState = (CPSR & 0x20) ? false : true;
	armIrqEnable = (CPSR & 0x80) ? false : true;
	armFiqEnable = (CPSR & 0x40) ? false : true;
	if(breakLoop) {
		if(armIrqEnable && (IF & IE) && (IME & 1)) {
			CPU_BREAK_LOOP_2;
		}
	}
}

void CArm7::CPUSoftwareInterrupt(int comment)
{
	u32 PC = reg[15].I;
	bool savedArmState = armState;
	CPUSwitchMode(0x13, true, false);
	reg[14].I = PC - (savedArmState ? 4 : 2);
	reg[15].I = 0x08;
	armState = true;
	armIrqEnable = false;
	armNextPC = 0x08;
	reg[15].I += 4;
}

void CArm7::CPUUndefinedException()
{
	u32 PC = reg[15].I;
	bool savedArmState = armState;
	CPUSwitchMode(0x1b, true, false);
	reg[14].I = PC - (savedArmState ? 4 : 2);
	reg[15].I = 0x04;
	armState = true;
	armIrqEnable = false;
	armNextPC = 0x04;
	reg[15].I += 4;  
}

void CArm7::Reset()
{
	m_bEnable = false;
	// clen registers
	memset(&m_aReg[0], 0, sizeof(m_aReg));

	IE       = 0x0000;
	IF       = 0x0000;
	IME      = 0x0000;

	armMode = 0x1F;

	reg[13].I = 0x03007F00;
	reg[15].I = 0x0000000;
	reg[16].I = 0x00000000;
	reg[R13_IRQ].I = 0x03007FA0;
	reg[R13_SVC].I = 0x03007FE0;
	armIrqEnable = true;      
	armFiqEnable = false;

	armState = true;
	C_FLAG = V_FLAG = N_FLAG = Z_FLAG = false;

	// disable FIQ
	reg[16].I |= 0x40;

	CPUUpdateCPSR();

	armNextPC = reg[15].I;
	reg[15].I += 4;

	m_eErrorCode = E_OK;
	m_bFiqPending = false; 
}

void CArm7::CPUInterrupt()
{
	u32 PC = reg[15].I;
	bool savedState = armState;
	CPUSwitchMode(0x12, true, false);
	reg[14].I = PC;
	if(!savedState)
		reg[14].I += 2;
	reg[15].I = 0x18;
	armState = true;
	armIrqEnable = false;

	armNextPC = reg[15].I;
	reg[15].I += 4;
}

void CArm7::CPUFiq()
{
	u32 PC = reg[15].I;
	bool savedState = armState;
	CPUSwitchMode(0x11, true, false);
	reg[14].I = PC;
	if(!savedState)
		reg[14].I += 2;
	reg[15].I = 0x1c;
	armState = true;
	armIrqEnable = false;
	armFiqEnable = false;

	armNextPC = reg[15].I;
	reg[15].I += 4;
}



CArm7::EErrorCode	CArm7::_StepArm7()
{
	EErrorCode eErrorCode;
	eErrorCode = ExecuteInstruction();			

	/*if (eErrorCode == E_OK)
	{
		if (armFiqEnable && m_bFiqPending)
		{
			m_bFiqPending = false;
			CPUFiq();
		}
	}*/

	return eErrorCode;
}

CArm7::EErrorCode	CArm7::_BlockStepArm7(DWORD uNumCycles)
{
  if (!g_bArm7Enable)
    return E_OK;
  //return E_OK;
	/*
	DWORD i;
	EErrorCode eErrorCode = E_OK;	
	for (i=0;eErrorCode == E_OK && i<uNumCycles;i++)
	{
		if (m_uArmNextPC != m_uStop)
			eErrorCode = ExecuteInstruction();
		else
			eErrorCode = E_BREAKPOINT;
	}

	if (m_uArmNextPC == m_uStop)
		eErrorCode = E_BREAKPOINT;

	if (eErrorCode == E_OK)
	{
		if (armFiqEnable && m_bFiqPending)
		{
			m_bFiqPending = false;
			CPUFiq();
		}
	}
	return eErrorCode;
	*/

	
	DWORD uCurrentClocks = 0;	
	while (uCurrentClocks<uNumCycles)
	{
		ExecuteInstruction();
		uCurrentClocks +=m_uClockTicks+1;
	}

	if (armFiqEnable && m_bFiqPending)
	{
		m_bFiqPending = false;
		CPUFiq();
	}
	

	return E_OK;
}

CArm7::EErrorCode	CArm7::ExecuteInstruction()
{
	
	if (armState)
	{	
#include "arm-new.h"
	}
	else
	{
//#include "thumb.h"
		printf("WTF ? ARM CODE CALLED ? WTF !! WTH !!!\n");
	}

	return E_OK;
}



void CArm7::RaiseInterrupt()
{
	m_bFiqPending = true;
}