//Compiled block code
//ALL OF IT WILL COME HERE !

#include "compiledblock.h"
#include "basicblock.h"

//Generic compiled block handling code , helpers ect
//the olny header needed to be included will be compiledblock.h

//Common Functions
//Called to Free
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
//btf vtable
#define BLOCK_VTABLE_ENTRY(bname) bname##_AddRef,bname##_BlockWasSuspended,bname##_Suspend,bname##_ClearBlock,bname##_Free

const block_functs block_vtable[COMPILED_BLOCK_TYPE_MASK+1]=
{
	//Basic Block
	{BLOCK_VTABLE_ENTRY(basic_block)},
	//SuperBlock -- isnt this marvelous ?
	{0,0,0,0,0}
};
//CompiledBlockInfo Helper functions
void CompiledBlockInfo::ClearBlock(CompiledBlockInfo* block)
{
	block_vtable[block_type.type].ClearBlock(this,block);
}

void CompiledBlockInfo::BlockWasSuspended(CompiledBlockInfo* block)
{
	block_vtable[block_type.type].BlockWasSuspended(this,block);
}

void CompiledBlockInfo::AddRef(CompiledBlockInfo* block)
{
	block_vtable[block_type.type].AddRef(this,block);
}
void CompiledBlockInfo::Suspend()
{
	block_vtable[block_type.type].Suspend(this);
	//if we jump to another block , we have to re compile it :)
	Discarded=true;
}

void dyna_free(CompiledBlockInfo* block);
void CompiledBlockInfo::Free()
{
		dyna_free(this);
		Code=0;	

		block_vtable[block_type.type].Free(this);
}

//Get Hotspot info (on Hotspot blocks)
HotSpotInfo* CompiledBlockInfo::GetHS()
{
	verify(block_type.HotSpot);
	//NP mod is after HS , so we dont have to care about it ;)
	switch(block_type.type)
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
	verify(block_type.nullProf);

	switch(block_type.type)
	{
	case COMPILED_BASIC_BLOCK:
		{	
			if (block_type.HotSpot)
				return &((CompiledBasicBlockHotSpotNullProf*)this)->np;
			else
				return &((CompiledBasicBlockNullProf*)this)->np;
		}
	case COMPILED_SUPER_BLOCK:
		{	
			if (block_type.HotSpot)
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
	verify(block_type.type== COMPILED_BASIC_BLOCK);
	return &((CompiledBasicBlock*)this)->ebi;
}
//Get SuperBlock info (on SuperBlock blocks)
CompiledSuperBlockInfo* CompiledBlockInfo::GetSP()
{
	verify(block_type.type== COMPILED_SUPER_BLOCK);
	return &((CompiledSuperBlock*)this)->ebi;
}

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

	switch(block->block_type.full)
	{
		GENERIC_CASE(Basic,BASIC);
		GENERIC_CASE(Super,SUPER);
	}
}

CompiledBlockInfo* CreateBlock(u32 type)
{
	CompiledBlockInfo* rv=0;

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
	rv->block_type.full=type;
	return rv;
}