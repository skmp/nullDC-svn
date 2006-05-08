#pragma once
#include "types.h"
#include "rec_v1_recompiler.h"
#include "rec_v1_basicblock.h"
#include "emitter\emitter.h"

rec_v1_BasicBlock* rec_v1_FindBlock(u32 address);
rec_v1_BasicBlock* rec_v1_AddBlock(u32 address);

void rec_v1_ResetBlockTest(u32 addr);
void rec_v1_SetBlockTest(u32 addr);
void __fastcall rec_v1_BlockTest(u32 addr);
rec_v1_BasicBlock* rec_v1_NewBlock(u32 address);
rec_v1_BasicBlock* rec_v1_FindOrRecompileCode(u32 pc);
rec_v1_BasicBlock* rec_v1_FindOrAnalyse(u32 pc);
void __fastcall rec_v1_NotifyMemWrite(u32 start , u32 size);

void rec_v1_CompileBlockTest(emitter<>* x86e,x86IntRegType r_addr,x86IntRegType temp);