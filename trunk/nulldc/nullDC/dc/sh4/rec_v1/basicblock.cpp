#include "BasicBlock.h"
#include "emitter/shil_compile_slow.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_registers.h"
#include "emitter/emitter.h"

#include <memory.h>
extern u32 rec_cycles;
void CompiledBasicBlock::ClearBlock(CompiledBasicBlock* block)
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
void CompiledBasicBlock::BlockWasSuspended(CompiledBasicBlock* block)
{
	for (u32 i=0;i<blocks_to_clear.size();i++)
	{
		if (blocks_to_clear[i]==block)
		{
			blocks_to_clear[i]=0;
		}
	}
}

void CompiledBasicBlock::AddRef(CompiledBasicBlock* block)
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
void CompiledBasicBlock::Suspend()
{
	for (u32 i=0;i<blocks_to_clear.size();i++)
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

void CompiledBasicBlock::Free()
{
		free(Code);
		Code=0;	
}
bool BasicBlock::Contains(u32 pc)
{
	u32 pc_real=pc & RAM_MASK;
	u32 real_start=start & RAM_MASK;
	u32 real_end=end & RAM_MASK;

	return ((pc_real>=(real_start-4)) && (pc_real<=(real_end+6)));
}

bool BasicBlock::IsMemLocked(u32 adr)
{
	if (flags.ProtectionType==BLOCK_PROTECTIONTYPE_MANUAL)
		return false;

	if (IsOnRam(adr)==false)
		return false;

	//if block isnt on ram , then there's no way to tell if its locked (well , bios mem is allways locked :p)
	if (OnRam()==false)
		return false;

	
	u32 adrP=GetRamPageFromAddress(adr);

	return (page_start() <=adrP) && (page_end()>=adrP);
}


//start page , olny valid if in ram
u32 CodeRegion::page_start()
{
	return GetRamPageFromAddress(start);
}
//end page , olny valid if in ram
u32 CodeRegion::page_end()
{
	return GetRamPageFromAddress(end);
}
//True if block is on ram :)
bool CodeRegion::OnRam()
{
	//code can't start and end on diff mem areas , so olny check start :)
	return IsOnRam(start);
}