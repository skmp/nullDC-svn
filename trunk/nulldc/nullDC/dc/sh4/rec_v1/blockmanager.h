#pragma once
#include "types.h"
#include "recompiler.h"
#include "compiledblock.h"

struct pginfo
{
	u32 invalidates;
	union
	{
		struct
		{
			u32 ManualCheck:1;	//bit 0 :1-> manual check , 0 -> locked check
			u32 reserved:31;	//bit 1-31: reserved
		};
		u32 full;
	} flags;
};

extern u32* block_stack_pointer;

#define PAGE_MANUALCHECK 1
pginfo GetPageInfo(u32 address);

#define FindBlock FindBlock_fast
#define FindCode FindCode_fast

INLINE CompiledBlockInfo* __fastcall FindBlock_fast(u32 address);
BasicBlockEP* __fastcall FindCode_fast(u32 address);

void RegisterBlock(CompiledBlockInfo* block);
void UnRegisterBlock(CompiledBlockInfo* block);


void FreeSuspendedBlocks();

CompiledBlockInfo* FindOrRecompileBlock(u32 pc);
void __fastcall SuspendBlock(CompiledBlockInfo* block);

void InitBlockManager();
void ResetBlockManager();
void TermBlockManager();

extern u8* DynarecCache;
extern u32 DynarecCacheSize;

void* dyna_malloc(u32 size);
void* dyna_realloc(void*ptr,u32 oldsize,u32 newsize);
void* dyna_finalize(void* ptr,u32 oldsize,u32 newsize);
void dyna_link(CompiledBlockInfo* block);
void dyna_free(CompiledBlockInfo* block);