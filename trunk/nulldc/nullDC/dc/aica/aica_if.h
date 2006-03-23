#pragma once
#include "types.h"

#define AICA_MEM_SIZE (2*1024*1024)
#define AICA_MEM_MASK (AICA_MEM_SIZE-1)

u32 aica_readreg(u32 addr,u32 sz);
void aica_writereg(u32 addr,u32 data,u32 sz);
u32 aica_readram(u32 addr,u32 sz);
void aica_writeram(u32 addr,u32 data,u32 sz);
u32 aica_readrtc(u32 addr,u32 sz);
void aica_writertc(u32 addr,u32 data,u32 sz);

void aica_Init();
void aica_Reset(bool Manual);
void aica_Term();

void UpdateAica(u32 cycles);