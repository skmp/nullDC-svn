#pragma once
//bleh stupid windoze header
#include "../../../nulldc/plugins/plugin_header.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#define BUILD 5
#define MINOR 2
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

u32 FASTCALL ReadPvrRegister(u32 addr,u32 size);
void FASTCALL WritePvrRegister(u32 addr,u32 data,u32 size);


extern u8*	vram_64;
extern void* Hwnd;
//extern RaiseInterruptFP* RaiseInterrupt;

float GetSeconds();

extern pvr_init_params param;
#define params param
extern emu_info em_inf;
extern char emu_name[256];

extern bool g_bForceSVP;
extern int g_iMultiSampleQuality;
extern int g_iMultiSampleCount;