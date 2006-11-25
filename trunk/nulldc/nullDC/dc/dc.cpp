//dc.cpp
//emulation driver - interface
#include "mem/sh4_mem.h"
#include "sh4/sh4_opcode_list.h"
#include "pvr/pvr_if.h"
#include "mem/sh4_internal_reg.h"
#include "aica/aica_if.h"
#include "maple/maple_if.h"
#include "dc.h"
#include "config/config.h"
#include "profiler/profiler.h"
#include <string.h>
#include <windows.h>
#include "gdb_stub/gdb_stub.h"

bool dc_inited=false;
bool dc_reseted=false;
bool dc_ingore_init=false;
//called from the new thread
void ThreadCallback_DC(bool start)
{
	//call Thread initialisers
	if (start)
	{
		plugins_ThreadInit();
		GDB_BOOT_TEST();
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
	{
		printf("+Sh4 thread started \n");
		HANDLE hThreadreal;
		DuplicateHandle(GetCurrentProcess(), 
                    GetCurrentThread(), 
                    GetCurrentProcess(),
                    &hThreadreal, 
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS);
		init_Profiler(hThreadreal);
	}
	else
	{
		term_Profiler();
		printf("-Sh4 thread stoped \n");
	}
	if (!dc_ingore_init)
		ThreadCallback_DC(shit);
	dc_ingore_init=false;
}

//Init mainly means allocate
//Reset is called before first run
//Init is called olny once
//When Init is called , cpu interface and all plugins configurations myst be finished
//Plugins/Cpu core must not change after this call is made.
bool Init_DC()
{
	char temp_dll[512];
	nullDC_PowerVR_plugin* pvrplg=new nullDC_PowerVR_plugin();
	nullDC_GDRom_plugin* gdrplg=new nullDC_GDRom_plugin();
	nullDC_AICA_plugin* aicaplg=new nullDC_AICA_plugin();
	nullDC_ExtDevice_plugin* extdevplg=new nullDC_ExtDevice_plugin();

	//load the selected plugins
	cfgLoadStr("nullDC_plugins","Current_PVR",temp_dll);
	pvrplg->LoadnullDCPlugin(temp_dll);

	cfgLoadStr("nullDC_plugins","Current_GDR",temp_dll);
	gdrplg->LoadnullDCPlugin(temp_dll);

	cfgLoadStr("nullDC_plugins","Current_AICA",temp_dll);
	aicaplg->LoadnullDCPlugin(temp_dll);

	cfgLoadStr("nullDC_plugins","Current_ExtDevice",temp_dll);
	extdevplg->LoadnullDCPlugin(temp_dll);

	//ok , all loaded , set em as selected
	SetPlugin(pvrplg,PluginType::PowerVR);
	SetPlugin(gdrplg,PluginType::GDRom);
	SetPlugin(aicaplg,PluginType::AICA);
	SetPlugin(extdevplg,PluginType::ExtDevice);

	sh4_cpu->Init();
	mem_Init();
	pvr_Init();
	aica_Init();
	mem_map_defualt();
	plugins_Init();
	maple_plugins_Init();

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
	if (dc_inited)
	{
		sh4_cpu->Term();
		maple_plugins_Term();
		plugins_Term();
		aica_Term();
		pvr_Term();
		mem_Term();
	}
}

void LoadBiosFiles()
{
	char* temp_path=GetEmuPath("data\\");
	u32 pl=(u32)strlen(temp_path);


#ifdef BUILD_DREAMCAST
	strcat(temp_path,"dc_boot.bin");
#elif	BUILD_NAOMI
	strcat(temp_path,"naomi_boot.bin");
#else	//BUILD_DEV_UNIT
	strcat(temp_path,"hkt_boot.bin");
#endif

	LoadFileToSh4Bootrom(temp_path);

#ifndef BUILD_NAOMI
	temp_path[pl]=0;
	//try to load saved flash
	strcat(temp_path,"dc_flash_wb.bin");
	if (!LoadFileToSh4Flashrom(temp_path))
	{
		//not found , load default :)
		temp_path[pl]=0;
		strcat(temp_path,"dc_flash.bin");
		LoadFileToSh4Flashrom(temp_path);
	}
	

	temp_path[pl]=0;
	strcat(temp_path,"syscalls.bin");
	LoadFileToSh4Mem(0x00000, temp_path);

	temp_path[pl]=0;
	strcat(temp_path,"IP.bin");
	LoadFileToSh4Mem(0x08000, temp_path);
	temp_path[pl]=0;

#else

	// Add Xicor Flash when dumped and others ... 

#endif

	free(temp_path);
}

void SaveFlash()
{
	char* temp_path=GetEmuPath("data\\");
	printf("ERROR : FLASH NOT SAVED\n");
//	u32 pl=(u32)strlen(temp_path);
	//SaveSh4FlashromToFile(
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

bool IsDCInited()
{
	return dc_inited;
}
void SwitchCPU_DC()
{
	dc_ingore_init=true;//hehe just ingore next thread callback :P
}