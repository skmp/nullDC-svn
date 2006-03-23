// nullDC.cpp : Defines the entry point for the console application.
//

//initialse Emu
#include "types.h"
#include "stdclass.h"
#include "dc/dc.h"
#include "gui/base.h"

#include <windows.h>
#include "plugins\plugin_manager.h"

int RunGui(int argc, char* argv[])
{
	return 0;
}
//Simple command line bootstrap
int RunDC(int argc, char* argv[])
{
	printf("%s\n",argv[1]==0?"Booting Bios":argv[1]);
	//look for plugins and load the first pvr plugin found
	GrowingList<PluginLoadInfo>* pli_pvr= EnumeratePlugins(PluginType::PowerVR);
	GrowingList<PluginLoadInfo>* pli_gdr= EnumeratePlugins(PluginType::GDRom);
	GrowingList<PluginLoadInfo>* pli_aica= EnumeratePlugins(PluginType::AICA);
	nullDC_PowerVR_plugin pvrplg;
	nullDC_GDRom_plugin gdrplg;
	nullDC_AICA_plugin aicaplg;

	pvrplg.LoadnullDCPlugin((*pli_pvr)[1].dll);
	gdrplg.LoadnullDCPlugin((*pli_gdr)[0].dll);
	aicaplg.LoadnullDCPlugin((*pli_aica)[0].dll);
	

	SetPlugin(&pvrplg,PluginType::PowerVR);
	SetPlugin(&gdrplg,PluginType::GDRom);
	SetPlugin(&aicaplg,PluginType::AICA);



	//Get an interface to the sh4 emu and set the sh4_cpu to it
	sh4_cpu=Get_Sh4Interpreter();

	//init and reset all h/w exept sh4 (sh4 has to be done manualy)
	//this includes curently loaded pvr/aica/arm/gdrom/maple plugin [if any]
	//Init and reset
	Init_DC();
	
	//hard reset
	Reset_DC(false);

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

	if (argv[1]!=0)
	{
		LoadBinfileToSh4Mem(0x10000,argv[1]==0?".bin":argv[1]);
		sh4_cpu->SetRegister(Sh4RegType::reg_pc,0x8c008300);//0x8c010000);//0xA0000000;//0x8c008300
	}
	else
		sh4_cpu->SetRegister(Sh4RegType::reg_pc,0xA0000000);//0x8c010000);//0xA0000000;//0x8c008300

	//when we boot from ip.bin , it's nice to have it seted up
	sh4_cpu->SetRegister(Sh4RegType::reg_gbr,0x8c000000);
	sh4_cpu->SetRegister(Sh4RegType::reg_sr,0x700000F0);
	sh4_cpu->SetRegister(Sh4RegType::reg_fpscr,0x0004001);

	
	GuiLoop();

	Term_DC();
	Release_Sh4If(sh4_cpu);
	free(temp_path);
	//getc(stdin);
	
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
	//get curent path and set plugin path
	char * plpath=GetEmuPath("plugins\\");

	SetPluginPath(plpath);
	
	free(plpath);
	

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

