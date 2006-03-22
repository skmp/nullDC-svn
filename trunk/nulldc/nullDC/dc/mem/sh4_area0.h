#pragma once
#include "..\..\types.h"

u32 ReadMem_area0(u32 addr,u32 sz);
void WriteMem_area0(u32 addr,u32 data,u32 sz);
//Init/Res/Term
void sh4_area0_Init();
void sh4_area0_Reset(bool Manual);
void sh4_area0_Term();