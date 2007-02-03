#include "types.h"
#include "base.h"
#include "dc/maple/maple_if.h"
#include "dc/dc.h"
#include "plugins/plugin_manager.h"

#include <windows.h>
#include "emuWinUI.h"

//LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
//int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance,LPSTR lpcmdline,int nshowcmd);

//HWND mainhwnd;
//HINSTANCE hinst;

bool CreateGUI()
{
	if (uiInit()!=UI_OK)
		return false;

	return true;
}
void DestroyGUI()
{
	plugins_Unload();
	uiTerm();
}
void GuiLoop()
{
	while (uiMain()!=UI_MAIN_QUIT)
		__noop;

	Stop_DC();
}

void* GetRenderTargetHandle()
{
	return g_hWnd;
}