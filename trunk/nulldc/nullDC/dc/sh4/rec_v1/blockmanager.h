#pragma once
#include "types.h"
#include "recompiler.h"
#include "BasicBlock.h"
#include "emitter\emitter.h"

//CompiledBasicBlock* __fastcall FindBlock(u32 address);
#define FindBlock FindBlock_fast
INLINE CompiledBasicBlock* __fastcall FindBlock_fast(u32 address);

void RegisterBlock(CompiledBasicBlock* block);
void UnRegisterBlock(CompiledBasicBlock* block);

void FillBlockLockInfo(BasicBlock* block);

void FreeSuspendedBlocks();


BasicBlock* FindOrAnalyse(u32 pc);

CompiledBasicBlock* FindOrRecompileCode(u32 pc);
void __fastcall SuspendBlock(CompiledBasicBlock* block);

void InitBlockManager();
void ResetBlockManager();
void TermBlockManager();