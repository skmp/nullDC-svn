#pragma once

#include<xmmintrin.h>

#include <vector>
#include <list>
#include <algorithm>
#include <tchar.h>
#include <stdio.h>
#include <D3D9.h>
#include <D3DX9.h>

#include "base.h"
//#include "SH4TextureMemory.h"
//#include "sh4HWRegisters.h"
//#include "sh4_asic.h"
//#include "sh4.h"
//#include "dc_globals.h"
//#include "sh4_dynaRec.h"

//#include "dc_globals.h"
//#include "virtualMemHandler.h"
//#include "dc_globals.h"
//#include "sh4_dynaRec.h"
//#include "sh4.h"

#include "chankaPvr.h"
void SaveSSERegs();
void LoadSSERegs();
#define g_uVideoMemorySize VRAM_SIZE

extern int CurrentFrame;
extern char* g_pSH4TextureMemory;
#define RegSize (0x8000)
#define RegMask (RegSize-1)

extern u8 regs[RegSize];

#define PALETTE_RAM_START_addr		0x00001000	//	RW	Palette RAM	
#define PvrReg(x) (*(u32*)&regs[(x) & RegMask])
#define PALETTE_RAM			(&PvrReg(PALETTE_RAM_START_addr))	//	RW	Palette RAM	
//#define SH4WriteLongMemory(aa,bb)  ((char*)0)

void SH4WriteLongMemory(const DWORD uAddress,const DWORD uData);

DWORD SH4HWRegistersGetValue(DWORD uAddress,DWORD uMask=0xffffffff,DWORD uShift=0);

#define SH4HWRegistersGetPaletteTable()  ((char*)PALETTE_RAM)
#define GETVALUEHWREGISTER(a,b) SH4HWRegistersGetValue(a,b##_MASK,b##_SHIFT)
#define SH4GetVideoRAMPtr(aa)  ((char*)&g_pSH4TextureMemory[(aa) & VRAM_MASK])
#define SH4HWRegistersGetPtr(aa) ((DWORD*)&regs[(aa)&RegMask])

enum
{
	SH4Memory_MASK = ~0xE0000000,
	SH4SystemFlash_START = 0x00200000,
	SH4SystemFlash_END	 = 0x00220000,
	SH4SystemFlash_TAM	 = 0x00020000,

	SH4RomBIOS_START		 = 0x00000000,
	SH4RomBIOS_END			 = 0x00200000,
	SH4RomBIOS_TAM			 = 0x00200000,

	SH4VideoRAM_START = 0x05000000,
	
	

	SH4SystemRAM_START = 0x0C000000,
	SH4SystemRAM_END   = 0x10000000,
	//SH4SystemRAM_TAM  = 0x01000000, // 16 MB
	//SH4SystemRAM_MASK  = ~0x03000000, // 16 MB

	SH4TAFifoPolygon_START = 0x10000000,
	SH4TAFifoPolygon_END =	 0x10800000,														 	

  SH4MMUAdrArray_START = 0xF6000000,
  SH4MMUAdrArray_END =   0xF7000000,

  SH4MMUDataArray_START = 0xF7000000,
  SH4MMUDataArray_END =   0xF8000000,
};



#include "pvr.h"
#include "taVideo.h"
#include "d3dVertexFormat.h"
#include "DisplayDevice.h"
#include "cacheTextures.h"
#include "d3dfont.h"

