/*
	memory mapping
	there are 2 memory maps, system memory map (named sh4, as its used by the sh4 cpu) and the audio block one (named arm because its used by the arm7/sound cpu).
	There are also G2/G1/etc bus mem maps that are currently ingored
	[sh4|arm7][MemRead|MemWrite][8|16|32]
	arm7 does not have native 32 bit reads (the bus is 16 bits long) but its faster for us to ingore that fact.
	the basic memmap doesnt have 64 bit addressing (the dynarec path is able to do 64bit reads from ram however).
*/

bool sh4MemInit();
bool sh4MemReset(bool phys);
void sh4MemTerm();

s32 sh4MemRead8(u32 addr);
s32 sh4MemRead16(u32 addr);
s32 sh4MemRead32(u32 addr);

void sh4MemWrite8(u32 addr,u8 data);
void sh4MemWrite16(u32 addr,u16 data);
void sh4MemWrite32(u32 addr,u32 data);

void* sh4MemPtr(u32 addr);
#define sh4MemPtr8(addr) ((u8*)sh4MemPtr(addr))
#define sh4MemPtr16(addr) ((u16*)sh4MemPtr(addr))
#define sh4MemPtr32(addr) ((u32*)sh4MemPtr(addr))

/*
	no arm7 for now ;p
*/
