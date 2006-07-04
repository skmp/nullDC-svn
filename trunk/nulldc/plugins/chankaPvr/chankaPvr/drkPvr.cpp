// drkPvr.cpp : Defines the entry point for the DLL application.
//

#include "drkPvr.h"

#include "spg.h"
#include "pvr.h"
#include "taVideo.h"
#include "regs.h"

pvr_init_params param;

RaiseInterruptFP* RaiseInterrupt;

int CurrentFrame=0;
void* Hwnd;

u8*	vram_64;
bool g_bWireframe=false;
char* g_pSH4TextureMemory;
int g_bCreationFullScreen=0;
HWND g_hWnd;
unsigned long g_framesLatency=0;
bool g_bShowStats=true;
unsigned long g_dwCreationWidth=640;
unsigned long g_dwCreationHeight=480;


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

//Give to the emu info for the plugin type
void dcShowConfigDD(PluginType type,void* window)
{
	printf("dcShowConfigDD-> wha ? ");
}
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,"chanka video [port by drkIIraziel] (" __DATE__ ")");
	info->PluginVersion.full=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	

	info->Init=dcInitPvr;
	info->Term=dcTermPvr;
	info->Reset=dcResetPvr;

	info->ThreadInit=dcThreadInitPvr;
	info->ThreadTerm=dcThreadTermPvr;

	info->Type=PluginType::PowerVR;
	info->ShowConfig=dcShowConfigDD;
}
/*

  void TASendPackedData(DWORD* pBuffer, DWORD uLength);

  -void TAInit   ();
  -void TAReset  ();
  void TASetVideoMode(DWORD uWidth,DWORD uHeight, BOOL bFullScreen = false);
 + void TANotifyDisplayAddressChange();
 - void TAStartVBlank();
 + void TAContinueRegistration();
 + void TAResetRegistration();
 + void TAStartRegistration();
 + void TAStartRender();
 - void TAEnd();
 - bool TAIsOldGfx();

  extern  DWORD m_uNumVerticesRegistered;
  extern  DWORD m_uNumPrimitivesRegistered;
  extern  bool  g_bDraw;
  extern  bool  g_bChangeDisplayEnable;

*/

void vramLockCB(vram_block *bl, u32 addr)
{
	InvTexture(bl->userdata);

	param.vram_unlock(bl);
}

void lock_vmem(void* pMem,unsigned __int32 bytes,void* puser)
{
	size_t offset=((u8*)pMem)-param.vram;
	vram_block* pblock =  param.vram_lock_64((u32)offset,(u32)offset + bytes -1,puser);
}
void TaFIFO(u32 address,u32* data,u32 size)
{
	size*=8;
	for (int i=0;i<size;i+=8)		
		TASendPackedData((DWORD*)&data[i],32);
}
//Give to the emu pointers for the PowerVR interface
EXPORT void dcGetPvrInfo(pvr_plugin_if* info)
{
	info->InterfaceVersion.full=PVR_PLUGIN_I_F_VERSION;

	info->UpdatePvr=spgUpdatePvr;
	info->TaFIFO=TaFIFO;
	info->ReadReg=ReadPvrRegister;
	info->WriteReg=WritePvrRegister;
	info->LockedBlockWrite = vramLockCB;
}


//called when plugin is used by emu (you should do first time init here)
void dcInitPvr(void* aparam,PluginType type)
{
	param=*(pvr_init_params*)aparam;

	vram_64=param.vram;
	g_pSH4TextureMemory=(char*)vram_64;
	Hwnd=param.WindowHandle;
	g_hWnd=(HWND)Hwnd;
	RaiseInterrupt=param.RaiseInterrupt;
	//param->vram_lock_64
	//g_bChangeDisplayEnable = true;
	//g_bDraw = true;
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void dcTermPvr(PluginType type)
{

}

//It's suposed to reset anything but vram (vram is set to 0 by emu)
void dcResetPvr(bool Manual,PluginType type)
{
	Regs_Reset(Manual);
	TAReset();
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInitPvr(PluginType type)
{
	//olny the renderer cares about thread speciacific shit ..
	TAInit();
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTermPvr(PluginType type)
{
	//olny the renderer cares about thread speciacific shit ..
	TAEnd();
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

