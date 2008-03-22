////////////////////////////////////////////////////////////////////////////////////////
/// @file  arm7Memory.cpp
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "base.h"
#include "arm7Memory.h"
#include "dc_globals.h"

#define AICA_MEM_SIZE (2*1024*1024)
#define AICA_MEM_MASK (AICA_MEM_SIZE-1)

DWORD Arm7SoundRAMEnd;
DWORD Arm7SoundRAMMask;


void InitArm7Memory()
{
	Arm7SoundRAMEnd = AICA_MEM_SIZE;
	Arm7SoundRAMMask = AICA_MEM_MASK;

	//g_pSH4SoundRAM=(u8*)malloc(AICA_MEM_SIZE);
}