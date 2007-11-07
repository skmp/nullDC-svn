#pragma once
#include "aica.h"
#include "_vmem.h"
/*
u32 arm_ReadMem8(u32 addr);
u32 arm_ReadMem16(u32 addr);
u32 arm_ReadMem32(u32 addr);

void arm_WriteMem8(u32 addr,u32 value);
void arm_WriteMem16(u32 addr,u32 value);
void arm_WriteMem32(u32 addr,u32 value);
*/

#define arm_ReadMem8  _vmem_ReadMem8
#define arm_ReadMem16 _vmem_ReadMem16
#define arm_ReadMem32 _vmem_ReadMem32

#define arm_WriteMem8  _vmem_WriteMem8
#define arm_WriteMem16 _vmem_WriteMem16
#define arm_WriteMem32 _vmem_WriteMem32


u32 FASTCALL sh4_ReadMem_reg(u32 addr,u32 size);
void FASTCALL sh4_WriteMem_reg(u32 addr,u32 data,u32 size);
u32 sh4_ReadMem_ram(u32 addr,u32 size);
void sh4_WriteMem_ram(u32 addr,u32 data,u32 size);

void init_mem();
void term_mem();

extern u8 *aica_reg;
extern u8 *aica_ram;
#define aica_reg_16 ((u16*)aica_reg)

#define AICA_RAM_SIZE (2*1024*1024)
#define AICA_RAM_MASK (AICA_RAM_SIZE-1)

#define AICA_MEMMAP_RAM_SIZE (8*1024*1024)				//this is the max for the map, the actual ram size is AICA_RAM_SIZE
#define AICA_MEMMAP_RAM_MASK (AICA_MEMMAP_RAM_SIZE-1)