#pragma once
#include "aica.h"

void arm_Init();
void arm_Reset();	
void arm_Run(u32 uNumCycles);
void arm_SetEnabled(bool enabled);

#define arm_sh4_bias (2)
#define arm_sh4_ratio (200/25)



