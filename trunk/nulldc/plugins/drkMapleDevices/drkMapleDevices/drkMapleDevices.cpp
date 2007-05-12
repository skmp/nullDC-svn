// drkMapleDevices.cpp : Defines the entry point for the DLL application.
//

#include "..\..\..\nullDC\plugins\plugin_header.h"
#include <memory.h>

emu_info host;
#ifdef UNICODE
#undef UNICODE
#endif
#define _WIN32_WINNT 0x500
#include <windowsx.h>
#include <winsock2.h>
#include <windows.h>

#include <ws2tcpip.h>
#include <windowsx.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <commctrl.h>
#include "resource.h"

u16 kcode[4]={0xFFFF,0xFFFF,0xFFFF,0xFFFF};
u32 vks[4]={0};
s8 joyx[4]={0},joyy[4]={0};
s8 joy2x[4]={0},joy2y[4]={0};
u8 rt[4]={0},lt[4]={0};
u32 mo_buttons = 0xFFFFFFFF;
s32 mo_x_delta = 0;
s32 mo_y_delta = 0;
s32 mo_wheel_delta = 0;
#define mo_Middle (1<<0) 
#define mo_Right (1<<1)
#define mo_Left (1<<2)
#define mo_Thumb (1<<3)
//1 Right button (B) 
//2 Left button (A) 
//3 Thumb button (START) 
#define dbgbreak {while(1) __noop;}
#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#pragma pack(1)
char testJoy_strName[64] = "Dreamcast Controller\0";
char testJoy_strName_nul[64] = "Null Dreamcast Controler\0";
char testJoy_strName_net[64] = "Net Dreamcast Controler\0";
char testJoy_strName_vmu[64] = "Emulated VMU\0";
char testJoy_strName_kbd[64] = "Emulated Dreamcast Keyboard\0";
char testJoy_strName_mouse[64] = "Emulated Dreamcast Mouse\0";
char testJoy_strName_dreameye_1[64] = "Dreamcast Camera Flash  Devic\0";
char testJoy_strName_dreameye_2[64] = "Dreamcast Camera Flash LDevic\0";
char testJoy_strName_mic[64] = "MicDevice for Dreameye\0";
char testJoy_strBrand[64] = "Faked by drkIIRaziel && ZeZu , made for nullDC\0";
char testJoy_strBrand_2[64] = "Produced By or Under License From SEGA ENTERPRISES,LTD.\0";

#define key_CONT_C  (1 << 0)
#define key_CONT_B  (1 << 1)
#define key_CONT_A  (1 << 2)
#define key_CONT_START  (1 << 3)
#define key_CONT_DPAD_UP  (1 << 4)
#define key_CONT_DPAD_DOWN  (1 << 5)
#define key_CONT_DPAD_LEFT  (1 << 6)
#define key_CONT_DPAD_RIGHT  (1 << 7)
#define key_CONT_Z  (1 << 8)
#define key_CONT_Y  (1 << 9)
#define key_CONT_X  (1 << 10)
#define key_CONT_D  (1 << 11)
#define key_CONT_DPAD2_UP  (1 << 12)
#define key_CONT_DPAD2_DOWN  (1 << 13)
#define key_CONT_DPAD2_LEFT  (1 << 14)
#define key_CONT_DPAD2_RIGHT  (1 << 15)

#define key_CONT_ANALOG_UP  (1 << 16)
#define key_CONT_ANALOG_DOWN  (1 << 17)
#define key_CONT_ANALOG_LEFT  (1 << 18)
#define key_CONT_ANALOG_RIGHT  (1 << 19)
#define key_CONT_LSLIDER  (1 << 20)
#define key_CONT_RSLIDER  (1 << 21)

struct joy_init_resp
{
	u32 ratio;
	u32 status;
};

struct joy_init
{
	u32 Version;
	char Name[512];
	u32 port;
};

struct joy_state
{
	u32 id;
	u16 state;
	s8 jy;
	s8 jx;
	u8 r;
	u8 l;
};

struct _joypad_settings_entry
{
	u8 KC;
	u32 BIT;
	char* name;
};
#define D(x) x ,#x
_joypad_settings_entry joypad_settings_K[] = 
{
	{'B',D(key_CONT_C)},
	{'X',D(key_CONT_B)},
	{'V',D(key_CONT_A)},
	{VK_SHIFT,D(key_CONT_START)},
	
	{VK_UP,D(key_CONT_DPAD_UP)},
	{VK_DOWN,D(key_CONT_DPAD_DOWN)},
	{VK_LEFT,D(key_CONT_DPAD_LEFT)},
	{VK_RIGHT,D(key_CONT_DPAD_RIGHT)},

	{'M',D(key_CONT_Z)},
	{'Z',D(key_CONT_Y)},
	{'C',D(key_CONT_X)},
	{0,D(key_CONT_DPAD2_UP)},
	{0,D(key_CONT_DPAD2_DOWN)},
	{0,D(key_CONT_DPAD2_LEFT)},
	{0,D(key_CONT_DPAD2_RIGHT)},

	{'I',D(key_CONT_ANALOG_UP)},
	{'K',D(key_CONT_ANALOG_DOWN)},
	{'J',D(key_CONT_ANALOG_LEFT)},
	{'L',D(key_CONT_ANALOG_RIGHT)},

	{'A',D(key_CONT_LSLIDER)},
	{'S',D(key_CONT_RSLIDER)},
	{0,0,0},
};
_joypad_settings_entry joypad_settings[4][32];
#undef D

void LoadSettings();
void SaveSettings();
u8 kb_shift          ; //shift keys pressed (bitmask)	//1
u8 kb_led            ; //leds currently lit			//1
u8 kb_key[6]={0}     ; //normal keys pressed			//6
u8 kb_used=0;
char kb_map[256];

struct VMU_info
{
	u8 data[256*1024];
	char file[512];
};
bool ikbmap=false;
void kb_down(u8 kc)
{
	if (ikbmap==false)
	{
		ikbmap=true;
		void Init_kb_map();
		Init_kb_map();
	}
	if (kc==VK_SHIFT)
		kb_shift|=0x02 | 0x20; //both shifts ;p
	kc=kb_map[kc & 0xFF];
	if (kc==0)
		return;
	if (kb_used<6)
	{
		for (int i=0;i<6;i++)
		{
			if (kb_key[i]==kc)
				return;
		}
		kb_key[kb_used]=kc;
		kb_used++;
	}
}
void kb_up(u8 kc)
{
	if (kc==VK_SHIFT)
		kb_shift&=~(0x02 | 0x20); //both shifts ;p
	kc=kb_map[kc & 0xFF];
	if (kc==0)
		return;
	if (kb_used>0)
	{
		for (int i=0;i<6;i++)
		{
			if (kb_key[i]==kc)
			{
				kb_used--;
				for (int j=i;j<5;j++)
					kb_key[j]=kb_key[j+1];
				kb_key[6]=0;
			}
		}
	}
}
typedef INT_PTR CALLBACK dlgp( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
dlgp* oldptr=0;
s32 old_pos_x=0;
s32 old_pos_y=0;
INT_PTR CALLBACK sch( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg)
	{
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:

		if (wParam & MK_LBUTTON)
			mo_buttons&=~mo_Left;
		else
			mo_buttons|=mo_Left;

		if (wParam & MK_MBUTTON)
			mo_buttons&=~mo_Middle;
		else
			mo_buttons|=mo_Middle;

		if (wParam & MK_RBUTTON)
			mo_buttons&=~mo_Right;
		else
			mo_buttons|=mo_Right;

		break;
	case WM_MOUSEMOVE:
		mo_x_delta+= GET_X_LPARAM(lParam)-old_pos_x ;
		mo_y_delta+= GET_Y_LPARAM(lParam) -old_pos_y ;

		old_pos_x=GET_X_LPARAM(lParam);
		old_pos_y=GET_Y_LPARAM(lParam);
		break;
	case WM_MOUSEWHEEL:
		{
		s32 diff=GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA;
		diff*=10;
		mo_wheel_delta+=diff;
		}
		break;
	case WM_KEYDOWN:
		kb_down(wParam);
		for (int port=0;port<4;port++)
		{
			for (int i=0;joypad_settings_K[i].name;i++)
			{
				if (wParam==joypad_settings[port][i].KC)
				{
					if (joypad_settings[port][i].BIT<=0x8000)
					{
						kcode[port] &= 0xFFFF -joypad_settings[port][i].BIT;
					}
					else
					{
						vks[port]|=joypad_settings[port][i].BIT;
						switch(joypad_settings[port][i].BIT)
						{
						case key_CONT_ANALOG_UP:
							joyy[port]= -126;
							break;
						case key_CONT_ANALOG_DOWN:
							joyy[port]= +126;
							break;
						case key_CONT_ANALOG_RIGHT:
							joyx[port]= +126;
							break;
						case key_CONT_ANALOG_LEFT:
							joyx[port]= -126;
							break;
						case key_CONT_LSLIDER:
							lt[port]=255;
							break;
						case key_CONT_RSLIDER:
							rt[port]=255;
							break;
						}
					}
				}
			}
		}
		break;

	case WM_KEYUP:
		kb_up(wParam & 0xFF);
		for (int port=0;port<4;port++)
		{
			for (int i=0;joypad_settings_K[i].name;i++)
			{
				if (wParam==joypad_settings[port][i].KC)
				{
					if (joypad_settings[port][i].BIT<=0x8000)
					{
						kcode[port] |= joypad_settings[port][i].BIT;
					}
					else
					{
						vks[port] &= ~joypad_settings[port][i].BIT;
						if ((vks[port] & (key_CONT_ANALOG_UP|key_CONT_ANALOG_DOWN)) !=(key_CONT_ANALOG_UP|key_CONT_ANALOG_DOWN))
						{
							if (vks[port] & key_CONT_ANALOG_UP)
								joyy[port]=-126;
							else if (vks[port] & key_CONT_ANALOG_DOWN)
								joyy[port]=+126;
							else
								joyy[port]=0;
						}
						if ((vks[port] & (key_CONT_ANALOG_LEFT|key_CONT_ANALOG_RIGHT)) !=(key_CONT_ANALOG_LEFT|key_CONT_ANALOG_RIGHT))
						{
							if (vks[port] & key_CONT_ANALOG_LEFT)
								joyx[port]=-126;
							else if (vks[port] & key_CONT_ANALOG_RIGHT)
								joyx[port]=+126;
							else
								joyx[port]=0;
						}
						switch(joypad_settings[port][i].BIT)
						{
						case key_CONT_LSLIDER:
							lt[port]=0;
							break;
						case key_CONT_RSLIDER:
							rt[port]=0;
							break;
						}
					}
				}
			}
		}
		break;
	}
	return oldptr(hWnd,uMsg,wParam,lParam);
}

u32 current_port=0;
bool waiting_key=false;
u32 edited_key=0;
u32 waiting_key_timer=6*4;

u32 kid_to_did[]=
{
	IDC_BUTTON1,
	IDC_BUTTON2,
	IDC_BUTTON3,
	IDC_BUTTON4,
	IDC_BUTTON5,
	IDC_BUTTON6,
	IDC_BUTTON7,
	IDC_BUTTON8,
	IDC_BUTTON9,
	IDC_BUTTON10,
	IDC_BUTTON11,
	IDC_BUTTON12,
	IDC_BUTTON13,
	IDC_BUTTON14,
	IDC_BUTTON15,
	IDC_BUTTON16,
	IDC_BUTTON17,
	IDC_BUTTON18,
	IDC_BUTTON19,
	IDC_BUTTON20,
	IDC_BUTTON21,
	IDC_BUTTON22,
	//IDC_BUTTON22,
};
u8 kbs[256];
const u32 kbratio=20;
void ENABLESHITFACE(HWND hWnd,u32 state)
{
	Static_Enable(hWnd,state);
	for (int kk=0;joypad_settings_K[kk].name;kk++)
	{
		Static_Enable(GetDlgItem(hWnd,kid_to_did[kk]),state);
	}
}
void get_name(int VK,char* text)
{
	int scancode = MapVirtualKey(VK,0);
	switch(VK) {
	  case VK_INSERT:
	  case VK_DELETE:
	  case VK_HOME:
	  case VK_END:
	  case VK_NEXT:  // Page down
	  case VK_PRIOR: // Page up
	  case VK_LEFT:
	  case VK_RIGHT:
	  case VK_UP:
	  case VK_DOWN:
		  scancode |= 0x100; // Add extended bit
	}
	GetKeyNameText(scancode*0x10000,text,512);
}
void UpdateKeySelectionNames(HWND hWnd)
{
	char temp[512];
	for (int i=0;joypad_settings_K[i].name;i++)
	{
		if (kid_to_did[i]==0)
			continue;
		get_name(joypad_settings[current_port][i].KC,temp);
		Button_SetText(GetDlgItem(hWnd,kid_to_did[i]),temp);
	}
}
INT_PTR CALLBACK ConfigKeysDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			TCITEM tci; 
			tci.mask = TCIF_TEXT | TCIF_IMAGE; 
			tci.iImage = -1; 
			tci.pszText = "Port A"; 
			TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_PORTTAB), 0, &tci); 
			tci.pszText = "Port B"; 
			TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_PORTTAB), 1, &tci); 
			tci.pszText = "Port C"; 
			TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_PORTTAB), 2, &tci); 
			tci.pszText = "Port D"; 
			TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_PORTTAB), 3, &tci); 

			TabCtrl_SetCurSel(GetDlgItem(hWnd,IDC_PORTTAB),current_port);

			SetTimer(hWnd,0,1000/kbratio,0);
			Static_SetText(GetDlgItem(hWnd,IDC_STATUS),"Click a button , then press the key you want to use for it.If you want to use joysticks try the joy2key utility");
			UpdateKeySelectionNames(hWnd);
		}
		return true;
	case WM_NOTIFY:
		{
			if ( ((LPNMHDR)lParam)->idFrom==IDC_PORTTAB && 
				 ((LPNMHDR)lParam)->code == TCN_SELCHANGE  )
			{
				current_port=TabCtrl_GetCurSel(GetDlgItem(hWnd,IDC_PORTTAB));
				UpdateKeySelectionNames(hWnd);
			}
			return true;
		}
	case WM_COMMAND:

		for (int i=0;joypad_settings_K[i].name;i++)
		{
			if (kid_to_did[i]==LOWORD(wParam))
			{
				edited_key=i;
				GetKeyboardState(kbs);
				ENABLESHITFACE(hWnd,0);
				waiting_key_timer=6*kbratio;
				waiting_key=true;
				return true;
			}
		}

		switch( LOWORD(wParam) )
		{

			break;
			
		case IDOK:
			{
			
			}
		case IDCANCEL:
			EndDialog(hWnd,0);
			return true;

		default: break;
		}
		return false;
	case WM_TIMER:
	{
		char temp[512];
		if (waiting_key)
		{
			int VK_down=-1;
			u8 temp_kbs[256];
			GetKeyboardState(temp_kbs);
			for (int i=0;i<256;i++)
			{
				if (temp_kbs[i]!=kbs[i] && temp_kbs[i]!=0)
				{
					VK_down=i;
				}
			}

			if (VK_down!=-1)
			{
				waiting_key=false;

				sprintf(temp,"Updated Key Mapping,%d",VK_down);
				Static_SetText(GetDlgItem(hWnd,IDC_STATUS),temp);
				joypad_settings[current_port][edited_key].KC=VK_down;
				SaveSettings();
				UpdateKeySelectionNames(hWnd);
			}	
		}

		if(waiting_key)
		{
			char temp[512];
			
			waiting_key_timer--;
			if (waiting_key_timer==0)
			{
				Static_Enable(hWnd,1);
				for (int kk=IDC_BUTTON1;kk<(IDC_BUTTON1+16);kk++)
				{
					Static_Enable(GetDlgItem(hWnd,kk),1);
				}
				waiting_key=false;
				waiting_key_timer=6;

				sprintf(temp,"Timed out while waiting for new key",waiting_key_timer/kbratio);
				Static_SetText(GetDlgItem(hWnd,IDC_STATUS),temp);
			}
			else
			{
				sprintf(temp,"Waiting for key ...%d\n",waiting_key_timer/kbratio);
				Static_SetText(GetDlgItem(hWnd,IDC_STATUS),temp);
			}
		}

		if (!waiting_key)
			ENABLESHITFACE(hWnd,1);
		GetKeyboardState(kbs);
	}
	return true;
	case WM_CLOSE:
	case WM_DESTROY:
		KillTimer(hWnd,0);
		EndDialog(hWnd,0);
		return true;

	default: break;
	}

	return false;
}
void cfgdlg(PluginType type,void* window)
{
	printf("ndcMAPLE :No config kthx\n");
}//called when plugin is used by emu (you should do first time init here)

void Init_kb_map();
s32 FASTCALL Load(emu_info* emu)
{
	memcpy(&host,emu,sizeof(host));
	for (int set=0;set<4;set++)
		memcpy(joypad_settings[set],joypad_settings_K,sizeof(joypad_settings_K));

	//maple_init_params* mpi=(maple_init_params*)aparam;
	//handle=mpi->WindowHandle;
	if (oldptr==0)
		oldptr = (dlgp*)SetWindowLongPtr((HWND)host.WindowHandle,GWL_WNDPROC,(LONG)sch);
	Init_kb_map();

	LoadSettings();
	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void FASTCALL  Unload()
{
	if (oldptr!=0)
	{
		SetWindowLongPtr((HWND)host.WindowHandle,GWL_WNDPROC,(LONG)oldptr);
		oldptr=0;
	}
}

/*
//It's suposed to reset anything but vram (vram is set to 0 by emu)
s32 FASTCALL Init(maple_init_params* p)
{
	UpdateConfig();
	//hahah do what ? ahahahahahaha
	return rv_ok;
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void FASTCALL Reset(bool Manual)
{
	//maby init here ?
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL Term()
{
	//term here ?
}
*/
HMODULE hModule;
HINSTANCE hInstance;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	::hModule=hModule;
	hInstance=(HINSTANCE)hModule;
    return TRUE;
}
#define sk(num,key)kb_map[key]=0x##num;
void Init_kb_map()
{
	memset(kb_map,0,sizeof(kb_map));
	//Keycode Key 
		//00 No key pressed 
		//01 Too many keys pressed error 
		//02-03 Not used(?) 
		//04-1D Letter keys A-Z (in alphabetic order) 
		for (int i=0x4;i<=0x1D;i++)
			kb_map['A'+i-4]=i;
		//1E-27 Number keys 1-0 
		for (int i=0x1E;i<=0x27;i++)
			kb_map['1'+i-0x1E]=i;
		kb_map['0']=0x27;
		sk(1F,'\'');
		//28 Enter 
		sk(28,'\r');
		//29 Esc 
		sk(29,VK_ESCAPE);
		//2A Backspace 
		sk(2A,'\b');
		//2B Tab 
		sk(2B,VK_TAB);
		//2C Space 
		sk(2C,VK_SPACE);
		//2D-2E "-" and "^" (the 2 keys right of the numbers) 
		//sk(28,VK_SUBTRACT);
		sk(2d,VK_SUBTRACT);
		//2F-30 "@" and "[" (the 2 keys right of P) 
		sk(2F,';');
		sk(30,'[');
		//31 Not used 
		//32-34 "]", ";" and ":" (the 3 keys right of L) 
		sk(32,']');
		sk(33,';');
		sk(34,':');
		//35 hankaku/zenkaku / kanji (top left) 
		//36-38 ",", "." and "/" (the 3 keys right of M) 
		sk(36,0xbc);
		sk(37,0xbc+2);
		sk(38,0xbc+3);
		//39 Caps Lock 
		//3A-45 Function keys F1-F12 
		for (int i=0;i<12;i++)
			kb_map[VK_F1+i]=0x3A+i;
		//46-4E Control keys above cursor keys 
		//4F Cursor right
		sk(4F,VK_RIGHT);
		//50 Cursor left 
		sk(50,VK_LEFT);
		//51 Cursor down 
		sk(51,VK_DOWN);
		//52 Cursor up 
		sk(52,VK_UP);
		//53 Num Lock (Numeric keypad) 
		sk(53,VK_NUMLOCK);
		//54 "/" (Numeric keypad) 
		sk(54,VK_DIVIDE);
		//55 "*" (Numeric keypad) 
		sk(55,VK_MULTIPLY);
		//56 "-" (Numeric keypad) 
		sk(56,VK_SUBTRACT);
		//57 "+" (Numeric keypad) 
		sk(57,VK_ADD);
		//58 Enter (Numeric keypad) 
		sk(58,VK_EXECUTE);	//enter ??
		//59-62 Number keys 1-0 (Numeric keypad) 
		sk(59,VK_NUMPAD1);
		sk(59+1,VK_NUMPAD2);
		sk(59+2,VK_NUMPAD3);
		sk(59+3,VK_NUMPAD4);
		sk(59+4,VK_NUMPAD5);
		sk(59+5,VK_NUMPAD6);
		sk(59+6,VK_NUMPAD7);
		sk(59+7,VK_NUMPAD8);
		sk(59+8,VK_NUMPAD9);
		sk(59+9,VK_NUMPAD0);
		//63 "." (Numeric keypad) 
		sk(59+9,'.');
		//64 "\" (right of left Shift) 
		sk(64,'\\');
		//65 S3 key 
		//66-86 Not used 
		//8C-FF Not used 
}



u8 GetBtFromSgn(s8 val);

#define w32(data) *(u32*)buffer_out_b=(data);buffer_out_b+=4;buffer_out_len+=4
#define w16(data) *(u16*)buffer_out_b=(data);buffer_out_b+=2;buffer_out_len+=2
#define w8(data) *(u8*)buffer_out_b=(data);buffer_out_b+=1;buffer_out_len+=1

void FASTCALL KbdDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(1 << 30);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_kbd[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x04FF); 

			//2
			w16(0x0069); 
			break;

		/* controller condition structure 
		typedef struct {//8 bytes
		int8 shift          ; shift keys pressed (bitmask)	//1
		int8 led            ; leds currently lit			//1
		int8 key[6]         ; normal keys pressed			//6
		} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			responce=0x08;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 30));
			//struct data
			//int8 shift          ; shift keys pressed (bitmask)	//1
			w8(kb_shift);
			//int8 led            ; leds currently lit			//1
			w8(kb_led);
			//int8 key[6]         ; normal keys pressed			//6
			for (int i=0;i<6;i++)
			{
				w8(kb_key[i]);
			}
			
			break;

		default:
			printf("UNKOWN MAPLE COMMAND %d\n",Command);
			break;
	}
}

u16 mo_cvt(s32 delta)
{
	delta+=0x200;
	if (delta<=0)
		delta=0;
	else if (delta>0x3FF)
		delta=0x3FF;

	return (u16) delta;
}
/*
	(06/01/07)[04:09] <BlueCrab> Device info for port A0:
	(06/01/07)[04:09] <BlueCrab> Functions: 01000000
	(06/01/07)[04:09] <BlueCrab> Function Data 0: 00080000
	(06/01/07)[04:09] <BlueCrab> Function Data 1: 00000000
	(06/01/07)[04:09] <BlueCrab> Function Data 2: 00000000
	(06/01/07)[04:09] <BlueCrab> Area code: FF
	(06/01/07)[04:09] <BlueCrab> Connector direction: 00
	(06/01/07)[04:09] <BlueCrab> Product name: Dreamcast Camera Flash  Devic
	(06/01/07)[04:09] <BlueCrab> Product licence: Produced By or Under License From SEGA ENTERPRISES,LTD.    
	(06/01/07)[04:09] <BlueCrab> Standby power: 07D0
	(06/01/07)[04:10] <BlueCrab> Max power: 0960
*/
void FASTCALL DreamEye_mainDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(0x01000000);

			//struct data
			//3*4
			w32( 0x00080000); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_dreameye_1[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x04FF); 

			//2
			w16(0x0069); 
			break;
			/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			responce=0x08;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 24));
			//struct data
			//2
			w16(0xF7FF); //camera button not pressed
			
			//triger
			//1 R
			w8(0);
			//1 L
			w8(0); 
			//joyx
			//1
			w8(GetBtFromSgn(0));
			//joyy
			//1
			w8(GetBtFromSgn(0));

			//1
			w8(GetBtFromSgn(0)); 
			//1
			w8(GetBtFromSgn(0)); 
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;

			break;
		default:
			printf("DreamEye_mainDMA : UNKOWN MAPLE COMMAND %d \n",Command);
			responce=7;//just ko
			break;
	}
}

/*
	(06/01/07)[04:10] <BlueCrab> Device info for port A1:
	(06/01/07)[04:10] <BlueCrab> Functions: 00080000
	(06/01/07)[04:10] <BlueCrab> Function Data 0: 30A800C0
	(06/01/07)[04:10] <BlueCrab> Function Data 1: 00000000
	(06/01/07)[04:10] <BlueCrab> Function Data 2: 00000000
	(06/01/07)[04:10] <BlueCrab> Area code: FF
	(06/01/07)[04:10] <BlueCrab> Connector direction: 00
	(06/01/07)[04:10] <BlueCrab> Product name: Dreamcast Camera Flash LDevic
	(06/01/07)[04:10] <BlueCrab> Product licence: Produced By or Under License From SEGA ENTERPRISES,LTD.    
	(06/01/07)[04:10] <BlueCrab> Standby power: 0000
	(06/01/07)[04:10] <BlueCrab> Max power: 0000
*/
u32 des_9_count=0;
void FASTCALL DreamEye_subDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(0x00080000);

			//struct data
			//3*4
			w32( 0x30A800C0); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_dreameye_2[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0); 

			//2
			w16(0); 
			break;

//(07/01/07)[02:36] <BlueCrab> dreameye: replied with 8, size 5
//(07/01/07)[02:36] <BlueCrab> 00080000, 000000D0, 1F000480, 1F000481, C0070094,
//(07/01/07)[02:46] <BlueCrab> ok... to the 6 command 9's after the one with data, it replies identically:
//(07/01/07)[02:46] <BlueCrab> dreameye: replied with 8, size 2
//(07/01/07)[02:46] <BlueCrab> 00080000, 000000D0, 
		case 9:

			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			responce=0x08;
			//caps
			//4
			w32(0x00080000);
			//struct data
			if (des_9_count==0)
			{
			//4 dwords
			w32(0x000000D0);
			w32(0x1F000480);
			w32(0x1F000481);
			w32(0xC0070094);
			}
			else
			{
				//1 dword
				w32(0x000000D0); //? ok maby ?
			}

			des_9_count++;
			printf("DreamEye_subDMA[0x%x | %d %x] : UNKOWN MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
			printf(" buffer in size : %d\n",buffer_in_len);
			for (u32 i=0;i<buffer_in_len;i+=4)
			{
				printf("%08X ",*buffer_in++);
			}
			printf("\n");
			//getchar();
			break;
//(07/01/07)[03:04] <BlueCrab> dreameye: replied with 11, size 2
//(07/01/07)[03:04] <BlueCrab>           responding to port B1
//(07/01/07)[03:04] <BlueCrab> 00080000, 000002FF, 
		case 17:
			{
				responce=17; //? fuck ?
				//caps
				//4
				w32(0x00080000);
				//mmwaaa?
				//w32(0x000002FF);
				w32(rand());
			}
			printf("DreamEye_subDMA[0x%x | %d %x] : UNKOWN MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
			printf(" buffer in size : %d\n",buffer_in_len);
			for (u32 i=0;i<buffer_in_len;i+=4)
			{
				printf("%08X ",*buffer_in++);
			}
			printf("\n");
			//getchar();
			break;
		default:
			printf("DreamEye_subDMA[0x%x | %d %x] : UNKOWN MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
			printf(" buffer in size : %d\n",buffer_in_len);
			for (u32 i=0;i<buffer_in_len;i+=4)
			{
				printf("%08X ",*buffer_in++);
			}
			printf("\n");
			//getchar();
			responce=7;//just ko
			break;
	}
}


void FASTCALL MicDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	printf("MicDMA[0x%x | %d %x] : UNKOWN MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
	printf(" buffer in size : %d\n",buffer_in_len);

	for (u32 i=0;i<buffer_in_len;i+=4)
	{
		printf("%08X ",*buffer_in++);
	}
	printf("\n");

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(0x10000000);

			//struct data
			//3*4
			w32( 0x3F000000);  // ?? wii pwns ps3 ...
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xF); //WTF ?
			//1	direction
			w8(1); //WTF ?
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_mic[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x012C); 

			//2
			w16(0x012C); 
			break;
		case 9:
			responce=0x08;
			//caps
			//4
			w32(0x10000000);
			//getchar();
			break;

		default:
			responce=7;//just ko
			break;
	}
}


void FASTCALL MouseDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(1 << 17);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_mouse[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x04FF); 

			//2
			w16(0x0069); 
			break;

		/* controller condition structure 
		typedef struct {//8 bytes
		int32 buttons       ; digital buttons bitfield (little endian)
		int16 axis1         ; horizontal movement (0-$3FF) (little endian)
		int16 axis2         ; vertical movement (0-$3FF) (little endian)
		int16 axis3         ; mouse wheel movement (0-$3FF) (little endian)
		int16 axis4         ; ? movement (0-$3FF) (little endian)
		int16 axis5         ; ? movement (0-$3FF) (little endian)
		int16 axis6         ; ? movement (0-$3FF) (little endian)
		int16 axis7         ; ? movement (0-$3FF) (little endian)
		int16 axis8         ; ? movement (0-$3FF) (little endian)
		} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			responce=0x08;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32(1 << 17);
			//struct data
			
			//int32 buttons       ; digital buttons bitfield (little endian)
			w32(mo_buttons);
			//int16 axis1         ; horizontal movement (0-$3FF) (little endian)
			w16(mo_cvt(mo_x_delta));
			//int16 axis2         ; vertical movement (0-$3FF) (little endian)
			w16(mo_cvt(mo_y_delta));
			//int16 axis3         ; mouse wheel movement (0-$3FF) (little endian)
			w16(mo_cvt(mo_wheel_delta));
			//int16 axis4         ; ? movement (0-$3FF) (little endian)
			w16(mo_cvt(0));
			//int16 axis5         ; ? movement (0-$3FF) (little endian)
			w16(mo_cvt(0));
			//int16 axis6         ; ? movement (0-$3FF) (little endian)
			w16(mo_cvt(0));
			//int16 axis7         ; ? movement (0-$3FF) (little endian)
			w16(mo_cvt(0));
			//int16 axis8         ; ? movement (0-$3FF) (little endian)
			w16(mo_cvt(0));

			mo_x_delta=0;
			mo_y_delta=0;
			break;

		default:
			printf("UNKOWN MAPLE COMMAND %d\n",Command);
			break;
	}
}


void FASTCALL ControllerDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;
	u32 port=device_instance->port>>6;
	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(1 << 24);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				if (testJoy_strName[i]!=0)
				{
					w8((u8)testJoy_strName[i]);
				}
				else
				{
					w8(0x20);
				}
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				if (testJoy_strBrand_2[i]!=0)
				{
					w8((u8)testJoy_strBrand_2[i]);
				}
				else
				{
					w8(0x20);
				}
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0xAE01); 

			//2
			w16(0xF401); 
			break;

		/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			responce=0x08;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32(1 << 24);
			//struct data
			//2
			w16(kcode[port] | 0xF901); 
			
			//triger
			//1 R
			w8(rt[port]);
			//1 L
			w8(lt[port]); 
			//joyx
			//1
			w8(GetBtFromSgn(joyx[port]));
			//joyy
			//1
			w8(GetBtFromSgn(joyy[port]));

			//1
			w8(0x80); 
			//1
			w8(0x80); 
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;

			break;

		default:
			printf("UNKOWN MAPLE COMMAND %d\n",Command);
			break;
	}
}
joy_state states[4];
SOCKET ConnectSocket = INVALID_SOCKET;
u32 local_port;
u32 send_ratio;
char server_addr[512];
char server_port[512];

#define MSG_WAITALL 0
void setups(SOCKET s)
{
	int flag = 1;
	int result = setsockopt(s,            /* socket affected */
		IPPROTO_TCP,     /* set option at TCP level */
		TCP_NODELAY,     /* name of option */
		(char *) &flag,  /* the cast is historical
						 cruft */
						 sizeof(int));    /* length of option value */
	flag=0;
	u_long t=0;
	ioctlsocket (s,FIONBIO ,&t);
}
u32 sync_counter=0;
u32 next_sync_counter=0;
bool np=false;
int Init_netplay()
{
	WSADATA wsaData;
    if (np)
		return 0;
	np=1;

    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    int iResult;
    

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
	iResult = getaddrinfo(server_addr, server_port, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("Error at socket(): %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }
		setups(ConnectSocket);
        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR && WSAEWOULDBLOCK!=WSAGetLastError()) {
			int rr=WSAGetLastError();
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
		Sleep(200);
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

	joy_init t;
	strcpy(t.Name,"nullDC hookjoy plugin");
	t.port=local_port;
	t.Version=DC_MakeVersion(1,0,0,0);
    // Send an initial buffer
    iResult = send( ConnectSocket, (char*)&t, (int)sizeof(t), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

	printf("Bytes Sent: %ld\n", iResult);

	joy_init_resp r;

	int dv=recv(ConnectSocket,(char*)&r,sizeof(r),MSG_WAITALL);
	//__asm int 3;
	printf("Server : %d,%d\n",r.ratio,r.status);
	send_ratio=r.ratio;
    


    // cleanup


    return 0;
}

void net_read()
{
	int rv = recv(ConnectSocket,(char*)&states,sizeof(states),MSG_WAITALL);
	if (rv==0xFFFFFF)
		printf("net_read fail %d\n",WSAGetLastError());
	printf("SYNC1 %d %d\nSYNC2 %d %d %d %d\n",sync_counter,next_sync_counter,states[0].id,states[1].id,states[2].id,states[3].id);
}
void net_send()
{
	//u32* at1=(u32*)0x00aaa150;
	//u32* at2=(u32*)0x00aaa17C;
	
	joy_state t;
	//t.id=*at1+*at2;
	t.jx=joyx[local_port];
	t.jy=joyy[local_port];
	t.l=lt[local_port];
	t.r=rt[local_port];
	t.state=kcode[local_port];
	int rv = send(ConnectSocket,(char*)&t,sizeof(t),0);
}

void termnet()
{
	closesocket(ConnectSocket);
    WSACleanup();
}
u32 GetMaplePort(u32 addr)
{
	/*
	for (int i=0;i<6;i++)
	{
		if ((1<<i)&addr)
			return i;
	}*/
	return addr>>6;
}

void sync_net(u32 port)
{
	if (port==0)
	{
		sync_counter++;
		if (sync_counter==1)
			net_send();
		verify(sync_counter<=next_sync_counter);

		if (sync_counter==next_sync_counter)
		{
			net_read();
			next_sync_counter=sync_counter+send_ratio;
			net_send();
			//printf("UPDATE %d-%d\n",sync_counter,port);
		}
		//printf("%d - %d - %d\n",sync_counter,next_sync_counter,send_ratio);
		verify(sync_counter<next_sync_counter);
	}
}
void FASTCALL ControllerDMA_net(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;
	
	bool islocal=device_instance->port==local_port;
	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(1 << 24);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_net[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x04FF); 

			//2
			w16(0x0069); 
			break;

		/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:
			{
				u32 aport=GetMaplePort(device_instance->port);
				sync_net(aport);
			/*
				char file[43];
			sprintf(file,"log_%d.raw",aport);
			FILE* log=fopen(file,"a");
			fseek(log,0,SEEK_END);
			char* bvvvv=(char*)buffer_out_b;
			*/
			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			responce=0x08;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 24));
			//struct data
			//2
			w16(states[aport].state); 
			
			//triger
			//1 R
			w8(states[aport].r);
			//1 L
			w8(states[aport].l); 
			//joyx
			//1
			w8(GetBtFromSgn(states[aport].jx));
			//joyy
			//1
			w8(GetBtFromSgn(states[aport].jy));

			//1
			w8(GetBtFromSgn(0)); 
			//1
			w8(GetBtFromSgn(0)); 
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			/*
			fwrite(bvvvv,12,1,log);
			fclose(log);
			*/
			}
			break;

		default:
			printf("UNKOWN MAPLE COMMAND %d\n",Command);
			break;
	}
}

void FASTCALL ControllerDMA_nul(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(1 << 24);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_nul[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x04FF); 

			//2
			w16(0x0069); 
			break;

		/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			responce=0x08;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 24));
			//struct data
			//2
			w16(0xFFFF); 
			
			//triger
			//1 R
			w8(0);
			//1 L
			w8(0); 
			//joyx
			//1
			w8(GetBtFromSgn(0));
			//joyy
			//1
			w8(GetBtFromSgn(0));

			//1
			w8(GetBtFromSgn(0)); 
			//1
			w8(GetBtFromSgn(0)); 
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;

			break;

		default:
			printf("UNKOWN MAPLE COMMAND %d\n",Command);
			break;
	}
}

u8 GetBtFromSgn(s8 val)
{
	return val+128;
}

typedef struct {
	u16 total_size;
	u16 partition_number;
	u16 system_area_block;
	u16 fat_area_block;
	u16 number_fat_areas_block;
	u16 file_info_block;
	u16 number_info_blocks;
	u8 volume_icon;
	u8 reserved1;
	u16 save_area_block;
	u16 number_of_save_blocks;
	u16 reserverd0;  
}maple_getvmuinfo_t;

#define SWAP32(val) ((u32) ( \
	(((u32) (val) & (u32) 0x000000ffU) << 24) | \
	(((u32) (val) & (u32) 0x0000ff00U) <<  8) | \
	(((u32) (val) & (u32) 0x00ff0000U) >>  8) | \
	(((u32) (val) & (u32) 0xff000000U) >> 24)))
void FASTCALL VmuDMA(maple_subdevice_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	u8*buffer_out_b=(u8*)buffer_out;
	//printf("VmuDMA Called for port 0x%X, Command %d\n",device_instance->port,Command);
	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(2 << 24);

			//struct data
			//3*4
			w32( 0x00410f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_vmu[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x7c00); 

			//2
			w16(0x8200); 
			break;

				//in[0] is function used ?
				//out[0] is function used ?
		case 10:
			if (buffer_in[0]& (2<<24))
			{
				buffer_out[0] = (2<<24);//is that ok ?
				maple_getvmuinfo_t* vmui = (maple_getvmuinfo_t*)(&buffer_out[1]);
				//ZeroMemory(pMediaInfo,sizeof(TMAPLE_MEDIAINFO));
				memset(vmui,0,sizeof(maple_getvmuinfo_t));
				vmui->total_size = 0xFF;//0x7FFF;//0xFF
				vmui->system_area_block = 0xFF;//0x7FFF;//0xff
				vmui->fat_area_block = 0xfe;//0x7F00;	//0xfe
				vmui->number_fat_areas_block = 1;//256;//1
				vmui->volume_icon = 0x0;//0
				vmui->save_area_block = 0xc8;//?
				vmui->number_of_save_blocks = 0x1f;
				//pMediaInfo->volume_icon = 0x0;
				vmui->file_info_block = 0xfd;//0x7E00;//0xfd
				vmui->number_info_blocks = 0xd;//0x100;//0xd
				vmui->reserverd0 = 0x0000;
				buffer_out_len=4+(sizeof(maple_getvmuinfo_t));
				responce=8;//data transfer
			}
			else
				responce=-2;//bad function
			break;

		case 11:
			if(buffer_in[0]&(2<<24))
			{
				VMU_info* dev=(VMU_info*)((*device_instance).data);

				buffer_out[0] = (2<<24);
				u32 Block = (SWAP32(buffer_in[1]))&0xffff;
				buffer_out[1] = buffer_in[1];
				if (Block>255)
				{
					printf("Block read : %d\n",Block);
					printf("BLOCK READ ERROR\n");
					Block&=255;
				}
				memcpy(&buffer_out[2],(dev->data)+Block*512,512);
				buffer_out_len=(512+8);
				responce=8;//data transfer
			}
			else
				responce=-2;//bad function
			break;
		case 12:
			if(buffer_in[0]&(2<<24))
			{
				VMU_info* dev=(VMU_info*)((*device_instance).data);

				u32 Block = (SWAP32(buffer_in[1]))&0xffff;
				u32 Phase = ((SWAP32(buffer_in[1]))>>16)&0xff; 
				//printf("Block wirte : %d:%d , %d bytes\n",Block,Phase,(buffer_in_len-8));
				memcpy(&dev->data[Block*512+Phase*(512/4)],&buffer_in[2],(buffer_in_len-8));
				buffer_out_len=0;
				FILE* f=fopen(dev->file,"wb");
				if (f)
				{
					fwrite(dev->data,1,128*1024,f);
					fclose(f);
				}
				else
					printf("Failed to open %s for saving vmu data\n",dev->file);
				responce=7;//just ko
			}
			else
				responce=-2;//bad function
			break;

		case 13:
			responce=7;//just ko
			break;

		default:
			printf("UNKOWN MAPLE COMMAND %d\n",Command);
			break;
	}

}

void FASTCALL config_keys(u32 id,void* w,void* p)
{
	maple_device_instance* mdd=(maple_device_instance*)p;
	current_port=mdd->port>>6;
	DialogBox(hInstance,MAKEINTRESOURCE(IDD_ConfigKeys),(HWND)w,ConfigKeysDlgProc);
}
s32 FASTCALL CreateMain(maple_device_instance* inst,u32 id,u32 flags,u32 rootmenu)
{
	char temp[512];
	if (id<=1)
	{
		sprintf(temp,"Config keys for Player %d",(inst->port>>6)+1);
		u32 ckid=host.AddMenuItem(rootmenu,-1,temp,config_keys,0);
		MenuItem mi;
		mi.PUser=inst;
		host.SetMenuItem(ckid,&mi,MIM_PUser);
	}
	if (id==0)
	{
		inst->dma=ControllerDMA;
		inst->data=0;
		sprintf(temp,"Controller[winhook] : 0x%02X",inst->port);
	}
	else if (id==1)
	{
		inst->dma=ControllerDMA_net;
		inst->data=0;
		sprintf(temp,"Controller[winhook,net] : 0x%02X",inst->port);
	}
	else if (id==3)
	{
		inst->dma=KbdDMA;
		inst->data=0;
		sprintf(temp,"Keyboard : 0x%02X",inst->port);
	}
	else if (id==4)
	{
		inst->dma=ControllerDMA_nul;
		inst->data=0;
		sprintf(temp,"Controller [no input] : 0x%02X",inst->port);
	}
	else if (id==5)
	{
		inst->dma=MouseDMA;
		inst->data=0;
		sprintf(temp,"Mouse [winhook] : 0x%02X",inst->port);
	}
	host.AddMenuItem(rootmenu,-1,temp,0,0);
/*
	else if (id==2)
	{
		
	}
	else if (id==3)
	{
		inst->dma=KbdDMA;
		inst->data=0;
	}
	else if (id==4)
	{
		inst->dma=MouseDMA;
		inst->data=0;
	}
	else if ( id==5)
	{
		inst->dma=DreamEye_mainDMA;
		inst->data=0;
	}
	else if ( id==6)
	{
		inst->dma=DreamEye_subDMA;
		inst->data=0;
	}
	else if ( id==7)
	{
		inst->dma=MicDMA;
		inst->data=0;
	}
	else
		return false;
*/
	return rv_ok;
	//printf("Created instance of device %s on port 0x%x\n",dev->name,port);
}


s32 FASTCALL InitMain(maple_device_instance* inst,u32 id,maple_init_params* params)
{
	if (id==1)
	{
		sync_counter=0;
		next_sync_counter=1;
		verify(Init_netplay()==0);
	}
	return rv_ok;
}
void FASTCALL TermMain(maple_device_instance* inst,u32 id)
{
}
void FASTCALL DestroyMain(maple_device_instance* inst,u32 id)
{
	if (inst->data)
		free( inst->data);
}
s32 FASTCALL CreateSub(maple_subdevice_instance* inst,u32 id,u32 flags,u32 rootmenu)
{
	char temp[512];
	sprintf(temp,"VMU :vmu_data_port%02X.bin",inst->port);
	host.AddMenuItem(rootmenu,-1,temp,0,0);
	inst->data=malloc(sizeof(VMU_info));
	sprintf(((VMU_info*)inst->data)->file,"vmu_data_port%02X.bin",inst->port);
	FILE* f=fopen(((VMU_info*)inst->data)->file,"rb");
	if (f)
	{
		fread(((VMU_info*)inst->data)->data,1,128*1024,f);
		fclose(f);
	}
	inst->dma=VmuDMA;

	return rv_ok;
}
s32 FASTCALL InitSub(maple_subdevice_instance* inst,u32 id,maple_init_params* params)
{
	return rv_ok;
}
void FASTCALL TermSub(maple_subdevice_instance* inst,u32 id)
{
}
void FASTCALL DestroySub(maple_subdevice_instance* inst,u32 id)
{
	if (inst->data)
		free(inst->data);
}

#define MMD(name,flags) \
	strcpy(km.devices[mdi].Name,name);	\
	km.devices[mdi].Type=MDT_Main;	\
	km.devices[mdi].Flags= flags;	\
	mdi++;

#define MSD(name,flags)	\
	strcpy(km.devices[mdi].Name,name);	\
	km.devices[mdi].Type=MDT_Sub;	\
	km.devices[mdi].Flags= flags;	\
	mdi++;

#define MDLE() km.devices[mdi].Type=MDT_EndOfList;
//Give a list of the devices to teh emu
void EXPORT_CALL dcGetInterface(plugin_interface* info)
{

#define km info->maple

#define c info->common
	
	info->InterfaceVersion=PLUGIN_I_F_VERSION;

	c.InterfaceVersion=MAPLE_PLUGIN_I_F_VERSION;

	c.Load=Load;
	c.Unload=Unload;
	c.Type=Plugin_Maple;
	c.PluginVersion=DC_MakeVersion(1,0,0,DC_VER_NORMAL);
	
	strcpy(c.Name,"nullDC Maple Devices (" __DATE__ ")");

	km.CreateMain=CreateMain;
	km.InitMain=InitMain;
	km.TermMain=TermMain;
	km.DestroyMain=DestroyMain;

	km.CreateSub=CreateSub;
	km.InitSub=InitSub;
	km.TermSub=TermSub;
	km.DestroySub=DestroySub;

	u32 mdi=0;
	//0
	MMD("nullDC Controller [WinHook] (" __DATE__ ")",MDTF_Hotplug|MDTF_Sub0|MDTF_Sub1);

	//1
	MMD("nullDC Controller [WinHook,NET] (" __DATE__ ")",MDTF_Hotplug|MDTF_Sub0|MDTF_Sub1);

	//2
	MSD("nullDC VMU (" __DATE__ ")",MDTF_Hotplug);

	//3
	MMD("nullDC Keyboard [WinHook] (" __DATE__ ")",MDTF_Hotplug);

	//4
	MMD("nullDC Controller [no input] (" __DATE__ ")",MDTF_Hotplug|MDTF_Sub0|MDTF_Sub1);

	//5
	MMD("nullDC Mouse [WinHook] (" __DATE__ ")",MDTF_Hotplug);

	/*

	//6
	MMD("nullDC DreamEye (" __DATE__ ")",MDTF_Hotplug);

	//7
	MSD("nullDC Mic (" __DATE__ ")",MDTF_Hotplug);
	*/

	//list terminator :P
	MDLE();

	/*
	info->InterfaceVersion.full=MAPLE_PLUGIN_I_F_VERSION;

	info->Devices[0].CreateInstance=CreateInstance;
	info->Devices[0].DestroyInstance=DestroyInstance;
	info->Devices[0].type=0;//Controller
	info->Devices[0].id=0;
	strcpy(info->Devices[0].name,"nullDC DC controller [WinHook] (" __DATE__ ")");
 
	info->Devices[1].CreateInstance=CreateInstance;
	info->Devices[1].DestroyInstance=DestroyInstance;
	info->Devices[1].type=1;//Vmu
	info->Devices[1].id=1;
	strcpy(info->Devices[1].name,"nullDC VMU (" __DATE__ ")");

	info->Devices[2].CreateInstance=CreateInstance;
	info->Devices[2].DestroyInstance=DestroyInstance;
	info->Devices[2].type=0;//Controller
	info->Devices[2].id=2;
	strcpy(info->Devices[2].name,"nullDC DC controller [no input](" __DATE__ ")");

	info->Devices[3].CreateInstance=CreateInstance;
	info->Devices[3].DestroyInstance=DestroyInstance;
	info->Devices[3].type=0;//Controller
	info->Devices[3].id=3;
	strcpy(info->Devices[3].name,"nullDC DC Keyboard(" __DATE__ ")");

	info->Devices[4].CreateInstance=CreateInstance;
	info->Devices[4].DestroyInstance=DestroyInstance;
	info->Devices[4].type=0;//Controller
	info->Devices[4].id=4;
	strcpy(info->Devices[4].name,"nullDC DC Mouse(" __DATE__ ")");

	info->Devices[5].CreateInstance=CreateInstance;
	info->Devices[5].DestroyInstance=DestroyInstance;
	info->Devices[5].type=0;//main
	info->Devices[5].id=5;
	strcpy(info->Devices[5].name,"nullDC DreamEye Main(" __DATE__ ")");

	info->Devices[6].CreateInstance=CreateInstance;
	info->Devices[6].DestroyInstance=DestroyInstance;
	info->Devices[6].type=1;//sub
	info->Devices[6].id=6;
	strcpy(info->Devices[6].name,"nullDC DreamEye Sub(" __DATE__ ")");

	info->Devices[7].CreateInstance=CreateInstance;
	info->Devices[7].DestroyInstance=DestroyInstance;
	info->Devices[7].type=1;//sub
	info->Devices[7].id=7;
	strcpy(info->Devices[7].name,"nullDC Dreamcast Mic(" __DATE__ ")");

	info->Devices[8].CreateInstance=0;
	info->Devices[8].DestroyInstance=0;
	*/
}
void LoadSettings()
{
	for (int port=0;port<4;port++)
	{
		for (int i=0;joypad_settings_K[i].name;i++)
		{
			char temp[512];
			sprintf(temp,"Port%c_%s",port+'A',&joypad_settings_K[i].name[4]);
			joypad_settings[port][i].KC=host.ConfigLoadInt("ndc_hookjoy",temp,joypad_settings_K[i].KC);
		}
	}
	local_port=host.ConfigLoadInt("ndc_hookjoy","local_port",0);
	host.ConfigLoadStr("ndc_hookjoy","server_addr",server_addr,"192.168.1.33");
	host.ConfigLoadStr("ndc_hookjoy","server_port",server_port,"11122");
}

void SaveSettings()
{
	for (int port=0;port<4;port++)
	{
		for (int i=0;joypad_settings_K[i].name;i++)
		{
			char temp[512];
			sprintf(temp,"Port%c_%s",port+'A',&joypad_settings_K[i].name[4]);
			host.ConfigSaveInt("ndc_hookjoy",temp,joypad_settings[port][i].KC);
		}
	}
	host.ConfigSaveInt("ndc_hookjoy","local_port",0);
	host.ConfigSaveStr("ndc_hookjoy","server_addr",server_addr);
	host.ConfigSaveStr("ndc_hookjoy","server_port",server_port);
}