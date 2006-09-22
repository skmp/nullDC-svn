#pragma once
#include "types.h"

void init_Profiler(void* param);
void term_Profiler();


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

	#define percent(x) (x##_tick_count*100)/(total_tick_count)
	void ToText(char* dest)
	{
		if (total_tick_count==0)
		{
			strcpy(dest,"No profile info");
			return;
		}

		dest+=sprintf(dest,"gfx %d%% ",percent(gfx));
		dest+=sprintf(dest,"aica %d%% ",percent(aica));
		dest+=sprintf(dest,"gdrom %d%% ",percent(gdrom));
		dest+=sprintf(dest,"main %d%% ",percent(main));
		dest+=sprintf(dest,"dyna %d%% ",percent(dyna));
		dest+=sprintf(dest,"rest %d%% ",percent(rest));
	}
	#undef percent
};

extern prof_info profile_info;