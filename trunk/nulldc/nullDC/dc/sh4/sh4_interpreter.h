#pragma once
#include "types.h"
#include "sh4_if.h"
#include "intc.h"

#undef sh4op
#define sh4op(str) void  __fastcall str (u32 op)
typedef void (__fastcall OpCallFP) (u32 op);

enum OpcodeType
{
	//basic
	Normal=0,			//heh , nothing special :P
	ReadsPC=1,			//pc must be set upon calling it
	WritesPC=2,			//it will write pc (branch)
	Delayslot=4,		//has a delayslot opcode , valid olny when WritesPC is set

	WritesSR=8,			//Writes to SR , and UpdateSR needs to be called
	WritesFPSCR=16,		//Writes to FPSCR , and UpdateSR needs to be called

	Invalid=128,			//invalid

	//heh not basic :P
	ReadWritePC=ReadsPC|WritesPC,		//Read and writes pc :P

	//branches : 
	//not delay slot
	Branch_dir=ReadWritePC,		//direct (eg , pc=r[xx]) -- this one is ReadWritePC b/c the delayslot may use pc ;)
	Branch_rel=ReadWritePC,		//relative (rg pc+=10);
	//delay slot
	Branch_dir_d=Delayslot|Branch_dir,	//direct (eg , pc=r[xx])
	Branch_rel_d=Delayslot|Branch_rel,	//relative (rg pc+=10);
};

//interface
void Sh4_int_Run();
void Sh4_int_Stop();
void Sh4_int_Step();
void Sh4_int_Skip();
void Sh4_int_Reset(bool Manual);
void Sh4_int_Init();
void Sh4_int_Term();
bool Sh4_int_IsCpuRunning();
void __fastcall sh4_int_RaiseExeption(u32 ExeptionCode,u32 VectorAddr);
u32 Sh4_int_GetRegister(Sh4RegType reg);
void Sh4_int_SetRegister(Sh4RegType reg,u32 regdata);
//Other things (mainly used by the cpu core
bool ExecuteDelayslot();
bool ExecuteDelayslot_RTE();
int  __fastcall UpdateSystem();
//timer that ticks @ ~ 13,216 khz , used to free blocks & time delta for block promotion
extern u32 gcp_timer;