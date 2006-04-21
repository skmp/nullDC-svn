#pragma once
#include "types.h"
#include "rec_v1_recompiler.h"
#include "rec_v1_basicblock.h"

rec_v1_BasicBlock* rec_v1_FindBlock(u32 address);
rec_v1_BasicBlock* rec_v1_AddBlock(u32 address);

void rec_v1_ResetBlockTest(u32 addr);
void rec_v1_SetBlockTest(u32 addr);
void rec_v1_BlockTest(u32 addr);
rec_v1_BasicBlock* rec_v1_NewBlock(u32 address);
rec_v1_BasicBlock* rec_v1_FindOrRecompileCode(u32 pc);