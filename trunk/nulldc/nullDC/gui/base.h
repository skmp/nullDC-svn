#pragma once
#include "types.h"
#include "dc/sh4/sh4_if.h"
#include "plugins/plugin_manager.h"

bool CreateGUI();
void DestroyGUI();
void GuiLoop();

void* EXPORT_CALL GetRenderTargetHandle();
bool SelectPluginsGui();
void EmuEventBroadcast();

#ifndef _MenuItemSelectedFP_
#define _MenuItemSelectedFP_
typedef void EXPORT_CALL MenuItemSelectedFP(u32 id,void* WindowHandle,void* user);


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
//void EXPORT_CALL DeleteAllMenuItemChilds(u32 id);
void SetMenuItemHandler(u32 id,MenuItemSelectedFP* h);
/*
u32 EXPORT_CALL AddMenuItem(u32 parent,s32 pos,char* text,MenuItemSelectedFP* handler ,u32 checked);
void EXPORT_CALL SetMenuItemStyle(u32 id,u32 style,u32 mask);
void EXPORT_CALL GetMenuItem(u32 id,MenuItem* info,u32 mask);
void EXPORT_CALL SetMenuItem(u32 id,MenuItem* info,u32 mask);
void EXPORT_CALL DeleteMenuItem(u32 id);
//These still exist , but are no longer given to plugins

void EXPORT_CALL SetMenuItemBitmap(u32 id,void* hbmp);
u32 EXPORT_CALL GetMenuItemStyle(u32 id);
void* EXPORT_CALL GetMenuItemBitmap(u32 id);
*/
extern MenuIDList MenuIDs;
