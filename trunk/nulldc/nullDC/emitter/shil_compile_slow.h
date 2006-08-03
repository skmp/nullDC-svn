#pragma once
#include "types.h"
#include "dc\sh4\shil\shil.h"
#include "dc\sh4\rec_v1\BasicBlock.h"

struct shil_scs
{
	bool RegAllocX86;
	u32  RegAllocCount;//max 4 atm
	bool RegAllocXMM;
	//bool RegAllocx87; //redutant ? x87 doesn't seem to be used atm :P
	bool InlineMemRead_const;
	bool InlineMemRead;
	bool InlineMemWrite;
	bool TBitsperate;
	bool RetPrediction;
};

extern shil_scs shil_compile_slow_settings;

void CompileBasicBlock_slow(BasicBlock* block); 
void link_compile_inject_TF_stub(CompiledBasicBlock* ptr);
void link_compile_inject_TT_stub(CompiledBasicBlock* ptr);