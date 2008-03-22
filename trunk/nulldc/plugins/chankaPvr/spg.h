#pragma once
#include "chankaPvr.h"

bool spg_Init();
void spg_Term();
void spg_Reset(bool Manual);

//#define Frame_Cycles (DCclock/60)

//need to replace 511 with correct value
//#define Line_Cycles (Frame_Cycles/511)

void FASTCALL spgUpdatePvr(u32 cycles);
bool spg_Init();
void spg_Term();
void spg_Reset(bool Manual);
void CalculateSync();

//hacks
#include "taVideo.h"
extern void rend_set_fps_text(char* text);

#define rend_end_render()
#define rend_vblank() Unai::TAStartVBlank()

extern u32 FrameCount;
extern u32 VertexCount;