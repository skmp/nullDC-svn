#pragma once
#include "types.h"
#include "rec_v1_recompiler.h"
#include "BasicBlock.h"
#include "emitter\emitter.h"

BasicBlock* rec_v1_FindBlock(u32 address);
void rec_v1_RegisterBlock(BasicBlock* block);
void rec_v1_UnRegisterBlock(BasicBlock* block);

void FreeSuspendedBlocks();
BasicBlock* rec_v1_NewBlock(u32 address);
BasicBlock* rec_v1_FindOrRecompileCode(u32 pc);
BasicBlock* rec_v1_FindOrAnalyse(u32 pc);
void __fastcall SuspendBlock(BasicBlock* block);