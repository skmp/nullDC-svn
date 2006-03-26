//dc.cpp
//emulation driver - interface
#include "mem\sh4_mem.h"
#include "sh4\sh4_opcode_list.h"
#include "pvr\pvr_if.h"
#include "mem\sh4_internal_reg.h"
#include "aica\aica_if.h"
#include "dc.h"

bool dc_inited=false;
bool dc_reseted=false;

//called from the new thread
void ThreadCallback_DC(bool start)
{
	//call Thread initialisers
	if (start)
	{
		plugins_ThreadInit();
	}
	//call Thread terminators
	else
	{
		plugins_ThreadTerm();
	}
}

void cputhreadcb(bool shit)
{
	if (shit)
		printf("+Sh4 thread started \n");
	else
		printf("-Sh4 thread stoped \n");

	ThreadCallback_DC(shit);
}

//Init mainly means allocate
//Reset is called before first run
//Init is called olny once
//When Init is called , cpu interface and all plugins configurations myst be finished
//Plugins/Cpu core must not change after this call is made.
bool Init_DC()
{
	sh4_cpu->Init();
	mem_Init();
	pvr_Init();
	aica_Init();
	plugins_Init();

	dc_inited=true;
	return true;
}

bool Reset_DC(bool Manual)
{
	sh4_cpu->Reset(Manual);
	mem_Reset(Manual);
	pvr_Reset(Manual);
	aica_Reset(Manual);
	plugins_Reset(Manual);

	dc_reseted=true;
	return true;
}

void Term_DC()
{
	sh4_cpu->Term();
	plugins_Term();
	aica_Term();
	pvr_Term();
	mem_Term();
}


void Start_DC()
{
	if (!sh4_cpu->IsCpuRunning())
	{
		if (!dc_inited)
			Init_DC();

		if (!dc_reseted)
			Reset_DC(false);//hard reset kthx

		sh4_cpu->Run(cputhreadcb);
	}
}
void Stop_DC()
{
	if (dc_inited)//sh4_cpu may not be inited ;)
	{
		//if (sh4_cpu->IsCpuRunning())
		{
			sh4_cpu->Stop();
		}
	}
}
