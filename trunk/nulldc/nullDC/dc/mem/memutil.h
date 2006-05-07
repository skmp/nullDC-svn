#pragma once
#include "types.h"

#define GDROM_OPCODE	((u16)0x30F9)	// GDROMCALL- 0011 0000 1111 1001

#define patch_syscall_system		32
#define patch_syscall_font			16
#define patch_syscall_flashrom		8
#define patch_syscall_GDrom_misc	4
#define patch_syscall_GDrom_HLE		2
#define patch_resets_Misc			1
#define patch_all					0xFFFFFFFF

void EnablePatch(int patch);
void DisablePatch(int patch);

u32 LoadFileToSh4Mem(u32 offset,char*file);
bool LoadFileToSh4Bootrom(char *szFile);
u32 LoadBinfileToSh4Mem(u32 offset,char*file);
bool LoadFileToSh4Flashrom(char *szFile);
bool SaveSh4FlashromToFile(char *szFile);

void LoadSyscallHooks();