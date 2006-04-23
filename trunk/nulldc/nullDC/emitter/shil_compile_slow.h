#pragma once
#include "types.h"
#include "dc\sh4\shil\shil.h"
#include "dc\sh4\rec_v1\rec_v1_basicblock.h"

void CompileBasicBlock_slow(rec_v1_BasicBlock* block); 
void link_compile_inject_TF_stub(rec_v1_BasicBlock* ptr);
void link_compile_inject_TT_stub(rec_v1_BasicBlock* ptr);