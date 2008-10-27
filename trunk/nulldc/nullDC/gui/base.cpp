#include "types.h"
#include "base.h"
#include "dc/maple/maple_if.h"
#include "dc/dc.h"
#include "plugins/plugin_manager.h"

#include <windows.h>
#include "base.h"
#include "config/config.h"
#include "dc/mem/Elf.h"
#include "dc/gdrom/gdrom_if.h"
#include "profiler/profiler.h"
#include "dc/sh4/rec_v1/blockmanager.h"

//LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
//int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance,LPSTR lpcmdline,int nshowcmd);

//HWND mainhwnd;
//HINSTANCE hinst;

MenuIDList MenuIDs;
cDllHandler gui;
/*

typedef s32 EXPORT_CALL dbgReadMemFP(u32 addr,u32 sz,void* dst);
typedef s32 EXPORT_CALL dbgWriteMemFP(u32 addr,u32 sz,void* dst);
typedef int EXPORT_CALL guiMsgBoxFP(char* text,int type);

typedef bool EXPORT_CALL EmuStartedFP();	//returns if emulation is started.

typedef bool EXPORT_CALL EmuInitFP();

typedef bool EXPORT_CALL EmuStartFP(); //returns false if it failed.It can fail if non inited and implicit init failed.
typedef void EXPORT_CALL EmuStopFP();
typedef void EXPORT_CALL EmuResetFP(bool Manual);	//well duh it resets =P.

typedef bool EXPORT_CALL EmuStepFP(); //returns false if it failed.It can fail if non inited or allready running.
typedef bool EXPORT_CALL EmuSkipFP(); //returns false if it failed.It can fail if non inited or allready running.

typedef void EXPORT_CALL EmuSetPatchFP(u32 Value,u32 Mask);	//Enable/Disable a patch (system area hooks)


typedef bool EXPORT_CALL EmuBootHLEFP();	//Copies the bin file from the disc , descrambles it if needed and sets up needed regs for boot.False if it failed.
typedef bool EXPORT_CALL EmuLoadBinaryFP(char* file,u32 address);	//Loads a binary on the address.If the file is elf the address is ingored and the elf's
														//offsets are used
typedef bool EXPORT_CALL EmuSelectPluginsFP();	//Request the emu to show the select plugins interface
typedef void EXPORT_CALL EmuStartProfilerFP();	//Start the TBP
typedef void EXPORT_CALL EmuStopProfilerFP();	//Stop the TBP

typedef void EXPORT_CALL DissasembleOpcodeFP(u16 opcode,u32 pc,char* Dissasm);
typedef u32 EXPORT_CALL Sh4GetRegisterFP(u32 reg);
typedef void EXPORT_CALL Sh4SetRegisterFP(u32 reg,u32 value);
typedef int EXPORT_CALL GetSymbNameFP(u32 address,char *szDesc,bool bUseUnkAddress);
*/

s32 EXPORT_CALL b_dbgReadMem(u32 addr,u32 sz,void* dst)
{
	u8* ptr=(u8*)dst;
	while(sz--)
		*ptr++=ReadMem8(addr++);
	return rv_ok;
}
s32 EXPORT_CALL b_dbgWriteMem(u32 addr,u32 sz,void* dst)
{
	u8* ptr=(u8*)dst;
	while(sz--)
		WriteMem8(addr++,*ptr++);
	return rv_ok;
}

bool EXPORT_CALL b_EmuStarted()	//returns if emulation is started.
{
	return sh4_cpu?sh4_cpu->IsCpuRunning():false;
}

bool EXPORT_CALL b_EmuInit()
{
	return Init_DC();
}

bool EXPORT_CALL b_EmuStart() //returns false if it failed.It can fail if non inited and implicit init failed.
{
	bool init=Init_DC();
	if (init==false)
		return false;

	Start_DC();
	return true;
}
void EXPORT_CALL b_EmuStop()
{
	Stop_DC();
}
bool EXPORT_CALL b_EmuReset(bool Manual)	//well duh it resets =P.
{
	return Reset_DC(Manual);
}

bool EXPORT_CALL b_EmuStep() //returns false if it failed.It can fail if non inited or allready running.
{
	if(!IsDCInited() || sh4_cpu->IsCpuRunning())
		return false;
	
	sh4_cpu->Step();
	return true;
}
bool EXPORT_CALL b_EmuSkip() //returns false if it failed.It can fail if non inited or allready running.
{
	if(!IsDCInited() || sh4_cpu->IsCpuRunning())
		return false;
	
	sh4_cpu->Skip();
	return true;
}

void EXPORT_CALL b_EmuSetPatch(u32 Value,u32 Mask)	//Enable/Disable a patch (system area hooks)
{
	SetPatches(Value,Mask);
}


bool EXPORT_CALL b_EmuBootHLE()	//Copies the bin file from the disc , descrambles it if needed and sets up needed regs for boot.False if it failed.
{
	return gdBootHLE();
}
bool EXPORT_CALL b_EmuLoadBinary(wchar* file,u32 address)	//Loads a binary on the address.If the file is elf the address is ingored and the elf's
{
	return LoadBinfileToSh4Mem(address,file)!=0;
}													//offsets are used
bool EXPORT_CALL b_EmuSelectPlugins()	//Request the emu to show the select plugins interface
{
	return plugins_Select();
}
void EXPORT_CALL b_EmuStartProfiler()	//Start the TBP
{
	start_Profiler();
}
void EXPORT_CALL b_EmuStopProfiler()	//Stop the TBP
{
	stop_Profiler();
}

void EXPORT_CALL b_DissasembleOpcode(u16 opcode,u32 pc,wchar* Dissasm)
{
	char temp[2048];
	DissasembleOpcode(opcode,pc,temp);
	mbstowcs(Dissasm,temp,2048);
}
u32 EXPORT_CALL b_Sh4GetRegister(u32 reg)
{
	return sh4_cpu->GetRegister((Sh4RegType)reg);
}
void EXPORT_CALL b_Sh4SetRegister(u32 reg,u32 value)
{
	sh4_cpu->SetRegister((Sh4RegType)reg,value);
}
int EXPORT_CALL b_GetSymbName(u32 address,wchar *szDesc,bool bUseUnkAddress)
{
	char temp[2048];
	int rv=GetSymbName(address,temp,bUseUnkAddress);
	mbstowcs(szDesc,temp,2048);
	return rv;
}

#define maple_sett(x,y) _T("Current_maple") _T(#x) _T("_") _T(#y)
#define maple_groop(x) maple_sett(x,0), maple_sett(x,1), maple_sett(x,2), maple_sett(x,3), maple_sett(x,4), maple_sett(x,5)

wchar* ndc_snames[]=
{
	_T("Current_PVR"),
	_T("Current_GDR"),
	_T("Current_AICA"),
	_T("Current_ExtDevice"),

	maple_groop(0),
	maple_groop(1),
	maple_groop(2),
	maple_groop(3),
};

#undef maple_sett
#undef maple_groop

#define _esai_(n,v,ma) 		case NDCS_##n:*tptr=settings.##v; break;
void SwitchCpu()
{
	if((settings.dynarec.Enable==0 && sh4_cpu->ResetCache==0) ||
	   (settings.dynarec.Enable!=0 && sh4_cpu->ResetCache!=0)  )
	{
		return;//nothing to do ...
	}
	
	bool bStart=false;

	if (sh4_cpu)
	{
		if (sh4_cpu->IsCpuRunning())
		{
			bStart=true;
			Stop_DC();
		}
		sh4_cpu->Term();
	}
	if(settings.dynarec.Enable)
	{
		sh4_cpu=Get_Sh4Recompiler();
		printf("Switched to Recompiler\n");
	}
	else
	{
		sh4_cpu=Get_Sh4Interpreter();
		printf("Switched to Interpreter\n");
	}

	sh4_cpu->Init();
	

	if (bStart)
		Start_DC();
}
s32 EXPORT_CALL GetEmuSetting(u32 sid,void* value)
{
	wchar* sptr=(wchar*)value;
	u32* tptr=(u32*)value;
	switch(sid)
	{
		
	_esai_(DYNAREC_ENABLED,dynarec.Enable,1);
	_esai_(DYNAREC_CPPASS,dynarec.CPpass,1);
	_esai_(DYNAREC_UCFPU,dynarec.UnderclockFpu,1);

	_esai_(DREAMCAST_CABLE,dreamcast.cable,3);
	_esai_(DREAMCAST_RTC,dreamcast.RTC,0xFFFFFFFF);

	_esai_(EMULATOR_ASTART,emulator.AutoStart,1);

	default:
		if (sid<NDCS_COUNT)
		{
			cfgLoadStr(_T("nullDC_plugins"),ndc_snames[sid-NDCS_PLUGIN_PVR],sptr,0);
			return rv_ok;
		}
		else
			return rv_error;
	}

	return rv_ok;
}
#undef _esai_

#define _esai_(n,v,ma) 		case NDCS_##n:settings.##v=min(*tptr,ma);SaveSettings();
s32 EXPORT_CALL SetEmuSetting(u32 sid,void* value)
{
	wchar* sptr=(wchar*)value;
	u32* tptr=(u32*)value;
	
	switch(sid)
	{	
		_esai_(DYNAREC_ENABLED,dynarec.Enable,1);
		SwitchCpu();//switch cpu if needed
		break;
		
		_esai_(DYNAREC_CPPASS,dynarec.CPpass,1);
		if (sh4_cpu->ResetCache)
			sh4_cpu->ResetCache();
		break;
		
		_esai_(DYNAREC_UCFPU,dynarec.UnderclockFpu,1);
		if (sh4_cpu->ResetCache)
			sh4_cpu->ResetCache();
		break;

		_esai_(DREAMCAST_CABLE,dreamcast.cable,3);
		break;

		_esai_(DREAMCAST_RTC,dreamcast.RTC,0xFFFFFFFF);
		break;

		_esai_(EMULATOR_ASTART,emulator.AutoStart,1);
		break;

	default:
		if (sid<NDCS_COUNT)
		{
			cfgSaveStr(_T("nullDC_plugins"),ndc_snames[sid-NDCS_PLUGIN_PVR],sptr);
			return rv_ok;
		}
		else
			return rv_error;
	}

	return rv_ok;
}
#define nlw _T("\r\n")
wchar about_text[] =
				_T("Credits :") nlw
				_T(" drk||Raziel \t: Main coder") nlw
				_T(" ZeZu \t\t: Main coder") nlw
				_T(" GiGaHeRz \t: Plugin work/misc stuff") nlw
				_T(" PsyMan \t\t: Mental support, managment,") nlw
				_T("        \t\t  beta testing & everything else") nlw
				_T(" General Plot \t\t: www & forum WIP") nlw
				nlw
				_T("Beta testing :") nlw
				_T(" emwearz, Miretank, gb_away, Raziel, General Plot,") nlw
				_T(" Refraction, Ckemu,Falcon4ever, ChaosCode") nlw
				nlw
				_T("Many thanks to :") nlw
				_T("Ector, Jim Denson, Flea, Jupi, Chankast team, lev|") nlw
				_T("and everyone else we forgot") nlw
				nlw
				_T("Hate list:") nlw
				_T("Xylene, for trying to mess channels by impersonating people...") nlw
				_T(" dont worry, you'l never get as good as they are.") nlw
				nlw
				;
wchar* EXPORT_CALL GetAboutText()
{
	return about_text;
}
PluginInfoList* EXPORT_CALL b_GetPluginList(u32 Type)
{
	List<PluginLoadInfo>* lst= GetPluginList((PluginType)Type);

	PluginInfoList* rt=0,*rc=0,*rv=0;
	for (u32 i=0;i<lst->itemcount;i++)
	{
		rc = new PluginInfoList();
		
		wcscpy(rc->Name,(*lst)[i].Name);
		wcscpy(rc->dll,(*lst)[i].dll);

		if(rt)
			rt->next=rc;
		else
			rv=rc;

		rt=rc;
	}

	delete lst;

	return rv;
}
PluginInfoList* EXPORT_CALL b_GetMapleDeviceList(u32 DeviceType)
{
	List<MapleDeviceDefinition>* lst= GetMapleDeviceList((MapleDeviceType)DeviceType);

	PluginInfoList* rt=0,*rc=0,*rv=0;
	for (u32 i=0;i<lst->itemcount;i++)
	{
		rc = new PluginInfoList();
		
		wcscpy(rc->Name,(*lst)[i].Name);
		wcscpy(rc->dll,(*lst)[i].dll);
		rc->Flags=(*lst)[i].Flags;

		if(rt)
			rt->next=rc;
		else
			rv=rc;

		rt=rc;
	}

	delete lst;

	return rv;
}
s32 EXPORT_CALL b_FreePluginList(PluginInfoList* list)
{
	if (!list)
		return rv_error;
	while(list)
	{
		PluginInfoList* next=list->next;
		delete list;
		list=next;
	}
	return rv_ok;
}
void EXPORT_CALL b_EmuResetCaches()
{
	if (sh4_cpu && sh4_cpu->ResetCache)
		sh4_cpu->ResetCache();
}
void EXPORT_CALL b_GetPerformanceInfo(nullDCPerfomanceInfo* dst)
{
	bm_stats bms;
	bm_GetStats(&bms);
	prof_info tbpi(profile_info);

	if (TBP_Enabled)
	{
		dst->TBP.Valid=1;
		dst->TBP.Ticks=tbpi.total_tick_count;

		dst->TBP.PowerVR=tbpi.gfx_tick_count;
		dst->TBP.AICA=tbpi.aica_tick_count;
		dst->TBP.GDRom=tbpi.gdrom_tick_count;
		dst->TBP.Maple=tbpi.maple_tick_count;
		dst->TBP.Main=tbpi.main_tick_count;
		dst->TBP.Dyna=tbpi.dyna_tick_count;
		dst->TBP.Rest=tbpi.rest_tick_count;
	}
	else
	{
		dst->TBP.Valid=0;
	}
	
	dst->Dynarec.CodeGen.CodeSize=bms.cache_size;
	dst->Dynarec.CodeGen.LockedBlocks=bms.locked_blocks;
	dst->Dynarec.CodeGen.ManualBlocks=bms.manual_blocks;
	dst->Dynarec.CodeGen.TotalBlocks=bms.block_count;

	#ifdef _BM_CACHE_STATS
		dst->Dynarec.Runtime.Lookups.Valid=1;
		dst->Dynarec.Runtime.Lookups.FastLookupDelta=bms.fast_lookups;
		dst->Dynarec.Runtime.Lookups.FullLookupDelta=bms.full_lookups;
		dst->Dynarec.Runtime.Lookups.LookupDelta=bms.full_lookups+bms.fast_lookups;
	#else
		dst->Dynarec.Runtime.Lookups.Valid=0;
	#endif

	#ifdef COUNT_BLOCK_LOCKTYPE_USAGE
		dst->Dynarec.Runtime.Execution.Valid=1;
		dst->Dynarec.Runtime.Execution.LockedBlocks=bms.locked_block_calls_delta;
		dst->Dynarec.Runtime.Execution.ManualBlocks=bms.manual_block_calls_delta;
		dst->Dynarec.Runtime.Execution.TotalBlocks=bms.locked_block_calls_delta+bms.manual_block_calls_delta;
	#else
		dst->Dynarec.Runtime.Execution.Valid=0;
	#endif
	
	#ifdef RET_CACHE_PROF
		dst->Dynarec.Runtime.RetCache.Valid=1;
		dst->Dynarec.Runtime.RetCache.Hits=ret_cache_hits;
		dst->Dynarec.Runtime.RetCache.Count=ret_cache_total;
		dst->Dynarec.Runtime.RetCache.Misses=ret_cache_total-ret_cache_hits;
		ret_cache_hits=ret_cache_total=0;
	#else
		dst->Dynarec.Runtime.RetCache.Valid=0;
	#endif

}


bool OpenAndLoadGUI(wchar* file)
{
	if (!gui.Load(file))
	{
		msgboxf(_T("Unable to open gui dll (%s)"),MBX_ICONERROR,file);
		return false;
	}

	ndcGetInterfaceFP* gi=(ndcGetInterfaceFP*)gui.GetProcAddress("ndcGetInterface");

	if (!gi)
	{
		msgboxf(_T("Unable to resolve %s:ndcGetInterface"),MBX_ICONERROR,file);
		return false;
	}
	gi(&libgui);

	gui_emu_info gpi;

	gpi.ConfigExists=cfgExists;
	gpi.ConfigLoadInt=cfgLoadInt;
	gpi.ConfigLoadStr=cfgLoadStr;
	gpi.ConfigSaveInt=cfgSaveInt;
	gpi.ConfigSaveStr=cfgSaveStr;

	gpi.dbgReadMem=b_dbgReadMem;
	gpi.dbgWriteMem=b_dbgWriteMem;

	gpi.EmuStarted=b_EmuStarted;

	gpi.EmuInit = b_EmuInit ;

	gpi.EmuStart = b_EmuStart ;
	gpi.EmuStop = b_EmuStop ;
	gpi.EmuStep = b_EmuStep ;
	gpi.EmuSkip = b_EmuSkip ;

	gpi.EmuSetPatch = b_EmuSetPatch ;
	gpi.EmuReset = b_EmuReset ;
	gpi.EmuResetCaches=b_EmuResetCaches;

	gpi.EmuBootHLE = b_EmuBootHLE ;
	gpi.EmuLoadBinary = b_EmuLoadBinary ;

	gpi.EmuSelectPlugins = b_EmuSelectPlugins ;
	gpi.EmuStartProfiler = b_EmuStartProfiler ;
	gpi.EmuStopProfiler = b_EmuStopProfiler ;

	gpi.DissasembleOpcode = b_DissasembleOpcode ;
	gpi.Sh4GetRegister = b_Sh4GetRegister ;
	gpi.Sh4SetRegister = b_Sh4SetRegister ;
	gpi.GetSymbName = b_GetSymbName ;
	
	gpi.GetSetting=GetEmuSetting;
	gpi.SetSetting=SetEmuSetting;
	gpi.GetAboutText=GetAboutText;
	
	
	gpi.GetPluginList=b_GetPluginList;
	gpi.GetMapleDeviceList=b_GetMapleDeviceList;
	gpi.FreePluginList=b_FreePluginList;

	gpi.GetPerformanceInfo=b_GetPerformanceInfo;

	gpi.BroardcastEvent=BroadcastEvent;
	gpi.EmuThread=hEmuThread;

	if (rv_ok != libgui.Load(&gpi))
		return false;
	
	libgui.GetMenuIDs(&MenuIDs);

	return true;
}
bool CreateGUI()
{
	wchar gui[128];
	cfgLoadStr(_T("nullDC_plugins"),_T("GUI"),gui,_T("nullDC_GUI_Win32.dll"));
	if (!OpenAndLoadGUI(gui))
	{
		if (msgboxf(_T("Do you want to load default gui ?"),MBX_YESNO) == MBX_RV_YES)
		{
			if (!OpenAndLoadGUI(_T("nullDC_GUI_Win32.dll")))
				return false;
			else
				return true;
		}
		else
			return false;
	}
	else
		return true;
}
void DestroyGUI()
{
	plugins_Unload();
	
	libgui.Unload();
	gui.Unload();
}
void GuiLoop()
{
	libgui.Mainloop();

	//Make sure emulation is stoped
	Stop_DC();
}

void* EXPORT_CALL GetRenderTargetHandle()
{
	return libgui.GetRenderTarget();
}
bool SelectPluginsGui()
{
	return libgui.SelectPluginsGui();
}

void EmuEventBroadcast()
{
}

u32 PowerVR_menu;
u32 GDRom_menu;
u32 Aica_menu;
u32 Maple_menu;
u32 Maple_menu_ports[4][6];
u32 ExtDev_menu;
u32 Debug_menu;


void SetMenuItemHandler(u32 id,MenuItemSelectedFP* h)
{
	MenuItem mi;
	mi.Handler=h;
	
	libgui.SetMenuItem(id,&mi,MIM_Handler);
}

enum ProfilerInfoFlags
{
	PIF_TBP_ON=1,		//Time based profiler is enabled (so the info returned here is valid)
	PIF_TBP_ONEX=2,		//Extra profile info is valid (block usage)

	PIF_BMS_DTCS=4	,	//Dynarec Translation Cache stats (size;block counts)
	PIF_BMS_LCS=8	,	//lookup cache stats (miss/hit)
	PIF_BMS_CSGS=16	,	//call stack guess stats (miss/hit)
	
};
struct profiler_info
{
	u32 flags;
};