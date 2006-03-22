#include "..\types.h"
#include "base.h"
#include "..\dc\maple\maple_if.h"
#include "..\dc\dc.h"
#include "..\plugins\plugin_manager.h"

#include <windows.h>
#include "emuWinUI.h"

//LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
//int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance,LPSTR lpcmdline,int nshowcmd);

//HWND mainhwnd;
//HINSTANCE hinst;

bool CreateGUI()
{
	/*WNDCLASSEXA window;	

	//fill in window class
	window.cbClsExtra = 0;//extra stuff
	window.cbSize = sizeof(WNDCLASSEXA);//set to sized of wndclassex
	window.cbWndExtra = 0;//more extra stuff
	window.hbrBackground =(HBRUSH)COLOR_BTNSHADOW;//set background
	window.hCursor = LoadCursor(NULL, IDC_ARROW);//load cursor
	window.hIcon = 0;//load icon
	window.hIconSm = 0;//load icon small
	window.hInstance = 0; //app instance
	window.lpfnWndProc = WinProc; //name of proc
	window.lpszClassName = "WINCLASS1";//name of class
	window.lpszMenuName = "MENU";
	window.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;// style of the stuff these being most used

	if(!RegisterClassExA(&window))//register window class
		return false;
	
	//create window
	mainhwnd = CreateWindowExA(NULL,"WINCLASS1","Huh ?",WS_OVERLAPPEDWINDOW|WS_VISIBLE | CS_OWNDC,0,0,640,480,NULL,NULL,NULL,NULL);
	SetWindowTextA(mainhwnd,"Huh ?");//wtf ?
	if(!(mainhwnd))
		return false;

	return true;*/
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

	/*MSG msg;
	//main event loop
	while(sh4_cpu->IsCpuRunning())
	{
		//test if theres a message
		WaitMessage();
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
			//check for wm_quit
			if(msg.message == WM_QUIT)
				break;
			//translate accelerator keys
			TranslateMessage(&msg);

			//send message to win proc
			DispatchMessage(&msg);
		}
	}
	if (sh4_cpu->IsCpuRunning())
		sh4_cpu->Stop();

//	return (int)(msg.wParam);*/
}
/*
LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
	switch(msg){
	case WM_DESTROY:
		{
			//kills app
			PostQuitMessage(0);
			return 0;
		}
	case WM_COMMAND:
		{
			
			
		}
		
	case WM_KEYDOWN:
		switch(wparam)
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
		switch(wparam)
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

	default:
		break;
	}

	// take care of other messages
	return(DefWindowProc(hwnd,msg,wparam,lparam));
}
*/

			
void* GetRenderTargetHandle()
{
	return g_hWnd;
}