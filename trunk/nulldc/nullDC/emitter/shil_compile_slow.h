#pragma once
#include "types.h"
#include "dc\sh4\shil\shil.h"
#include "dc\sh4\rec_v1\rec_v1_basicblock.h"

void CompileBasicBlock_slow(rec_v1_BasicBlock* block, u32 pre_cycles=0);