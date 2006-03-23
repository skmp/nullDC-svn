#pragma once
#include "nullAICA.h"

u32 ReadMem_reg(u32 addr,u32 size);
void WriteMem_reg(u32 addr,u32 data,u32 size);

u32 ReadMem_ram(u32 addr,u32 size);
void WriteMem_ram(u32 addr,u32 data,u32 size);

void UpdateSystem(u32 Cycles);

#define AICA_MEM_SIZE (2*1024*1024)
#define AICA_MEM_MASK (AICA_MEM_SIZE-1)

void init_mem();
void term_mem();

extern u8 *aica_reg;
extern u8 *aica_ram;