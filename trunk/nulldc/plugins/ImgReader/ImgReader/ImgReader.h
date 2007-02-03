#pragma once
//bleh stupid windoze header
#include "..\..\..\nullDC\plugins\plugin_header.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#define BUILD 0
#define MINOR 0
#define MAJOR 1
extern emu_info emu;

#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); __asm {int 3}}