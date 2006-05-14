#pragma once
#include "types.h"
#include "rec_v1_recompiler.h"
#include "rec_v1_basicblock.h"
#include "emitter\emitter.h"

rec_v1_BasicBlock* rec_v1_FindBlock(u32 address);
rec_v1_BasicBlock* rec_v1_AddBlock(u32 address);

rec_v1_BasicBlock* rec_v1_NewBlock(u32 address);
rec_v1_BasicBlock* rec_v1_FindOrRecompileCode(u32 pc);
rec_v1_BasicBlock* rec_v1_FindOrAnalyse(u32 pc);