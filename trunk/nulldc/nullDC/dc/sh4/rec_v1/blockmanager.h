#pragma once
#include "types.h"
#include "recompiler.h"
#include "BasicBlock.h"
#include "emitter\emitter.h"

BasicBlock* FindBlock(u32 address);
void RegisterBlock(BasicBlock* block);
void UnRegisterBlock(BasicBlock* block);

void FreeSuspendedBlocks();
BasicBlock* NewBlock(u32 address);
BasicBlock* FindOrRecompileCode(u32 pc);
BasicBlock* FindOrAnalyse(u32 pc);
void __fastcall SuspendBlock(BasicBlock* block);