#pragma once
#include "recompiler.h"
#include "BasicBlock.h"
#include "ops.h"
#include "dc\sh4\sh4_opcode_list.h"


void AnalyseCode(u32 start,BasicBlock* to); 
