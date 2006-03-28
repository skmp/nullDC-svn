#pragma once
#include "types.h"
#include "rec_v0_recompiler.h"

struct recBlock
{
	u32 StartAddr;
	u32 Cycles;
	u32 Size;
	u32 NativeSize;
	RecCodeCall* Code;
};

recBlock* FindBlock(u32 start);
recBlock* AddBlock(u32 start);
bool RemoveBlock(u32 start);