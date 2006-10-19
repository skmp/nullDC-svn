// nullGDR.cpp : Defines the entry point for the DLL application.
//

#include "ImgReader.h"
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

#define PLUGIN_NAME "Image Reader plugin by drk||Raziel & GiGaHeRz [" __DATE__ "]"
void cfgdlg(PluginType type,void* window)
{
	printf("No config kthx\n");
}
void GetSessionInfo(u8* out,u8 ses);
//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,PLUGIN_NAME);
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
//	printf("SUB CODE READ DOES NOTHING : 0x%p,0x%X,%d\n",buff,format,len);
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


#define INTERFACE_VERSION	MAKEWORD(1,0)
struct chanka_cd_play_pos
{
	BYTE Track;
	BYTE m,s,f;		//Relative to start of track
	BYTE am,as,af;	//Absolute
};
	struct TInfoSession
	{
		int iNumSessions;
		DWORD uLeadOut;
		DWORD aTrackStart[100];
	};

//Chankast plugin code :)
struct chanka_cdrom_info
{
  int Version;
  char *Description;
  int (__cdecl *Init)(const char* pszFileName);
  void (__cdecl *End)();
  void (__cdecl *Refresh)();
  int (__cdecl *ReadSectors)(void* pBuffer, int sector, int cnt, bool bMode2, bool bWait);	  
  void (__cdecl *AbortCommand)();
  int (__cdecl *ReadTOCHW)(void* pDriveToc);
  int (__cdecl *ReadInfoSession)(TInfoSession* pDriveSession);  
  void (__cdecl *PauseAudio)();
  DWORD (__cdecl *seek_audio_msf)(BYTE m, BYTE s, BYTE f);  
  bool (__cdecl *IsLastLoadCompleted)();
  int (__cdecl *Configure)(void* hWnd);
  bool (_cdecl *PlayAudio)(BYTE m1,BYTE s1,BYTE f1,BYTE m2,BYTE s2,BYTE f2,BYTE nReps);
  bool (_cdecl *StopAudio)();
  bool (_cdecl *ResumeAudio)();
  bool (_cdecl *GetCurrentPlayPos)(struct chanka_cd_play_pos *PlayPos);
};

void chanka_DriveNotifyEvent(DriveEvent info,void* param)
{
}
int _cdecl chanka_Init(const char* pszFileName)
{
	gdr_init_params params;
	params.DriveNotifyEvent=chanka_DriveNotifyEvent;

	dcInitGDR(&params,GDRom);
	dcThreadInitGDR(GDRom);
	return 0;
}
void _cdecl chanka_End()
{
	dcThreadTermGDR(GDRom);
	dcTermGDR(GDRom);
}
void _cdecl chanka_Refresh()
{
	//hmm ?
}
int _cdecl chanka_ReadSectors(void* pBuffer, int sector, int cnt, bool bMode2, bool bWait)
{
	//__asm int 3;
	//sector+=150;
	DriveReadSector((u8*)pBuffer,sector,cnt,2048);	//allways 2048?
	return true;
}
void _cdecl chanka_AbortCommand()
{
	//nothing ?
}
int _cdecl chanka_ReadTOCHW(void* pDriveToc)
{
	DriveGetTocInfo((u32*)pDriveToc,SingleDensity);
	return 0;
}
int _cdecl chanka_ReadInfoSession(TInfoSession* pDriveSession)
{
	//__asm int 3;
	//this actualy needs some work
	u8 temp[8];
	GetSessionInfo(temp,0);//?
	u32 session_count=temp[2];

	pDriveSession->iNumSessions=session_count;
	pDriveSession->uLeadOut=(*(u32*)&temp[2]);

	for (int i=0;i<session_count;i++)
	{
		GetSessionInfo(temp,i+1);//?
		pDriveSession->aTrackStart[i]=(*(s32*)(&temp[2]));
		
	}
	return 0;
}
void _cdecl chanka_PauseAudio()
{
	//nothing ? 
}
DWORD _cdecl chanka_seek_audio_msf(BYTE m, BYTE s, BYTE f)
{
	//nothing ? 
	return 0;
}
bool _cdecl chanka_IsLastLoadCompleted()
{
	//nothing ? 
	return true;
}
int  _cdecl chanka_Configure(void* hWnd)
{
	printf("Config!\n");
	return 0;
}
bool _cdecl chanka_PlayAudio(BYTE m1,BYTE s1,BYTE f1,BYTE m2,BYTE s2,BYTE f2,BYTE nReps)
{
	//nothing ? 
	return true;
}
bool _cdecl chanka_StopAudio()
{
	//nothing ? 
	return true;
}
bool _cdecl chanka_ResumeAudio()
{
	//nothing ? 
	return true;
}
bool _cdecl chanka_GetCurrentPlayPos(chanka_cd_play_pos *PlayPos)
{
	//nothing ? 
	return true;
}

chanka_cdrom_info chanka_cdif=
{
	INTERFACE_VERSION,
    PLUGIN_NAME,
    chanka_Init,
    chanka_End,
    chanka_Refresh,
    chanka_ReadSectors,
    chanka_AbortCommand,
    chanka_ReadTOCHW,
    chanka_ReadInfoSession,
    chanka_PauseAudio,
    chanka_seek_audio_msf,
	chanka_IsLastLoadCompleted,
	chanka_Configure,
	chanka_PlayAudio,
	chanka_StopAudio,
	chanka_ResumeAudio,
	chanka_GetCurrentPlayPos
};
//typedef chanka_cdrom_info* (__cdecl *GetCDRomInterface)();
EXPORT chanka_cdrom_info* __cdecl GetCDRomInterface()
{
	return &chanka_cdif;
}