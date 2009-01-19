#pragma once

#include "types.h"

class SBL_BasicBlock;

#include "dc/sh4/shil/shil_ce.h"
CompiledBlockInfo* fastcall AnalyseCodeSuperBlock(u32 pc);

class SBL_BasicBlock:public BasicBlock
{
public:
	SBL_BasicBlock* TT_block_ptr;
	SBL_BasicBlock* TF_block_ptr;

	bool TT_done;
	bool TF_done;
	shil_ce_info in_info;
	shil_ce_info out_info;
};