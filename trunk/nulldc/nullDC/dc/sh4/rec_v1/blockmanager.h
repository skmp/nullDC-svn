#pragma once
#include "types.h"
#include "recompiler.h"
#include "BasicBlock.h"
#include "emitter\emitter.h"

//CompiledBlock* __fastcall FindBlock(u32 address);
#define FindBlock FindBlock_fast
INLINE CompiledBlock* __fastcall FindBlock_fast(u32 address);

void RegisterBlock(CompiledBlock* block);
void UnRegisterBlock(CompiledBlock* block);

void FillBlockLockInfo(BasicBlock* block);

void FreeSuspendedBlocks();


BasicBlock* FindOrAnalyse(u32 pc);

CompiledBlock* FindOrRecompileCode(u32 pc);
void __fastcall SuspendBlock(CompiledBlock* block);

void InitBlockManager();
void ResetBlockManager();
void TermBlockManager();