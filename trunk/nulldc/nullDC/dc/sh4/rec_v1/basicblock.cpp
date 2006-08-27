#include "BasicBlock.h"
#include "emitter/shil_compile_slow.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_registers.h"
#include "emitter/emitter.h"

#include <memory.h>
//extern u32 rec_cycles;

//Common Functions
//Called to Free :p yeshrly
typedef void __fastcall FreeBlockFP(CompiledBlockInfo* pthis);
//Called when this block is suspended
typedef void __fastcall SuspendBlockFP(CompiledBlockInfo* pthis);
//Called when a block we reference is suspended
typedef void __fastcall BlockWasSuspendedFP(CompiledBlockInfo* pthis,CompiledBlockInfo* block);
//Called when a block adds reference to this
typedef void __fastcall AddBlockRefFP(CompiledBlockInfo* pthis,CompiledBlockInfo* block);
//remote pthis reference to block *warning* it was the oposite before
typedef void __fastcall ClearBlockFP(CompiledBlockInfo* pthis,CompiledBlockInfo* block);

struct block_functs
{
	AddBlockRefFP* AddRef;
	BlockWasSuspendedFP* BlockWasSuspended;
	SuspendBlockFP* Suspend;
	ClearBlockFP* ClearBlock;
	FreeBlockFP* Free;
};

//Compiled basic block Common interface
#define bbthis	verify((p_this->block_type&COMPILED_BLOCK_TYPE_MASK)==COMPILED_BASIC_BLOCK);\
				CompiledBasicBlockInfo* pthis=&((CompiledBasicBlock*)p_this)->ebi;

void __fastcall basic_block_AddRef(CompiledBlockInfo* p_this,CompiledBlockInfo* block)
{
	bbthis;
	verify(p_this->Discarded==false);
	//if we reference ourselfs we dont care ;) were suspended anyhow
	if (block !=p_this)
		pthis->blocks_to_clear.push_back(block);
}
void __fastcall basic_block_BlockWasSuspended(CompiledBlockInfo* p_this,CompiledBlockInfo* block)
{
	bbthis;
	for (u32 i=0;i<pthis->blocks_to_clear.size();i++)
	{
		if (pthis->blocks_to_clear[i]==block)
		{
			pthis->blocks_to_clear[i]=0;
		}
	}
}
void __fastcall basic_block_ClearBlock(CompiledBlockInfo* p_this,CompiledBlockInfo* block)
{
	bbthis;
	if (pthis->TF_block==block)
	{
		pthis->TF_block=0;
		pthis->pTF_next_addr=bb_link_compile_inject_TF_stub;
	}

	if (pthis->TT_block==block)
	{
		pthis->TT_block=0;
		pthis->pTT_next_addr=bb_link_compile_inject_TT_stub;
	}
}
void __fastcall basic_block_Suspend(CompiledBlockInfo* p_this)
{
	bbthis;
	for (u32 i=0;i<pthis->blocks_to_clear.size();i++)
	{
		if (pthis->blocks_to_clear[i])
		{
			pthis->blocks_to_clear[i]->ClearBlock(p_this);
		}
	}
	pthis->blocks_to_clear.clear();

	if (pthis->TF_block)
		pthis->TF_block->BlockWasSuspended(p_this);

	if (pthis->TT_block)
		pthis->TT_block->BlockWasSuspended(p_this);
}
void __fastcall basic_block_Free(CompiledBlockInfo* p_this)
{
	bbthis;
}

//btf vtable
#define BLOCK_VTABLE_ENTRY(bname) bname##_AddRef,bname##_BlockWasSuspended,bname##_Suspend,bname##_ClearBlock,bname##_Free
block_functs block_vtable[COMPILED_BLOCK_TYPE_MASK+1]=
{
	//Basic Block
	{BLOCK_VTABLE_ENTRY(basic_block)},
	//SuperBlock -- isnt this marvelous ?
	{0,0,0,0,0}
};
//CompiledBlockInfo Helper functions
void CompiledBlockInfo::ClearBlock(CompiledBlockInfo* block)
{
	block_vtable[block_type&COMPILED_BLOCK_TYPE_MASK].ClearBlock(this,block);
}

void CompiledBlockInfo::BlockWasSuspended(CompiledBlockInfo* block)
{
	block_vtable[block_type&COMPILED_BLOCK_TYPE_MASK].BlockWasSuspended(this,block);
}

void CompiledBlockInfo::AddRef(CompiledBlockInfo* block)
{
	block_vtable[block_type&COMPILED_BLOCK_TYPE_MASK].AddRef(this,block);
}
void CompiledBlockInfo::Suspend()
{
	block_vtable[block_type&COMPILED_BLOCK_TYPE_MASK].Suspend(this);
	//if we jump to another block , we have to re compile it :)
	Discarded=true;
}

void CompiledBlockInfo::Free()
{
		free(Code);
		Code=0;	

		block_vtable[block_type&COMPILED_BLOCK_TYPE_MASK].Free(this);
}

//Get Hotspot info (on Hotspot blocks)
HotSpotInfo* CompiledBlockInfo::GetHS()
{
	verify(block_type & COMPILED_BLOCK_HOTSPOT);
	//NP mod is after HS , so we dont have to care about it ;)
	switch(block_type & COMPILED_BLOCK_TYPE_MASK)
	{
	case COMPILED_BASIC_BLOCK:
		return &((CompiledBasicBlockHotSpot*)this)->hs;
	case COMPILED_SUPER_BLOCK:
		return &((CompiledSuperBlockHotSpot*)this)->hs;
	}
	return 0;
}
//Get NullProf info (on NullProf blocks)
NullProfInfo* CompiledBlockInfo::GetNP()
{
	verify(block_type & COMPILED_BLOCK_NULLPROF);

	switch(block_type & COMPILED_BLOCK_TYPE_MASK)
	{
	case COMPILED_BASIC_BLOCK:
		{	
			if (block_type & COMPILED_BLOCK_HOTSPOT)
				return &((CompiledBasicBlockHotSpotNullProf*)this)->np;
			else
				return &((CompiledBasicBlockNullProf*)this)->np;
		}
	case COMPILED_SUPER_BLOCK:
		{	
			if (block_type & COMPILED_BLOCK_HOTSPOT)
				return &((CompiledSuperBlockHotSpotNullProf*)this)->np;
			else
				return &((CompiledBasicBlockNullProf*)this)->np;
		}
	}

	return 0;
}
//Get BasicBlock info (on BasicBlock blocks)
CompiledBasicBlockInfo* CompiledBlockInfo::GetBB()
{
	verify((block_type & COMPILED_BLOCK_TYPE_MASK)== COMPILED_BASIC_BLOCK);
	return &((CompiledBasicBlock*)this)->ebi;
}
//Get SuperBlock info (on SuperBlock blocks)
CompiledSuperBlockInfo* CompiledBlockInfo::GetSP()
{
	verify((block_type & COMPILED_BLOCK_TYPE_MASK)== COMPILED_SUPER_BLOCK);
	return &((CompiledSuperBlock*)this)->ebi;
}
//Basic Block
bool BasicBlock::IsMemLocked(u32 adr)
{
	if (IsOnRam(adr)==false)
		return false;

	if (flags.ProtectionType==BLOCK_PROTECTIONTYPE_MANUAL)
		return false;

	//if block isnt on ram , then there's no way to tell if its locked (well , bios mem is allways locked :p)
	if (OnRam()==false)
		return false;

	verify(page_start()<=page_end());

	u32 adrP=GetRamPageFromAddress(adr);

	return (page_start() <=adrP) && (adrP<=page_end());
}

void BasicBlock::SetCompiledBlockInfo(CompiledBasicBlock* cBl)
	{
		verify((cBl->cbi.block_type & COMPILED_BLOCK_TYPE_MASK)==COMPILED_BASIC_BLOCK);
		cBB= cBl;

		cBB->cbi.start=start;
		cBB->cbi.end=end;
		cBB->cbi.cpu_mode_tag=flags.FpuMode;
		cBB->cbi.lookups=0;
		cBB->cbi.Discarded=false;

		if (cBl->cbi.block_type & COMPILED_BLOCK_NULLPROF)
			cBB->cbi.GetNP()->cycles=cycles;

		cBB->ebi.TF_next_addr=TF_next_addr;
		cBB->ebi.TT_next_addr=TT_next_addr;

		cBB->ebi.TF_block=cBB->ebi.TT_block=0;

		if (cBB->cbi.block_type & COMPILED_BLOCK_NULLPROF)
		{
			cBB->cbi.GetNP()->time=0;
			cBB->cbi.GetNP()->calls=0;
		}
		
		//cBB->block_type=BASIC_BLOCK;
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
	return  !(cr_start<tr_end || cr_end<tr_start);
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

		u32 c_page=(sbspan.page_start()*PAGE_SIZE)+(PAGE_SIZE>>1);
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

		return rv;
	}
};

void DeleteBlock(CompiledBlockInfo* block)
{
#define GENERIC_CASE(name,NAME)\
case COMPILED_##NAME##_BLOCK:\
		delete (Compiled##name##Block*)block;\
		break;\
\
	case COMPILED_##NAME##_BLOCK | COMPILED_BLOCK_HOTSPOT:\
		delete (Compiled##name##BlockHotSpot*)block;\
		break;\
\
	case COMPILED_##NAME##_BLOCK | COMPILED_BLOCK_NULLPROF:\
		delete (Compiled##name##BlockNullProf*)block;\
		break;\
\
	case COMPILED_##NAME##_BLOCK | COMPILED_BLOCK_NULLPROF | COMPILED_BLOCK_HOTSPOT:\
		delete (Compiled##name##BlockHotSpotNullProf*)block;\
		break;

	switch(block->block_type)
	{
		GENERIC_CASE(Basic,BASIC);
		GENERIC_CASE(Super,SUPER);
	}
}

CompiledBlockInfo* CreateBlock(u32 type)
{
	CompiledBlockInfo* rv;

#define GENERIC_CASE(name,NAME)\
case COMPILED_##NAME##_BLOCK:\
		rv=(CompiledBlockInfo*) new Compiled##name##Block();\
		break;\
\
	case COMPILED_##NAME##_BLOCK | COMPILED_BLOCK_HOTSPOT:\
		rv=(CompiledBlockInfo*) new Compiled##name##BlockHotSpot();\
		break;\
\
	case COMPILED_##NAME##_BLOCK | COMPILED_BLOCK_NULLPROF:\
		rv=(CompiledBlockInfo*) new Compiled##name##BlockNullProf();\
		break;\
\
	case COMPILED_##NAME##_BLOCK | COMPILED_BLOCK_NULLPROF | COMPILED_BLOCK_HOTSPOT:\
		rv=(CompiledBlockInfo*) new Compiled##name##BlockHotSpotNullProf();\
		break;

	switch(type)
	{
		GENERIC_CASE(Basic,BASIC);
		GENERIC_CASE(Super,SUPER);
	}
	rv->block_type=type;
	return rv;
}