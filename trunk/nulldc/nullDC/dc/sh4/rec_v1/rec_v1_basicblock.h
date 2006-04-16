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

class rec_v1_BasicBlock
{
public :
	//start pc
	u32 start;
	//end pc
	u32 end;

	//flags
	u32 flags;
	u32 cycles;

	list<shil_opcode> ilst;

	rec_v1_CompiledBlock* compiled;
};