/*
				--nullDC Runtime profiler--
	The runtime profiler is a mini-sampling profiler.
	A background thread collects samples of EIP , and it determines
	in witch dll the EIP points to.After we have some samples (MAX_TICK_COUNT)
	we print the usage statistics on the console and reset the counters
*/
#include "profiler.h"
#include "dc\sh4\rec_v1\blockmanager.h"
#include "plugins/plugin_manager.h"
#include <windows.h>

#define MAX_TICK_COUNT 300

prof_info profile_info;

cThread* prof_thread;

u32 THREADCALL ProfileThead(void* param);

bool RunProfiler;
void init_Profiler(void* param)
{
	//Clear profile info
	memset(&profile_info,0,sizeof(prof_info));

	RunProfiler=true;

	prof_thread = new cThread(ProfileThead,param);
	prof_thread->Start();
}

void term_Profiler()
{
	RunProfiler=false;
	prof_thread->WaitToEnd(666);
	delete prof_thread;
	//Clear profile info
	memset(&profile_info,0,sizeof(prof_info));
}

void AnalyseTick(u32 pc,prof_info* to)
{
	u32 main_base=((u32)AnalyseTick) & 0xFFE00000;
	u32 aica_base=((u32)libAICA->info.Init) & 0xFFE00000;
	u32 pvr_base=((u32)libPvr->info.Init) & 0xFFE00000;
	u32 gdrom_base=((u32)libGDR->info.Init) & 0xFFE00000;

	//printf("0x%X 0x%X to 0x%X\n",pc,DynarecRam_Start,DynarecRam_End);
	if (aica_base==(pc& 0xFFE00000))
	{
		to->aica_tick_count++;
	}
	else if (pvr_base==(pc& 0xFFE00000))
	{
		to->gfx_tick_count++;
	}
	else if (gdrom_base==(pc& 0xFFE00000))
	{
		to->gdrom_tick_count++;
	}
	else if (main_base==(pc& 0xFFE00000))
	{
		to->main_tick_count++;
	}
	else if ((pc>=DynarecRam_Start) && (pc<=(DynarecRam_End+4096)))
	{
		to->dyna_tick_count++;
	}
	else
		to->rest_tick_count++;
}
 u32 THREADCALL ProfileThead(void* param)
 {
	 prof_info info;
	 memset(&info,0,sizeof(prof_info));

	 CONTEXT cntx;

	 while(RunProfiler)
	 {
		 //get emulation thread's pc
		 memset(&cntx,0,sizeof(cntx));
		 cntx.ContextFlags= CONTEXT_FULL;
		 
		 verify(GetThreadContext((HANDLE)param,&cntx));

		 //count ticks
		 info.total_tick_count++;
		 AnalyseTick(cntx.Eip,&info);

		 //Update Stats if needed
		 if (info.total_tick_count>MAX_TICK_COUNT)
		 {
			 char temp[512];

			 memcpy(&profile_info,&info,sizeof(prof_info));
			 memset(&info,0,sizeof(prof_info));

			 profile_info.ToText(temp);
			 printf("%s \n",temp,DynarecRam_Start,DynarecRam_End);
		 }

		 //Sleep , so we dont realy use the cpu much
		 Sleep(1);
	 }

	 CloseHandle((HANDLE)param);
	 return 0;
 }