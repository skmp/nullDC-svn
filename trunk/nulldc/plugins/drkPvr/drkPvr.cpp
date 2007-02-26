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
pvr_init_params params;

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

//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Load(emu_info* emu_inf)
{
	memcpy(&emu,emu_inf,sizeof(emu));
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

//Give to the emu info for the plugin type
void EXPORT_CALL dcGetInterfaceInfo(plugin_interface_info* info)
{
	info->InterfaceVersion=PLUGIN_I_F_VERSION;
	info->count=1;
}
//Give to the emu pointers for the PowerVR interface
bool EXPORT_CALL dcGetInterface(u32 id,plugin_interface* info)
{
#define c  info->common
#define p info->pvr
	
	c.Type=PowerVR;
	c.InterfaceVersion=PVR_PLUGIN_I_F_VERSION;

	strcpy(c.Name,"drkpvr -- OpenGL/Direct3D/Software PowerVR plugin");
	c.PluginVersion=NDC_MakeVersion(0,9,0);

	c.Load=Load;
	c.Unload=Unload;

	p.ExeptionHanlder=0;
	p.Init=InitPvr;
	p.Reset=ResetPvr;
	p.Term=TermPvr;
	p.ShowConfig=dcShowConfig;

	
	p.ReadReg=ReadPvrRegister;
	p.WriteReg=WritePvrRegister;
	p.UpdatePvr=spgUpdatePvr;

	p.TaFIFO=TASplitter::Dma;
	p.LockedBlockWrite=vramLockCB;
	
#undef c
#undef p
	return true;
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
	char temp[100];
	emu.ConfigLoadStr("drkpvr",key,temp);
	if (strcmp("NONE",temp)==0)
		return def;
	return atoi(temp);
}
