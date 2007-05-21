// drkPvr.cpp : Defines the entry point for the DLL application.
//

#include "drkPvr.h"

#include "ta.h"
#include "spg.h"
#include "regs.h"
#include "renderer_if.h"

//RaiseInterruptFP* RaiseInterrupt;

//void* Hwnd;

emu_info emu;
char emu_name[512];

pvr_init_params params;
_settings_type settings;

//u8*	params.vram;
//vramlock_Lock_32FP* lock32;
//vramlock_Lock_64FP* lock64;
//vramlock_Unlock_blockFP* unlock;

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

void UpdateRRect()
{
	float rect[4]={0,0,640,480};
	if (settings.Enhancements.WidescreenHack)
	{
		WINDOWINFO winf;
		GetWindowInfo((HWND)emu.WindowHandle,&winf);
		
		int sx=winf.rcClient.right-winf.rcClient.left;
		int sy=winf.rcClient.bottom-winf.rcClient.top;
		//printf("New rect %d %d\n",sx,sy);

		float nw=((float)sx/(float)sy)*480.0f;
		rect[0]=(nw-640)/2;
		rect[2]=nw;
	}
	rend_set_render_rect(rect);
}
void FASTCALL dcShowConfig(void* window)
{
	printf("No config for now\n");
}


void FASTCALL vramLockCB (vram_block* block,u32 addr)
{
	//rend_if_vram_locked_write(block,addr);
	//renderer->VramLockedWrite(block);
	rend_text_invl(block);
}
#include <vector>
using std::vector;

vector<u32> res;

void FASTCALL handler_ShowFps(u32 id,void* win,void* puser)
{
	if (settings.ShowFPS)
		settings.ShowFPS=0;
	else
		settings.ShowFPS=1;

	emu.SetMenuItemStyle(id,settings.ShowFPS?MIS_Checked:0,MIS_Checked);
	
	SaveSettings();
}
void FASTCALL handler_WSH(u32 id,void* win,void* puser)
{
	if (settings.Enhancements.WidescreenHack)
		settings.Enhancements.WidescreenHack=0;
	else
		settings.Enhancements.WidescreenHack=1;

	emu.SetMenuItemStyle(id,settings.Enhancements.WidescreenHack?MIS_Checked:0,MIS_Checked);
	
	SaveSettings();
	UpdateRRect();
}
void FASTCALL handler_VerPTex(u32 id,void* win,void* puser)
{
	if (settings.VersionedPalleteTextures)
		settings.VersionedPalleteTextures=0;
	else
		settings.VersionedPalleteTextures=1;

	emu.SetMenuItemStyle(id,settings.VersionedPalleteTextures?MIS_Checked:0,MIS_Checked);
	
	SaveSettings();
}
u32 enable_FS_mid;
u32 AA_mid_menu;
u32 AA_mid_0;
void FASTCALL handler_SetFullscreen(u32 id,void* win,void* puser)
{
	if (settings.Fullscreen.Enabled)
		settings.Fullscreen.Enabled=0;
	else
		settings.Fullscreen.Enabled=1;

	emu.SetMenuItemStyle(id,settings.Fullscreen.Enabled?MIS_Checked:0,MIS_Checked);
	
	SaveSettings();
}

#define makeres(a,b) {#a "x" #b,a,b},
struct 
{
	char* name;
	u32 x;
	u32 y;
} resolutions[]=
{
	makeres(640,480)
	makeres(800,600)
	makeres(1024,768)
	makeres(1280,800)
	makeres(1280,960)
	makeres(1280,1024)
	makeres(1440,1050)
	makeres(1600,900)
	makeres(1600,1200)
	makeres(1920,1080)
	makeres(1920,1200)
	makeres(2048,1536)
	{0,0,0}
};
u32 special_res=0;
void FASTCALL handler_SetRes(u32 id,void* win,void* puser)
{
	for (size_t i=0;i<res.size();i++)
	{
		emu.SetMenuItemStyle(res[i],res[i]==id?MIS_Checked:0,MIS_Checked);
		if (res[i]==id)
		{
			settings.Fullscreen.Res_X=resolutions[i].x;
			settings.Fullscreen.Res_Y=resolutions[i].y;
		}
	}

	if (special_res)
	{
		emu.DeleteMenuItem(special_res);
		special_res=0;
	}
	SaveSettings();
}

void FASTCALL handle_About(u32 id,void* w,void* p)
{
	MessageBox((HWND)w,"Made by the nullDC Team","About nullPVR...",MB_ICONINFORMATION);
}
//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Load(emu_info* emu_inf)
{
	memcpy(&emu,emu_inf,sizeof(emu));
	emu.ConfigLoadStr("emu","shortname",emu_name,0);
	
	LoadSettings();

	u32 Resolutions_menu=emu.AddMenuItem(emu.RootMenu,-1,"Fullscreen",0,0);
	
	emu.SetMenuItemStyle(emu.AddMenuItem(emu.RootMenu,-1,"-",0,0),MIS_Seperator,MIS_Seperator);

	
	enable_FS_mid=emu.AddMenuItem(Resolutions_menu,-1,"Enable",handler_SetFullscreen,settings.Fullscreen.Enabled);
	emu.SetMenuItemStyle(emu.AddMenuItem(Resolutions_menu,-1,"-",0,0),MIS_Seperator,MIS_Seperator);

	bool sel_any=false;
	for (u32 rc=0;resolutions[rc].name;rc++)
	{
		bool sel=(resolutions[rc].x==settings.Fullscreen.Res_X) && (resolutions[rc].y==settings.Fullscreen.Res_Y);
		if (sel)
			sel_any=true;
		u32 nmi=emu.AddMenuItem(Resolutions_menu,-1,resolutions[rc].name,handler_SetRes,sel);
		res.push_back(nmi);
		emu.SetMenuItemStyle(nmi,MIS_Radiocheck,MIS_Radiocheck);
	}
	special_res=0;
	if (!sel_any)
	{
		char temp[512];
		sprintf(temp,"%dx%d",settings.Fullscreen.Res_X,settings.Fullscreen.Res_Y);
		special_res=emu.AddMenuItem(Resolutions_menu,-1,temp,handler_SetRes,1);
		emu.SetMenuItemStyle(special_res,MIS_Grayed|MIS_Checked,MIS_Grayed|MIS_Checked);
	}

	emu.AddMenuItem(emu.RootMenu,-1,"Widescreen Hack",handler_WSH,settings.Enhancements.WidescreenHack);
	emu.AddMenuItem(emu.RootMenu,-1,"Versioned Textures",handler_VerPTex,settings.VersionedPalleteTextures);
	emu.AddMenuItem(emu.RootMenu,-1,"Show Fps",handler_ShowFps,settings.ShowFPS);

	emu.SetMenuItemStyle(emu.AddMenuItem(emu.RootMenu,-1,"-",0,0),MIS_Seperator,MIS_Seperator);

	emu.AddMenuItem(emu.RootMenu,-1,"About",handle_About,0);
//	SetRenderer(RendererType::Hw_D3d,params.WindowHandle);
	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void FASTCALL Unload()
{
	
}

//It's suposed to reset anything but vram (vram is set to 0 by emu)
void FASTCALL ResetPvr(bool Manual)
{
	Regs_Reset(Manual);
	spg_Reset(Manual);
	rend_reset(Manual);
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
s32 FASTCALL InitPvr(pvr_init_params* param)
{
	memcpy(&params,param,sizeof(params));

	
	if ((!Regs_Init()))
	{
		//failed
		return rv_error;
	}
	if (!spg_Init())
	{
		//failed
		return rv_error;
	}
	if (!rend_init())
	{
		//failed
		return rv_error;
	}
	UpdateRRect();
	//olny the renderer cares about thread speciacific shit ..
	if (!rend_thread_start())
	{
		return rv_error;
	}

	emu.SetMenuItemStyle(enable_FS_mid,MIS_Grayed,MIS_Grayed);
	return rv_ok;
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL TermPvr()
{
	rend_thread_end();

	rend_term();
	spg_Term();
	Regs_Term();

	emu.SetMenuItemStyle(enable_FS_mid,0,MIS_Grayed);
}

//Helper functions
float GetSeconds()
{
	return timeGetTime()/1000.0f;
}

//Needed for EMUWARN/EMUERROR to work properly
//Misc function to get relative source directory for printf's
char temp[1000];
char* GetNullDCSoruceFileName(char* full)
{
	size_t len = strlen(full);
	while(len>18)
	{
		if (full[len]=='\\')
		{
			memcpy(&temp[0],&full[len-14],15*sizeof(char));
			temp[15*sizeof(char)]=0;
			if (strcmp(&temp[0],"\\nulldc\\nulldc\\")==0)
			{
				strcpy(temp,&full[len+1]);
				return temp;
			}
		}
		len--;
	}
	strcpy(temp,full);
	return &temp[0];
}

//Give to the emu pointers for the PowerVR interface
void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
#define c  info->common
#define p info->pvr

	info->InterfaceVersion=PLUGIN_I_F_VERSION;
	
	c.Type=Plugin_PowerVR;
	c.InterfaceVersion=PVR_PLUGIN_I_F_VERSION;

	strcpy(c.Name,"nullPVR -- " REND_NAME " built : " __DATE__);
	c.PluginVersion=DC_MakeVersion(1,0,0,DC_VER_NORMAL);

	c.Load=Load;
	c.Unload=Unload;

	p.ExeptionHanlder=0;
	p.Init=InitPvr;
	p.Reset=ResetPvr;
	p.Term=TermPvr;

	
	p.ReadReg=ReadPvrRegister;
	p.WriteReg=WritePvrRegister;
	p.UpdatePvr=spgUpdatePvr;

	p.TaDMA=TASplitter::Dma;
	p.TaSQ=TASplitter::SQ;
	p.LockedBlockWrite=vramLockCB;
	
#undef c
#undef p
}



//Windoze Code implementation of commong classes from here and after ..

//Thread class
cThread::cThread(ThreadEntryFP* function,void* prm)
{
	Entry=function;
	param=prm;
	hThread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)function,prm,CREATE_SUSPENDED,NULL);
}
cThread::~cThread()
{
	//gota think of something !
}
	
void cThread::Start()
{
	ResumeThread(hThread);
}
void cThread::Suspend()
{
	SuspendThread(hThread);
}
void cThread::WaitToEnd(u32 msec)
{
	WaitForSingleObject(hThread,msec);
}
//End thread class

//cResetEvent Calss
cResetEvent::cResetEvent(bool State,bool Auto)
{
		hEvent = CreateEvent( 
        NULL,             // default security attributes
		Auto?FALSE:TRUE,  // auto-reset event?
		State?TRUE:FALSE, // initial state is State
        NULL			  // unnamed object
        );
}
cResetEvent::~cResetEvent()
{
	//Destroy the event object ?
	 CloseHandle(hEvent);
}
void cResetEvent::Set()//Signal
{
	SetEvent(hEvent);
}
void cResetEvent::Reset()//reset
{
	ResetEvent(hEvent);
}
void cResetEvent::Wait(u32 msec)//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,msec);
}
void cResetEvent::Wait()//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,(u32)-1);
}
//End AutoResetEvent

//(const char * lpSection, const char * lpKey, char * lpReturn);
//(const char * lpSection, const char * lpKey, const char * lpString);
int cfgGetInt(char* key,int def)
{
	/*
	char temp1[100];
	char temp2[100];
	sprintf(temp2,"%d",def);
	
	emu.ConfigLoadStr("drkpvr",key,temp,temp1);
	/*if (strcmp("NULL",temp)==0)
		return def;
	return atoi(temp1);*/
	return emu.ConfigLoadInt("drkpvr",key,def);
}
void cfgSetInt(char* key,int val)
{
	/*
	char temp1[100];
	char temp2[100];
	sprintf(temp2,"%d",def);
	
	emu.ConfigLoadStr("drkpvr",key,temp,temp1);
	/*if (strcmp("NULL",temp)==0)
		return def;
	return atoi(temp1);*/
	emu.ConfigSaveInt("drkpvr",key,val);
}

void LoadSettings()
{
	settings.Fullscreen.Enabled					=	cfgGetInt("Fullscreen.Enabled",0);
	settings.Fullscreen.Res_X					=	cfgGetInt("Fullscreen.Res_X",640);
	settings.Fullscreen.Res_Y					=	cfgGetInt("Fullscreen.Res_Y",480);
	settings.Fullscreen.Refresh_Rate			=	cfgGetInt("Fullscreen.Refresh_Rate",60);

	settings.ShowFPS							=	cfgGetInt("ShowFPS",0);
	settings.VersionedPalleteTextures			=	cfgGetInt("VersionedPalleteTextures",1);

	settings.Enhancements.MultiSampleCount		=	cfgGetInt("Enhancements.MultiSampleCount",0);
	settings.Enhancements.MultiSampleQuality	=	cfgGetInt("Enhancements.MultiSampleQuality",0);
	settings.Enhancements.WidescreenHack		=	cfgGetInt("Enhancements.WidescreenHack",0);
}


void SaveSettings()
{
	cfgSetInt("Fullscreen.Enabled",settings.Fullscreen.Enabled);
	cfgSetInt("Fullscreen.Res_X",settings.Fullscreen.Res_X);
	cfgSetInt("Fullscreen.Res_Y",settings.Fullscreen.Res_Y);
	cfgSetInt("Fullscreen.Refresh_Rate",settings.Fullscreen.Refresh_Rate);

	cfgSetInt("ShowFPS",settings.ShowFPS);
	cfgSetInt("VersionedPalleteTextures",settings.VersionedPalleteTextures);

	cfgSetInt("Enhancements.MultiSampleCount",settings.Enhancements.MultiSampleCount);
	cfgSetInt("Enhancements.MultiSampleQuality",settings.Enhancements.MultiSampleQuality);
	cfgSetInt("Enhancements.WidescreenHack",settings.Enhancements.WidescreenHack);
}

int msgboxf(char* text,unsigned int type,...)
{
	va_list args;

	char temp[2048];
	va_start(args, type);
	vsprintf(temp, text, args);
	va_end(args);


	return MessageBox(NULL,temp,emu_name,type | MB_TASKMODAL);
}
