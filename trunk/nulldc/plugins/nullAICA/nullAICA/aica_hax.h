#pragma once
#include "nullAICA.h"

u32 ReadMem_reg(u32 addr,u32 size);
void WriteMem_reg(u32 addr,u32 data,u32 size);

u32 ReadMem_ram(u32 addr,u32 size);
void WriteMem_ram(u32 addr,u32 data,u32 size);