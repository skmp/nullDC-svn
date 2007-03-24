/*
**	emuWinUI.h
*/

#pragma once // msc > 1000 ?

#ifndef __EMU_WIN_UI_H__
#define __EMU_WIN_UI_H__


# include <windows.h>
# include "resource.h"

u32 uiInit(void);
u32 uiTerm(void);
u32 uiMain(void);	// uiMain(void) this function should pass-thru (not mainloop)

void RefreshDebugger(HWND hDlg);
extern HWND hDebugger;
extern HWND g_hWnd;

void uiGetFN(char *szFileName, char *szParse);
bool SelectPluginsGui();

//	*FIXME* these should be in emuUI.h as they should not be OS dependant

#define E_OK			0
#define E_ERR			1

#define UI_OK			0x00000000	// no errors
#define UI_ERR			0x00000001	// there is some error check the rest of EC per function

#define UI_INIT_XX		0x10FFFF01	// ...
#define UI_TERM_XX		0x20FFFF01	// ...

#define UI_MAIN_QUIT	0x88888889	// EXECUTION IS FINISHED

#ifndef _MenuItemSelectedFP_
#define _MenuItemSelectedFP_
typedef void FASTCALL MenuItemSelectedFP(u32 id,void* WindowHandle,void* user);


enum MenuItemMask
{
	MIM_Text=1,
	MIM_Handler=2,
	MIM_Bitmap=4,
	MIM_Style=8,
	MIM_PUser=16,
	MIM_All=0xFFFFFFFF,
};
struct MenuItem
{
	char* Text;			//Text of the menu item
	MenuItemSelectedFP* Handler;	//called when the menu is clicked
	void* Bitmap;		//bitmap handle
	u32 Style;			//MIS_* combination
	void* PUser;		//User defined pointer :)
};
#endif

u32 FASTCALL AddMenuItem(u32 parent,s32 pos,char* text,MenuItemSelectedFP* handler ,u32 checked);
void FASTCALL DeleteAllMenuItemChilds(u32 id);
void FASTCALL SetMenuItemStyle(u32 id,u32 style,u32 mask);
void FASTCALL GetMenuItem(u32 id,MenuItem* info,u32 mask);
void FASTCALL SetMenuItem(u32 id,MenuItem* info,u32 mask);
void FASTCALL DeleteMenuItem(u32 id);
//These still exist , but are no longer given to plugins
void FASTCALL SetMenuItemHandler(u32 id,MenuItemSelectedFP* h);
void FASTCALL SetMenuItemBitmap(u32 id,void* hbmp);
u32 FASTCALL GetMenuItemStyle(u32 id);
void* FASTCALL GetMenuItemBitmap(u32 id);

extern u32 PowerVR_menu;
extern u32 GDRom_menu;
extern u32 Aica_menu;
extern u32 Maple_menu;
extern u32 Maple_menu_ports[4][6];
extern u32 ExtDev_menu;

#endif // __EMU_WIN_UI_H__