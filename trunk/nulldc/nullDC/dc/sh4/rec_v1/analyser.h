#pragma once
#include "rec_v1_recompiler.h"
#include "BasicBlock.h"
#include "rec_v1_ops.h"
#include "dc\sh4\sh4_opcode_list.h"


void rec_v1_AnalyseCode(u32 start,BasicBlock* to); 
