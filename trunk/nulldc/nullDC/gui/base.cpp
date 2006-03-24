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
	return uiInit()==UI_OK;
}
void DestroyGUI()
{
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