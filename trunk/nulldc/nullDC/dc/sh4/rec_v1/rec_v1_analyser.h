#pragma once
#include "rec_v1_recompiler.h"
#include "rec_v1_basicblock.h"
#include "rec_v1_ops.h"
#include "dc\sh4\sh4_opcode_list.h"


void rec_v1_AnalyseCode(u32 start,rec_v1_BasicBlock* to,u32 cycles_before=0);