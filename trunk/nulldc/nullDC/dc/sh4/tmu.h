#pragma once

#include "..\..\types.h"

bool tmu_CNT0Pending();
u32 tmu_CNT0Priority();
bool tmu_CNT1Pending();
u32 tmu_CNT1Priority();
bool tmu_CNT2Pending();
u32 tmu_CNT2Priority();

void UpdateTMU(u32 Cycles);
void tmu_Init();
void tmu_Reset(bool Manual);
void tmu_Term();