// drkPvr.cpp : Defines the entry point for the DLL application.
//

#include "drkPvr.h"

#include "ta.h"
#include "spg.h"
#include "regs.h"
#include "renderer_if.h"

//RaiseInterruptFP* RaiseInterrupt;

//void* Hwnd;

emu_info emu;
char emu_name[512];

pvr_init_params params;
_settings_type settings;

//u8*	params.vram;
//vramlock_Lock_32FP* lock32;
//vramlock_Lock_64FP* lock64;
//vramlock_Unlock_blockFP* unlock;

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

class OptionGroop
{
public:
	OptionGroop()
	{
		root_menu=0;
		format=0;
	}
	u32 root_menu;
	char* format;
	struct itempair { u32 id;int value;char* ex_name;};
	vector<itempair> items;

	void (*callback) (int val) ;
	void Add(u32 id,int val,char* ex_name) { itempair t={id,val,ex_name}; items.push_back(t); }
	void Add(u32 root,char* name,int val,char* ex_name=0,int style=0) 
	{ 
		if (root_menu==0)
			root_menu=root;
		u32 ids=emu.AddMenuItem(root,-1,name,handler,0);
		emu.SetMenuItemStyle(ids,style,style);
		
		MenuItem t;
		t.PUser=this;
		emu.SetMenuItem(ids,&t,MIM_PUser);

		Add(ids,val,ex_name);
	}

	static void EXPORT_CALL handler(u32 id,void* win,void* puser)
	{
		OptionGroop* pthis=(OptionGroop*)puser;
		pthis->handle(id);
	}
	void SetValue(int val)
	{
		for (u32 i=0;i<items.size();i++)
		{
			if (val==items[i].value)
			{
				emu.SetMenuItemStyle(items[i].id,MIS_Checked,MIS_Checked);
				if (root_menu && format)
				{
					MenuItem t;
					emu.GetMenuItem(items[i].id,&t,MIM_Text);
					char temp[512];
					sprintf(temp,format,items[i].ex_name==0?t.Text:items[i].ex_name);
					t.Text=temp;
					emu.SetMenuItem(root_menu,&t,MIM_Text);
				}
			}
			else
				emu.SetMenuItemStyle(items[i].id,0,MIS_Checked);
		}
		callback(val);
	}
	void handle(u32 id)
	{
		int val=0;
		for (u32 i=0;i<items.size();i++)
		{
			if (id==items[i].id)
			{
				val=items[i].value;
			}
		}

		SetValue(val);
	}

};
void AddSeperator(u32 menuid)
{
	emu.SetMenuItemStyle(emu.AddMenuItem(menuid,-1,"-",0,0),MIS_Seperator,MIS_Seperator);
}
OptionGroop menu_res;
OptionGroop menu_sortmode;
OptionGroop menu_palmode;
OptionGroop menu_widemode;
int oldmode=-1;
int osx=-1,osy=-1;
void UpdateRRect()
{
	float rect[4]={0,0,640,480};
	if (settings.Enhancements.AspectRatioMode!=0)
	{
		WINDOWINFO winf;
		GetWindowInfo((HWND)emu.GetRenderTarget(),&winf);
		
		int sx=winf.rcClient.right-winf.rcClient.left;
		int sy=winf.rcClient.bottom-winf.rcClient.top;
		if (osx!=sx || osy!=sy)
		{
			osx=sx;
			osy=sy;
			oldmode=-1;
		}
		//printf("New rect %d %d\n",sx,sy);

		float nw=((float)sx/(float)sy)*480.0f;
		rect[0]=(nw-640)/2;
		rect[2]=nw;
	}
	rend_set_render_rect(rect,oldmode!=settings.Enhancements.AspectRatioMode);
	oldmode=settings.Enhancements.AspectRatioMode;
}
void FASTCALL dcShowConfig(void* window)
{
	printf("No config for now\n");
}


void FASTCALL vramLockCB (vram_block* block,u32 addr)
{
	//rend_if_vram_locked_write(block,addr);
	//renderer->VramLockedWrite(block);
	rend_text_invl(block);
}
#include <vector>
using std::vector;

vector<u32> res;

void EXPORT_CALL handler_ShowFps(u32 id,void* win,void* puser)
{
	if (settings.OSD.ShowFPS)
		settings.OSD.ShowFPS=0;
	else
		settings.OSD.ShowFPS=1;

	emu.SetMenuItemStyle(id,settings.OSD.ShowFPS?MIS_Checked:0,MIS_Checked);
	
	SaveSettings();
}
void handler_widemode(int mode)
{
	settings.Enhancements.AspectRatioMode=mode;
		
	SaveSettings();
	UpdateRRect();
}
void handler_PalMode(int  mode)
{
	settings.Emulation.PaletteMode=mode;
	SaveSettings();
}
u32 enable_FS_mid;
u32 AA_mid_menu;
u32 AA_mid_0;
void EXPORT_CALL handler_SetFullscreen(u32 id,void* win,void* puser)
{
	if (settings.Fullscreen.Enabled)
		settings.Fullscreen.Enabled=0;
	else
		settings.Fullscreen.Enabled=1;

	emu.SetMenuItemStyle(id,settings.Fullscreen.Enabled?MIS_Checked:0,MIS_Checked);
	
	SaveSettings();
}

#define makeres(a,b) {#a "x" #b,a,b},

struct 
{
	char* name;
	u32 x;
	u32 y;
} resolutions[]=
{
	makeres(640,480)
	makeres(800,600)
	makeres(1024,768)
	makeres(1280,800)
	makeres(1280,960)
	makeres(1280,1024)
	makeres(1440,1050)
	makeres(1600,900)
	makeres(1600,1200)
	makeres(1920,1080)
	makeres(1920,1200)
	makeres(2048,1536)
	{0,0,0}
};
u32 special_res=0;
void handler_SetRes(int val)
{
	settings.Fullscreen.Res_X=resolutions[val].x;
	settings.Fullscreen.Res_Y=resolutions[val].y;
	
	if (special_res)
	{
		emu.DeleteMenuItem(special_res);
		special_res=0;
	}
	SaveSettings();
}

void EXPORT_CALL handle_About(u32 id,void* w,void* p)
{
	MessageBox((HWND)w,"Made by the nullDC Team","About nullPVR...",MB_ICONINFORMATION);
}
u32 sort_menu;
u32 sort_sm[3];


u32 ssm(u32 nm)
{
	nm=min(nm,2);

	return nm;
}

void handler_SSM(int val)
{
	settings.Emulation.AlphaSortMode=val;

	SaveSettings();
}
void CreateSortMenu()
{
	sort_menu=emu.AddMenuItem(emu.RootMenu,-1,"Sort : %s",0,0);
	
	menu_sortmode.format="Sort : %s";

	menu_sortmode.callback=handler_SSM;
	menu_sortmode.Add(sort_menu,"Off (Fastest)",0,"Off");
	menu_sortmode.Add(sort_menu,"Per Strip",1,"Strip");
	menu_sortmode.Add(sort_menu,"Per Triangle (Slowest)",2,"Triangle");
	menu_sortmode.SetValue(settings.Emulation.AlphaSortMode);
}
//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Load(emu_info* emu_inf)
{
	memcpy(&emu,emu_inf,sizeof(emu));
	emu.ConfigLoadStr("emu","shortname",emu_name,0);
	
	LoadSettings();

	u32 Resolutions_menu=emu.AddMenuItem(emu.RootMenu,-1,"Fullscreen",0,0);
	
	AddSeperator(emu.RootMenu);

	
	enable_FS_mid=emu.AddMenuItem(Resolutions_menu,-1,"Enable",handler_SetFullscreen,settings.Fullscreen.Enabled);

	AddSeperator(Resolutions_menu);
	
	//Resolutions !
	menu_res.callback=handler_SetRes;
	bool sel_any=false;
	for (u32 rc=0;resolutions[rc].name;rc++)
	{
		bool sel=(resolutions[rc].x==settings.Fullscreen.Res_X) && (resolutions[rc].y==settings.Fullscreen.Res_Y);
		if (sel)
			sel_any=true;
		menu_res.Add(Resolutions_menu,resolutions[rc].name,rc);
		if (sel)
			menu_res.SetValue(rc);
	}
	special_res=0;
	if (!sel_any)
	{
		char temp[512];
		sprintf(temp,"%dx%d",settings.Fullscreen.Res_X,settings.Fullscreen.Res_Y);
		special_res=emu.AddMenuItem(Resolutions_menu,-1,temp,0,1);
		emu.SetMenuItemStyle(special_res,MIS_Grayed|MIS_Checked|MIS_Radiocheck,MIS_Grayed|MIS_Checked|MIS_Radiocheck);
	}

	u32 WSM=emu.AddMenuItem(emu.RootMenu,-1,"Aspect Ratio: %s",0,0);
	
	menu_widemode.format="Aspect Ratio: %s";
	menu_widemode.callback=handler_widemode;

	menu_widemode.Add(WSM,"Stretch",0);
	menu_widemode.Add(WSM,"Borders",1);
	menu_widemode.Add(WSM,"Extra Geom",2);
	menu_widemode.SetValue(settings.Enhancements.AspectRatioMode);

	u32 PMT=emu.AddMenuItem(emu.RootMenu,-1,"Palette Handling",0,0);
	
	menu_palmode.callback=handler_PalMode;

	menu_palmode.format="Paletted Textures: %s";
	menu_palmode.Add(PMT,"Static",0);
	menu_palmode.Add(PMT,"Versioned",1);
	AddSeperator(PMT);
	menu_palmode.Add(PMT,"Dynamic,Point",2);
	menu_palmode.Add(PMT,"Dynamic,Full",3);

	menu_palmode.SetValue(settings.Emulation.PaletteMode);
	CreateSortMenu();

	emu.AddMenuItem(emu.RootMenu,-1,"Show Fps",handler_ShowFps,settings.OSD.ShowFPS);

	AddSeperator(emu.RootMenu);
	
	emu.AddMenuItem(emu.RootMenu,-1,"About",handle_About,0);

	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void FASTCALL Unload()
{
	
}

//It's suposed to reset anything but vram (vram is set to 0 by emu)
void FASTCALL ResetPvr(bool Manual)
{
	Regs_Reset(Manual);
	spg_Reset(Manual);
	rend_reset(Manual);
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
s32 FASTCALL InitPvr(pvr_init_params* param)
{
	memcpy(&params,param,sizeof(params));

	
	if ((!Regs_Init()))
	{
		//failed
		return rv_error;
	}
	if (!spg_Init())
	{
		//failed
		return rv_error;
	}
	if (!rend_init())
	{
		//failed
		return rv_error;
	}
	UpdateRRect();
	//olny the renderer cares about thread speciacific shit ..
	if (!rend_thread_start())
	{
		return rv_error;
	}

	emu.SetMenuItemStyle(enable_FS_mid,MIS_Grayed,MIS_Grayed);
	return rv_ok;
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL TermPvr()
{
	rend_thread_end();

	rend_term();
	spg_Term();
	Regs_Term();

	emu.SetMenuItemStyle(enable_FS_mid,0,MIS_Grayed);
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
#define c  info->common
#define p info->pvr

	info->InterfaceVersion=PLUGIN_I_F_VERSION;
	
	c.Type=Plugin_PowerVR;
	c.InterfaceVersion=PVR_PLUGIN_I_F_VERSION;

	strcpy(c.Name,"nullPVR -- " REND_NAME " built : " __DATE__);
	c.PluginVersion=DC_MakeVersion(1,0,0,DC_VER_NORMAL);

	c.Load=Load;
	c.Unload=Unload;

	p.ExeptionHanlder=0;
	p.Init=InitPvr;
	p.Reset=ResetPvr;
	p.Term=TermPvr;

	
	p.ReadReg=ReadPvrRegister;
	p.WriteReg=WritePvrRegister;
	p.UpdatePvr=spgUpdatePvr;

	p.TaDMA=TASplitter::Dma;
	p.TaSQ=TASplitter::SQ;
	p.LockedBlockWrite=vramLockCB;
	
#undef c
#undef p
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

//(const char * lpSection, const char * lpKey, char * lpReturn);
//(const char * lpSection, const char * lpKey, const char * lpString);
int cfgGetInt(char* key,int def)
{
	/*
	char temp1[100];
	char temp2[100];
	sprintf(temp2,"%d",def);
	
	emu.ConfigLoadStr("drkpvr",key,temp,temp1);
	/*if (strcmp("NULL",temp)==0)
		return def;
	return atoi(temp1);*/
	return emu.ConfigLoadInt("drkpvr",key,def);
}
void cfgSetInt(char* key,int val)
{
	/*
	char temp1[100];
	char temp2[100];
	sprintf(temp2,"%d",def);
	
	emu.ConfigLoadStr("drkpvr",key,temp,temp1);
	/*if (strcmp("NULL",temp)==0)
		return def;
	return atoi(temp1);*/
	emu.ConfigSaveInt("drkpvr",key,val);
}

void LoadSettings()
{
	settings.Emulation.AlphaSortMode			=	cfgGetInt("Emulation.AlphaSortMode",1);
	settings.Emulation.PaletteMode				=	cfgGetInt("Emulation.VersionedPalleteTextures",1);

	settings.OSD.ShowFPS						=	cfgGetInt("OSD.ShowFPS",0);
	settings.OSD.ShowStats						=	cfgGetInt("OSD.ShowStats",0);

	settings.Fullscreen.Enabled					=	cfgGetInt("Fullscreen.Enabled",0);
	settings.Fullscreen.Res_X					=	cfgGetInt("Fullscreen.Res_X",640);
	settings.Fullscreen.Res_Y					=	cfgGetInt("Fullscreen.Res_Y",480);
	settings.Fullscreen.Refresh_Rate			=	cfgGetInt("Fullscreen.Refresh_Rate",60);

	settings.Enhancements.MultiSampleCount		=	cfgGetInt("Enhancements.MultiSampleCount",0);
	settings.Enhancements.MultiSampleQuality	=	cfgGetInt("Enhancements.MultiSampleQuality",0);
	settings.Enhancements.AspectRatioMode		=	cfgGetInt("Enhancements.AspectRatioMode",1);
}


void SaveSettings()
{
	cfgSetInt("Emulation.AlphaSortMode",settings.Emulation.AlphaSortMode);
	cfgSetInt("Emulation.VersionedPalleteTextures",settings.Emulation.PaletteMode);

	cfgSetInt("OSD.ShowFPS",settings.OSD.ShowFPS);
	cfgSetInt("OSD.ShowStats",settings.OSD.ShowStats);

	cfgSetInt("Fullscreen.Enabled",settings.Fullscreen.Enabled);
	cfgSetInt("Fullscreen.Res_X",settings.Fullscreen.Res_X);
	cfgSetInt("Fullscreen.Res_Y",settings.Fullscreen.Res_Y);
	cfgSetInt("Fullscreen.Refresh_Rate",settings.Fullscreen.Refresh_Rate);

	cfgSetInt("Enhancements.MultiSampleCount",settings.Enhancements.MultiSampleCount);
	cfgSetInt("Enhancements.MultiSampleQuality",settings.Enhancements.MultiSampleQuality);
	cfgSetInt("Enhancements.AspectRatioMode",settings.Enhancements.AspectRatioMode);
}

int msgboxf(char* text,unsigned int type,...)
{
	va_list args;

	char temp[2048];
	va_start(args, type);
	vsprintf(temp, text, args);
	va_end(args);


	return MessageBox(NULL,temp,emu_name,type | MB_TASKMODAL);
}
