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
#include "dc/dc.h"

/////////////////////////////
#include "DBG\\CtrlMemView.h"
#include "DBG\\CtrlDisAsmView.h"
//////////////////////////////////

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

	g_hWnd = CreateWindow( "Debugger", " emu Debug Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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
		if( WM_QUIT == msg.message )
			return UI_MAIN_QUIT;

		if( !TranslateAccelerator(g_hWnd, hAccel, &msg) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
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
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				if(GetOpenFileName(&ofn)>0)
				{
					if(!LoadBinfileToSh4Mem(0x10000, g_szFileName))
						return 0;
					sh4_cpu->Reset(false);//do a hard reset
					sh4_cpu->SetRegister(Sh4RegType::reg_gbr,0x8c000000);
					sh4_cpu->SetRegister(Sh4RegType::reg_pc,0x8c008300);
					Start_DC();
					return 0;

				}
			}
			//add warn message
			return 0;

		case ID_FILE_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0,0);
			return 0;

			///////// SYSTEM MENU
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
			return 0;


			//////// DEBUG MENU
		case ID_DEBUG_DEBUGGER:

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
		break;
	
	case WM_KEYDOWN:
		switch(wParam)
		{
		case 'Z':
			kcode &= 0xFFFF - key_CONT_A;
			break;
		case 'X':
			kcode &= 0xFFFF - key_CONT_B;
			break;
		case 'C':
			kcode &= 0xFFFF - key_CONT_C;
			break;
		case 'V':
			kcode &= 0xFFFF - key_CONT_D;
			break;
		
		case 'B':
			kcode &= 0xFFFF - key_CONT_Z;
			break;
		
		case 'N':
			kcode &= 0xFFFF - key_CONT_Y;
			break;
		
		case 'M':
			kcode &= 0xFFFF - key_CONT_X;
			break;

		case VK_SHIFT:
			kcode &= 0xFFFF - key_CONT_START;
			break;

		case VK_UP:
			kcode &= 0xFFFF - key_CONT_DPAD_UP;
			break;
		case VK_DOWN:
			kcode &= 0xFFFF - key_CONT_DPAD_DOWN;
			break;
		case VK_LEFT:
			kcode &= 0xFFFF - key_CONT_DPAD_LEFT;
			break;
		case VK_RIGHT:
			kcode &= 0xFFFF - key_CONT_DPAD_RIGHT;
			break;

		case 'K'://analog right
			joyx= +126;
			break;
		case 'H'://alalog left
			joyx= -126;
			break;

		case 'U'://analog up
			joyy= -126;
			break;
		case 'J'://analog down
			joyy= +126;
			break;

		case 'A'://ltriger
			lt=255;
			break;
		case 'S'://rtriger
			rt=255;
			break;
		}
		break;

	case WM_KEYUP:
		switch(wParam)
		{
		case 'Z':
			kcode |= key_CONT_A;
			break;
		case 'X':
			kcode |= key_CONT_B;
			break;
		case 'C':
			kcode |= key_CONT_C;
			break;
		case 'V':
			kcode |= key_CONT_D;
			break;

		case 'B':
			kcode |= key_CONT_Z;
			break;
		
		case 'N':
			kcode |= key_CONT_Y;
			break;
		
		case 'M':
			kcode |= key_CONT_X;
			break;

		case VK_SHIFT:
			kcode |= key_CONT_START;
			break;

		case VK_UP:
			kcode |= key_CONT_DPAD_UP;
			break;
		case VK_DOWN:
			kcode |= key_CONT_DPAD_DOWN;
			break;
		case VK_LEFT:
			kcode |= key_CONT_DPAD_LEFT;
			break;
		case VK_RIGHT:
			kcode |= key_CONT_DPAD_RIGHT;
			break;

		case 'K'://analog right
			joyx=0;
			break;
		case 'H'://alalog left
			joyx=0;
			break;

		case 'U'://analog up
			joyy=0;
			break;
		case 'J'://analog down
			joyy=0;
			break;

		case 'A'://ltriger
			lt=0;
			break;
		case 'S'://rtriger
			rt=0;
			break;
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
	
	if( cDisView != NULL )
		cDisView->gotoPC();
	if( cMemView != NULL )
		cMemView->gotoAddr( sh4_cpu->GetRegister(reg_pc));

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
	sr.full=sh4_cpu->GetRegister(reg_sr);

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

void AddItemsToCB(GrowingList<PluginLoadInfo>* list,HWND hw)
{
		for (u32 i=0;i<list->itemcount;i++)
		{
			char temp[512];
			char dll[512];
			GetFileNameFromPath(list->items[i].item.dll,dll);
			sprintf(temp,"%s v%d.%d.%d (%s)",list->items[i].item.plugin_info.Name
				,list->items[i].item.plugin_info.PluginVersion.major
				,list->items[i].item.plugin_info.PluginVersion.minnor
				,list->items[i].item.plugin_info.PluginVersion.build
				,dll);
			
			char* lp = (char *)malloc(strlen(temp)+8); 
			int i2 = ComboBox_AddString(hw, temp); 
			ComboBox_SetItemData(hw, i2, lp); 
			//if (stricmp(str, lp)==0) 
			//	ComboBox_SetCurSel(hw, i2); 

			//ComboBox_AddString(to,temp);
		}
}
INT_PTR CALLBACK PluginDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
		GrowingList<PluginLoadInfo>* pvr= EnumeratePlugins(PluginType::PowerVR);	
		GrowingList<PluginLoadInfo>* gdrom= EnumeratePlugins(PluginType::GDRom);
		GrowingList<PluginLoadInfo>* aica= EnumeratePlugins(PluginType::AICA);

		AddItemsToCB(pvr,GetDlgItem(hWnd,IDC_C_PVR));
		AddItemsToCB(gdrom,GetDlgItem(hWnd,IDC_C_GDR));
		AddItemsToCB(aica,GetDlgItem(hWnd,IDC_C_AICA));
		
		delete gdrom,pvr,aica;
		}
		return true;
		
	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDOK:
			//save settings
		case IDCANCEL://close plugin
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