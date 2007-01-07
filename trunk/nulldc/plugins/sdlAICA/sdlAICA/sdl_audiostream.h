#pragma once
#include "SDL.h"
#include "sdlAICA.h"

void InitAudBuffers(u32 buff_samples);
void WriteSample(s16 r, s16 l);
void WriteSamples(s16* r , s16* l , u32 sample_count);
void WriteSamples(s16* rl , u32 sample_count);

void InitAudio();
void TermAudio();