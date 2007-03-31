/*
**	emuWinUI.cpp
*/

#include "types.h"

#include <windows.h>
#include <windowsx.h>
#include <math.h>

#include "emuMode.h"
#include "emuWinUI.h"
#include "commctrl.h"

#include "dc/mem/memutil.h"
#include "dc/mem/sh4_mem.h"
#include "plugins/plugin_manager.h"
#include "dc/maple/maple_if.h"
#include "dc/sh4/sh4_if.h"
#include "dc/sh4/sh4_cst.h"
#include "dc/gdrom/gdrom_if.h"
#include "dc/dc.h"
#include "config/config.h"
#include "plugins/plugin_manager.h"
#include "profiler/profiler.h"
/////////////////////////////
#include "DBG/CtrlMemView.h"
#include "DBG/CtrlDisAsmView.h"
//////////////////////////////////

#include "screenshot.h"

u32 PowerVR_menu;
u32 GDRom_menu;
u32 Aica_menu;
u32 Maple_menu;
u32 Maple_menu_ports[4][6];
u32 ExtDev_menu;

/// i dont like it but ....
CtrlMemView *cMemView;
CtrlDisAsmView *cDisView;
/////////////////////////

HWND g_hWnd;
HINSTANCE g_hInst;

TCHAR g_szFileName[MAX_PATH];

INT_PTR CALLBACK DlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK ArmDlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK DlgProcModal_about( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK DlgProcModal_config( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK PluginDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

#pragma warning(disable: 4311)
#pragma warning(disable: 4312)

void* GetWindowPtr(HWND hWnd,int nIndex)
{
#if X64
	return (void*)GetWindowLongPtr(hWnd,nIndex);
#else
	return (void*)GetWindowLongPtr(hWnd,nIndex);
#endif
}

void SetWindowPtr( HWND hWnd,int nIndex,void* dwNewLong)
{
#if X64
	SetWindowLongPtr(hWnd,nIndex,(LONG_PTR)dwNewLong);
#else
	SetWindowLongPtr(hWnd,nIndex,(LONG)dwNewLong);
#endif
}

#pragma warning(default: 4311)
#pragma warning(default: 4312)


 int power(int base, int n)
 {
    int i, p;
 
    if (n == 0)
       return 1;
 
    p = 1;
    for (i = 1; i <= n; ++i)
       p = p * base;
    return p;
 }

 int htoi(char s[]) {
 
	size_t len;
	int  value = 1, digit = 0,  total = 0;
    int c, x, y, i = 0;
    char hexchars[] = "abcdef"; /* Helper string to find hex digit values */
 
    /* Test for 0s, '0x', or '0X' at the beginning and move on */
 
    if (s[i] == '0')
    {
       i++;
       if (s[i] == 'x' || s[i] == 'X')
       {
          i++;
       }
    }
 
    len = strlen(s);
 
    for (x = i; x < (int)len; x++)
    {
       c = tolower(s[x]);
       if (c >= '0' && c <= '9')
       {
          digit = c - '0';
       } 
       else if (c >= 'a' && c <= 'f')
       {
          for (y = 0; hexchars[y] != '\0'; y++)
          {
             if (c == hexchars[y])
             {
                digit = y + 10;
             }
          }
       } else {
          return 0; /* Return 0 if we get something unexpected */
       }
       value = power(16, (int)(len-x-1));
       total += value * digit;
    }
    return total;
 }
 
void InitMenu();
HMENU GetHMenu();
u32 uiInit(void)
{
	g_hInst =(HINSTANCE)GetModuleHandle(0);
	WNDCLASS wc;
	memset(&wc,0,sizeof(wc));
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hIcon			= LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NDC_ICON));
	wc.hInstance		= g_hInst;
	wc.lpfnWndProc		= WndProc;
	wc.lpszClassName	= "ndc_main_window";
	wc.lpszMenuName		= 0;
	wc.style			= CS_VREDRAW | CS_HREDRAW ;

	if( !RegisterClass(&wc) ) {
		MessageBox( NULL, "Couldn't Register ndc_main_window Class !","ERROR",MB_ICONERROR );
		return false;
	}

	InitCommonControls();

	InitMenu();

	g_hWnd = CreateWindowA( "ndc_main_window", VER_FULLNAME, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, 640,480, NULL, GetHMenu(), g_hInst, NULL );
	if( !IsWindow(g_hWnd) ) {
		MessageBox( NULL, "Couldn't Create nullDC Window!","ERROR",MB_ICONERROR );
		return false;
	}
	RECT rect;
	RECT rect2;
	GetClientRect(g_hWnd,&rect);
	int xsz=rect.right-rect.left;
	int ysz=rect.bottom-rect.top;

	GetWindowRect(g_hWnd,&rect2);
	int xsz_2=rect2.right-rect2.left;
	int ysz_2=rect2.bottom-rect2.top;
	xsz_2-=xsz;
	ysz_2-=ysz;

	SetWindowPos(g_hWnd,NULL,0,0,xsz_2+640,ysz_2+480,SWP_NOZORDER|SWP_NOMOVE);

	
	return UI_OK;
}

u32 uiTerm(void)
{
	DestroyWindow(g_hWnd);
	return UI_OK;
}

u32 uiMain(void)
{
	static MSG msg;
	static HACCEL hAccel = LoadAccelerators(g_hInst, NULL);
	
	//proc all waiting messages
	WaitMessage();
	while( PeekMessage(&msg, NULL, 0,0, PM_REMOVE) != 0 )
	{
		if( !TranslateAccelerator(g_hWnd, hAccel, &msg) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} 

		if( WM_QUIT == msg.message )
			return UI_MAIN_QUIT;
	}
    return UI_OK;
}


void uiGetFN(TCHAR *szFileName, TCHAR *szParse)
{
	static OPENFILENAME ofn;
	static TCHAR szFile[MAX_PATH];    
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= g_hWnd;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFilter		= szParse;
	ofn.nFilterIndex	= 1;
	ofn.nMaxFileTitle	= 128;
	ofn.lpstrFileTitle	= szFile;
	ofn.lpstrInitialDir	= NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn)<=0)
		printf("GetISOfname() Failed !\n");
}


#define WM_GOTOPC WM_USER+0xC0D3

///////////////////
HWND hDebugger;

__inline static char* _ext( char* szFN, u32 size ) {
	for( u32 i=0; i<size; i++ )	{
		if( szFN[i] == 0x2E ) { return &szFN[i]; }	// 0x2E == '.'
	}	return szFN;
}

//Dynamic menu code :)
struct _MenuItem;
struct MenuStrip
{
	_MenuItem* owner;
	HMENU hmenu;
	vector<u32> items;

	bool Empty()  { return items.size()==0; }
	void AddItem(u32 id,u32 pos);
	void RemoveItem(u32 id);

	void Delete();
	MenuStrip(_MenuItem* p=0)
	{
		owner=p;
		hmenu=0;
	}
	~MenuStrip()
	{
		Delete();
	}
};
struct _MenuItem
{
	MenuStrip* owner;
	MenuStrip submenu;
	u32 gmid;	//menu item id
	u32 nid;	//notification id
	char* txt;
	void* puser;
	MenuItemSelectedFP* handler;
	u32 Style;
	void* hbitmap;

	_MenuItem(char* text,u32 id,u32 gid)
	{
		owner=0;
		submenu.owner=this;
		gmid=gid;
		nid=id;
		txt=text;
		puser=0;
		handler=0;
		Style=0;
		hbitmap=0;
	}

	void Insert(MenuStrip* menu,u32 pos);
	void Remove(HMENU menu);

	void Update();
	~_MenuItem();
	void Create(char* text);

	void AddChild(u32 id);
	void RemoveChild(u32 id);
	void Clicked(void* hWnd);
};
vector<_MenuItem*> MenuItems;
MenuStrip MainMenu;

void MenuStrip::AddItem(u32 id,u32 pos)
{
	if (hmenu==0)
	{
		if (owner!=0)
			hmenu=CreatePopupMenu();
		else
			hmenu=CreateMenu();
	}
	items.push_back(id);

	MenuItems[id]->Insert(this,pos);
	if (owner)
		owner->AddChild(id);
	DrawMenuBar(g_hWnd);
}
void MenuStrip::RemoveItem(u32 id)
{
	verify(hmenu!=0);
	MenuItems[id]->Remove(hmenu);

	for (size_t i=0;i<items.size();i++)
	{
		if (items[i]==id)
		{
			items.erase(items.begin()+i);
			break;
		}
	}
	if (owner)
		owner->RemoveChild(id);
	DrawMenuBar(g_hWnd);
}

void MenuStrip::Delete()
{
	while(items.size())
	{
		RemoveItem(items[0]);
	}
	if (hmenu)
		DestroyMenu(hmenu);

	hmenu=0;
}


void _MenuItem::Clicked(void* hWnd)
{
	if (handler)
		handler(gmid,hWnd,puser);
}
void _MenuItem::Insert(MenuStrip* menu,u32 pos)
{
	MENUITEMINFO mif;
	memset(&mif,0,sizeof(mif));
	mif.cbSize=sizeof(mif);
	mif.fMask=MIIM_ID|MIIM_STRING|MIIM_STATE;
	mif.dwTypeData=txt;
	mif.wID=nid;
	if (handler==0)
		mif.fState=MFS_GRAYED;

	owner=menu;
	BOOL rv=InsertMenuItem(owner->hmenu,pos,TRUE,&mif);
}
void _MenuItem::Remove(HMENU menu)
{
	DeleteMenu(menu,nid,MF_BYCOMMAND);
	owner=0;
}

void _MenuItem::Update()
{
	MENUITEMINFO mif;
	memset(&mif,0,sizeof(mif));
	mif.cbSize=sizeof(mif);
	mif.fMask=MIIM_SUBMENU | MIIM_STATE | MIIM_FTYPE | MIIM_BITMAP | MIIM_STRING;
	mif.hSubMenu=submenu.hmenu;

	mif.dwTypeData=txt;

	mif.fType = MFT_STRING;

	if (Style & MIS_Bitmap)
	{
		mif.hbmpItem=(HBITMAP)hbitmap;
	}
	if (Style & MIS_Radiocheck)
	{
		mif.fType|=MFT_RADIOCHECK;
	}
	
	if (Style & MIS_Seperator)
	{
		mif.fType|=MFT_SEPARATOR;
	}
	
	if (Style&MIS_Checked)
	{
		mif.fState|=MFS_CHECKED;
	}
	if (Style&MIS_Grayed)
	{
		mif.fState|=MFS_GRAYED;
	}
	
	if (submenu.Empty() && handler==0)
	{
		mif.fState|=MFS_GRAYED;
	}

	SetMenuItemInfo(owner->hmenu,nid,FALSE,&mif);
}
_MenuItem::~_MenuItem()
{
	if (txt)
		free(txt);
	if (owner)
	owner->RemoveItem(gmid);
	MenuItems[gmid]=0;
}
void _MenuItem::Create(char* text)
{
	//smth
}
void _MenuItem::AddChild(u32 id)
{
	//submenu.AddItem(id,pos);

	//update item info
	Update();
}
void _MenuItem::RemoveChild(u32 id)
{
	//submenu.RemoveItem(id);
	if (submenu.Empty())
		submenu.Delete();

	//update item info
	Update();

	//delete MenuItems[id];
}
u32 CreateMenuItem(char* text,MenuItemSelectedFP* handler , void* puser)
{
	u32 gmid = MenuItems.size();
	_MenuItem* t=new _MenuItem(strdup(text),gmid+10,gmid);
	t->puser=puser;
	t->handler=handler;
	MenuItems.push_back(t);
	return gmid;
}
u32 FASTCALL AddMenuItem(u32 parent,s32 pos,char* text,MenuItemSelectedFP* handler ,u32 checked)
{
	u32 rv= CreateMenuItem(text,handler,0);
	
	if (parent==0)
		MainMenu.AddItem(rv,pos);
	else
	{
		MenuItems[parent]->submenu.AddItem(rv,(u32)pos);
	}

	SetMenuItemStyle(rv,checked?MIS_Checked:0,MIS_Checked);
	
	return rv;
}
void FASTCALL SetMenuItemStyle(u32 id,u32 style,u32 mask)
{
	MenuItems[id]->Style= (MenuItems[id]->Style & (~mask))|style;
	MenuItems[id]->Update();
}
void FASTCALL GetMenuItem(u32 id,MenuItem* info,u32 mask)
{
	if (mask & MIM_Bitmap)
		info->Bitmap=MenuItems[id]->hbitmap;

	if (mask & MIM_Handler)
		info->Handler=MenuItems[id]->handler;

	if (mask & MIM_PUser)
		info->PUser=MenuItems[id]->puser;

	if (mask & MIM_Style)
		info->Style=MenuItems[id]->Style;

	if (mask & MIM_Text)
		info->Text=MenuItems[id]->txt;
}
void FASTCALL SetMenuItem(u32 id,MenuItem* info,u32 mask)
{
	if (mask & MIM_Bitmap)
		MenuItems[id]->hbitmap=info->Bitmap;

	if (mask & MIM_Handler)
		MenuItems[id]->handler=info->Handler;

	if (mask & MIM_PUser)
		MenuItems[id]->puser=info->PUser;

	if (mask & MIM_Style)
		MenuItems[id]->Style=info->Style;

	if (mask & MIM_Text)
	{
		if (MenuItems[id]->txt)
			free(MenuItems[id]->txt);
		MenuItems[id]->txt=strdup(info->Text);
	}

	MenuItems[id]->Update();
}

void FASTCALL DeleteAllMenuItemChilds(u32 id)
{
	MenuItems[id]->submenu.Delete();
}
void FASTCALL SetMenuItemHandler(u32 id,MenuItemSelectedFP* h)
{
	MenuItems[id]->handler=h;
	MenuItems[id]->Update();
}
void FASTCALL SetMenuItemBitmap(u32 id,void* hbmp)
{
	MenuItems[id]->hbitmap=hbmp;
	MenuItems[id]->Update();
}
u32 FASTCALL GetMenuItemStyle(u32 id)
{
	return MenuItems[id]->Style;
}
void* FASTCALL GetMenuItemBitmap(u32 id)
{
	return MenuItems[id]->hbitmap;
}

void FASTCALL DeleteMenuItem(u32 id)
{
	if (id==0)
		return;

	delete MenuItems[id];
}


//Wow , that was quite big :p

//Nice helper :)
void AddSeperator(u32 menu)
{
	SetMenuItemStyle(AddMenuItem(menu,-1,"-",0,0),MIS_Seperator,MIS_Seperator);
}
#define MENU_HANDLER(name) void FASTCALL name (u32 id,void* hWnd,void* stuff)
///////Menu Handlers\\\\\\\

MENU_HANDLER( HandleMenu1 )
{
	DeleteMenuItem(id);
}
MENU_HANDLER( HandleMenu0 )
{
	msgboxf("Menu %d -- not implemented",MBX_ICONEXCLAMATION,id);
}

//File 
MENU_HANDLER(Handle_File_OpenBin)
{
	if (!sh4_cpu->IsCpuRunning())	//if cpu is stoped
	{
		OPENFILENAME ofn;
		TCHAR szFile[128];

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize		= sizeof(OPENFILENAME);
		ofn.hwndOwner		= (HWND)hWnd;
		ofn.lpstrFile		= g_szFileName;
		ofn.nMaxFile		= MAX_PATH;
		ofn.lpstrFilter		= "All(.BIN\\.ELF)\0*.BIN;*.ELF\0Binary\0*.BIN\0Elf\0*.ELF\0All\0*.*\0";
		ofn.nFilterIndex	= 1;
		ofn.nMaxFileTitle	= 128;
		ofn.lpstrFileTitle	= szFile;
		ofn.lpstrInitialDir	= NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if(GetOpenFileName(&ofn)>0)
		{
			if (Init_DC())
			{
				Reset_DC(false);
				if(!LoadBinfileToSh4Mem(0x10000, g_szFileName))
					return;
				EnablePatch(patch_resets_Misc);//mwhaha
				sh4_cpu->Reset(false);//do a hard reset
				sh4_cpu->SetRegister(Sh4RegType::reg_sr,0x70000000);
				sh4_cpu->SetRegister(Sh4RegType::reg_gbr,0x8c000000);
				sh4_cpu->SetRegister(Sh4RegType::reg_pc,0x8c008300);
				Start_DC();
			}
		}
	}
	//add warn message
}
MENU_HANDLER(Handle_File_LoadBin)
{
	//if (!sh4_cpu->IsCpuRunning())	//if cpu is stoped
	{
		OPENFILENAME ofn;
		TCHAR szFile[128];

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize		= sizeof(OPENFILENAME);
		ofn.hwndOwner		= (HWND)hWnd;
		ofn.lpstrFile		= g_szFileName;
		ofn.nMaxFile		= MAX_PATH;
		ofn.lpstrFilter		= "All(.BIN\\.ELF)\0*.BIN;*.ELF\0Binary\0*.BIN\0Elf\0*.ELF\0All\0*.*\0";
		ofn.nFilterIndex	= 1;
		ofn.nMaxFileTitle	= 128;
		ofn.lpstrFileTitle	= szFile;
		ofn.lpstrInitialDir	= NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if(GetOpenFileName(&ofn)>0)
		{
			if(!LoadBinfileToSh4Mem(0x10000, g_szFileName))
				return;
		}
	}
	//add warn message
}
MENU_HANDLER(Handle_File_BootHLE)
{
	if (Init_DC())
	{
		Reset_DC(false);
		if (gdBootHLE()==false)
		{
			MessageBox((HWND)hWnd,"Failed to find ip.bin/bootfile\nTry to boot using the Normal boot method.","HLE Boot Error",MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		EnablePatch(patch_resets_Misc);//mwhaha
		sh4_cpu->Reset(false);//do a hard reset
		sh4_cpu->SetRegister(Sh4RegType::reg_sr,0x70000000);
		sh4_cpu->SetRegister(Sh4RegType::reg_gbr,0x8c000000);
		sh4_cpu->SetRegister(Sh4RegType::reg_pc,0x8c008300);
		Start_DC();
	}
}
MENU_HANDLER(Handle_File_Exit)
{
	SendMessage((HWND)hWnd, WM_CLOSE, 0,0);
}
//System
MENU_HANDLER( Handle_System_Start)
{
	Start_DC();
}
MENU_HANDLER( Handle_System_Stop)
{
	Stop_DC();
}
MENU_HANDLER( Handle_System_Reset)
{
	sh4_cpu->Stop();
	printf(">>\tDreamcast Reset\n");
	sh4_cpu->Reset(false);//do a hard reset
	sh4_cpu->SetRegister(Sh4RegType::reg_pc,0xA0000000);
	DisablePatch(patch_all);
}
//Debug
MENU_HANDLER( Handle_Debug_Sh4Debugger)
{
	if (!IsDCInited())
	{
		printf("Debugger opened w/o init , initing everything..\n");
		Init_DC();
		Reset_DC(false);
	}


	CtrlMemView::init();
	CtrlDisAsmView::init();

	hDebugger = CreateDialog( g_hInst, MAKEINTRESOURCE(IDD_SH4DEBUG), NULL, DlgProc);
	if( !IsWindow(hDebugger) )
		MessageBox( (HWND)hWnd, "Couldn't open Sh4 debugger","",MB_OK );
}
/*
void Handle_ArmDebugger(u32 id,void* stuff)
{
	hDebugger = CreateDialog( g_hInst, MAKEINTRESOURCE(IDD_ARM7DEBUG), NULL, ArmDlgProc);
	if( !IsWindow(hDebugger) )
	MessageBox( hWnd, "Couldn't open ARM7 debugger","",MB_OK );

	return 0;
}
*/
//Options
MENU_HANDLER( Handle_Options_Config)
{
	DialogBox(g_hInst,MAKEINTRESOURCE(IDD_CONFIG),(HWND)hWnd,DlgProcModal_config);
}
MENU_HANDLER( Handle_Options_SelectPlugins)
{
	plugins_Select();
}
//Profiler
MENU_HANDLER( Handle_Profiler_Show)
{
	msgboxf("Profiler gui not yet implemented",MB_ICONERROR);
}
MENU_HANDLER( Handle_Profiler_Enable )
{
	if (GetMenuItemStyle(id) & MIS_Checked)
	{
		stop_Profiler();
		SetMenuItemStyle(id,0,MIS_Checked);
	}
	else
	{
		start_Profiler();
		SetMenuItemStyle(id,MIS_Checked,MIS_Checked);
	}
}
//Help
MENU_HANDLER( Handle_Help_About )
{
	DialogBox(g_hInst,MAKEINTRESOURCE(IDD_ABOUT),(HWND)hWnd,DlgProcModal_about);
}
template<bool* setting>
MENU_HANDLER( Handle_Option_Bool_Template )
{
	if (*setting)
		*setting=0;
	else
		*setting=1;

	SetMenuItemStyle(id,*setting?MIS_Checked:0,MIS_Checked);
	SaveSettings();
}

u32 rec_cpp_mid;
u32 rec_enb_mid;
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
			sh4_cpu->Term();
		}
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

	if (bStart)
	{
		sh4_cpu->Init();
		Start_DC();
	}
}

void UpdateMenus()
{
	if(settings.dynarec.Enable)
	{
		SetMenuItemStyle(rec_enb_mid,MIS_Checked,MIS_Checked);
		SetMenuItemStyle(rec_cpp_mid,0,MIS_Grayed);
	}
	else
	{
		SetMenuItemStyle(rec_enb_mid,0,MIS_Checked);
		SetMenuItemStyle(rec_cpp_mid,MIS_Grayed,MIS_Grayed);
	}

	SetMenuItemStyle(rec_cpp_mid,settings.dynarec.CPpass?MIS_Checked:0,MIS_Checked);
}
MENU_HANDLER( Handle_Option_EnableRec )
{
	Handle_Option_Bool_Template<&settings.dynarec.Enable>(id,hWnd,stuff);

	SwitchCpu();
	UpdateMenus();
}
MENU_HANDLER( Handle_Option_EnableCP )
{
	Handle_Option_Bool_Template<&settings.dynarec.CPpass>(id,hWnd,stuff);
	if (sh4_cpu->ResetCache)
		sh4_cpu->ResetCache();
	UpdateMenus();
}
//Create the menus and set the handlers :)
void CreateBasicMenus()
{
	u32 menu_file=AddMenuItem(0,-1,"File",0,0);
	u32 menu_system=AddMenuItem(0,-1,"System",0,0);
	u32 menu_options=AddMenuItem(0,-1,"Options",0,0);
	u32 menu_debug=AddMenuItem(0,-1,"Debug",0,0);
	u32 menu_profiler=AddMenuItem(0,-1,"Profiler",0,0);
	u32 menu_help=AddMenuItem(0,-1,"Help",0,0);

	//File menu
	AddMenuItem(menu_file,-1,"Normal Boot",Handle_System_Start,0);
	AddMenuItem(menu_file,-1,"Hle GDROM boot",Handle_File_BootHLE,0);
	AddSeperator(menu_file);
	AddMenuItem(menu_file,-1,"Open bin/elf",Handle_File_OpenBin,0);
	AddMenuItem(menu_file,-1,"Load bin/elf",Handle_File_LoadBin,0);
	AddSeperator(menu_file);
	AddMenuItem(menu_file,-1,"Exit",Handle_File_Exit,0);


	//System Menu
	AddMenuItem(menu_system,-1,"Start",Handle_System_Start,0);
	AddMenuItem(menu_system,-1,"Stop",Handle_System_Stop,0);
	AddMenuItem(menu_system,-1,"Reset",Handle_System_Reset,0);

	//Options Menu
	u32 menu_setts=AddMenuItem(menu_options,-1,"nullDC Settings",Handle_Options_Config,0);
		AddMenuItem(menu_setts,-1,"Show",Handle_Options_Config,0);
		AddSeperator(menu_setts);
		rec_enb_mid=AddMenuItem(menu_setts,-1,"Enable Dynarec",Handle_Option_EnableRec,0);
		rec_cpp_mid=AddMenuItem(menu_setts,-1,"Enable CP pass",Handle_Option_EnableCP,0);

	AddMenuItem(menu_options,-1,"Select Plugins",Handle_Options_SelectPlugins,0);
	AddSeperator(menu_options);
	PowerVR_menu = AddMenuItem(menu_options,-1,"PowerVR",0,0);
	GDRom_menu = AddMenuItem(menu_options,-1,"GDRom",0,0);
	Aica_menu = AddMenuItem(menu_options,-1,"Aica",0,0);
	Maple_menu = AddMenuItem(menu_options,-1,"Maple",0,0);
	ExtDev_menu = AddMenuItem(menu_options,-1,"ExtDevice",0,0);

	//Maple Menu
	Maple_menu_ports[0][5]=AddMenuItem(Maple_menu,-1,"Port A",0,0);
	Maple_menu_ports[1][5]=AddMenuItem(Maple_menu,-1,"Port B",0,0);
	Maple_menu_ports[2][5]=AddMenuItem(Maple_menu,-1,"Port C",0,0);
	Maple_menu_ports[3][5]=AddMenuItem(Maple_menu,-1,"Port D",0,0);

	//Debug
	AddMenuItem(menu_debug,-1,"Debugger",Handle_Debug_Sh4Debugger,0);

	//Profiler
	AddMenuItem(menu_profiler,-1,"Enable",Handle_Profiler_Enable,0);
	AddMenuItem(menu_profiler,-1,"Show",Handle_Profiler_Show,0);

	//Help
	AddMenuItem(menu_help,-1,"About",Handle_Help_About,0);

	//Update menu ticks to match settings :)
	UpdateMenus();
}
void InitMenu()
{
	MenuItems.push_back(0);

	CreateBasicMenus();
}
HMENU GetHMenu()
{
	return MainMenu.hmenu;
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static RECT rc;

	switch(uMsg)
	{
	case WM_CREATE:
		InitCommonControls();
		break;

	case WM_COMMAND:
		{
			for (size_t i=1;i<MenuItems.size();i++)
			{
				if (MenuItems[i] && MenuItems[i]->nid==LOWORD(wParam))
				{
					MenuItems[i]->Clicked(hWnd);
					break;
				}
			}
			//printf("Menu item %d selected\n",LOWORD(wParam));
		}
		break;


	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	//case WM_SIZE:
	//	GetClientRect(hWnd, &rc);
//		PvrPlugin.UpdatePvr(0);
		//TODO : Fix that
	//	PvrUpdate(0);
	case WM_KEYDOWN:
		{
			int val = (int)wParam;
			switch(val)
			{
			case VK_F7:
				{
					if (sh4_cpu->ResetCache)
						sh4_cpu->ResetCache();
				}
				break;
			case VK_F8:
				{
					char fn[100];
					
					int i;
					i=0;
					char* fn2=0;

					while( i >=0)
					{
						sprintf(fn,"screenshot_%d.bmp",i);
						fn2 = GetEmuPath(fn);
						FILE* tf=fopen(fn,"rb");
						if (tf)
							fclose(tf);
						else
							break;
						//fseek(tf,0,SEEK_END);
						//size_t ft=ftell();
						i++;
					}
					if (Screenshot(fn2,g_hWnd))
						printf("Screenshot saved to %s\n",fn2);
					else
						printf("failed to save screenshot to \"%s\"\n",fn2);
				}
				break;
			}
		}
		break;
	default: break;
	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
bool plgdlg_cancel=false;
bool SelectPluginsGui()
{
	DialogBox(g_hInst,MAKEINTRESOURCE(IDD_PLUGIN_SELECT),g_hWnd,PluginDlgProc);
	return !plgdlg_cancel;
}
INT_PTR CALLBACK DlgProcModal_about( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			Edit_SetText(GetDlgItem(hWnd,IDC_THXLIST),
				"Credits :" "\r\n"
				" drk||Raziel \t: main coder" "\r\n"
				" ZeZu \t\t: main coder" "\r\n"
				" GiGaHeRz \t: plugin work/misc stuff"  "\r\n"
				" PsyMan \t\t: Mental (and metal) support ,managment, ""\r\n"
				"        \t\t  beta testing, everything else"  "\r\n"
				" Xant \t\t: www & forum, beta testing"   "\r\n"
				"\r\n"
				"Beta testing :"  "\r\n"
				" emwearz,Miretank,gb_away,Raziel,General Plot,"  "\r\n"
				" Refraction,Ckemu"  "\r\n"
				"\r\n"
				"Many thanks to :" "\r\n"
				" •fill this later" "\r\n"
				"\r\n"
				"\r\n"
				"All UR BUGZ BELONGZ TO UZ" "\r\n"
				);
			Static_SetText(GetDlgItem(hWnd,IDC_NDC_VER),VER_SHORTNAME);
		}
		return true;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDOK:
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

INT_PTR CALLBACK DlgProcModal_config( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			int tmp;
			CheckDlgButton(hWnd,IDC_REC,settings.dynarec.Enable!=0?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hWnd,IDC_REC_CPP,settings.dynarec.CPpass!=0?BST_CHECKED:BST_UNCHECKED);
		}
		return true;

	case WM_COMMAND:

		switch( LOWORD(wParam) )
		{
		case IDOK:
			{
				settings.dynarec.Enable = (BST_CHECKED==IsDlgButtonChecked(hWnd,IDC_REC)) ? 1 : 0 ;
				settings.dynarec.CPpass = (BST_CHECKED==IsDlgButtonChecked(hWnd,IDC_REC_CPP)) ? 1 : 0 ;
				
				SaveSettings();
				SwitchCpu();
				UpdateMenus();
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


INT_PTR CALLBACK ChildDlgProc( HWND hChild, UINT uMsg, WPARAM wParam, LPARAM lParan )
{
	switch( uMsg ) {
	case WM_INITDIALOG:	return true;
	default: break;
	}
	return false;
}


//////////////

void RefreshDebugger(HWND);	// *TEMP*

/////////////////////////////////////////// TAB CTRL STUFF /////////////////////////////////

#define C_PAGES 3 
 
typedef struct tag_dlghdr { 
    HWND hTab;       // tab control 
    HWND hDisplay;   // current child dialog box 
    RECT rcDisplay;     // display rectangle for the tab control 

    DLGTEMPLATE *apRes[C_PAGES]; 
} DLGHDR; 

// DoLockDlgRes - loads and locks a dialog box template resource. 
// Returns the address of the locked resource. 
// lpszResName - name of the resource 
 
DLGTEMPLATE * WINAPI DoLockDlgRes(LPCSTR lpszResName) 
{ 
    HRSRC hrsrc = FindResource(NULL, lpszResName, RT_DIALOG); 
    HGLOBAL hglb = LoadResource(g_hInst, hrsrc); 
    return (DLGTEMPLATE *) LockResource(hglb); 
} 


// OnChildDialogInit - Positions the child dialog box to fall 
//     within the display area of the tab control. 


/////////////////////////////////////////////////////////////////////////////////////
#define YOFFS 20
#define BUTTONOFFS 25;

void WINAPI OnSizeTab(HWND hDlg) 
{
    DLGHDR *pHdr = (DLGHDR *) GetWindowPtr(hDlg, GWLP_USERDATA); 
	////////////////////////////////////////////////////////////

	RECT rcTab, rcDlg;
	GetWindowRect( hDlg, &rcDlg );
	GetWindowRect( pHdr->hTab, &rcTab );
	
	UINT y = rcTab.top - rcDlg.top - YOFFS;
	UINT x = rcTab.left - rcDlg.left;
	UINT w = rcTab.right - rcTab.left;
	UINT h = rcTab.bottom - rcTab.top - BUTTONOFFS;

	HWND hCust = GetDlgItem(pHdr->hDisplay, IDC_VIEW);
	SetWindowPos( hCust, HWND_TOP, 0, 0, w-8, h, SWP_SHOWWINDOW );
	SetWindowPos( pHdr->hDisplay, HWND_TOP, x, y, w, h, SWP_SHOWWINDOW );
} 

// OnSelChanged - processes the TCN_SELCHANGE notification. 
// hwndDlg - handle to the parent dialog box. 
 
VOID WINAPI OnSelChanged(HWND hDlg) 
{ 
    DLGHDR *pHdr = (DLGHDR *) GetWindowPtr( hDlg, GWLP_USERDATA); 

    int iSel = TabCtrl_GetCurSel(pHdr->hTab); 
 
    // Destroy the current child dialog box, if any. 
    if (pHdr->hDisplay != NULL) 
        DestroyWindow(pHdr->hDisplay); 
 
    // Create the new child dialog box. 
    pHdr->hDisplay = CreateDialogIndirect(g_hInst, pHdr->apRes[iSel], hDlg, ChildDlgProc); 

	switch( iSel ) {
	case 0: cDisView = CtrlDisAsmView::getFrom(GetDlgItem(pHdr->hDisplay,IDC_VIEW)); cMemView=NULL; break;
	case 1: cMemView = CtrlMemView::getFrom(GetDlgItem(pHdr->hDisplay,IDC_VIEW)); cDisView=NULL; break;
	case 2:
	default: cDisView = NULL; cMemView = NULL; break;
	}
	OnSizeTab(hDlg);
} 

void WINAPI OnDestroyTab(HWND hDlg)
{
    DLGHDR *pHdr = (DLGHDR *) GetWindowPtr( hDlg, GWLP_USERDATA); 

	if( IsWindow(pHdr->hDisplay) )
		DestroyWindow( pHdr->hDisplay );
	if( IsWindow(pHdr->hTab) )
		DestroyWindow( pHdr->hTab );
}

void WINAPI OnInitTab(HWND hDlg)
{
	TCITEM tci; 
	DLGHDR *pHdr = (DLGHDR *) LocalAlloc(LPTR, sizeof(DLGHDR)); 

	SetWindowPtr(hDlg, GWLP_USERDATA, pHdr);

	pHdr->hTab = GetDlgItem(hDlg, IDC_VIEWSEL);
	
	// Add a tab for each of the three child dialog boxes. 
    tci.mask = TCIF_TEXT | TCIF_IMAGE; 
    tci.iImage = -1; 
    tci.pszText = "DisAsm"; 
    TabCtrl_InsertItem(pHdr->hTab, 0, &tci); 
    tci.pszText = "Memory"; 
    TabCtrl_InsertItem(pHdr->hTab, 1, &tci); 
    tci.pszText = "PVR List"; 
    TabCtrl_InsertItem(pHdr->hTab, 2, &tci); 
 
    // Lock the resources for the three child dialog boxes. 
    pHdr->apRes[0] = DoLockDlgRes(MAKEINTRESOURCE(IDD_DISVIEW)); 
    pHdr->apRes[1] = DoLockDlgRes(MAKEINTRESOURCE(IDD_MEMVIEW__)); 
    pHdr->apRes[2] = DoLockDlgRes(MAKEINTRESOURCE(IDD_PVRVIEW)); 
 
    OnSelChanged(hDlg);
}


////////////////////////////// END TAB CTRL	///////////////////////////////////////////////

INT_PTR CALLBACK DlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static int iSel=0;
	static char szAddr[32];
	static DWORD dwIndex=0, dwAddr=0, i=0;


	switch( uMsg )
	{
	case WM_INITDIALOG:

		if( IsWindow(GetDlgItem(hWnd,IDC_VIEWSEL)) )
		{
			OnInitTab(hWnd);
			OnSizeTab(hWnd);	/// have to do this or postsizemsg

			RefreshDebugger(hWnd);
		}
		if( IsWindow( GetDlgItem(hWnd, IDC_MEMSEL) ) )
		{
		//	SendDlgItemMessage(hWnd, IDC_MEMSEL, CB_SETITEMDATA, 0xBADC0D3, 0x666 );
		//	for( i=0; i<MMAP_NUM_BLOCKS; i++ ) {
		//		dwIndex = SendDlgItemMessage( hWnd, IDC_MEMSEL, CB_ADDSTRING, 0, (LPARAM)MemBlock[i].szDesc );
		//		SendDlgItemMessage( hWnd, IDC_MEMSEL, CB_SETITEMDATA, dwIndex, i );
		//	}
		}
		return true;

	case WM_COMMAND: 

		switch (LOWORD(wParam)) 
		{
		case IDC_FIND:
			MessageBox( hWnd, "FIND","", MB_OK );
			return true;

		/////////////////////////////
		case IDB_STOP:
			Stop_DC();
			goto refresh;

		case IDB_START:
			Start_DC();
			goto refresh;

		case IDB_STEP:
			sh4_cpu->Step();
			goto refresh;

		case IDB_SKIP:
			sh4_cpu->Skip();
			goto refresh;

		case IDB_GOTO:
			GetDlgItemText( hWnd, IDC_ADDR, szAddr, 32 );
			cDisView->gotoAddr(htoi(szAddr));
			return true;	// dont goto refresh .. will goto pc instead of IDC_GOTO

		//////////////////////////////

			/*
		case IDC_MEMSEL:
			if( HIWORD(wParam) == CBN_SELCHANGE )
			{
				dwIndex = SendDlgItemMessage( hWnd, IDC_MEMSEL, CB_GETCURSEL, 0,0 );
				i = SendDlgItemMessage( hWnd, IDC_MEMSEL, CB_GETITEMDATA, dwIndex, 0 );
				MemViewUpdate( i );	// can just pass this to MV_UPDATE

				SendMessage( hWnd, WM_SIZE, 0, 0 );
				SendDlgItemMessage( hWnd, IDC_CUSTOM, MV_UPDATE, 0,0 );
			}
			return true;
			*/

			////////////

		case IDC_XF:
		case IDC_FPSEL1:
		case IDC_FPSEL2:
		case IDC_FPSEL3:
			RefreshDebugger(hWnd);
			return true;

		default: break;
		}
		break;


	case WM_NOTIFY:
		switch( ((LPNMHDR)lParam)->idFrom )
		{
		case IDC_VIEWSEL:
			if( ((LPNMHDR)lParam)->code == TCN_SELCHANGE )
			{
				OnSelChanged(hWnd);
				goto refresh;
			}
			break;
		}
		break;
		

	case WM_CLOSE:
	case WM_DESTROY:			// for mem view dlg
		OnDestroyTab(hWnd);
		DestroyWindow(hWnd);
		return TRUE; 

	case WM_MOVE:
	case WM_SIZE:
	//	OnSizeTab(hWnd);
	//	return TRUE;
		goto refresh;

	case WM_GOTOPC:
		goto refresh;
	}

	return false;

refresh:
	
	if( cDisView != NULL ) cDisView->gotoPC();
	if( cMemView != NULL ) cMemView->gotoAddr(sh4_cpu->GetRegister(reg_pc));
	RefreshDebugger(hWnd);
	return true;
}


///////////////////////////////////////////////////////////////////////

char szBuf[256];	// more than big enough for this
char *xs( u32 x ) { sprintf(szBuf, "%X", x); return szBuf; }
char *fls( float f ) { sprintf(szBuf, "%f", f); return szBuf; }
char *x8s( u32 x ) { sprintf(szBuf, "%08X", x); return szBuf; }

f32 GetXf(u32 r)
{
	u32 temp=sh4_cpu->GetRegister((Sh4RegType)(xf_0+r));
	return *(float*)&temp;
}
f32 GetFr(u32 r)
{
	u32 temp=sh4_cpu->GetRegister((Sh4RegType)(fr_0+r));
	return *(float*)&temp;
}

f64 dbGetXD(u32 r)
{
	double t;
	((u32*)(&t))[1]=sh4_cpu->GetRegister((Sh4RegType)(xf_0+(r<<1)));
	((u32*)(&t))[0]=sh4_cpu->GetRegister((Sh4RegType)(xf_0+(r<<1)+1));
	return t;
}
f64 dbGetDR(u32 r)
{
	double t;
	((u32*)(&t))[1]=sh4_cpu->GetRegister((Sh4RegType)(fr_0+(r<<1)));
	((u32*)(&t))[0]=sh4_cpu->GetRegister((Sh4RegType)(fr_0+(r<<1)+1));
	return t;
}

void RefreshDebugger(HWND hDlg)
{
	
	SetDlgItemText( hDlg, IDS_PC, x8s(sh4_cpu->GetRegister(reg_pc) ));
	SetDlgItemText( hDlg, IDS_PR, x8s(sh4_cpu->GetRegister(reg_pr) ));

	SetDlgItemText( hDlg, IDS_GBR, x8s(sh4_cpu->GetRegister(reg_gbr) ));
	SetDlgItemText( hDlg, IDS_SSR, x8s(sh4_cpu->GetRegister(reg_ssr) ));
	SetDlgItemText( hDlg, IDS_SPC, x8s(sh4_cpu->GetRegister(reg_spc) ));
	SetDlgItemText( hDlg, IDS_VBR, x8s(sh4_cpu->GetRegister(reg_vbr) ));
	SetDlgItemText( hDlg, IDS_SGR, x8s(sh4_cpu->GetRegister(reg_sgr) ));
	SetDlgItemText( hDlg, IDS_DBR, x8s(sh4_cpu->GetRegister(reg_dbr) ));

	SetDlgItemText( hDlg, IDS_MACH, x8s(sh4_cpu->GetRegister(reg_mach) ));
	SetDlgItemText( hDlg, IDS_MACL, x8s(sh4_cpu->GetRegister(reg_macl) ));
	SetDlgItemText( hDlg, IDS_FPUL, x8s(sh4_cpu->GetRegister(reg_fpul) ));
	SetDlgItemText( hDlg, IDS_FPSCR, x8s(sh4_cpu->GetRegister(reg_fpscr) ));

	StatusReg sr;
	sr.SetFull(sh4_cpu->GetRegister(reg_sr));

	SetDlgItemText( hDlg, IDS_SR_T, (sr.T?"1":"0") );
	SetDlgItemText( hDlg, IDS_SR_S, (sr.S?"1":"0") );
	SetDlgItemText( hDlg, IDS_SR_M, (sr.M?"1":"0") );
	SetDlgItemText( hDlg, IDS_SR_Q, (sr.Q?"1":"0") );
	SetDlgItemText( hDlg, IDS_SR_BL, (sr.BL?"1":"0") );
	SetDlgItemText( hDlg, IDS_SR_RB, (sr.RB?"1":"0") );
	SetDlgItemText( hDlg, IDS_SR_MD, (sr.MD?"1":"0") );
	SetDlgItemText( hDlg, IDS_SR_FD, (sr.FD?"1":"0") );
	SetDlgItemText( hDlg, IDS_SR_IMASK, xs(sr.IMASK) );

	char tbuff[1024*80];
	GetCallStackText(&tbuff[0]);
	SetDlgItemText( hDlg, IDC_CALLSTACK,&tbuff[0] );


	for( u32 i=0; i<16; i++ )
		SetDlgItemText( hDlg, IDS_R0+i, x8s(sh4_cpu->GetRegister((Sh4RegType)(r0+i)) ));

	for( u32 i=0; i<8; i++ )
		SetDlgItemText( hDlg, IDS_B0+i, x8s(sh4_cpu->GetRegister((Sh4RegType)(r0_Bank+i)) ));


	////////////////////////////////////////////////////////
	u32 fpm =	IsDlgButtonChecked(hDlg, IDC_XF) ? (4) : (0);
	fpm |=	IsDlgButtonChecked(hDlg, IDC_FPSEL3) ? (3) :
			IsDlgButtonChecked(hDlg, IDC_FPSEL2) ? (2) : (1) ;

	char szFPU[1024];
	sprintf(szFPU, "FPU Regs\n========\nFPSCR: %08X \nFPUL: %08X\n", sh4_cpu->GetRegister(reg_fpscr), sh4_cpu->GetRegister(reg_fpul));

	switch( fpm &3 )	// IDC_FPSEL
	{
	case 3:	// Vector: XMTX | FV
		if(fpm&4) {
			sprintf(szBuf, "XMTX\n"
				"(%.3f\t%.3f\t%.3f\t%.3f)\n" "(%.3f\t%.3f\t%.3f\t%.3f)\n"
				"(%.3f\t%.3f\t%.3f\t%.3f)\n" "(%.3f\t%.3f\t%.3f\t%.3f)\n\n",
				GetXf(0), GetXf(4), GetXf(8), GetXf(12), GetXf(1), GetXf(5), GetXf(9), GetXf(13),
				GetXf(2), GetXf(6), GetXf(10), GetXf(14), GetXf(3), GetXf(7), GetXf(11), GetXf(15) );
			strcat(szFPU,szBuf);
		} else {
			sprintf(szBuf,
				"FV0 :\n%.3f\t%.3f\t%.3f\t%.3f\n" "FV4 :\n%.3f\t%.3f\t%.3f\t%.3f\n"
				"FV8 :\n%.3f\t%.3f\t%.3f\t%.3f\n" "FV12:\n%.3f\t%.3f\t%.3f\t%.3f\n\n",
				GetFr(0), GetFr(4), GetFr(8), GetFr(12),	GetFr(1), GetFr(5), GetFr(9), GetFr(13),
				GetFr(2), GetFr(6), GetFr(10), GetFr(14),GetFr(3), GetFr(7), GetFr(11), GetFr(15) );
			strcat(szFPU,szBuf);
		}
	break;

	case 2:	// Double: XD | DR
		for( u32 i=0; i<8; i++ ) {
			sprintf(szBuf, " %s%02d :%+G \n", (fpm&4)?"XD":"DR", (i), ((fpm&4)? dbGetXD(i) : dbGetDR(i)) );
			strcat(szFPU,szBuf);
		}
	break;

	case 1:	// Single: XF | FR
	default:	// default to single precision mode | wtf is the first entry non working for ?
		for( u32 i=0; i<16; i+=2 ) {
			sprintf(szBuf, " %s%02d :%+G \t", ((fpm&4)?"XF":"FR"), (i+0), ((fpm&4)? GetXf(i+0) : GetFr(i+0)) );	strcat(szFPU,szBuf);
			sprintf(szBuf, " %s%02d :%+G \n", ((fpm&4)?"XF":"FR"), (i+1), ((fpm&4)? GetXf(i+1) : GetFr(i+1)) );	strcat(szFPU,szBuf);
		}
	break;
	}

	SetDlgItemText( hDlg, IDS_FPUREGS, szFPU );
	///////////////////////////////////////////

//	SetDlgItemText( hDlg, IDS_CCR, x8s(ccr) );
//	SetDlgItemText( hDlg, IDS_QACR0, x8s(g_dwQACR0) );
//	SetDlgItemText( hDlg, IDS_QACR1, x8s(g_dwQACR1) );

//	SetDlgItemText( hDlg, IDS_MMUCR, x8s(g_dwMMUCR) );

}



inline static void RefreshArmDbg(void)
{

}


INT_PTR CALLBACK ArmDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		return TRUE;

	default: return FALSE;
	}


}
char SelectedPlugin_Aica[512]={0};
char SelectedPlugin_ExtDev[512]={0};
char SelectedPlugin_Pvr[512]={0};
char SelectedPlugin_Gdr[512]={0};
char SelectedPlugin_maple[4][6][512]={0};

int IDC_maple[6]=
{
	IDC_MAPLESUB0,
	IDC_MAPLESUB1,
	IDC_MAPLESUB2,
	IDC_MAPLESUB3,
	IDC_MAPLESUB4,
	IDC_MAPLEMAIN,
};

int current_maple_port=0;
void InitMaplePorts(HWND hw)
{
	int idef=0;
	int i2 = ComboBox_AddString(hw,"A"); 
	ComboBox_SetItemData(hw, i2, 0); 
	idef=i2;

	i2 = ComboBox_AddString(hw,"B"); 
	ComboBox_SetItemData(hw, i2, 1); 

	i2 = ComboBox_AddString(hw,"C"); 
	ComboBox_SetItemData(hw, i2, 2); 

	i2 = ComboBox_AddString(hw,"D"); 
	ComboBox_SetItemData(hw, i2, 3); 
	
	ComboBox_SetCurSel(hw,idef);
}
void SetSelected(HWND hw,char* selected)
{
	int item_count=ComboBox_GetCount(hw);
	for (int i=0;i<item_count;i++)
	{
		char * it=(char*)ComboBox_GetItemData(hw,i);
		if (strcmp(it,selected)==0)
		{
			ComboBox_SetCurSel(hw,i);
			return;
		}
	}
	ComboBox_SetCurSel(hw,0);
}

template<typename T>
void AddItemsToCB(List<T>* list,HWND hw,char* selected)
{
		for (u32 i=0;i<list->itemcount;i++)
		{
			char temp[512];
			char dll[512];
			GetFileNameFromPath((*list)[i].dll,dll);
			sprintf(temp,"%s v%d.%d.%d (%s)",(*list)[i].Name
				,(*list)[i].PluginVersion.major
				,(*list)[i].PluginVersion.minnor
				,(*list)[i].PluginVersion.build
				,dll);
			
			size_t dll_len=strlen(dll);
			char* lp = (char *)malloc(dll_len+1); 
			strcpy(lp,dll);
			int i2 = ComboBox_AddString(hw, temp); 
			ComboBox_SetItemData(hw, i2, lp); 
		}
		SetSelected(hw,selected);
}

void AddMapleItemsToCB(List<MapleDeviceDefinition>* list,HWND hw,char* selected)
{
		char temp[512]="None";
		char dll[512]="NULL";

		char* lp = (char * )malloc(5); 
		strcpy(lp,dll);
		
		int i2 = ComboBox_AddString(hw, temp); 
		ComboBox_SetItemData(hw, i2, lp); 

		AddItemsToCB(list,hw,selected);
}

void GetCurrent(HWND hw,char* dest)
{
	int sel=ComboBox_GetCurSel(hw);
	char* source=(char*)ComboBox_GetItemData(hw,sel);
	if (source==0 || source==(((char*)0)-1))
		source="";
	strcpy(dest,source);
}

void SetMapleMain_Mask(char* plugin,HWND hWnd)
{
	if (strcmp(plugin,"NULL")==0)
	{
		for (int j=0;j<5;j++)
		{
			SetSelected(GetDlgItem(hWnd,IDC_maple[j]),"NULL");
			ComboBox_Enable(GetDlgItem(hWnd,IDC_maple[j]),FALSE);
		}
	}
	else
	{
		List<MapleDeviceDefinition>* lst=GetMapleDeviceList(MDT_Main);
		int i;
		for (i=0;i<lst->size();i++)
		{
			if (strcmp(plugin,(*lst)[i].dll)==0)
				break;
		}
		if (i==lst->size())
		{
			//wtf ?
			return;
		}

		for (int j=0;j<5;j++)
		{
			if ((*lst)[i].Flags & (1<<j))
			{
				ComboBox_Enable(GetDlgItem(hWnd,IDC_maple[j]),TRUE);
			}
			else
			{
				SetSelected(GetDlgItem(hWnd,IDC_maple[j]),"NULL");
				ComboBox_Enable(GetDlgItem(hWnd,IDC_maple[j]),FALSE);
			}
		}
	}
}
void UpdateMapleSelections(HWND hw,HWND hWnd)
{
	LRESULT new_port=TabCtrl_GetCurSel(hw);

	//save selected ones
	if (current_maple_port!=-1)
	{
		for (int j=0;j<6;j++)
		{
			GetCurrent(GetDlgItem(hWnd,IDC_maple[j]),SelectedPlugin_maple[current_maple_port][j]);
		}
	}
	//load new ones
	for (int j=0;j<6;j++)
	{
		SetSelected(GetDlgItem(hWnd,IDC_maple[j]),SelectedPlugin_maple[new_port][j]);
	}
	SetMapleMain_Mask(SelectedPlugin_maple[new_port][5],hWnd);
	current_maple_port=new_port;
}
void SaveMaple()
{
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<6;j++)
		{
			char temp[512];
			sprintf(temp,"Current_maple%d_%d",i,j);
			cfgSaveStr("nullDC_plugins",temp,SelectedPlugin_maple[i][j]);
		}
	}
}

void LoadMaple()
{
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<6;j++)
		{
			char temp[512];
			sprintf(temp,"Current_maple%d_%d",i,j);
			cfgLoadStr("nullDC_plugins",temp,SelectedPlugin_maple[i][j],"NULL");
		}
	}
}
INT_PTR CALLBACK PluginDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{

		TCITEM tci; 
		tci.mask = TCIF_TEXT | TCIF_IMAGE; 
		tci.iImage = -1; 
		tci.pszText = "Port A"; 
		TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_MAPLETAB), 0, &tci); 
		tci.pszText = "Port B"; 
		TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_MAPLETAB), 1, &tci); 
		tci.pszText = "Port C"; 
		TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_MAPLETAB), 2, &tci); 
		tci.pszText = "Port D"; 
		TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_MAPLETAB), 3, &tci); 

		current_maple_port=-1;
		List<PluginLoadInfo>* pvr= GetPluginList(Plugin_PowerVR);	
		List<PluginLoadInfo>* gdrom= GetPluginList(Plugin_GDRom);
		List<PluginLoadInfo>* aica= GetPluginList(Plugin_AICA);
		List<PluginLoadInfo>* extdev= GetPluginList(Plugin_ExtDevice);

		List<MapleDeviceDefinition>* MapleMain=GetMapleDeviceList(MDT_Main);
		List<MapleDeviceDefinition>* MapleSub=GetMapleDeviceList(MDT_Sub);

		char temp[512];

		
		cfgLoadStr("nullDC_plugins","Current_PVR",temp,"NULL");
		AddItemsToCB(pvr,GetDlgItem(hWnd,IDC_C_PVR),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_PVR),SelectedPlugin_Pvr);

		cfgLoadStr("nullDC_plugins","Current_GDR",temp,"NULL");
		AddItemsToCB(gdrom,GetDlgItem(hWnd,IDC_C_GDR),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_GDR),SelectedPlugin_Gdr);

		cfgLoadStr("nullDC_plugins","Current_AICA",temp,"NULL");
		AddItemsToCB(aica,GetDlgItem(hWnd,IDC_C_AICA),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_AICA),SelectedPlugin_Aica);

		cfgLoadStr("nullDC_plugins","Current_ExtDevice",temp,"NULL");
		AddItemsToCB(extdev,GetDlgItem(hWnd,IDC_C_EXTDEV),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_EXTDEV),SelectedPlugin_ExtDev);

		AddMapleItemsToCB(MapleMain,GetDlgItem(hWnd,IDC_MAPLEMAIN),"NONE");

		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB0),"NONE");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB1),"NONE");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB2),"NONE");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB3),"NONE");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB4),"NONE");

		delete gdrom,pvr,aica,MapleMain,MapleSub;

		LoadMaple();
		InitMaplePorts(GetDlgItem(hWnd,IDC_MAPLETAB));
		UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLETAB),hWnd);
		}
		return true;
		
	case WM_COMMAND:

		switch( LOWORD(wParam) )
		{
		case IDC_C_AICA:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				GetCurrent(GetDlgItem(hWnd,IDC_C_AICA),SelectedPlugin_Aica);
			break;
		case IDC_C_GDR:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				GetCurrent(GetDlgItem(hWnd,IDC_C_GDR),SelectedPlugin_Gdr);
			break;
		case IDC_C_PVR:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				GetCurrent(GetDlgItem(hWnd,IDC_C_PVR),SelectedPlugin_Pvr);
			break;
		case IDC_MAPLEPORT:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLETAB),hWnd);
			break;
		case IDC_MAPLEMAIN:
			if (HIWORD(wParam)==CBN_SELCHANGE)
			{
				char temp[512];
				GetCurrent(GetDlgItem(hWnd,IDC_MAPLEMAIN),temp);
				SetMapleMain_Mask(temp,hWnd);
				//ENABLE/DISABLE THINGS	
			}
			//	UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLETAB),hWnd);
			break;

		case IDC_C_EXTDEV:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				GetCurrent(GetDlgItem(hWnd,IDC_C_EXTDEV),SelectedPlugin_ExtDev);
			break;

		case IDOK:
			//save settings
			cfgSaveStr("nullDC_plugins","Current_PVR",SelectedPlugin_Pvr);
			cfgSaveStr("nullDC_plugins","Current_GDR",SelectedPlugin_Gdr);
			cfgSaveStr("nullDC_plugins","Current_AICA",SelectedPlugin_Aica);
			cfgSaveStr("nullDC_plugins","Current_ExtDevice",SelectedPlugin_ExtDev);
			UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLEPORT),hWnd);
			SaveMaple();
			plgdlg_cancel=false;
			EndDialog(hWnd,0);
			return true;

		case IDCANCEL://close plugin
			plgdlg_cancel=true;
			EndDialog(hWnd,0);
			return true;
/*
		case IDC_EXTDEV_CONF:
			{
				nullDC_ExtDevice_plugin t;
				if (t.Load(SelectedPlugin_ExtDev)==PluginLoadError::NoError)
				{
//					t.info.ShowConfig(PluginType::ExtDevice,hWnd);
				}
				else
				{
					//error
					printf("Failed to load \"%s\"\n",SelectedPlugin_Aica);
				}
			}
			break;

		case IDC_AICA_CONF:
			{
				nullDC_AICA_plugin t;
				if (t.Load(SelectedPlugin_Aica)==PluginLoadError::NoError)
				{
					//t.info.ShowConfig(PluginType::AICA,hWnd);
				}
				else
				{
					//error
					printf("Failed to load \"%s\"\n",SelectedPlugin_Aica);
				}
			}
			break;

		case IDC_PVR_CONF:
			{
				nullDC_PowerVR_plugin t;
				if (t.Load(SelectedPlugin_Pvr)==PluginLoadError::NoError)
				{
					//t.info.ShowConfig(PluginType::PowerVR,hWnd);
				}
				else
				{
					//error
					printf("Failed to load \"%s\"\n",SelectedPlugin_Pvr);
				}
			}
			break;

		case IDC_GDR_CONF:
			{
				nullDC_GDRom_plugin t;
				if (t.Load(SelectedPlugin_Gdr)==PluginLoadError::NoError)
				{
					//t.info.ShowConfig(PluginType::GDRom,hWnd);
				}
				else
				{
					//error
					printf("Failed to load \"%s\"\n",SelectedPlugin_Gdr);
				}
			}
			break;*/
		default: break;
		}
		return false;
	case WM_NOTIFY:
		{
			if ( ((LPNMHDR)lParam)->idFrom==IDC_MAPLETAB && 
				 ((LPNMHDR)lParam)->code == TCN_SELCHANGE  )
			{
				UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLETAB),hWnd);
			}
			return true;
		}
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hWnd,0);
		return true;

	default: break;
	}

	return false;
}