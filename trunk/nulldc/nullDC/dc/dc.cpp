//dc.cpp
//emulation driver - interface
#include "mem\sh4_mem.h"
#include "sh4\sh4_opcode_list.h"
#include "pvr\pvr_if.h"
#include "mem\sh4_internal_reg.h"
#include "aica\aica_if.h"
#include "dc.h"

#include <string.h>

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
	//look for plugins and load the first pvr plugin found
	GrowingList<PluginLoadInfo>* pli_pvr= EnumeratePlugins(PluginType::PowerVR);
	GrowingList<PluginLoadInfo>* pli_gdr= EnumeratePlugins(PluginType::GDRom);
	GrowingList<PluginLoadInfo>* pli_aica= EnumeratePlugins(PluginType::AICA);
	nullDC_PowerVR_plugin* pvrplg=new nullDC_PowerVR_plugin();
	nullDC_GDRom_plugin* gdrplg=new nullDC_GDRom_plugin();
	nullDC_AICA_plugin* aicaplg=new nullDC_AICA_plugin();

	pvrplg->LoadnullDCPlugin((*pli_pvr)[1].dll);
	gdrplg->LoadnullDCPlugin((*pli_gdr)[0].dll);
	aicaplg->LoadnullDCPlugin((*pli_aica)[0].dll);
	

	SetPlugin(pvrplg,PluginType::PowerVR);
	SetPlugin(gdrplg,PluginType::GDRom);
	SetPlugin(aicaplg,PluginType::AICA);

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

	//when we boot from ip.bin , it's nice to have it seted up
	sh4_cpu->SetRegister(Sh4RegType::reg_gbr,0x8c000000);
	sh4_cpu->SetRegister(Sh4RegType::reg_sr,0x700000F0);
	sh4_cpu->SetRegister(Sh4RegType::reg_fpscr,0x0004001);

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

void LoadBiosFiles()
{
	char* temp_path=GetEmuPath("data\\");
	u32 pl=strlen(temp_path);

	//mwhahaha
	strcat(temp_path,"dc_boot.bin");
	LoadFileToSh4Bootrom(temp_path);

	temp_path[pl]=0;
	strcat(temp_path,"dc_flash.bin");
	LoadFileToSh4Flashrom(temp_path);
	

	temp_path[pl]=0;
	strcat(temp_path,"syscalls.bin");
	LoadFileToSh4Mem(0x00000, temp_path);

	temp_path[pl]=0;
	strcat(temp_path,"IP.bin");
	LoadFileToSh4Mem(0x08000, temp_path);
	temp_path[pl]=0;
	
	free(temp_path);
}
void Start_DC()
{
	if (!sh4_cpu->IsCpuRunning())
	{
		if (!dc_inited)
		{
			Init_DC();
		}

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
