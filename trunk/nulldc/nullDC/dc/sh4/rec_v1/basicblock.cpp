#include "BasicBlock.h"
#include "emitter/shil_compile_slow.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_registers.h"
#include "emitter/emitter.h"

#include <memory.h>
extern u32 rec_cycles;
void CompiledBlock::ClearBlock(CompiledBlock* block)
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
void CompiledBlock::BlockWasSuspended(CompiledBlock* block)
{
	for (u32 i=0;i<blocks_to_clear.size();i++)
	{
		if (blocks_to_clear[i]==block)
		{
			blocks_to_clear[i]=0;
		}
	}
}

void CompiledBlock::AddRef(CompiledBlock* block)
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
void CompiledBlock::Suspend()
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

void CompiledBlock::Free()
{
		free(Code);
		Code=0;	
}
/*bool BasicBlock::Contains(u32 pc)
{
	u32 pc_real=pc & RAM_MASK;
	u32 real_start=start & RAM_MASK;
	u32 real_end=end & RAM_MASK;

	return ((pc_real>=(real_start-4)) && (pc_real<=(real_end+6)));
}*/

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
u32 CodeSpan::page_start()
{
	return GetRamPageFromAddress(start);
}
//end page , olny valid if in ram
u32 CodeSpan::page_end()
{
	return GetRamPageFromAddress(end);
}
//True if block is on ram :)
bool CodeSpan::OnRam()
{
	//code can't start and end on diff mem areas , so olny check start :)
	return IsOnRam(start);
}
//Retuns the size of the code span (in bytes)
u32 CodeSpan::Size()
{
	return end-start+2;
}

//Returns the opcode count of the code span
u32 CodeSpan::OpcodeCount()
{
	return Size()>>1;
}


//Returns the Page count of the code span [olny valid if in vram]
u32 CodeSpan::PageCount()
{
	verify(OnRam());
	return page_end()-page_start()+1;
}
//Checks if this CodeSpan contains an adress.Valid olny if on block is on ram and address is on ram :)
bool CodeSpan::Contains(u32 address,u32 sz)
{
	verify(OnRam() && IsOnRam(address));
	u32 r_address=address&RAM_MASK;
	u32 r_start=start&RAM_MASK;
	u32 r_end=end&RAM_MASK;

	return r_start<=r_address && (r_address+sz)<=r_end;
}

//checks if this CodeSpan contains another Codespan
bool CodeSpan::Contains(CodeSpan* to)
{
	verify(OnRam() && to->OnRam());
	return Contains(to->start,2) && Contains(to->end,2);
}

//Checks if this CodeSpan contains an adress , counting in page units.
bool CodeSpan::ContainsPage(u32 address)
{
	verify(OnRam() && IsOnRam(address));
	u32 r_address=GetRamPageFromAddress(address);
	u32 r_start=page_start();
	u32 r_end=page_end();

	return r_start<=r_address && r_address<=r_end;
}

//Checks if this CodeSpan contains another code span , counting in page units.
bool CodeSpan::ContainsPage(CodeSpan* to)
{
	verify(OnRam() && to->OnRam());
	return ContainsPage(to->start) && ContainsPage(to->end);
}

//Creates a Union w/ the CodeSpan , assuming this CodeSpan is on ram.The to codespan contains decoded ram offsets
void CodeSpan::Union(CodeSpan* to)
{
	verify(OnRam());
	to->start=min(to->start&RAM_MASK,start&RAM_MASK);
	to->start=max(to->end&RAM_MASK,end&RAM_MASK);
}

//Checks if this CodeSpan Intersects w/ another , assuming both are on ram
bool CodeSpan::Intersect(CodeSpan* to)
{
	verify(OnRam() && to->OnRam());
	u32 cr_start=start&RAM_MASK;
	u32 cr_end=end&RAM_MASK;

	u32 tr_start=to->start&RAM_MASK;
	u32 tr_end=to->end&RAM_MASK;

	//in order not to inersect , the block must be before  (this-> start < to-> end) or after (this->end <to->start)
	return  !(cr_start<tr_end || cr_end<tr_end);
}

//A tree , made by many BasicBlocks
class SuperBlock
{
public:
	BasicBlock* rootblock;
	vector<BasicBlock*> blocks;
	//vector<BasicBlock> blocks;
	u32* LockPointCache;

	u32* CalculateLockPoints()
	{
		if (blocks.size()==0)
			return 0;

		CodeSpan sbspan;

		sbspan.start=blocks[0]->start;
		sbspan.end=blocks[0]->end;

		for (u32 i=1;i<blocks.size();i++)
			blocks[i]->Union(&sbspan);

		u8* temp_map=(u8*)malloc(sbspan.PageCount());
		memset(temp_map,0,sbspan.PageCount()*sizeof(temp_map[0]));

		u32 c_page=sbspan.page_start()*PAGE_SIZE+PAGE_SIZE>>1;
		u32 lock_count=0;
		for (u32 i=0;i<sbspan.PageCount();i++)
		{
			u32 j=0;
			while(temp_map[i]==0 && j<blocks.size())
			{
				temp_map[i]|=(u8)blocks[j]->ContainsPage(c_page);
			}	
			if (temp_map[i])
				lock_count++;
			c_page+=PAGE_SIZE;
		}

		u32* lock_map=(u32*)malloc(lock_count*sizeof(lock_map[0])+4);
		
		*lock_map=lock_count;
		lock_map++;
		u32 * rv=lock_map;

		c_page=sbspan.page_start();
		for (u32 i=0;i<sbspan.PageCount();i++)
		{
			if (lock_map[i])
				*lock_map++=c_page+i;
		}

		free(temp_map);

		return lock_map;
	}
};