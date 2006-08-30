// nullGDR.cpp : Defines the entry point for the DLL application.
//

#include "nullGDR.h"
//Get a copy of the operators for structs ... ugly , but works :)
#include "common.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

void cfgdlg(PluginType type,void* window)
{
	printf("No config kthx\n");
}
void GetSessionInfo(u8* out,u8 ses);
//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,"Image Reader plugin by drk||Raziel & GiGaHeRz [" __DATE__ "]");
	info->PluginVersion.full=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	
	info->Init=dcInitGDR;
	info->Term=dcTermGDR;
	info->Reset=dcResetGDR;
	info->ThreadInit=dcThreadInitGDR;
	info->ThreadTerm=dcThreadTermGDR;
	info->Type=PluginType::GDRom;
	info->ShowConfig=cfgdlg;
}

void DriveReadSubChannel(u8 * buff, u32 format, u32 len)
{
	printf("SUB CODE READ DOES NOTHING : 0x%p,0x%X,%d\n",buff,format,len);
}
//Give to the emu pointers for the gd rom interface
EXPORT void dcGetGDRInfo(gdr_plugin_if* info)
{
	info->InterfaceVersion.full=GDR_PLUGIN_I_F_VERSION;

	info->GetDiskType=DriveGetDiskType;
	info->GetToc=DriveGetTocInfo;
	info->ReadSector=DriveReadSector;
	info->GetSessionInfo=GetSessionInfo;
	info->ReadSubChannel=DriveReadSubChannel;
}


void DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	CurrDrive->ReadSector(buff,StartSector,SectorCount,secsz);
}

void DriveGetTocInfo(u32* toc,DiskArea area)
{
	GetDriveToc(toc,area);
}
//TODO : fix up
DiskType DriveGetDiskType()
{
	return CurrDrive->GetDiskType();
}

void GetSessionInfo(u8* out,u8 ses)
{
	GetDriveSessionInfo(out,ses);
}

//called when plugin is used by emu (you should do first time init here)
void dcInitGDR(void* param,PluginType type)
{
	gdr_init_params* ip=(gdr_init_params*)param;
	DriveNotifyEvent=ip->DriveNotifyEvent;
	InitDrive();
	DriveNotifyEvent(DriveEvent::DiskChange,0);
}

//called when plugin is unloaded by emu , olny if dcInitGDR is called (eg , not called to enumerate plugins)
void dcTermGDR(PluginType type)
{
	TermDrive();
}

//It's suposed to reset everything (if not a manual reset)
void dcResetGDR(bool Manual,PluginType type)
{
	DriveNotifyEvent(DriveEvent::DiskChange,0);
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInitGDR(PluginType type)
{
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTermGDR(PluginType type)
{
}
