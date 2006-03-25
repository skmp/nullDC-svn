#pragma once
#include "types.h"

#define GDROM_OPCODE	((u16)0x30F9)	// GDROMCALL- 0011 0000 1111 1001
u32 LoadFileToSh4Mem(u32 offset,char*file);
bool LoadFileToSh4Bootrom(char *szFile);
u32 LoadBinfileToSh4Mem(u32 offset,char*file);
bool LoadFileToSh4Flashrom(char *szFile);
void LoadSyscallHooks();