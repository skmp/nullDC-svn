////////////////////////////////////////////////////////////////////////////////////////
/// @file  aica.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICA_H_
#define AICA_H_

enum
{
	SH4AicaMemory_START = 0x00700000,
	SH4AicaMemory_END = 0x00710000,												 
};

DWORD AICAReadDword(const DWORD uAddress);
WORD AICAReadWord(const DWORD uAddress);
BYTE AICAReadByte(const DWORD uAddress);


void AICAWriteDword(const DWORD uAddress, const DWORD uData);
void AICAWriteWord(const DWORD uAddress, const WORD uData);
void AICAWriteByte(const DWORD uAddress, const BYTE uData);
void AICARefresh(u32 cycles);

TError AicaInit();
void AicaEnd();


#endif
