#pragma once
#define REND_D3D  1
#define REND_OGL  2
#define REND_SW   3

#define REND_API REND_D3D

//bleh stupid windoze header
#include "..\..\nullDC\plugins\plugin_header.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#define BUILD 0
#define MINOR 1
#define MAJOR 0
#define DCclock (200*1000*1000)
/*
//called when plugin is used by emu (you should do first time init here)
void dcInitPvr(void* param,PluginType type);

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void dcTermPvr(PluginType type);

//It's suposed to reset anything but vram (vram is set to 0 by emu , on a non manual reset)
void dcResetPvr(bool Manual,PluginType type);

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInitPvr(PluginType type);

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTermPvr(PluginType type);

//called from sh4 context , should update pvr/ta state and evereything else
void dcUpdatePvr(u32 cycles);

void dcTADma(u32 address,u32* data,u32 size);

u32 ReadPvrRegister(u32 addr,u32 size);
void WritePvrRegister(u32 addr,u32 data,u32 size);

*/
//extern u8*	params.vram;
//extern void* Hwnd;
//extern RaiseInterruptFP* RaiseInterrupt;

float GetSeconds();


#define dbgbreak __asm {int 3}

#define fastcall __fastcall
#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define verifyf(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define verifyc(x) if(FAILED(x)){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define die(reason) { printf("Fatal error : " #reason "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define fverify verify

#define log(xx) printf(xx " (from "__FUNCTION__ ")\n");
#define log1(xx,yy) printf(xx " (from "__FUNCTION__ ")\n",yy);
#define log2(xx,yy,zz) printf(xx " (from "__FUNCTION__ ")\n",yy,zz);
#define log3(xx,yy,gg) printf(xx " (from "__FUNCTION__ ")\n",yy,zz,gg);

#include "helper_classes.h"

extern bool render_end_pending;
extern u32 render_end_pending_cycles;
/*
extern vramlock_Lock_32FP* lock32;
extern vramlock_Lock_64FP* lock64;
extern vramlock_Unlock_blockFP* unlock;*/
extern pvr_init_params params;
extern emu_info emu;
extern char emu_name[512];

void LoadSettings();

#if REND_API == REND_D3D
	#define REND_NAME "Direct3D"
	#define GetRenderer GetDirect3DRenderer
#elif REND_API == REND_OGL
	#define REND_NAME "OpenGL"
	#define GetRenderer GetOpenGLRenderer
#elif  REND_API == REND_SW
	#define REND_NAME "Software TileEmu"
	#define GetRenderer GetNullRenderer
#else
	#error invalid config.REND_API must be set with one of REND_D3D/REND_OGL/REND_SW
#endif

struct _settings_type
{
	struct 
	{
		u32 Enabled;
		u32 Res_X;
		u32 Res_Y;
		u32 Refresh_Rate;
	} Fullscreen;
	struct 
	{
		u32 MultiSampleCount;
		u32 MultiSampleQuality;
	} Enhancements;
};

extern _settings_type settings;