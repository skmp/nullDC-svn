// drkPvr.cpp : Defines the entry point for the DLL application.
//

#include "chankaPvr.h"

#include "spg.h"
#include "pvr.h"
#include "taVideo.h"
#include "regs.h"

pvr_init_params param;
emu_info em_inf;

//RaiseInterruptFP* RaiseInterrupt;

int CurrentFrame=0;
void* Hwnd;
char emu_name[256];
u8*	vram_64;
bool g_bWireframe=false;
char* g_pSH4TextureMemory;
int g_bCreationFullScreen=0;
HWND g_hWnd;
unsigned long g_framesLatency=0;
bool g_bShowStats=true;
bool g_bForceSVP=true;
unsigned long g_dwCreationWidth=1280;
unsigned long g_dwCreationHeight=1024;
int g_iMultiSampleQuality;
int g_iMultiSampleCount;

void cfgSetInt(char* key,int v)
{
	wchar t[512];
	mbstowcs(t,key,512);
	em_inf.ConfigSaveInt(L"chankast_pvr",t,v);
}
int cfgGetInt(char* key,int def)
{
	wchar t[512];
	mbstowcs(t,key,512);
	return em_inf.ConfigLoadInt(L"chankast_pvr",t,def);
}
extern bool g_bForceSVP;
void LoadSettings()
{
	g_bUSE_ZWRITE           = cfgGetInt("Use_ZWrite",1)!=0;
    g_bUSE_ALPHATEST_ZWRITE = cfgGetInt("Use_AlphaTest_ZWrite",1)!=0;

	g_bShowStats=cfgGetInt("ShowStats",0)!=0;
	g_bWireframe=cfgGetInt("Wireframe",0)!=0;

	g_dwCreationWidth=cfgGetInt("Width",640);
	g_dwCreationHeight=cfgGetInt("Height",480);
	g_bForceSVP=cfgGetInt("ForceSoftwareVertexProceccing",0)!=0;
	g_iMultiSampleCount=cfgGetInt("MultiSampleCount",0);
	g_iMultiSampleQuality=cfgGetInt("MultiSampleQuality",0);
	g_bCreationFullScreen=cfgGetInt("FullScreen",0);	
}

void SaveSettings()
{
	cfgSetInt("Use_ZWrite",g_bUSE_ZWRITE);
	cfgSetInt("Use_AlphaTest_ZWrite",g_bUSE_ALPHATEST_ZWRITE);

	cfgSetInt("ShowStats",g_bShowStats);
	cfgSetInt("Wireframe",g_bWireframe);

	cfgSetInt("Width",g_dwCreationWidth);
	cfgSetInt("Height",g_dwCreationHeight);
	cfgSetInt("ForceSoftwareVertexProceccing",g_bForceSVP);
	cfgSetInt("MultiSampleCount",g_iMultiSampleCount);
	cfgSetInt("MultiSampleQuality",g_iMultiSampleQuality);
	cfgSetInt("FullScreen",g_bCreationFullScreen);	
}


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
void FASTCALL TaDMA(u32* data,u32 size)
{
	size*=8;
	for (u32 i=0;i<size;i+=8)		
		TASendPackedData((DWORD*)&data[i],32);
}
void FASTCALL TaSQ(u32* data)
{	
	TASendPackedData((DWORD*)data,32);
}
void EXPORT_CALL handle_About(u32 id,void* w,void* p)
{
	MessageBox((HWND)w,"Chankast PowerVR core , made by the chankast team\nPort by drk||Raziel","About ChankaPvr...",MB_ICONINFORMATION);
}

template<bool* stuff>
void EXPORT_CALL handle_TCH(u32 id,void* w,void* p)
{
	if (*stuff)
		*stuff=0;
	else
		*stuff=1;

	em_inf.SetMenuItemStyle(id,*stuff?MIS_Checked:0,MIS_Checked);
	SaveSettings();
}

s32 FASTCALL Load(emu_info* inf)
{
	wchar ename[512];
	u32 rmenu=inf->RootMenu;
	em_inf=*inf;
	em_inf.ConfigLoadStr(L"emu",L"shortname",ename,0);
	wcstombs(emu_name,ename,512);
	Hwnd=em_inf.GetRenderTarget();
	g_hWnd=(HWND)Hwnd;
	LoadSettings();

	u32 fs_menu=em_inf.AddMenuItem(rmenu,-1,L"Fullscreen",0,0);
	em_inf.AddMenuItem(fs_menu,-1,L"Enable",handle_TCH<(bool*)&g_bCreationFullScreen>,g_bCreationFullScreen);
	em_inf.AddMenuItem(fs_menu,-1,0,0,0);
	em_inf.AddMenuItem(fs_menu,-1,L"640x480",0,0);

	em_inf.AddMenuItem(rmenu,-1,L"Use ZWrite",handle_TCH<&g_bUSE_ZWRITE>,g_bUSE_ZWRITE);
	em_inf.AddMenuItem(rmenu,-1,L"Use Alpha Test ZWrite",handle_TCH<&g_bUSE_ALPHATEST_ZWRITE>,g_bUSE_ALPHATEST_ZWRITE);
	em_inf.AddMenuItem(rmenu,-1,L"Wireframe",handle_TCH<&g_bWireframe>,g_bWireframe);
	em_inf.AddMenuItem(rmenu,-1,L"Show Stats",handle_TCH<&g_bShowStats>,g_bShowStats);

	em_inf.AddMenuItem(rmenu,-1,0,0,0);

	em_inf.AddMenuItem(rmenu,-1,L"About",handle_About,0);

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
	//g_bChangeDisplayEnable = true;
	//g_bDraw = true;
	LoadSettings();
	Regs_Init();
	spg_Init();
	TAInit();
	return rv_ok;
}


//It's suposed to reset anything but vram (vram is set to 0 by emu)
void FASTCALL ResetPvr(bool Manual)
{
	Regs_Reset(Manual);
	spg_Reset(Manual);
	TAReset();
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void FASTCALL TermPvr()
{
	Regs_Term();
	spg_Term();
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

//Give to the emu pointers for the PowerVR interface
void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
	info->InterfaceVersion=PLUGIN_I_F_VERSION;

#define c info->common
#define p info->pvr
	
	
	c.InterfaceVersion=PVR_PLUGIN_I_F_VERSION;
	c.Type=Plugin_PowerVR;
	
	char namet[512];
	strcpy(namet,"Chankast's video(" __DATE__ ")");
	mbstowcs(c.Name,namet,512);
	c.PluginVersion=DC_MakeVersion(MAJOR,MINOR,BUILD);
	
	c.Load=Load;
	c.Unload=Unload;

	
	p.Init=InitPvr;
	p.Term=TermPvr;
	p.Reset=ResetPvr;

	p.UpdatePvr=spgUpdatePvr;
	p.TaDMA=TaDMA;
	p.TaSQ=TaSQ;
	p.ReadReg=ReadPvrRegister;
	p.WriteReg=WritePvrRegister;
	p.LockedBlockWrite = vramLockCB;
	p.ExeptionHanlder=0;	//we don't use that feature , we'l use default locking ;)
}