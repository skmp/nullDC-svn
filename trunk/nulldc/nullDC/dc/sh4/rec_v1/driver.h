#pragma once
#include "types.h"



//interface
void rec_Sh4_int_Run(ThreadCallbackFP* tcb);
void rec_Sh4_int_Stop();
void rec_Sh4_int_Step();
void rec_Sh4_int_Skip();
void rec_Sh4_int_Reset(bool Manual);
void rec_Sh4_int_Init();
void rec_Sh4_int_Term();
bool rec_Sh4_int_IsCpuRunning();   