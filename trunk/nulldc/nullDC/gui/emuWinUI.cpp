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


/////////////////////////////
#include "DBG\\CtrlMemView.h"
#include "DBG\\CtrlDisAsmView.h"
//////////////////////////////////

#include "screenshot.h"

/// i dont like it but ....
CtrlMemView *cMemView;
CtrlDisAsmView *cDisView;
/////////////////////////

HWND g_hWnd;
HINSTANCE g_hInst;

TCHAR g_szFileName[MAX_PATH];

INT_PTR CALLBACK DlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK ArmDlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK DlgProcModal( HWND, UINT, WPARAM, LPARAM );
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
 
u32 uiInit(void)
{
	WNDCLASS wc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor			= LoadCursor(g_hInst, IDC_ARROW);
	wc.hIcon			= 0;//LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
	wc.hInstance		= g_hInst;
	wc.lpfnWndProc		= WndProc;
	wc.lpszClassName	= "Debugger";
	wc.lpszMenuName		= MAKEINTRESOURCE(IDR_FMENU);
	wc.style			= CS_VREDRAW | CS_HREDRAW ;

	if( !RegisterClass(&wc) ) {
		MessageBox( NULL, "Couldn't Register DbgWnd Class !","ERROR",MB_ICONERROR );
		return false;
	}

	InitCommonControls();

	g_hWnd = CreateWindow( "Debugger", "nullDC v1.0.0 beta", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, 640,480, NULL, NULL, g_hInst, NULL );
	if( !IsWindow(g_hWnd) ) {
		MessageBox( NULL, "Couldn't Create Debug Window!","ERROR",MB_ICONERROR );
		return false;
	}

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

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static RECT rc;
	static TCHAR szFile[128];
	static OPENFILENAME ofn;

	switch(uMsg)
	{
	case WM_CREATE:
		InitCommonControls();
		return 0;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
			//////// FILE MENU
		case ID_FILE_OPENBIN:
			if (!sh4_cpu->IsCpuRunning())	//if cpu is stoped
			{
				ZeroMemory(&ofn, sizeof(OPENFILENAME));
				ofn.lStructSize		= sizeof(OPENFILENAME);
				ofn.hwndOwner		= hWnd;
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
					Init_DC();
					Reset_DC(false);
					if(!LoadBinfileToSh4Mem(0x10000, g_szFileName))
						return 0;
					EnablePatch(patch_resets_Misc);//mwhaha
					sh4_cpu->Reset(false);//do a hard reset
					sh4_cpu->SetRegister(Sh4RegType::reg_sr,0x70000000);
					sh4_cpu->SetRegister(Sh4RegType::reg_gbr,0x8c000000);
					sh4_cpu->SetRegister(Sh4RegType::reg_pc,0x8c008300);
					Start_DC();
					return 0;

				}
			}
			//add warn message
			return 0;

		case ID_FILE_LOADBIN:
			//if (!sh4_cpu->IsCpuRunning())	//if cpu is stoped
			{
				ZeroMemory(&ofn, sizeof(OPENFILENAME));
				ofn.lStructSize		= sizeof(OPENFILENAME);
				ofn.hwndOwner		= hWnd;
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
						return 0;
					return 0;
				}
			}
			//add warn message
			return 0;

		case ID_FILE_BOOTHLE:
		 {
			 Init_DC();
			 Reset_DC(false);
			 if (gdBootHLE()==false)
			 {
				 MessageBox(hWnd,"Failed to find ip.bin/bootfile\nTry to boot using the Normal boot method.","HLE Boot Error",MB_ICONEXCLAMATION | MB_OK);
				 return 0;
			 }
			 EnablePatch(patch_resets_Misc);//mwhaha
			 sh4_cpu->Reset(false);//do a hard reset
			 sh4_cpu->SetRegister(Sh4RegType::reg_sr,0x70000000);
			 sh4_cpu->SetRegister(Sh4RegType::reg_gbr,0x8c000000);
			 sh4_cpu->SetRegister(Sh4RegType::reg_pc,0x8c008300);
			 Start_DC();
			 return 0;
		 }

		case ID_FILE_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0,0);
			return 0;

			///////// SYSTEM MENU
		case ID_FILE_NORMALBOOT:
		case ID_SYSTEM_START:
			Start_DC();
			return 0;

		case ID_SYSTEM_STOP:
			Stop_DC();
			return 0;

		case ID_SYSTEM_RESET:
			sh4_cpu->Stop();
			printf(">>\tDreamcast Reset\n");
			sh4_cpu->Reset(false);//do a hard reset
			sh4_cpu->SetRegister(Sh4RegType::reg_pc,0xA0000000);
			DisablePatch(patch_all);
			return 0;


			//////// DEBUG MENU
		case ID_DEBUG_DEBUGGER:
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
				MessageBox( hWnd, "Couldn't open Sh4 debugger","",MB_OK );

			return 0;

		case ID_DEBUG_ARM7DEBUGGER:

			hDebugger = CreateDialog( g_hInst, MAKEINTRESOURCE(IDD_ARM7DEBUG), NULL, ArmDlgProc);
			if( !IsWindow(hDebugger) )
				MessageBox( hWnd, "Couldn't open ARM7 debugger","",MB_OK );

			return 0;

			/////// HELP MENU
		case ID_HELP_ABOUT:
			DialogBox(NULL,MAKEINTRESOURCE(IDD_ABOUT),hWnd,DlgProcModal);
			return 0;

		case ID_OPTIONS_CONFIG:
			DialogBox(NULL,MAKEINTRESOURCE(IDD_CONFIG),hWnd,DlgProcModal);
			return 0;

		//Plugin Selection Menu
		case ID_OPTIONS_SELECTPLUGINS:
			DialogBox(NULL,MAKEINTRESOURCE(IDD_PLUGIN_SELECT),hWnd,PluginDlgProc);
			return 0;

		}
		break;


	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SIZE:
	//	GetClientRect(hWnd, &rc);
//		PvrPlugin.UpdatePvr(0);
		//TODO : Fix that
	//	PvrUpdate(0);
	case WM_KEYDOWN:
		{
			int val = (int)wParam;
			switch(val)
			{
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


INT_PTR CALLBACK DlgProcModal( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			int tmp;
			tmp=cfgLoadInt("nullDC","enable_recompiler");
			CheckDlgButton(hWnd,IDC_REC,tmp==1?BST_CHECKED:BST_UNCHECKED);
		}
		return true;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDOK:
			if(IsWindow(GetDlgItem(hWnd,IDC_REC)))	// this is Config dialog
			{
				int tmp = 0;
				tmp = (BST_CHECKED==IsDlgButtonChecked(hWnd,IDC_REC)) ? 1 : 0 ;
				cfgSaveInt("nullDC","enable_recompiler",tmp);
				bool bStart=false;
				if (sh4_cpu)
				{
					if (sh4_cpu->IsCpuRunning())
					{
						//SwitchCPU_DC();
						//sh4_cpu->Stop();
						Stop_DC();
						bStart=true;
					}
				}
				if(0 != cfgLoadInt("nullDC","enable_recompiler"))
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
					//SwitchCPU_DC();
					Start_DC();
					//sh4_cpu->Run();
					
				}
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
char SelectedPlugin_Pvr[512]={0};
char SelectedPlugin_Gdr[512]={0};
char SelectedPlugin_maple[4][6][512]={0};

int IDC_maple[6]=
{
	IDC_MAPLEMAIN,
	IDC_MAPLESUB0,
	IDC_MAPLESUB1,
	IDC_MAPLESUB2,
	IDC_MAPLESUB3,
	IDC_MAPLESUB4
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

void AddMapleItemsToCB(List<MapleDeviceLoadInfo>* list,HWND hw,char* selected)
{
		char temp[512]="None";
		char dll[512]="NULL";

		char* lp = (char * )malloc(5); 
		strcpy(lp,dll);
		
		int i2 = ComboBox_AddString(hw, temp); 
		ComboBox_SetItemData(hw, i2, lp); 

		for (u32 i=0;i<list->itemcount;i++)
		{

			GetFileNameFromPath((*list)[i].dll,dll);
			sprintf(temp,"%s v%d.%d.%d (%s:%d)",(*list)[i].name
				,(*list)[i].PluginVersion.major
				,(*list)[i].PluginVersion.minnor
				,(*list)[i].PluginVersion.build
				,dll,(*list)[i].id);
			
			size_t dll_len=strlen(dll);
			lp = (char * )malloc(dll_len+1+2); 
			//strcpy(lp,dll);
			sprintf(lp,"%s:%d",dll,(*list)[i].id);
			i2 = ComboBox_AddString(hw, temp); 
			ComboBox_SetItemData(hw, i2, lp); 
		}

		ComboBox_SetCurSel(hw,0);
}

void AddItemsToCB(List<PluginLoadInfo>* list,HWND hw,char* selected)
{
		for (u32 i=0;i<list->itemcount;i++)
		{
			char temp[512];
			char dll[512];
			GetFileNameFromPath((*list)[i].dll,dll);
			sprintf(temp,"%s v%d.%d.%d (%s)",(*list)[i].plugin_info.Name
				,(*list)[i].plugin_info.PluginVersion.major
				,(*list)[i].plugin_info.PluginVersion.minnor
				,(*list)[i].plugin_info.PluginVersion.build
				,dll);
			
			size_t dll_len=strlen(dll);
			char* lp = (char *)malloc(dll_len+1); 
			strcpy(lp,dll);
			int i2 = ComboBox_AddString(hw, temp); 
			ComboBox_SetItemData(hw, i2, lp); 
		}
		SetSelected(hw,selected);
}

void GetCurrent(HWND hw,char* dest)
{
	int sel=ComboBox_GetCurSel(hw);
	char* source=(char*)ComboBox_GetItemData(hw,sel);
	if (source==0 || source==(((char*)0)-1))
		source="";
	strcpy(dest,source);
}
void UpdateMapleSelections(HWND hw,HWND hWnd)
{
	int cs=ComboBox_GetCurSel(hw);
	LRESULT new_port=ComboBox_GetItemData(hw,cs);
//	char temp[512];
	
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
	current_maple_port=new_port;
	//cfgSaveStr("ASD","Asd","asd");
}
void SaveMaple()
{
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<6;j++)
		{
			char temp[512];
//			char plugin[512];
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
//			char plugin[512];
			sprintf(temp,"Current_maple%d_%d",i,j);
			cfgLoadStr("nullDC_plugins",temp,SelectedPlugin_maple[i][j]);
		}
	}
}
INT_PTR CALLBACK PluginDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{

		current_maple_port=-1;
		List<PluginLoadInfo>* pvr= EnumeratePlugins(PluginType::PowerVR);	
		List<PluginLoadInfo>* gdrom= EnumeratePlugins(PluginType::GDRom);
		List<PluginLoadInfo>* aica= EnumeratePlugins(PluginType::AICA);

		char temp[512];

		
		cfgLoadStr("nullDC_plugins","Current_PVR",temp);
		AddItemsToCB(pvr,GetDlgItem(hWnd,IDC_C_PVR),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_PVR),SelectedPlugin_Pvr);

		cfgLoadStr("nullDC_plugins","Current_GDR",temp);
		AddItemsToCB(gdrom,GetDlgItem(hWnd,IDC_C_GDR),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_GDR),SelectedPlugin_Gdr);

		cfgLoadStr("nullDC_plugins","Current_AICA",temp);
		AddItemsToCB(aica,GetDlgItem(hWnd,IDC_C_AICA),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_AICA),SelectedPlugin_Aica);

		
		delete gdrom,pvr,aica;

		List<MapleDeviceLoadInfo>* MapleMain=GetMapleMainDevices();
		List<MapleDeviceLoadInfo>* MapleSub=GetMapleSubDevices();
		AddMapleItemsToCB(MapleMain,GetDlgItem(hWnd,IDC_MAPLEMAIN),"NONE");

		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB0),"NONE");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB1),"NONE");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB2),"NONE");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB3),"NONE");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB4),"NONE");
		LoadMaple();
		InitMaplePorts(GetDlgItem(hWnd,IDC_MAPLEPORT));
		UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLEPORT),hWnd);
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
				UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLEPORT),hWnd);
			break;

		case IDOK:
			//save settings
			cfgSaveStr("nullDC_plugins","Current_PVR",SelectedPlugin_Pvr);
			cfgSaveStr("nullDC_plugins","Current_GDR",SelectedPlugin_Gdr);
			cfgSaveStr("nullDC_plugins","Current_AICA",SelectedPlugin_Aica);
			UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLEPORT),hWnd);
			SaveMaple();
		case IDCANCEL://close plugin
			EndDialog(hWnd,0);
			return true;

		case IDC_AICA_CONF:
			{
				nullDC_AICA_plugin t;
				if (t.LoadnullDCPlugin(SelectedPlugin_Aica)==PluginLoadError::NoError)
				{
					t.info.ShowConfig(PluginType::AICA,hWnd);
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
				if (t.LoadnullDCPlugin(SelectedPlugin_Pvr)==PluginLoadError::NoError)
				{
					t.info.ShowConfig(PluginType::PowerVR,hWnd);
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
				if (t.LoadnullDCPlugin(SelectedPlugin_Gdr)==PluginLoadError::NoError)
				{
					t.info.ShowConfig(PluginType::GDRom,hWnd);
				}
				else
				{
					//error
					printf("Failed to load \"%s\"\n",SelectedPlugin_Gdr);
				}
			}
			break;
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