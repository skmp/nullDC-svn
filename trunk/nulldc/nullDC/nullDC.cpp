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

int RunGui(int argc, char* argv[])
{
	return 0;
}
//Simple command line bootstrap
int RunDC(int argc, char* argv[])
{

	if(0 != cfgLoadInt("nullDC","enable_recompiler",1))
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
	List<PluginLoadInfo>* pvr= EnumeratePlugins(Plugin_PowerVR);
	List<PluginLoadInfo>* gdrom= EnumeratePlugins(Plugin_GDRom);
	List<PluginLoadInfo>* aica= EnumeratePlugins(Plugin_AICA);
	List<PluginLoadInfo>* maple= EnumeratePlugins(Plugin_Maple);
	List<PluginLoadInfo>* maplesub= EnumeratePlugins(Plugin_MapleSub);
	List<PluginLoadInfo>* extdev= EnumeratePlugins(Plugin_ExtDevice);

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
		printf("*\tFound %s v%d.%d.%d [main]\n" ,(*maple)[i].Name,
			(*maple)[i].PluginVersion.major,
			(*maple)[i].PluginVersion.minnor,
			(*maple)[i].PluginVersion.build);
	}
	for (u32 i=0;i<maplesub->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d [sub]\n" ,(*maplesub)[i].Name,
			(*maplesub)[i].PluginVersion.major,
			(*maplesub)[i].PluginVersion.minnor,
			(*maplesub)[i].PluginVersion.build);
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
	delete maplesub;
	delete extdev;
}

int main___(int argc,char* argv[])
{
	PrintSerialIPUsage(argc,argv);
	char * currpath=GetEmuPath("");
	SetCurrentDirectoryA(currpath);
	free(currpath);
	
	// Could Change plugin path even, do first, is always relative to execution dir.
	if(!cfgVerify())
		printf("~ERROR: cfgVerify() Failed!\n");
/*
	if(TRUE == cfgLoadInt("nullDC","bNeedsCfg"))
		printf(" >>>>>>>>>>> NEEDS A CFG !\n");
	*/

	//get curent path and set plugin path
//	("plugins\\");
//	SetPluginPath(plpath);
//	free(plpath);

	char * plpath = new char[MAX_PATH];
	cfgLoadStr("nullDC_paths","PluginPath", plpath,"NULL");
	SetPluginPath(plpath);
	delete[] plpath;
	
	EnumPlugins();

	if (!CreateGUI())
	{
		printf("Creating GUI failed\n");
		return -1;
	}

	while (!plugins_Load())
	{
		if (!plugins_Select())
		{
			printf("Unable to load plugins -- exiting\n");
			return -2;
		}
	}
	
	if (1==0)
	{
		printf("Unable to locate dreamcast bios in \"%s\"\n","bios\\dc_boot.bin");
		return -3; 
	}
	int rv= RunDC(argc,argv);
	
	DestroyGUI();
	
	return rv;
}

int main(int argc, char* argv[])
{
	if (!_vmem_reserve())
	{
		printf("Unable to reserve nullDC memory ...\n");
		getchar();
		return -1;
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

