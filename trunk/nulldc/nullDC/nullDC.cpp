// nullDC.cpp : Defines the entry point for the console application.
//

//initialse Emu
#include "types.h"
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

	if(0 != cfgLoadInt("nullDC","enable_recompiler"))
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

void PrintHeader()
{
	printf("nullDC version %d.%d [%s]\n",MAJOR_VER,MINOR_VER,VER_STRING);
}

void EnumPlugins()
{
	List<PluginLoadInfo>* pvr= EnumeratePlugins(PluginType::PowerVR);
	List<PluginLoadInfo>* gdrom= EnumeratePlugins(PluginType::GDRom);
	List<PluginLoadInfo>* aica= EnumeratePlugins(PluginType::AICA);
	List<PluginLoadInfo>* maple= EnumeratePlugins(PluginType::MapleDevice);
	List<PluginLoadInfo>* extdev= EnumeratePlugins(PluginType::ExtDevice);

	printf("PowerVR plugins :\n");
	for (u32 i=0;i<pvr->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,(*pvr)[i].plugin_info.Name,
			(*pvr)[i].plugin_info.PluginVersion.major,
			(*pvr)[i].plugin_info.PluginVersion.minnor,
			(*pvr)[i].plugin_info.PluginVersion.build);
	}

	printf("\nGDRom plugins :\n");
	for (u32 i=0;i<gdrom->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,(*gdrom)[i].plugin_info.Name,
			(*gdrom)[i].plugin_info.PluginVersion.major,
			(*gdrom)[i].plugin_info.PluginVersion.minnor,
			(*gdrom)[i].plugin_info.PluginVersion.build);
	}

	
	printf("\nAica plugins :\n");
	for (u32 i=0;i<aica->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,(*aica)[i].plugin_info.Name,
			(*aica)[i].plugin_info.PluginVersion.major,
			(*aica)[i].plugin_info.PluginVersion.minnor,
			(*aica)[i].plugin_info.PluginVersion.build);
	}

	printf("\nMaple plugins :\n");
	for (u32 i=0;i<maple->itemcount;i++)
	{
		nullDC_Maple_plugin mpl;
		mpl.LoadnullDCPlugin((*maple)[i].dll);
		printf("*\tFound %s v%d.%d.%d [devices : " ,(*maple)[i].plugin_info.Name,
			(*maple)[i].plugin_info.PluginVersion.major,
			(*maple)[i].plugin_info.PluginVersion.minnor,
			(*maple)[i].plugin_info.PluginVersion.build);
		for (int j=0;mpl.maple_info.Devices[j].CreateInstance;j++)
		{
			printf("%s%s",mpl.maple_info.Devices[j].name,mpl.maple_info.Devices[j+1].CreateInstance==0?"]\n":";");
		}
	}
	
	printf("\nExtDevice plugins :\n");
	for (u32 i=0;i<extdev->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,(*extdev)[i].plugin_info.Name,
			(*extdev)[i].plugin_info.PluginVersion.major,
			(*extdev)[i].plugin_info.PluginVersion.minnor,
			(*extdev)[i].plugin_info.PluginVersion.build);
	}

	delete pvr;
	delete gdrom;
	delete aica;
	delete maple;
	delete extdev;

	//getc(stdin);
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

	if(TRUE == cfgLoadInt("nullDC","bNeedsCfg"))
		printf(" >>>>>>>>>>> NEEDS A CFG !\n");

	//get curent path and set plugin path
//	("plugins\\");
//	SetPluginPath(plpath);
//	free(plpath);

	char * plpath = new char[MAX_PATH];
	cfgLoadStr("nullDC_paths","PluginPath", plpath);
	SetPluginPath(plpath);
	delete[] plpath;
	

	PrintHeader();
	EnumPlugins();
	//getc(stdin);
	//return 0;

	if (!CreateGUI())
	{
		printf("Creating GUI failed\n");
		return -1;
	}
	int rv= RunDC(argc,argv);
	
	DestroyGUI();
	
	return rv;
}
int main(int argc, char* argv[])
{
	__try
	{
		main___(argc,argv);
	}
	__except( ExeptionHandler( GetExceptionCode(), (GetExceptionInformation()) ) )
	{

	}
}

