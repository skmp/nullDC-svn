#pragma once
#include "types.h"

extern bool TBP_Enabled;
void init_Profiler(void* param);
void term_Profiler();

void start_Profiler();
void stop_Profiler();

u64 CycleDiff();
struct prof_info
{
	int total_tick_count;		//total tics

	int gfx_tick_count;			//on gfx dll
	int aica_tick_count;		//on aica dll
	int gdrom_tick_count;		//on gdrom dll
	int maple_tick_count;		//on maple dll
	int dyna_tick_count;		//on dynarec mem
	int main_tick_count;		//on main exe
	int rest_tick_count;		//dunno where :p

	#define percent(x) (x##_tick_count*100.0)/(total_tick_count)
	#define effsceas(x)    (cd/(double)x##_tick_count/1000.0/1000.0/20.0) 
	void ToText(char* dest)
	{
		if (total_tick_count==0)
		{
			strcpy(dest,"No profile info");
			return;
		}
		u64 cd=CycleDiff();
		dest+=sprintf(dest,"gfx %.3f,% 3.1f%% | ",effsceas(gfx),percent(gfx));
		dest+=sprintf(dest,"aica %.3f,% 3.1f%% | ",effsceas(aica),percent(aica));
		if (percent(gdrom)!=0)
		{
			dest+=sprintf(dest,"gdrom %.3f,% 3.1f%% | ",effsceas(gdrom),percent(gdrom));
		}
		dest+=sprintf(dest,"main %.3f,% 3.1f%% | ",effsceas(main),percent(main));
		dest+=sprintf(dest,"dyna %.3f,% 3.1f%% | ",effsceas(dyna),percent(dyna));
		dest+=sprintf(dest,"rest %.3f,% 3.1f%%",effsceas(rest),percent(rest));
		
	}
	#undef percent
};

extern prof_info profile_info;