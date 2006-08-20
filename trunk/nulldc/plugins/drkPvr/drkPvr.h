#pragma once
//bleh stupid windoze header
#include "..\..\nullDC\plugins\plugin_header.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "helper_classes.h"

#define BUILD 0
#define MINOR 1
#define MAJOR 0
#define DCclock (200*1000*1000)

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


extern u8*	vram_64;
extern void* Hwnd;
extern RaiseInterruptFP* RaiseInterrupt;

float GetSeconds();


#define dbgbreak __asm {int 3}

#define fastcall __fastcall
#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define die(reason) { printf("Fatal error : " #reason "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define fverify verify

#define log(xx) printf(xx " (from "__FUNCTION__ ")\n");
#define log1(xx,yy) printf(xx " (from "__FUNCTION__ ")\n",yy);
#define log2(xx,yy,zz) printf(xx " (from "__FUNCTION__ ")\n",yy,zz);
#define log3(xx,yy,gg) printf(xx " (from "__FUNCTION__ ")\n",yy,zz,gg);

extern bool render_end_pending;
extern u32 render_end_pending_cycles;