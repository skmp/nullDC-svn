// nullDC.cpp : Defines the entry point for the console application.
//

//initialse Emu
#include "types.h"
#include "dc/mem/_vmem.h"
#include "stdclass.h"
#include "dc/dc.h"
#include "gui/base.h"
#include "config/config.h"
#define _WIN32_WINNT 0x0500 
#include <windows.h>
#include "plugins/plugin_manager.h"
#include "serial_ipc/serial_ipc_client.h"
#include "cl/cl.h"

__settings settings;

BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ 
  switch( fdwCtrlType ) 
  { 
	  case CTRL_SHUTDOWN_EVENT: 
	  case CTRL_LOGOFF_EVENT: 
	  // Pass other signals to the next handler. 
    case CTRL_BREAK_EVENT: 
	  // CTRL-CLOSE: confirm that the user wants to exit. 
    case CTRL_CLOSE_EVENT: 
    // Handle the CTRL-C signal. 
    case CTRL_C_EVENT: 
		SendMessageA((HWND)GetRenderTargetHandle(),WM_CLOSE,0,0); //FIXME
      return( TRUE );
    default: 
      return FALSE; 
  } 
} 

//Simple command line bootstrap
int RunDC(int argc, wchar* argv[])
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
	
	if (settings.emulator.AutoStart)
		Start_DC();

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
		wprintf(L"*\tFound %s v%d.%d.%d\n" ,(*pvr)[i].Name,
			(*pvr)[i].PluginVersion.major,
			(*pvr)[i].PluginVersion.minnor,
			(*pvr)[i].PluginVersion.build);
	}

	printf("\nGDRom plugins :\n");
	for (u32 i=0;i<gdrom->itemcount;i++)
	{
		wprintf(L"*\tFound %s v%d.%d.%d\n" ,(*gdrom)[i].Name,
			(*gdrom)[i].PluginVersion.major,
			(*gdrom)[i].PluginVersion.minnor,
			(*gdrom)[i].PluginVersion.build);
	}

	
	printf("\nAica plugins :\n");
	for (u32 i=0;i<aica->itemcount;i++)
	{
		wprintf(L"*\tFound %s v%d.%d.%d\n" ,(*aica)[i].Name,
			(*aica)[i].PluginVersion.major,
			(*aica)[i].PluginVersion.minnor,
			(*aica)[i].PluginVersion.build);
	}

	printf("\nMaple plugins :\n");
	for (u32 i=0;i<maple->itemcount;i++)
	{
		wprintf(L"*\tFound %s v%d.%d.%d\n" ,(*maple)[i].Name,
			(*maple)[i].PluginVersion.major,
			(*maple)[i].PluginVersion.minnor,
			(*maple)[i].PluginVersion.build);
	}
	printf("\nExtDevice plugins :\n");
	for (u32 i=0;i<extdev->itemcount;i++)
	{
		wprintf(L"*\tFound %s v%d.%d.%d\n" ,(*extdev)[i].Name,
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

int main___(int argc,wchar* argv[])
{
	if(ParseCommandLine(argc,argv))
	{
		printf("\n\n(Exiting due to command line, without starting nullDC)\n");
		return 69;
	}

	if(!cfgOpen())
	{
		msgboxf(_T("Unable to open config file"),MBX_ICONERROR);
		return -4;
	}
	LoadSettings();

	if (!CreateGUI())
	{
		msgboxf(_T("Creating GUI failed\n"),MBX_ICONERROR);
		return -1;
	}
	int rv= 0;

	wchar* temp_path=GetEmuPath(_T("data\\dc_boot.bin"));
	
	FILE* fr=_wfopen(temp_path,L"r");
	if (!fr)
	{
		msgboxf(_T("Unable to find bios -- exiting\n%s"),MBX_ICONERROR,temp_path);
		rv=-3;
		goto cleanup;
	}
	free(temp_path);

	temp_path=GetEmuPath(_T("data\\dc_flash.bin"));
	
	fr=_wfopen (temp_path,L"r");
	if (!fr)
	{
		msgboxf(_T("Unable to find flash -- exiting\n%s"),MBX_ICONERROR,temp_path);
		rv=-6;
		goto cleanup;
	}

	free(temp_path);

	fclose(fr);
	wchar * currpath=GetEmuPath(L"");
	SetCurrentDirectory(currpath);
	free(currpath);

	EnumPlugins();

	while (!plugins_Load())
	{
		if (!plugins_Select())
		{
			msgboxf(L"Unable to load plugins -- exiting\n",MBX_ICONERROR);
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
	
	SaveSettings();
	return rv;
}

int _tmain(int argc, wchar* argv[])
{
	if (!_vmem_reserve())
	{
		msgboxf(L"Unable to reserve nullDC memory ...",MBX_OK | MBX_ICONERROR);
		return -5;
	}
	int rv=0;
	
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE ) ;
	//EnableMenuItem(GetSystemMenu(GetConsoleWindow(), false), SC_CLOSE, MF_GRAYED);

	__try
	{
		rv=main___(argc,argv);
	}
	__except( ExeptionHandler( GetExceptionCode(), (GetExceptionInformation()) ) )
	{

	}

	//EnableMenuItem(GetSystemMenu(GetConsoleWindow(), false), SC_CLOSE, MF_ENABLED);

	_vmem_release();
	return rv;
}


void LoadSettings()
{
	settings.dynarec.Enable=cfgLoadInt(L"nullDC",L"Dynarec.Enabled",1);
	settings.dynarec.CPpass=cfgLoadInt(L"nullDC",L"Dynarec.DoConstantPropagation",1);
	settings.dynarec.UnderclockFpu=cfgLoadInt(L"nullDC",L"Dynarec.UnderclockFpu",0);
	
	settings.dreamcast.cable=cfgLoadInt(L"nullDC",L"Dreamcast.Cable",3);
	settings.dreamcast.RTC=cfgLoadInt(L"nullDC",L"Dreamcast.RTC",GetRTC_now());

	settings.emulator.AutoStart=cfgLoadInt(L"nullDC",L"Emulator.AutoStart",0);

	//make sure values are valid
	settings.dreamcast.cable=min(max(settings.dreamcast.cable,0),3);
}
void SaveSettings()
{
	cfgSaveInt(L"nullDC",L"Dynarec.Enabled",settings.dynarec.Enable);
	cfgSaveInt(L"nullDC",L"Dynarec.DoConstantPropagation",settings.dynarec.CPpass);
	cfgSaveInt(L"nullDC",L"Dynarec.UnderclockFpu",settings.dynarec.UnderclockFpu);
	cfgSaveInt(L"nullDC",L"Dreamcast.Cable",settings.dreamcast.cable);
	cfgSaveInt(L"nullDC",L"Dreamcast.RTC",settings.dreamcast.RTC);
	cfgSaveInt(L"nullDC",L"Emulator.AutoStart",settings.emulator.AutoStart);
}