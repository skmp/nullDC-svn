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

//	*FIXME* these should be in emuUI.h as they should not be OS dependant

#define E_OK			0
#define E_ERR			1

#define UI_OK			0x00000000	// no errors
#define UI_ERR			0x00000001	// there is some error check the rest of EC per function

#define UI_INIT_XX		0x10FFFF01	// ...
#define UI_TERM_XX		0x20FFFF01	// ...

#define UI_MAIN_QUIT	0x88888889	// EXECUTION IS FINISHED


#endif // __EMU_WIN_UI_H__