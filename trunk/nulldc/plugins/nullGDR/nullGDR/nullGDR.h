#pragma once
//bleh stupid windoze header
#include "..\..\..\nullDC\plugins\plugin_header.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#define BUILD 0
#define MINOR 0
#define MAJOR 1

void DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
void DriveGetTocInfo(u32* toc,DiskArea area);
DiskType DriveGetDiskType();

//called when plugin is used by emu (you should do first time init here)
void dcInitGDR(void* param,PluginType type);

//called when plugin is unloaded by emu , olny if dcInitGDR is called (eg , not called to enumerate plugins)
void dcTermGDR(PluginType type);

//It's suposed to reset everything (if not a manual reset)
void dcResetGDR(bool Manual,PluginType type);

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInitGDR(PluginType type);

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTermGDR(PluginType type);

extern DriveNotifyEventFP* DriveNotifyEvent;


#pragma pack(1)
	struct TDriveTOC
	{		
		DWORD	entry[99];
		DWORD	first, last;
		DWORD	leadout_sector;		
	};

	struct TInfoSession
	{
		int iNumSessions;
		DWORD uLeadOut;
		DWORD aTrackStart[100];
	};
#pragma pack()

void uiGetFN(TCHAR *szFileName, TCHAR *szParse);