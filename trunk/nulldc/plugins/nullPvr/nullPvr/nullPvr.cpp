// nullPvr.cpp : Defines the entry point for the DLL application.
//

#include "nullPvr.h"

#include "regs\pvr_mmr.h"
#include "icpvr.h"

RaiseInterruptFP* RaiseInterrupt;

void* Hwnd;

u8*	vram_64;
vramlock_Lock_32FP*			vram_lock_32;
vramlock_Lock_64FP*			vram_lock_64;
vramlock_Unlock_blockFP*	vram_unlock;

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
	printf("drkIIRaziel's nullPvr plugin:No config kthx\n");
	//if (cur_icpl->PvrDllConfig)
		//cur_icpl->PvrDllConfig((HWND)window);
}

//This plugin will actualy Redirect the work on an icarus plugin
HMODULE iclib;

//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,"drkIIRaziel's nullPvr plugin");
	info->PluginVersion.full=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	

	info->Init=dcInitPvr;
	info->Term=dcTermPvr;
	info->Reset=dcResetPvr;

	info->ThreadInit=dcThreadInitPvr;
	info->ThreadTerm=dcThreadTermPvr;

	info->ShowConfig=cfgdlg;
	info->Type=PluginType::PowerVR;
}

//Give to the emu pointers for the PowerVR interface
EXPORT void dcGetPvrInfo(pvr_plugin_if* info)
{
	info->InterfaceVersion.full=PVR_PLUGIN_I_F_VERSION;

	info->UpdatePvr=dcUpdatePvr;
	info->TaFIFO=dcTADma;
	info->ReadReg=ReadPvrRegister;
	info->WriteReg=WritePvrRegister;
	info->LockedBlockWrite=onLockedBlockWrite;
}


//called when plugin is used by emu (you should do first time init here)
void dcInitPvr(void* aparam,PluginType type)
{
	pvr_init_params* param=(pvr_init_params*)aparam;

	vram_64=param->vram;
	Hwnd=param->WindowHandle;
	RaiseInterrupt=param->RaiseInterrupt;
	
	//u32 start_offset64,u32 end_offset64,void* userdata
	param->vram_lock_64(0x000000,0x800000-1,0);

	vram_lock_32=param->vram_lock_64;
	vram_lock_64=param->vram_lock_64;
	vram_unlock=param->vram_unlock;

	LoadIcPvrDll("plugins\\icarus\\gfxEctorD3D.dll");//gfxEctorD3D//IcGfxDX.dll
	icInit();
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void dcTermPvr(PluginType type)
{
	icTerm();
}

//It's suposed to reset anything but vram (vram is set to 0 by emu)
void dcResetPvr(bool Manual,PluginType type)
{
	//hahah do what ? ahahahahahaha
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInitPvr(PluginType type)
{
	//maby init here ?
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTermPvr(PluginType type)
{
	//term here ?
}

//called from sh4 context , should update pvr/ta state and evereything else
void dcUpdatePvr(u32 cycles)
{
	icUpdatePvr(cycles);
}


//called from sh4 context , in case of dma or SQ to TA memory , size is 32 byte transfer counts
void dcTADma(u32 address,u32* data,u32 size)
{
	PvrSQWrite(data,size);
}


u32 ReadPvrRegister(u32 addr,u32 size)
{
	return icReadMem_reg(addr,size);
}

void WritePvrRegister(u32 addr,u32 data,u32 size)
{
	icWriteMem_reg(addr,data,size);
}

void onLockedBlockWrite (vram_block* block,u32 addr)
{
	printf("Locked Block Write %x to %x\n",block->start,block->end);
	vram_unlock(block);
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