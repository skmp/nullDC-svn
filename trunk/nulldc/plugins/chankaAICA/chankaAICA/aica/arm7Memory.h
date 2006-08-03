////////////////////////////////////////////////////////////////////////////////////////
/// @file  arm7Memory.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef ARM7MEMORY_H_
#define ARM7MEMORY_H_

extern DWORD Arm7SoundRAMMask;
extern DWORD Arm7SoundRAMEnd;


#define Arm7AICARegsBase 0x00800000
#define Arm7AICARegsEnd (Arm7AICARegsBase+0x10000)

//#include "SH4Memory.h"
#include "aica.h"

void InitArm7Memory();



inline unsigned char* Arm7GetSystemRAMPtr(DWORD uAddress)
{
	if (uAddress<Arm7SoundRAMEnd)
		return &g_pSH4SoundRAM[uAddress];
	else
		return NULL;
}


inline DWORD Arm7ReadMemory(DWORD uAddress)
{
	if (uAddress<Arm7SoundRAMEnd)
		return *((DWORD*)&g_pSH4SoundRAM[uAddress]);
	else if (uAddress>=Arm7AICARegsBase && uAddress<Arm7AICARegsEnd)
		return AICAReadDword(uAddress-Arm7AICARegsBase);	
	else
	{
		ASSERT(false);
		return 0;
	}
}

inline WORD Arm7ReadHalfWord(DWORD uAddress)
{
	if (uAddress<Arm7SoundRAMEnd)
		return *((WORD*)&g_pSH4SoundRAM[uAddress]);
	else if (uAddress>=Arm7AICARegsBase && uAddress<Arm7AICARegsEnd)
		return AICAReadWord(uAddress-Arm7AICARegsBase);	
	else
	{
		ASSERT(false);
		return 0;
	}
}

inline WORD Arm7ReadHalfWordQuick(DWORD uAddress)
{
	return *((WORD*)&g_pSH4SoundRAM[uAddress&Arm7SoundRAMMask]);
}

inline BYTE Arm7ReadByteQuick(DWORD uAddress)
{
	return *((BYTE*)&g_pSH4SoundRAM[uAddress&Arm7SoundRAMMask]);
}

inline short Arm7ReadHalfWordSigned(DWORD uAddress)
{
	return (short) Arm7ReadHalfWord(uAddress);
}

inline BYTE Arm7ReadByte(const DWORD uAddress)
{
	if (uAddress<Arm7SoundRAMEnd)
		return *((BYTE*)&g_pSH4SoundRAM[uAddress]);
	else if (uAddress>=Arm7AICARegsBase && uAddress<Arm7AICARegsEnd)
		return AICAReadByte(uAddress-Arm7AICARegsBase);	
	else
	{
		ASSERT(false);
		return 0;
	}
}


inline DWORD Arm7ReadMemoryQuick(DWORD uAddress)
{
	return *((DWORD*)&g_pSH4SoundRAM[uAddress&Arm7SoundRAMMask]);
}

inline void Arm7WriteMemory(DWORD uAddress, DWORD uData)
{	
	if (uAddress<Arm7SoundRAMEnd)
		*((DWORD*)&g_pSH4SoundRAM[uAddress]) = uData;
	else if (uAddress>=Arm7AICARegsBase && uAddress<Arm7AICARegsEnd)
		AICAWriteDword(uAddress-Arm7AICARegsBase,uData);	
	else
		ASSERT(false);
}

inline void Arm7WriteHalfWord(DWORD uAddress, WORD uData)
{	
	if (uAddress<Arm7SoundRAMEnd)
		*((WORD*)&g_pSH4SoundRAM[uAddress]) = uData;
	else if (uAddress>=Arm7AICARegsBase && uAddress<Arm7AICARegsEnd)
		AICAWriteWord(uAddress-Arm7AICARegsBase,uData);	
	else
		ASSERT(false);
}

inline void Arm7WriteByte(const DWORD uAddress, const BYTE uData)
{	
	if (uAddress<Arm7SoundRAMEnd)
		*((BYTE*)&g_pSH4SoundRAM[uAddress]) = uData;
	else if (uAddress>=Arm7AICARegsBase && uAddress<Arm7AICARegsEnd)
		AICAWriteByte(uAddress-Arm7AICARegsBase,uData);	
	else
		ASSERT(false);
}


#endif
