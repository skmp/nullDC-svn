#pragma once
#include "..\..\types.h"
#include "sh4_if.h"
#include "intc.h"

#define sh4op(str) void  __fastcall str (u32 op)
typedef void (__fastcall OpCallFP) (u32 op);

enum OpcodeType
{
	UsePC=1,		//uses pc
	UseDslot=2,		//writes to dslot 
	UseDslot_c=4,   //may write to dslot
	Branch=8,		//direct branch
	BranchDelay=16,	//Branch with delay
	//////////////////////////////
	Normal=0,			//does not modify the state of the cpu nor uses pc :)
	Normal_needpc=Normal|UsePC,//does not modify the state of the cpu, uses PC

	Branch_c=Branch|UseDslot_c,			//conditional branch
	Branch_c_UsePC=Branch_c|UsePC,

	BranchDelay_c=BranchDelay|UseDslot_c,//conditional branch with delay
	BranchDelay_c_UsePC=BranchDelay_c|UsePC,
	

	Branch_u=Branch|UseDslot,			//unconditional branch
	Branch_u_UsePC=Branch_u|UsePC,

	BranchDelay_u=BranchDelay|UseDslot,//unconditional branch with delay
	BranchDelay_u_UsePC=BranchDelay_u|UsePC,
	
	/////////////////////////////
	Arithm_FPU=4096	,	//fpu opcode
	Arithm_FPU_SP=Arithm_FPU|256,	//Fpu opcode , single prec (ops)
	Arithm_FPU_DP=Arithm_FPU|512,	//Fpu opcode , double prec (ops)

	Arithm_FPU_SM=Arithm_FPU|1024,	//Fpu opcode , single data (mov's)
	Arithm_FPU_DM=Arithm_FPU|2048,	//Fpu opcode , double data (mov's)
	Arithm_FPU_Mode_Mask=0x0F00,

	SystemSt=96,		//changes system status (register banks ect)
	NoOperation=112,	//nop
	Invalid=128			//invalid
};



//interface
void Sh4_int_Run(ThreadCallbackFP* tcb);
void Sh4_int_Stop();
void Sh4_int_Step();
void Sh4_int_Skip();
void Sh4_int_Reset(bool Manual);
void Sh4_int_Init();
void Sh4_int_Term();
bool Sh4_int_IsCpuRunning();
u32 Sh4_int_GetRegister(Sh4RegType reg);
void Sh4_int_SetRegister(Sh4RegType reg,u32 regdata);
//Other things (mainly used by the cpu core
bool ExecuteDelayslot();
int UpdateSystem(u32 Cycles);