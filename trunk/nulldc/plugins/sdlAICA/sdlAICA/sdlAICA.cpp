// nullAICA.cpp : Defines the entry point for the DLL application.
//

#include "sdlAICA.h"
#include "sgc_if.h"
#include "aica.h"
#include "arm7.h"
#include "mem.h"
#include "audiostream.h"

setts settings;
aica_init_params aica_params;
emu_info eminf;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


void cfgdlg(PluginType type,void* window)
{
	printf("Config coming soon [blame ms/msvc for that , its their fault for making window related code suck so much]");
}

/*
//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,"nullDC AICA plugin [sdl] , built :" __DATE__ "");
	info->PluginVersion.full=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	
	info->Init=dcInit;
	info->Term=dcTerm;
	info->Reset=dcReset;

	info->ThreadInit=dcThreadInit;
	info->ThreadTerm=dcThreadTerm;
	info->ShowConfig=cfgdlg;
	info->Type=PluginType::AICA;
}

//Give to the emu pointers for the PowerVR interface
EXPORT void dcGetAICAInfo(aica_plugin_if* info)
{
	info->InterfaceVersion.full=AICA_PLUGIN_I_F_VERSION;

	info->ReadMem_aica_ram=sh4_ReadMem_ram;
	info->WriteMem_aica_ram=sh4_WriteMem_ram;
	info->ReadMem_aica_reg=sh4_ReadMem_reg;
	info->WriteMem_aica_reg=sh4_WriteMem_reg;
	info->UpdateAICA=UpdateAICA;
}
*/
void FASTCALL handle_About(u32 id,void* w,void* p)
{
	MessageBoxA((HWND)w,"Made by drk||Raziel","About nullDC Aica...",MB_ICONINFORMATION);
}
s32 FASTCALL OnLoad(emu_info* em,u32 rmenu)
{
	memcpy(&eminf,em,sizeof(eminf));

	eminf.AddMenuItem(rmenu,-1,"About",handle_About,0);
	return rv_ok;
}

void FASTCALL OnUnload()
{
}

//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Init(aica_init_params* initp)
{
	memcpy(&aica_params,initp,sizeof(aica_params));

	//load default settings before init
	settings.BufferSize=cfgGetInt("BufferSize",1024);
	settings.LimitFPS=cfgGetInt("LimitFPS",1);
	settings.HW_mixing=cfgGetInt("HW_mixing",0);
	settings.SoundRenderer=cfgGetInt("SoundRenderer",1);
	settings.GlobalFocus=cfgGetInt("GlobalFocus",1);
	settings.BufferCount=cfgGetInt("BufferCount",1);

	init_mem();
	arm_Init();
	AICA_Init();
	InitAudio();

	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
void FASTCALL Term()
{
	TermAudio();
	AICA_Term();
	term_mem();
}

//It's suposed to reset anything 
void FASTCALL Reset(bool Manual)
{
	arm_Reset();
}


//Give to the emu pointers for the PowerVR interface
EXPORT void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
	info->InterfaceVersion=PLUGIN_I_F_VERSION;
/*
	info->Init=dcInit;
	info->Term=dcTerm;
	info->Reset=dcReset;

	info->ThreadInit=dcThreadInit;
	info->ThreadTerm=dcThreadTerm;
	info->ShowConfig=cfgdlg;

	info->Type=PluginType::AICA;

	info->InterfaceVersion.full=AICA_PLUGIN_I_F_VERSION;

	info->ReadMem_aica_ram=ReadMem_ram;
	info->WriteMem_aica_ram=WriteMem_ram;
	info->ReadMem_aica_reg=ReadMem_reg;
	info->WriteMem_aica_reg=WriteMem_reg;
	info->UpdateAICA=UpdateSystem;
*/
#define c info->common
#define a info->aica

	strcpy(c.Name,"nullDC AICA plugin [sdl] , built :" __DATE__ "");
	c.PluginVersion=DC_MakeVersion(MAJOR,MINOR,BUILD,DC_VER_NORMAL);

	c.InterfaceVersion=AICA_PLUGIN_I_F_VERSION;
	c.Type=Plugin_AICA;

	c.Load=OnLoad;
	c.Unload=OnUnload;

	a.Init=Init;
	a.Reset=Reset;
	a.Term=Term;

	a.ExeptionHanlder=0;

	a.UpdateAICA=UpdateAICA;

	a.ReadMem_aica_reg=sh4_ReadMem_reg;
	a.WriteMem_aica_reg=sh4_WriteMem_reg;
}

int cfgGetInt(char* key,int def)
{
	return eminf.ConfigLoadInt("sdlaica",key,def);
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