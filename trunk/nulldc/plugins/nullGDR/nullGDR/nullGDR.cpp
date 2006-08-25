// nullGDR.cpp : Defines the entry point for the DLL application.
//

#include "nullGDR.h"
#include "cdi_if.h"

DriveNotifyEventFP* DriveNotifyEvent;

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
	printf("drkIIRaziel's nullGDR plugin:No config kthx\n");
}
void GetSessionInfo(u8* out,u8 ses);

//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,"Image Reader (cdi) [" __DATE__ "]");
	info->PluginVersion.full=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	
	info->Init=dcInitGDR;
	info->Term=dcTermGDR;
	info->Reset=dcResetGDR;
	info->ThreadInit=dcThreadInitGDR;
	info->ThreadTerm=dcThreadTermGDR;
	info->Type=PluginType::GDRom;
	info->ShowConfig=cfgdlg;
	info->UnhandledWriteExeption=0;//no , we dont use that :p
}
void error_exit(long errcode, const char *string)
{
	printf("Error 0x%X : %s\n",errcode,string);
}
void DriveReadSubChannel(u8 * buff, u32 format, u32 len)
{
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


//Note : the name of these functions can be diferent too

void DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
//	CurrDrive->ReadSector(buff,StartSector,SectorCount,secsz);
	bool mode2=secsz==2048?false:true;
	ReadSectorsCDI(buff,StartSector,SectorCount,true,true);
}

void DriveGetTocInfo(u32* toc,DiskArea area)
{
	ReadTOCHWCDI(toc);
	/*
	TocInfo tempToc;
	CurrDrive->GetToc(&tempToc,area);
	ConvToc(toc,&tempToc);*/
//	GetDriveToc(toc,area);
}
//TODO : fix up
DiskType DriveGetDiskType()
{
	return CdRom_XA;//CurrDrive->GetDiskType();
}

void GetSessionInfo(u8* out,u8 ses)
{
	TInfoSession infoSession;
	ReadInfoSessionCDI(&infoSession);
	out[0] = 0x1;
	out[1] = 0;			

	if (ses == 0)
	{
		*((DWORD*)&out[2]) = infoSession.uLeadOut;
	}
	else
	{
		*((DWORD*)&out[2]) = infoSession.aTrackStart[ses-1];				
	}	
}

//called when plugin is used by emu (you should do first time init here)
void dcInitGDR(void* param,PluginType type)
{
	gdr_init_params* ip=(gdr_init_params*)param;
	DriveNotifyEvent=ip->DriveNotifyEvent;

	char temp[512]="\0";
	uiGetFN(temp,"CDI images (*.cdi)\0 *.cdi\0\0");
	InitCDI(temp);
	DriveNotifyEvent(DriveEvent::DiskChange,0);
}

//called when plugin is unloaded by emu , olny if dcInitGDR is called (eg , not called to enumerate plugins)
void dcTermGDR(PluginType type)
{
	EndCDI();
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

void uiGetFN(TCHAR *szFileName, TCHAR *szParse)
{
	static OPENFILENAME ofn;
	static TCHAR szFile[MAX_PATH];    
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= NULL;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFilter		= szParse;
	ofn.nFilterIndex	= 1;
	ofn.nMaxFileTitle	= 128;
	ofn.lpstrFileTitle	= szFile;
	ofn.lpstrInitialDir	= NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if(GetOpenFileName(&ofn)<=0)
		printf("uiGetFN() Failed !\n");
}