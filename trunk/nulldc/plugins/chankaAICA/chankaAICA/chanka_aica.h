#pragma once


u32 ReadMem_reg(u32 addr,u32 size);
void WriteMem_reg(u32 addr,u32 data,u32 size);

u32 ReadMem_ram(u32 addr,u32 size);
void WriteMem_ram(u32 addr,u32 data,u32 size);

void UpdateSystem(u32 Cycles);
void InitARM7(void* winh);

extern RaiseInterruptFP* Sh4RaiseInterrupt;
extern u32* SB_ISTEXT;
extern u32 g_videoCableType;
extern u32 sh4_cycles;