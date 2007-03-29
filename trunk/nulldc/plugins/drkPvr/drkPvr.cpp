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
void FASTCALL handler_SetRes(u32 id,void* win,void* puser)
{
	for (size_t i=0;i<res.size();i++)
		emu.SetMenuItemStyle(res[i],res[i]==id?MIS_Checked:0,MIS_Checked);

	SaveSettings();
}

void FASTCALL handle_About(u32 id,void* w,void* p)
{
	MessageBox((HWND)w,"Made by drk||Raziel","About drkpvr...",MB_ICONINFORMATION);
}
//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Load(emu_info* emu_inf,u32 rmenu)
{
	memcpy(&emu,emu_inf,sizeof(emu));
	emu.ConfigLoadStr("emu","shortname",emu_name,0);
	
	LoadSettings();

	emu.AddMenuItem(rmenu,-1,"Show Fps",handler_ShowFps,settings.ShowFPS);

	u32 Resolutions_menu=emu.AddMenuItem(rmenu,-1,"Resolution",0,0);

	res.push_back(emu.AddMenuItem(Resolutions_menu,-1,"640x480",handler_SetRes,0));
	res.push_back(emu.AddMenuItem(Resolutions_menu,-1,"800x600",handler_SetRes,0));
	res.push_back(emu.AddMenuItem(Resolutions_menu,-1,"1024x768",handler_SetRes,0));
	res.push_back(emu.AddMenuItem(Resolutions_menu,-1,"1152x864",handler_SetRes,0));
	res.push_back(emu.AddMenuItem(Resolutions_menu,-1,"1280x1024",handler_SetRes,0));

	emu.SetMenuItemStyle(res[0],MIS_Checked,MIS_Checked);
	for (size_t i=0;i<res.size();i++)
		emu.SetMenuItemStyle(res[i],MIS_Radiocheck,MIS_Radiocheck);


	emu.AddMenuItem(rmenu,-1,"About",handle_About,0);
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

	//olny the renderer cares about thread speciacific shit ..
	if (!rend_thread_start())
	{
		return rv_error;
	}

	return rv_ok;
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL TermPvr()
{
	rend_thread_end();

	rend_term();
	spg_Term();
	Regs_Term();
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

	strcpy(c.Name,"drkpvr -- " REND_NAME);
	c.PluginVersion=DC_MakeVersion(0,9,0,DC_VER_NORMAL);

	c.Load=Load;
	c.Unload=Unload;

	p.ExeptionHanlder=0;
	p.Init=InitPvr;
	p.Reset=ResetPvr;
	p.Term=TermPvr;

	
	p.ReadReg=ReadPvrRegister;
	p.WriteReg=WritePvrRegister;
	p.UpdatePvr=spgUpdatePvr;

	p.TaFIFO=TASplitter::Dma;
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

	settings.Enhancements.MultiSampleCount		=	cfgGetInt("Enhancements.MultiSampleCount",0);
	settings.Enhancements.MultiSampleQuality	=	cfgGetInt("Enhancements.MultiSampleQuality",0);
}


void SaveSettings()
{
	cfgSetInt("Fullscreen.Enabled",settings.Fullscreen.Enabled);
	cfgSetInt("Fullscreen.Res_X",settings.Fullscreen.Res_X);
	cfgSetInt("Fullscreen.Res_Y",settings.Fullscreen.Res_Y);
	cfgSetInt("Fullscreen.Refresh_Rate",settings.Fullscreen.Refresh_Rate);

	cfgSetInt("ShowFPS",settings.ShowFPS);

	cfgSetInt("Enhancements.MultiSampleCount",settings.Enhancements.MultiSampleCount);
	cfgSetInt("Enhancements.MultiSampleQuality",settings.Enhancements.MultiSampleQuality);
}