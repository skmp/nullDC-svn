#pragma once
#include "../../types.h"

u32 LoadFileToSh4Mem(u32 offset,char*file);
bool LoadFileToSh4Bootrom(char *szFile);
u32 LoadBinfileToSh4Mem(u32 offset,char*file);
bool LoadFileToSh4Flashrom(char *szFile);
void LoadSyscallHooks();