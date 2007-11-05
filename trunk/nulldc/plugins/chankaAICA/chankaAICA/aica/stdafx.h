#pragma once
#include "../chankaAICA.h"
#include "base.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

extern u8*g_pSH4SoundRAM;
extern u32 g_videoCableType;

extern u32 sh4_cycles;

#define NUM_CYCLES_PER_BLOCK 448
#define SH4_CYCLES_PER_FRAME ((200*1000*1000)/60)



