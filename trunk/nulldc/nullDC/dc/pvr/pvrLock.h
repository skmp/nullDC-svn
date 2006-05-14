#pragma once

#include "types.h"

//16 mb for naomi.. so be carefull not to hardcode it anywhere
#define VRAM_SIZE (0x800000)	//0x1000000 for 16 mb :)
#define VRAM_MASK (VRAM_SIZE-1)

#define Is_64_Bit(addr) ((addr &0x1000000)==0)
 
struct vram_block
{
	u32 start;
	u32 end;
	u32 len;
	u32 type;
 
	void* userdata;
};
 
typedef void vramLockCBFP (vram_block* block,u32 addr);
 
u32 vramlock_ConvAddrtoOffset64(u32 Address);

void vramlock_Unlock_block(vram_block* block);
vram_block* vramlock_Lock_32(u32 start_offset32,u32 end_offset32,void* userdata);
vram_block* vramlock_Lock_64(u32 start_offset64,u32 end_offset64,void* userdata);

void vram_LockedWrite(u32 offset64);