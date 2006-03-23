#pragma once
//bleh stupid windoze header
#include "..\..\..\nullDC\plugins\plugin_header.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#define BUILD 0
#define MINOR 0
#define MAJOR 1

#define DCclock (200*1000*1000)

//called when plugin is used by emu (you should do first time init here)
void dcInit(void* param,PluginType type);

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
void dcTerm(PluginType type);

//It's suposed to reset anything 
void dcReset(bool Manual,PluginType type);

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInit(PluginType type);

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTerm(PluginType type);