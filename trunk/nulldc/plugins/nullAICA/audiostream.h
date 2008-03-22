#pragma once
#include "sdlAICA.h"

void WriteSample(s16 r, s16 l);
void WriteSamples1(s16* r , s16* l , u32 sample_count);
void WriteSamples2(s16* rl , u32 sample_count);

void InitAudio();
void TermAudio();