#pragma once
#include "types.h"

typedef void RecCodeCall();

void recStartRecompile();
void recEndRecompile();
bool recRecompileOp(u32 op,u32 rec_pc);
RecCodeCall* recGetFunction();
u32 recGetCodeSize();