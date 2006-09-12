#include "types.h"
#include <windows.h>

#include "dc/sh4/sh4_interpreter.h"
#include "dc/sh4/sh4_opcode_list.h"
#include "dc/sh4/sh4_registers.h"
#include "dc/sh4/sh4_if.h"
#include "dc/pvr/pvr_if.h"
#include "dc/aica/aica_if.h"
#include "dc/gdrom/gdrom_if.h"
#include "dc/sh4/intc.h"
#include "dc/sh4/tmu.h"
#include "dc/sh4/sh4_cst.h"
#include "dc/mem/sh4_mem.h"

#include "emitter/shil_compile_slow.h"

#include "recompiler.h"
#include "blockmanager.h"
#include "analyser.h"
#include "dc/sh4/shil/shil_ce.h"

#include <time.h>
#include <float.h>

//not used
#define MAX_SBL_ANALYSE_LEVEL 8
//new method uses that one ;)
#define MAX_SBL_ANALYSE_COUNT 60


vector<SBL_BasicBlock*> blocks;
u32 analyse_level=0;
SBL_BasicBlock* SBL_AnalyseOrFindBB(u32 pc)
{
	for (u32 i=1;i<blocks.size();i++)
	{
		if (blocks[i]->start==pc)
			return blocks[i];
	}

	SBL_BasicBlock* block=new SBL_BasicBlock();
	//scan code
	ScanCode(pc,block);
	//Fill block lock type info
	FillBlockLockInfo(block);
	if (block->flags.ProtectionType==BLOCK_PROTECTIONTYPE_MANUAL)
	{
		delete block;
		return 0;
	}

	//analyse code [generate il/block type]
	AnalyseCode(block);
	blocks.push_back(block);
	block->TT_block_ptr=block->TF_block_ptr=0;
	block->TT_done=block->TF_done=false;
	block->in_info.Init();

	shil_optimise_pass_ce_driver(block);
	
	
	return block;
}
void SBL_analyse_full_path(SBL_BasicBlock* block)
{
	if (block==0)
		return;
	if (analyse_level>MAX_SBL_ANALYSE_LEVEL)
		return;
	analyse_level++;
	if (block->flags.ExitType==BLOCK_EXITTYPE_COND)
	{
		if (block->TF_done==false)
		{
			block->TF_done=true;
			block->TF_block_ptr=SBL_AnalyseOrFindBB(block->TF_next_addr);
			SBL_analyse_full_path(block->TF_block_ptr);
		}
		if (block->TT_done==false)
		{
			block->TT_done=true;
			block->TT_block_ptr=SBL_AnalyseOrFindBB(block->TT_next_addr);
			SBL_analyse_full_path(block->TT_block_ptr);
		}
	}
	else if (block->flags.ExitType==BLOCK_EXITTYPE_FIXED ||
			 block->flags.ExitType==BLOCK_EXITTYPE_FIXED_CALL)
	{
		if (block->TF_done==false)
		{
			block->TF_done=true;
			block->TF_block_ptr=SBL_AnalyseOrFindBB(block->TF_next_addr);
			SBL_analyse_full_path(block->TF_block_ptr);
		}
	}
	analyse_level--;
}
void SBL_analyse_min_path(SBL_BasicBlock* block)
{
	if (block==0)
		return;
	if (blocks.size()>MAX_SBL_ANALYSE_COUNT)
		return;
	if (block->flags.ExitType==BLOCK_EXITTYPE_COND)
	{
		if (block->TF_done==false)
		{
			block->TF_done=true;
			block->TF_block_ptr=SBL_AnalyseOrFindBB(block->TF_next_addr);
		}
		if (block->TT_done==false)
		{
			block->TT_done=true;
			block->TT_block_ptr=SBL_AnalyseOrFindBB(block->TT_next_addr);
		}
	}
	else if (block->flags.ExitType==BLOCK_EXITTYPE_FIXED ||
			 block->flags.ExitType==BLOCK_EXITTYPE_FIXED_CALL)
	{
		if (block->TF_done==false)
		{
			block->TF_done=true;
			block->TF_block_ptr=SBL_AnalyseOrFindBB(block->TF_next_addr);
		}
	}
}
void PrintTree(SBL_BasicBlock* block, u32 lvl)
{
	if (block==0)
	{
		printf("DynamicResolve\n");
		return;
	}
	else
		printf("0x%X\n",block->start);

	if (block->flags.ExitType==BLOCK_EXITTYPE_COND)
	{
		printf("if (T==0) \n");
			PrintTree(block->TT_block_ptr,lvl+1);
		printf("else \n");
			PrintTree(block->TF_block_ptr,lvl+1);
	}
	else
	{
		printf("call ");
		PrintTree(block->TT_block_ptr,lvl+1);
	}
}
void fastcall AnalyseCodeSuperBlock(u32 pc)
{
	//blocks[0] is allways entry point ;)
	blocks.clear();
	SBL_AnalyseOrFindBB(pc);
	
	for (u32 i=0;i<sh4_reg_count;i++)
		blocks[0]->in_info.reginfo[i].can_be_const=false;

	u32 last_block=0;
	u32 last_block_count=1;
	while ((blocks.size()<MAX_SBL_ANALYSE_COUNT) && (last_block_count!=0))
	{
		for (u32 i=0;i<last_block_count;i++)
		{
			SBL_analyse_min_path(blocks[last_block+i]);
		}
		last_block+=last_block_count;
		last_block_count=blocks.size()-last_block;
	}
	
	/*
	u32 pre_toc=0;
	u32 pre_tec=0;
	u32 pre_tsc=0;
	for (u32 i=0;i<blocks.size();i++)
	{
		pre_toc+=blocks[i]->OpcodeCount();
		pre_tsc+=blocks[i]->ilst.op_count;
		pre_tec+=blocks[i]->cycles;
	}*/

	//Superblock level constant elimination :)
	
	for (u32 k=0;k<5;k++)
	{
		//set the input info for each block
		for (u32 i=1;i<blocks.size();i++)
		{
			if (blocks[i]->TF_block_ptr)
			{
				blocks[i]->TF_block_ptr->in_info.Merge(&blocks[i]->out_info);
			}
			if (blocks[i]->TT_block_ptr)
			{
				blocks[i]->TT_block_ptr->in_info.Merge(&blocks[i]->out_info);
			}
		}

		//perform constant elimination on all blocks
		for (u32 i=1;i<blocks.size();i++)
		{
			shil_optimise_pass_ce_driver(blocks[i]);
		}
		//loop a few times :p
	}

	
	u32 toc=0;
	u32 tec=0;
	u32 tsc=0;

	//PrintTree(blocks[0],0);

	for (u32 i=0;i<blocks.size();i++)
	{
		toc+=blocks[i]->OpcodeCount();
		tsc+=blocks[i]->ilst.op_count;
		tec+=blocks[i]->cycles;
		delete blocks[i];
	}
	printf("SBL size : %d ,%d[%d] ops , %d shil ops\n",blocks.size(),toc,tec,tsc);
	
	//block->flags.DisableHS=true;
	//Compile code
	//CompileBasicBlock_slow(block);
	//RegisterBlock(cblock=&block->cBB->cbi);
	//delete block;
	//compile code
	//return pointer
}