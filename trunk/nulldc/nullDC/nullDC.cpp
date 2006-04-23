// nullDC.cpp : Defines the entry point for the console application.
//

//initialse Emu
#include "types.h"
#include "stdclass.h"
#include "dc/dc.h"
#include "gui/base.h"
#include "config/config.h"

#include <windows.h>
#include "plugins\plugin_manager.h"

int RunGui(int argc, char* argv[])
{
	return 0;
}
//Simple command line bootstrap
int RunDC(int argc, char* argv[])
{

	if(0 != cfgLoadInt("nullDC","enable_recompiler"))
		sh4_cpu=Get_Sh4Recompiler();
	else
		sh4_cpu=Get_Sh4Interpreter();
	
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
	GrowingList<PluginLoadInfo>* pvr= EnumeratePlugins(PluginType::PowerVR);
	GrowingList<PluginLoadInfo>* gdrom= EnumeratePlugins(PluginType::GDRom);
	GrowingList<PluginLoadInfo>* aica= EnumeratePlugins(PluginType::AICA);
	GrowingList<PluginLoadInfo>* mdm= EnumeratePlugins(PluginType::MapleDeviceMain);
	GrowingList<PluginLoadInfo>* mds= EnumeratePlugins(PluginType::MapleDeviceSub);

	printf("PowerVR plugins :\n");
	for (u32 i=0;i<pvr->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,pvr->items[i].item.plugin_info.Name,
			pvr->items[i].item.plugin_info.PluginVersion.major,
			pvr->items[i].item.plugin_info.PluginVersion.minnor,
			pvr->items[i].item.plugin_info.PluginVersion.build);
	}

	printf("\nGDRom plugins :\n");
	for (u32 i=0;i<gdrom->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,gdrom->items[i].item.plugin_info.Name,
			gdrom->items[i].item.plugin_info.PluginVersion.major,
			gdrom->items[i].item.plugin_info.PluginVersion.minnor,
			gdrom->items[i].item.plugin_info.PluginVersion.build);
	}

	printf("\nAica plugins :\n");
	for (u32 i=0;i<aica->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,aica->items[i].item.plugin_info.Name,
			aica->items[i].item.plugin_info.PluginVersion.major,
			aica->items[i].item.plugin_info.PluginVersion.minnor,
			aica->items[i].item.plugin_info.PluginVersion.build);
	}

	printf("\nMaple plugins :\n");
	for (u32 i=0;i<mdm->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,mdm->items[i].item.plugin_info.Name,
			mdm->items[i].item.plugin_info.PluginVersion.major,
			mdm->items[i].item.plugin_info.PluginVersion.minnor,
			mdm->items[i].item.plugin_info.PluginVersion.build);
	}

	printf("\nMaple subdevice plugins :\n");
	for (u32 i=0;i<mds->itemcount;i++)
	{
		printf("*\tFound %s v%d.%d.%d\n" ,mds->items[i].item.plugin_info.Name,
			mds->items[i].item.plugin_info.PluginVersion.major,
			mds->items[i].item.plugin_info.PluginVersion.minnor,
			mds->items[i].item.plugin_info.PluginVersion.build);
	}
	
	delete pvr;
	delete gdrom;
	delete aica;
	delete mdm;
	delete mds;

	//getc(stdin);
}

int main(int argc, char* argv[])
{
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

