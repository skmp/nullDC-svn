#pragma once
#include "types.h"
#include "recompiler.h"
#include "BasicBlock.h"

//CompiledBlockInfo* __fastcall FindBlock(u32 address);
#define FindBlock FindBlock_fast
#define FindCode FindCode_fast

INLINE CompiledBlockInfo* __fastcall FindBlock_fast(u32 address);
BasicBlockEP* __fastcall FindCode_fast(u32 address);

void RegisterBlock(CompiledBlockInfo* block);
void UnRegisterBlock(CompiledBlockInfo* block);

void FillBlockLockInfo(BasicBlock* block);

void FreeSuspendedBlocks();


BasicBlock* FindOrAnalyse(u32 pc);

CompiledBlockInfo* FindOrRecompileCode(u32 pc);
void __fastcall SuspendBlock(CompiledBlockInfo* block);

void InitBlockManager();
void ResetBlockManager();
void TermBlockManager();

extern u32 DynarecRam_Start;
extern u32 DynarecRam_End;