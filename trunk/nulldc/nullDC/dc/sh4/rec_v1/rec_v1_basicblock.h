#pragma once
#include "rec_v1_recompiler.h"

class rec_v1_BasicBlock;

class rec_v1_CompiledBlock
{
public :
	rec_v1_BasicBlockEP* Code;						//compiled code ptr
	rec_v1_BasicBlock* parent;		//basic block that the compiled code is for
	u32 size;						//compiled code size (bytes)
	u32 count;						//compiled code opcode count
};

#define BLOCK_TEMP 1

class rec_v1_BasicBlock
{
public :
	rec_v1_BasicBlock()
	{
		start=0;
		end=0;
		flags=0;
		cycles=0;
		compiled=0;
		TF_next=0;
		TF_next_addr=0xFFFFFFFF;
		TT_next=0;
		TT_next_addr=0xFFFFFFFF;
	}	
	//start pc
	u32 start;
	//end pc
	u32 end;

	//flags
	u32 flags;
	u32 cycles;

	shil_stream ilst;

	rec_v1_CompiledBlock* compiled;

	rec_v1_BasicBlock* TF_next;	//b=f or jmp test
	u32 TF_next_addr;

	rec_v1_BasicBlock* TT_next;	//b=t
	u32 TT_next_addr;
};

typedef void (__fastcall RecOpCallFP) (u32 op,u32 pc,rec_v1_BasicBlock* bb);