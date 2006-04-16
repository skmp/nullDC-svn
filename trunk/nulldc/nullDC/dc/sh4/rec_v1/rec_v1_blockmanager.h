#pragma once
#include "types.h"
#include "rec_v1_recompiler.h"
#include "rec_v1_basicblock.h"

rec_v1_BasicBlock* FindBlock(u32 address);
rec_v1_BasicBlock* AddBlock(u32 address);
