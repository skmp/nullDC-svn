/*
	nullDCe -- A Sega Dreamcast emulator for embedded systems
	Designed and implemented by drk||Raziel (drkiiraziel@gmail.com),all rights reserved.

	nulldce - main nullDCe header
*/
#define HOST_X86 1
#include "types.h"
#include "debug\ioif.h"

#define EMU_FULLNAME "nullDCe v0"
#define EMU_NAME "nullDCe"

#define BUILD_DREAMCAST 1
	
//DC : 16 mb ram, 8 mb vram, 2 mb aram, 2 mb bios, 128k flash
#define RAM_SIZE (16*1024*1024)
#define VRAM_SIZE (8*1024*1024)
#define ARAM_SIZE (2*1024*1024)
#define BIOS_SIZE (2*1024*1024)

#define RAM_MASK	(RAM_SIZE-1)
#define VRAM_MASK	(VRAM_SIZE-1)
#define ARAM_MASK	(ARAM_SIZE-1)
#define BIOS_MASK	(BIOS_SIZE-1)
#define FLASH_MASK	(FLASH_SIZE-1)
