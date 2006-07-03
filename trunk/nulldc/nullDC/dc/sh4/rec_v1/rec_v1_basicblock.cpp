#include "rec_v1_basicblock.h"
#include "emitter/shil_compile_slow.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_registers.h"
#include "emitter/emitter.h"

#include <memory.h>

void rec_v1_BasicBlock::ClearBlock(rec_v1_BasicBlock* block)
{
	if (block->TF_block==this)
	{
		block->TF_block=0;
		block->pTF_next_addr=link_compile_inject_TF_stub;
	}

	if (block->TT_block==this)
	{
		block->TT_block=0;
		block->pTT_next_addr=link_compile_inject_TT_stub;
	}
}
void rec_v1_BasicBlock::BlockWasSuspended(rec_v1_BasicBlock* block)
{
	for (int i=0;i<blocks_to_clear.size();i++)
	{
		if (blocks_to_clear[i]==block)
		{
			blocks_to_clear[i]=0;
		}
	}
}

void rec_v1_BasicBlock::AddRef(rec_v1_BasicBlock* block)
{
	if (!Discarded)
	{
		//if we reference ourselfs we dont care ;) were suspended anyhow
		if (block !=this)
			blocks_to_clear.push_back(block);
	}
	else
		printf("Warning : Discarded, AddRef call\n");
}
void rec_v1_BasicBlock::Suspend()
{
	for (int i=0;i<blocks_to_clear.size();i++)
	{
		if (blocks_to_clear[i])
		{
			ClearBlock(blocks_to_clear[i]);
		}
	}
	blocks_to_clear.clear();

	if (TF_block)
		TF_block->BlockWasSuspended(this);

	if (TT_block)
		TT_block->BlockWasSuspended(this);

	//if we jump to another block , we have to re compile it :)
	Discarded=true;
}

void rec_v1_BasicBlock::Free()
{
	if (compiled)
	{
		free(compiled->Code);
		compiled->Code=0;
		delete compiled;
		compiled=0;
	}
	else
	{
		printf("FREE!!! 0x%x\n",start);
	}	
}
bool rec_v1_BasicBlock::Contains(u32 pc)
{
	u32 pc_real=pc & RAM_MASK;
	u32 real_start=start & RAM_MASK;
	u32 real_end=end & RAM_MASK;

	return ((pc_real>=(real_start-4)) && (pc_real<=(real_end+6)));
}