#pragma once
#include "types.h"
#include "plugins/plugin_manager.h"


//pvr plugins
bool SetPvrPlugin(nullDC_PowerVR_plugin* plg);

//regs
u32 pvr_readreg_TA(u32 addr,u32 sz);
void pvr_writereg_TA(u32 addr,u32 data,u32 sz);

//vram 32-64b
extern Array<u8> vram;
//read
u8 pvr_read_area1_8(u32 addr);
u16 pvr_read_area1_16(u32 addr);
u32 pvr_read_area1_32(u32 addr);
//write
void pvr_write_area1_8(u32 addr,u8 data);
void pvr_write_area1_16(u32 addr,u16 data);
void pvr_write_area1_32(u32 addr,u32 data);
void pvr_write_area1_block(u32 addr,u32* data,u32 size);

void pvr_Update(u32 cycles);

//Init/Term , global
void pvr_Init();
void pvr_Term();
//Reset -> Reset - Initialise
void pvr_Reset(bool Manual);

void TAWrite(u32 address,u32* data,u32 count);
//
#define UpdatePvr(clc) libPvr->pvr_info.UpdatePvr(clc)


//registers 
#define PVR_BASE 0x005F8000