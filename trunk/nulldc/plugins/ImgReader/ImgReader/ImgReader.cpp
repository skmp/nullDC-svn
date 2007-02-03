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
void FASTCALL cfgdlg(void* window)
{
	printf("No config kthx\n");
}
void FASTCALL GetSessionInfo(u8* out,u8 ses);


void FASTCALL DriveReadSubChannel(u8 * buff, u32 format, u32 len)
{
//	printf("SUB CODE READ DOES NOTHING : 0x%p,0x%X,%d\n",buff,format,len);
}

void FASTCALL DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	CurrDrive->ReadSector(buff,StartSector,SectorCount,secsz);
}

void FASTCALL DriveGetTocInfo(u32* toc,u32 area)
{
	GetDriveToc(toc,(DiskArea)area);
}
//TODO : fix up
u32 FASTCALL DriveGetDiscType()
{
	return CurrDrive->GetDiscType();
}

void FASTCALL GetSessionInfo(u8* out,u8 ses)
{
	GetDriveSessionInfo(out,ses);
}
emu_info emu;
//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Load(emu_info* emu_inf)
{
	memcpy(&emu,emu_inf,sizeof(emu));
	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInitGDR is called (eg , not called to enumerate plugins)
void FASTCALL Unload()
{
	
}

//It's suposed to reset everything (if not a manual reset)
void FASTCALL ResetGDR(bool Manual)
{
	DriveNotifyEvent(DriveEvent::DiskChange,0);
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
s32 FASTCALL InitGDR(gdr_init_params* prm)
{
	DriveNotifyEvent=prm->DriveNotifyEvent;
	if (!InitDrive())
		return rv_serror;
	DriveNotifyEvent(DiskChange,0);

	return rv_ok;
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL TermGDR()
{
	TermDrive();
}

//Give to the emu info for the plugin type
void EXPORT_CALL dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion=PLUGIN_I_F_VERSION;
	info->count=1;

}
//Give to the emu pointers for the gd rom interface
bool EXPORT_CALL dcGetPlugin(u32 id , plugin_info_entry* info)
{
#define c info->common
#define g info->gdr
	
	c.Type=GDRom;
	c.InterfaceVersion=GDR_PLUGIN_I_F_VERSION;

	strcpy(c.Name,PLUGIN_NAME);
	c.PluginVersion=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	
	c.Load=Load;
	c.Unload=Unload;
	
	
	g.Init=InitGDR;
	g.Term=TermGDR;
	g.Reset=ResetGDR;
	g.ShowConfig=cfgdlg;
	
	g.GetDiscType=DriveGetDiscType;
	g.GetToc=DriveGetTocInfo;
	g.ReadSector=DriveReadSector;
	g.GetSessionInfo=GetSessionInfo;
	g.ReadSubChannel=DriveReadSubChannel;
	g.ExeptionHanlder=0;

	return true;
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

void FASTCALL chanka_DriveNotifyEvent(u32 info,void* param)
{
}
int _cdecl chanka_Init(const char* pszFileName)
{
	gdr_init_params params;
	params.DriveNotifyEvent=chanka_DriveNotifyEvent;

	emu_info emuif;
	strcpy(emuif.Name,"Chankast 0.2.5");

	Load(&emuif);
	InitGDR(&params);
	return 0;
}
void _cdecl chanka_End()
{
	TermGDR();
	Unload();
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