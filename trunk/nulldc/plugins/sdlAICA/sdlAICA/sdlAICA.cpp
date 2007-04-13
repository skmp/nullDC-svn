// nullAICA.cpp : Defines the entry point for the DLL application.
//

#include "sdlAICA.h"
#include "sgc_if.h"
#include "aica.h"
#include "arm7.h"
#include "mem.h"
#include "audiostream.h"
#include "resource.h"
#include <windowsx.h>

setts settings;
aica_init_params aica_params;
emu_info eminf;
HINSTANCE hinst;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	hinst=(HINSTANCE)hModule;
    return TRUE;
}
void UpdateMenuSelections();
u32 scmi=0,stami=0;
INT_PTR CALLBACK ConfigDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			CheckDlgButton(hWnd,IDC_HWMIX,settings.HW_mixing!=0?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hWnd,IDC_GFOCUS,settings.GlobalFocus!=0?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hWnd,IDC_FSYNC,settings.LimitFPS!=0?BST_CHECKED:BST_UNCHECKED);

			bool hideo=true;
			if (settings.BufferSize==884)
			{
				CheckDlgButton(hWnd,IDC_RADIO1,BST_CHECKED);
			}
			else if (settings.BufferSize==1024)
			{
				CheckDlgButton(hWnd,IDC_RADIO2,BST_CHECKED);
			}
			else if (settings.BufferSize==1536)
			{
				CheckDlgButton(hWnd,IDC_RADIO3,BST_CHECKED);
			}
			else if (settings.BufferSize==1768)
			{
				CheckDlgButton(hWnd,IDC_RADIO4,BST_CHECKED);
			}
			else if (settings.BufferSize==2048)
			{
				CheckDlgButton(hWnd,IDC_RADIO5,BST_CHECKED);
			}
			else if (settings.BufferSize==4096)
			{
				CheckDlgButton(hWnd,IDC_RADIO6,BST_CHECKED);
			}
			else
			{
				CheckDlgButton(hWnd,IDC_RADIO7,BST_CHECKED);
				char temp[512];
				sprintf(temp,"%d",settings.BufferSize);
				SetDlgItemTextA(hWnd,IDC_RADIO7,temp);
				hideo=false;
			}
			if (hideo)
				ShowWindow(GetDlgItem(hWnd,IDC_RADIO7), SW_HIDE);
		}
		return true;

	case WM_COMMAND:

		switch( LOWORD(wParam) )
		{
		case IDOK:
			{
				if (IsDlgButtonChecked(hWnd,IDC_RADIO1))
				{
					settings.BufferSize==884;
				}
				else if (IsDlgButtonChecked(hWnd,IDC_RADIO2))
				{
					settings.BufferSize=1024;
				}
				else if (IsDlgButtonChecked(hWnd,IDC_RADIO3))
				{
					settings.BufferSize=1536;
				}
				else if (IsDlgButtonChecked(hWnd,IDC_RADIO4))
				{
					settings.BufferSize=1768;
				}
				else if (IsDlgButtonChecked(hWnd,IDC_RADIO5))
				{
					settings.BufferSize=2048;
				}
				else if (IsDlgButtonChecked(hWnd,IDC_RADIO6))
				{
					settings.BufferSize=4096;
				}

				settings.HW_mixing=IsDlgButtonChecked(hWnd,IDC_HWMIX);
				settings.GlobalFocus=IsDlgButtonChecked(hWnd,IDC_GFOCUS);
				settings.LimitFPS=IsDlgButtonChecked(hWnd,IDC_FSYNC);
				
				SaveSettings();
			}
		case IDCANCEL:
			EndDialog(hWnd,0);
			return true;

		default: break;
		}
		return false;

	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hWnd,0);
		return true;

	default: break;
	}

	return false;
}


void FASTCALL handle_About(u32 id,void* w,void* p)
{
	MessageBoxA((HWND)w,"Made by the nullDC Team","About nullAica...",MB_ICONINFORMATION);
}
void FASTCALL handle_Config(u32 id,void* w,void* p)
{
	DialogBox(hinst,MAKEINTRESOURCE(IDD_SETTINGS),(HWND)w,ConfigDlgProc);
}
void FASTCALL handle_SA(u32 id,void* w,void* p)
{
	if (settings.LimitFPS)
		settings.LimitFPS=0;
	else
		settings.LimitFPS=1;

	eminf.SetMenuItemStyle(id,settings.LimitFPS?MIS_Checked:0,MIS_Checked);
	SaveSettings();
}
void FASTCALL handle_MCDDA(u32 id,void* w,void* p)
{
	if (settings.CDDAMute)
		settings.CDDAMute=0;
	else
		settings.CDDAMute=1;

	eminf.SetMenuItemStyle(id,settings.CDDAMute?MIS_Checked:0,MIS_Checked);
	SaveSettings();
}
void FASTCALL handle_GS(u32 id,void* w,void* p)
{
	if (settings.GlobalMute)
		settings.GlobalMute=0;
	else
		settings.GlobalMute=1;

	eminf.SetMenuItemStyle(id,settings.GlobalMute?MIS_Checked:0,MIS_Checked);
	SaveSettings();
}
void UpdateMenuSelections()
{
	eminf.SetMenuItemStyle(stami,settings.LimitFPS?MIS_Checked:0,MIS_Checked);
}
s32 FASTCALL OnLoad(emu_info* em,u32 rmenu)
{
	memcpy(&eminf,em,sizeof(eminf));

	LoadSettings();

	scmi=eminf.AddMenuItem(rmenu,-1,"Config",handle_Config,0);
	stami=eminf.AddMenuItem(rmenu,-1,"Sync Audio",handle_SA,settings.LimitFPS);
	eminf.AddMenuItem(rmenu,-1,"Mute CDDA",handle_MCDDA,settings.CDDAMute);
	eminf.AddMenuItem(rmenu,-1,"Mute Sound",handle_GS,settings.GlobalMute);

	eminf.SetMenuItemStyle(eminf.AddMenuItem(rmenu,-1,"-",0,0),MIS_Seperator,MIS_Seperator);
	eminf.AddMenuItem(rmenu,-1,"About",handle_About,0);

	return rv_ok;
}

void FASTCALL OnUnload()
{
}

//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Init(aica_init_params* initp)
{
	memcpy(&aica_params,initp,sizeof(aica_params));

	init_mem();
	arm_Init();
	AICA_Init();
	InitAudio();

	eminf.SetMenuItemStyle(scmi,MIS_Grayed,MIS_Grayed);
	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
void FASTCALL Term()
{
	TermAudio();
	AICA_Term();
	term_mem();

	eminf.SetMenuItemStyle(scmi,0,MIS_Grayed);
}

//It's suposed to reset anything 
void FASTCALL Reset(bool Manual)
{
	arm_Reset();
}


//Give to the emu pointers for the PowerVR interface
EXPORT void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
	info->InterfaceVersion=PLUGIN_I_F_VERSION;
/*
	info->Init=dcInit;
	info->Term=dcTerm;
	info->Reset=dcReset;

	info->ThreadInit=dcThreadInit;
	info->ThreadTerm=dcThreadTerm;
	info->ShowConfig=cfgdlg;

	info->Type=PluginType::AICA;

	info->InterfaceVersion.full=AICA_PLUGIN_I_F_VERSION;

	info->ReadMem_aica_ram=ReadMem_ram;
	info->WriteMem_aica_ram=WriteMem_ram;
	info->ReadMem_aica_reg=ReadMem_reg;
	info->WriteMem_aica_reg=WriteMem_reg;
	info->UpdateAICA=UpdateSystem;
*/
#define c info->common
#define a info->aica

	strcpy(c.Name,"nullAICA , built :" __DATE__ "");
	c.PluginVersion=DC_MakeVersion(MAJOR,MINOR,BUILD,DC_VER_NORMAL);

	c.InterfaceVersion=AICA_PLUGIN_I_F_VERSION;
	c.Type=Plugin_AICA;

	c.Load=OnLoad;
	c.Unload=OnUnload;

	a.Init=Init;
	a.Reset=Reset;
	a.Term=Term;

	a.ExeptionHanlder=0;

	a.UpdateAICA=UpdateAICA;

	a.ReadMem_aica_reg=sh4_ReadMem_reg;
	a.WriteMem_aica_reg=sh4_WriteMem_reg;
}

int cfgGetInt(char* key,int def)
{
	return eminf.ConfigLoadInt("nullAica",key,def);
}
void cfgSetInt(char* key,int def)
{
	eminf.ConfigSaveInt("nullAica",key,def);
}

void LoadSettings()
{
	//load default settings before init
	settings.BufferSize=cfgGetInt("BufferSize",1024);
	settings.LimitFPS=cfgGetInt("LimitFPS",1);
	settings.HW_mixing=cfgGetInt("HW_mixing",0);
	settings.SoundRenderer=cfgGetInt("SoundRenderer",1);
	settings.GlobalFocus=cfgGetInt("GlobalFocus",1);
	settings.BufferCount=cfgGetInt("BufferCount",1);
	settings.CDDAMute=cfgGetInt("CDDAMute",0);
	settings.GlobalMute=cfgGetInt("GlobalMute",0);
}

void SaveSettings()
{
	UpdateMenuSelections();
	//load default settings before init
	cfgSetInt("BufferSize",settings.BufferSize);
	cfgSetInt("LimitFPS",settings.LimitFPS);
	cfgSetInt("HW_mixing",settings.HW_mixing);
	cfgSetInt("SoundRenderer",settings.SoundRenderer);
	cfgSetInt("GlobalFocus",settings.GlobalFocus);
	cfgSetInt("BufferCount",settings.BufferCount);
	cfgSetInt("CDDAMute",settings.CDDAMute);
	cfgSetInt("GlobalMute",settings.GlobalMute);
}

//Windoze Code implementation of commong classes from here and after ..

//Thread class
cThread::cThread(ThreadEntryFP* function,void* prm)
{
	Entry=function;
	param=prm;
	hThread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)function,prm,CREATE_SUSPENDED,NULL);
}
cThread::~cThread()
{
	//gota think of something !
}
	
void cThread::Start()
{
	ResumeThread(hThread);
}
void cThread::Suspend()
{
	SuspendThread(hThread);
}
void cThread::WaitToEnd(u32 msec)
{
	WaitForSingleObject(hThread,msec);
}
//End thread class

//cResetEvent Calss
cResetEvent::cResetEvent(bool State,bool Auto)
{
		hEvent = CreateEvent( 
        NULL,             // default security attributes
		Auto?FALSE:TRUE,  // auto-reset event?
		State?TRUE:FALSE, // initial state is State
        NULL			  // unnamed object
        );
}
cResetEvent::~cResetEvent()
{
	//Destroy the event object ?
	 CloseHandle(hEvent);
}
void cResetEvent::Set()//Signal
{
	SetEvent(hEvent);
}
void cResetEvent::Reset()//reset
{
	ResetEvent(hEvent);
}
void cResetEvent::Wait(u32 msec)//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,msec);
}
void cResetEvent::Wait()//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,(u32)-1);
}
//End AutoResetEvent