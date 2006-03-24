#pragma once
#include "base.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//basic types
typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef float f32;
typedef double f64;

extern u8*g_pSH4SoundRAM;

//intc function pointer and enums
enum InterruptType
{
	sh4_int   = 0x00000000,
	sh4_exp   = 0x01000000,
	holly_nrm = 0x20000000,
	holly_ext = 0x21000000,
	holly_err = 0x22000000,
	InterruptTypeMask = 0x7F000000,
	InterruptIDMask=0x00FFFFFF
};

enum InterruptID
{
		//internal interups
		//TODO : Add more internal interrrupts
		sh4_TMU0_TUNI0 = sh4_int |	0x0400,  /* TMU0 underflow */
		sh4_TMU1_TUNI1 = sh4_int |  0x0420,  /* TMU1 underflow */
		sh4_TMU2_TUNI2 = sh4_int |  0x0440,  /* TMU2 underflow */

		//sh4 exeptions 
		sh4_ex_USER_BREAK_BEFORE_INSTRUCTION_EXECUTION = sh4_exp | 0x1e0,
		sh4_ex_INSTRUCTION_ADDRESS_ERROR =sh4_exp | 0x0e0,
		sh4_ex_INSTRUCTION_TLB_MISS =sh4_exp | 0x040,
		sh4_ex_INSTRUCTION_TLB_PROTECTION_VIOLATION = sh4_exp |0x0a0,
		sh4_ex_GENERAL_ILLEGAL_INSTRUCTION = sh4_exp |0x180,
		sh4_ex_SLOT_ILLEGAL_INSTRUCTION = sh4_exp |0x1a0,
		sh4_ex_GENERAL_FPU_DISABLE = sh4_exp |0x800,
		sh4_ex_SLOT_FPU_DISABLE = sh4_exp |0x820,
		sh4_ex_DATA_ADDRESS_ERROR_READ =sh4_exp |0x0e0,
		sh4_ex_DATA_ADDRESS_ERROR_WRITE = sh4_exp | 0x100,
		sh4_ex_DATA_TLB_MISS_READ = sh4_exp | 0x040,
		sh4_ex_DATA_TLB_MISS_WRITE = sh4_exp | 0x060,
		sh4_ex_DATA_TLB_PROTECTION_VIOLATION_READ = sh4_exp | 0x0a0,
		sh4_ex_DATA_TLB_PROTECTION_VIOLATION_WRITE = sh4_exp | 0x0c0,
		sh4_ex_FPU = sh4_exp | 0x120,
		sh4_ex_TRAP = sh4_exp | 0x160,
		sh4_ex_INITAL_PAGE_WRITE = sh4_exp | 0x080,
		
		// asic9a /sh4 external holly normal [internal]
		holly_RENDER_DONE_vd = holly_nrm | 0x00,
		holly_RENDER_DONE_isp = holly_nrm | 0x01,
		holly_RENDER_DONE = holly_nrm | 0x02,
		holly_SCANINT1 = holly_nrm | 0x03,
		holly_SCANINT2 = holly_nrm | 0x04,
		holly_VBLank = holly_nrm | 0x05,
		holly_OPAQUE = holly_nrm | 0x07,
		holly_OPAQUEMOD = holly_nrm | 0x08,
		holly_TRANS = holly_nrm | 0x09,
		holly_TRANSMOD = holly_nrm | 0x0a,
		holly_MAPLE_DMA = holly_nrm | 0x0c,
		holly_MAPLE_ERR = holly_nrm | 0x0d,
		holly_GDROM_DMA = holly_nrm | 0x0e,
		holly_SPU_DMA = holly_nrm | 0x0f,
		holly_PVR_DMA = holly_nrm | 0x13,
		holly_PUNCHTHRU = holly_nrm | 0x15,

		// asic9c/sh4 external holly external [EXTERNAL]
		holly_GDROM_CMD = holly_ext | 0x00,
		holly_SPU_IRQ = holly_ext | 0x01,
		holly_EXP_8BIT = holly_ext | 0x02,
		holly_EXP_PCI = holly_ext | 0x03,

		// asic9b/sh4 external holly err only error [error]
		holly_PRIM_NOMEM = holly_err | 0x02,
		holly_MATR_NOMEM = holly_err | 0x03
};





typedef void RaiseInterruptFP(InterruptID intr);
extern u32* SB_ISTEXT;
extern u32 g_videoCableType;
extern RaiseInterruptFP* Sh4RaiseInterrupt;
extern u32 sh4_cycles;

#define NUM_CYCLES_PER_BLOCK 448
#define SH4_CYCLES_PER_FRAME ((200*1000*1000)/60)



