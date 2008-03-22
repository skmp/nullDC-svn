#pragma once


u32 FASTCALL ReadMem_reg(u32 addr,u32 size);
void FASTCALL WriteMem_reg(u32 addr,u32 data,u32 size);

void FASTCALL UpdateSystem(u32 Cycles);
void InitARM7();
void TerminateARM7();

extern u32 g_videoCableType;
extern u32 sh4_cycles;