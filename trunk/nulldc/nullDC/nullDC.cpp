// nullDC.cpp : Defines the entry point for the console application.
//

//initialse Emu
#include "types.h"
#include "dc/mem/_vmem.h"
#include "stdclass.h"
#include "dc/dc.h"
#include "gui/base.h"
#include "config/config.h"

#include <windows.h>
#include "plugins/plugin_manager.h"
#include "serial_ipc/serial_ipc_client.h"

__settings settings;

int RunGui(int argc, char* argv[])
{
	return 0;
}
//Simple command line bootstrap
int RunDC(int argc, char* argv[])
{

	if(settings.dynarec.Enable)
	{
		sh4_cpu=Get_Sh4Recompiler();
		printf("Using Recompiler\n");
	}
	else
	{
		sh4_cpu=Get_Sh4Interpreter();
		printf("Using Interpreter\n");
	}
	
	GuiLoop();

	Term_DC();
	Release_Sh4If(sh4_cpu);
	return 0;
}


void EnumPlugins()
{
	EnumeratePlugins();

	List<PluginLoadInfo>* pvr= GetPluginList(Plugin_PowerVR);
	List<PluginLoadInfo>* gdrom= GetPluginList(Plugin_GDRom);
	List<PluginLoadInfo>* aica= GetPluginList(Plugin_AICA);
	List<PluginLoadInfo>* maple= GetPluginList(Plugin_Maple);
	List<PluginLoadInfo>* extdev= GetPluginList(Plugin_ExtDevice);

	printf("PowerVR plugins :\n");
	for (u32 i=0;i<pvr->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,(*pvr)[i].Name,
			(*pvr)[i].PluginVersion.major,
			(*pvr)[i].PluginVersion.minnor,
			(*pvr)[i].PluginVersion.build);
	}

	printf("\nGDRom plugins :\n");
	for (u32 i=0;i<gdrom->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,(*gdrom)[i].Name,
			(*gdrom)[i].PluginVersion.major,
			(*gdrom)[i].PluginVersion.minnor,
			(*gdrom)[i].PluginVersion.build);
	}

	
	printf("\nAica plugins :\n");
	for (u32 i=0;i<aica->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,(*aica)[i].Name,
			(*aica)[i].PluginVersion.major,
			(*aica)[i].PluginVersion.minnor,
			(*aica)[i].PluginVersion.build);
	}

	printf("\nMaple plugins :\n");
	for (u32 i=0;i<maple->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,(*maple)[i].Name,
			(*maple)[i].PluginVersion.major,
			(*maple)[i].PluginVersion.minnor,
			(*maple)[i].PluginVersion.build);
	}
	printf("\nExtDevice plugins :\n");
	for (u32 i=0;i<extdev->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,(*extdev)[i].Name,
			(*extdev)[i].PluginVersion.major,
			(*extdev)[i].PluginVersion.minnor,
			(*extdev)[i].PluginVersion.build);
	}

	delete pvr;
	delete gdrom;
	delete aica;
	delete maple;
	delete extdev;
}

int main___(int argc,char* argv[])
{
	if(!cfgOpen())
	{
		msgboxf("Unable to open config file",MBX_ICONERROR);
		return -4;
	}
	LoadSettings();

	if (!CreateGUI())
	{
		msgboxf("Creating GUI failed\n",MBX_ICONERROR);
		return -1;
	}
	int rv= 0;

	char* temp_path=GetEmuPath("data\\dc_boot.bin");
	
	FILE* fr=fopen(temp_path,"r");
	if (!fr)
	{
		msgboxf("Unable to find bios -- exiting\n%s",MBX_ICONERROR,temp_path);
		rv=-3;
		goto cleanup;
	}
	free(temp_path);

	temp_path=GetEmuPath("data\\dc_flash.bin");
	
	fr=fopen(temp_path,"r");
	if (!fr)
	{
		msgboxf("Unable to find flash -- exiting\n%s",MBX_ICONERROR,temp_path);
		rv=-6;
		goto cleanup;
	}

	free(temp_path);

	fclose(fr);
	PrintSerialIPUsage(argc,argv);
	char * currpath=GetEmuPath("");
	SetCurrentDirectoryA(currpath);
	free(currpath);

	EnumPlugins();

	while (!plugins_Load())
	{
		if (!plugins_Select())
		{
			msgboxf("Unable to load plugins -- exiting\n",MBX_ICONERROR);
			rv = -2;
			goto cleanup;
		}
	}
	
	/*
	if (1==0)
	{
		msgboxf("Unable to locate dreamcast bios in \"%s\"\n",MBX_ICONERROR,"bios\\dc_boot.bin");
		rv = -3; 
		goto cleanup;
	}
	*/
	rv = RunDC(argc,argv);
	
cleanup:
	DestroyGUI();
	
	return rv;
}

int main(int argc, char* argv[])
{
	if (!_vmem_reserve())
	{
		msgboxf("Unable to reserve nullDC memory ...",MBX_OK | MBX_ICONERROR);
		return -5;
	}
	int rv=0;
	__try
	{
		rv=main___(argc,argv);
	}
	__except( ExeptionHandler( GetExceptionCode(), (GetExceptionInformation()) ) )
	{

	}
	_vmem_release();
	return rv;
}


void LoadSettings()
{
	settings.dynarec.Enable=cfgLoadInt("nullDC","Dynarec.Enabled",1);
	settings.dynarec.CPpass=cfgLoadInt("nullDC","Dynarec.DoConstantPropagation",1);
	settings.dynarec.UnderclockFpu=cfgLoadInt("nullDC","Dynarec.UnderclockFpu",0);
	settings.dreamcast.cable=cfgLoadInt("nullDC","Dreamcast.Cable",3);
	settings.dreamcast.cable=min(max(settings.dreamcast.cable,0),3);
}
void SaveSettings()
{
	cfgSaveInt("nullDC","Dynarec.Enabled",settings.dynarec.Enable);
	cfgSaveInt("nullDC","Dynarec.DoConstantPropagation",settings.dynarec.CPpass);
	cfgSaveInt("nullDC","Dynarec.UnderclockFpu",settings.dynarec.UnderclockFpu);
	cfgSaveInt("nullDC","Dreamcast.Cable",settings.dreamcast.cable);
}