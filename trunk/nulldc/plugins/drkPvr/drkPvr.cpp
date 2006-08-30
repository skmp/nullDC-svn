// drkPvr.cpp : Defines the entry point for the DLL application.
//

#include "drkPvr.h"

#include "ta.h"
#include "spg.h"
#include "regs.h"
#include "renderer_if.h"

RaiseInterruptFP* RaiseInterrupt;

void* Hwnd;

u8*	vram_64;
vramlock_Lock_32FP* lock32;
vramlock_Lock_64FP* lock64;
vramlock_Unlock_blockFP* unlock;

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

void dcShowConfig(PluginType type,void* window)
{
	printf("No config for now\n");
}
//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,"drkpvr -- OpenGL/Direct3D/Software PowerVR plugin");
	info->PluginVersion.full=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	

	info->Init=dcInitPvr;
	info->Term=dcTermPvr;
	info->Reset=dcResetPvr;

	info->ThreadInit=dcThreadInitPvr;
	info->ThreadTerm=dcThreadTermPvr;

	info->Type=PluginType::PowerVR;
	info->ShowConfig=dcShowConfig;
}

void vramLockCB (vram_block* block,u32 addr)
{
	renderer->VramLockedWrite(block);
}
//Give to the emu pointers for the PowerVR interface
EXPORT void dcGetPvrInfo(pvr_plugin_if* info)
{
	info->InterfaceVersion.full=PVR_PLUGIN_I_F_VERSION;

	info->UpdatePvr=spgUpdatePvr;
	info->TaFIFO=TASplitter::Dma;
	info->ReadReg=ReadPvrRegister;
	info->WriteReg=WritePvrRegister;
	info->LockedBlockWrite=vramLockCB;
}


//called when plugin is used by emu (you should do first time init here)
void dcInitPvr(void* aparam,PluginType type)
{
	pvr_init_params* param=(pvr_init_params*)aparam;
	
	Hwnd=param->WindowHandle;
	
	vram_64=param->vram;

	lock32 = param->vram_lock_32;
	lock64 = param->vram_lock_64;
	unlock = param->vram_unlock;

	RaiseInterrupt=param->RaiseInterrupt;

	SetRenderer(RendererType::Hw_OGL,Hwnd);

	if ((!Regs_Init()))
	{
		//failed
	}
	if (!spg_Init())
	{
		//failed
	}
	if (!rend_if_Init())
	{
		//failed
	}
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void dcTermPvr(PluginType type)
{
	rend_if_Term();
	spg_Term();
	Regs_Term();
}

//It's suposed to reset anything but vram (vram is set to 0 by emu)
void dcResetPvr(bool Manual,PluginType type)
{
	Regs_Reset(Manual);
	spg_Reset(Manual);
	rend_if_Reset(Manual);
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInitPvr(PluginType type)
{
	//olny the renderer cares about thread speciacific shit ..
	rend_if_ThreadInit();
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTermPvr(PluginType type)
{
	//olny the renderer cares about thread speciacific shit ..
	rend_if_ThreadTerm();
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