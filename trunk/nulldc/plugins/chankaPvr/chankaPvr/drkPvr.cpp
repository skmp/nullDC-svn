// drkPvr.cpp : Defines the entry point for the DLL application.
//

#include "drkPvr.h"

#include "spg.h"
#include "pvr.h"
#include "taVideo.h"
#include "regs.h"

pvr_init_params param;
emu_info em_inf;

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
unsigned long g_dwCreationWidth=1280;
unsigned long g_dwCreationHeight=1024;


__declspec(align(32)) byte sse_regs[4*8*4];
__declspec(align(32)) u32 mxcsr_reg;
void __declspec(noinline) ncs()
{
		__asm 
	{
		//stmxcsr mxcsr_reg;
		//movaps sse_regs[4*4*0],xmm0;
		//movaps sse_regs[4*4*1],xmm1;
		//movaps sse_regs[4*4*2],xmm2;
		//movaps sse_regs[4*4*3],xmm3;
		//movaps sse_regs[4*4*4],xmm4;
		//movaps sse_regs[4*4*5],xmm5;
		//movaps sse_regs[4*4*6],xmm6;
		movaps sse_regs[4*4*7],xmm7;
	}
}
void __declspec(noinline) SaveSSERegs()
{
	//return;
	//ncs();
	__asm 
	{
		stmxcsr mxcsr_reg;
		movaps sse_regs[4*4*0],xmm0;
		movaps sse_regs[4*4*1],xmm1;
		movaps sse_regs[4*4*2],xmm2;
		movaps sse_regs[4*4*3],xmm3;
		movaps sse_regs[4*4*4],xmm4;
		movaps sse_regs[4*4*5],xmm5;
		movaps sse_regs[4*4*6],xmm6;
		movaps sse_regs[4*4*7],xmm7;
	}
}
void __declspec(noinline) LoadSSERegs()
{
	__asm 
	{
		ldmxcsr mxcsr_reg;
		movaps xmm0,sse_regs[4*4*0];
		movaps xmm1,sse_regs[4*4*1];
		movaps xmm2,sse_regs[4*4*2];
		movaps xmm3,sse_regs[4*4*3];
		movaps xmm4,sse_regs[4*4*4];
		movaps xmm5,sse_regs[4*4*5];
		movaps xmm6,sse_regs[4*4*6];
		movaps xmm7,sse_regs[4*4*7];
	}
}
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
void FASTCALL dcShowConfigDD(void* window)
{
	printf("dcShowConfigDD-> wha ? ");
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

void FASTCALL vramLockCB(vram_block *bl, u32 addr)
{
	InvTexture(bl->userdata);

	param.vram_unlock(bl);
}

void  unlock_vmem(void* block)
{
	param.vram_unlock((vram_block*)block);
}
void* lock_vmem(void* pMem,unsigned __int32 bytes,void* puser)
{
	size_t offset=((u8*)pMem)-param.vram;
	vram_block* pblock =  param.vram_lock_64((u32)offset,(u32)offset + bytes -1,puser);
	return pblock;
}
void FASTCALL TaFIFO(u32 address,u32* data,u32 size)
{
	size*=8;
	for (int i=0;i<size;i+=8)		
		TASendPackedData((DWORD*)&data[i],32);
}

s32 FASTCALL Load(emu_info* inf)
{
	em_inf=*inf;

	Hwnd=em_inf.WindowHandle;
	g_hWnd=(HWND)Hwnd;
	return rv_ok;
}

void FASTCALL Unload()
{
}
//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL InitPvr(pvr_init_params* aparam)
{
	param=*(pvr_init_params*)aparam;

	vram_64=param.vram;
	g_pSH4TextureMemory=(char*)vram_64;

	RaiseInterrupt=param.RaiseInterrupt;
	//g_bChangeDisplayEnable = true;
	//g_bDraw = true;
	TAInit();
	return rv_ok;
}


//It's suposed to reset anything but vram (vram is set to 0 by emu)
void FASTCALL ResetPvr(bool Manual)
{
	Regs_Reset(Manual);
	TAReset();
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void FASTCALL TermPvr()
{
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


void EXPORT_CALL dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion=PLUGIN_I_F_VERSION;
	info->count=1;
}
//Give to the emu pointers for the PowerVR interface
bool EXPORT_CALL dcGetPlugin(u32 id,plugin_info_entry* info)
{
	if (id!=0)
		return false;

#define c info->common
#define p info->pvr
	
	
	c.InterfaceVersion=PVR_PLUGIN_I_F_VERSION;
	c.Type=PowerVR;
	
	strcpy(c.Name,"chanka's video [port by drk||Raziel] (" __DATE__ ")");
	c.PluginVersion=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	
	c.Load=Load;
	c.Unload=Unload;

	
	p.Init=InitPvr;
	p.Term=TermPvr;
	p.Reset=ResetPvr;

	p.ShowConfig=dcShowConfigDD;

	p.UpdatePvr=spgUpdatePvr;
	p.TaFIFO=TaFIFO;
	p.ReadReg=ReadPvrRegister;
	p.WriteReg=WritePvrRegister;
	p.LockedBlockWrite = vramLockCB;
	p.ExeptionHanlder=0;	//we don't use that feature , we'l use default locking ;)

	return true;
}