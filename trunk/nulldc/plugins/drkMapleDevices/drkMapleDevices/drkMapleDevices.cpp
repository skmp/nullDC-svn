// drkMapleDevices.cpp : Defines the entry point for the DLL application.
//

#include "..\..\..\nullDC\plugins\plugin_header.h"
#include <memory.h>

#ifdef UNICODE
#undef UNICODE
#endif
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

u16 kcode=0xFFFF;
s8 joyx=0,joyy=0;
s8 joy2x=0,joy2y=0;
u8 rt=0,lt=0;
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

#pragma pack(1)
char testJoy_strName[64] = "Emulated Dreamcast Controler\0";
char testJoy_strName_nul[64] = "Null Dreamcast Controler\0";
char testJoy_strName_vmu[64] = "Emulated VMU\0";
char testJoy_strName_kbd[64] = "Emulated Dreamcast Keyboard\0";
char testJoy_strName_mouse[64] = "Emulated Dreamcast Mouse\0";
char testJoy_strName_dreameye_1[64] = "Dreamcast Camera Flash  Devic\0";
char testJoy_strName_dreameye_2[64] = "Dreamcast Camera Flash LDevic\0";
char testJoy_strName_mic[64] = "MicDevice for Dreameye\0";
char testJoy_strBrand[64] = "Faked by drkIIRaziel && ZeZu , made for nullDC\0";
char testJoy_strBrand_2[64] = "Produced By or Under License From SEGA ENTERPRISES,LTD.\0";

#define key_CONT_C  (1 << 0);
#define key_CONT_B  (1 << 1);
#define key_CONT_A  (1 << 2);
#define key_CONT_START  (1 << 3);
#define key_CONT_DPAD_UP  (1 << 4);
#define key_CONT_DPAD_DOWN  (1 << 5);
#define key_CONT_DPAD_LEFT  (1 << 6);
#define key_CONT_DPAD_RIGHT  (1 << 7);
#define key_CONT_Z  (1 << 8);
#define key_CONT_Y  (1 << 9);
#define key_CONT_X  (1 << 10);
#define key_CONT_D  (1 << 11);
#define key_CONT_DPAD2_UP  (1 << 12);
#define key_CONT_DPAD2_DOWN  (1 << 13);
#define key_CONT_DPAD2_LEFT  (1 << 14);
#define key_CONT_DPAD2_RIGHT  (1 << 15);	

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
void kb_down(u8 kc)
{
	void Init_kb_map();
	Init_kb_map();
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
dlgp* oldptr;
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
		kb_down(kb_map[wParam & 0xFF]);
		switch(wParam)
		{
		case 'V':
			kcode &= 0xFFFF - key_CONT_A;
			break;
		case 'X':
			kcode &= 0xFFFF - key_CONT_B;
			break;
		case 'B':
			kcode &= 0xFFFF - key_CONT_C;
			break;
		case 'N':
			kcode &= 0xFFFF - key_CONT_D;
			break;
		
		case 'M':
			kcode &= 0xFFFF - key_CONT_Z;
			break;
		
		case 'Z':
			kcode &= 0xFFFF - key_CONT_Y;
			break;
		
		case 'C':
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

		case 'L'://analog right
			joyx= +126;
			break;
		case 'J'://alalog left
			joyx= -126;
			break;

		case 'I'://analog up
			joyy= -126;
			break;
		case 'K'://analog down
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
		kb_up(kb_map[wParam & 0xFF]);
		switch(wParam)
		{
		case 'V':
			kcode |= key_CONT_A;
			break;
		case 'X':
			kcode |= key_CONT_B;
			break;
		case 'B':
			kcode |= key_CONT_C;
			break;
		case 'N':
			kcode |= key_CONT_D;
			break;

		case 'M':
			kcode |= key_CONT_Z;
			break;
		
		case 'Z':
			kcode |= key_CONT_Y;
			break;
		
		case 'C':
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

		case 'J'://analog right
			joyx=0;
			break;
		case 'L'://alalog left
			joyx=0;
			break;

		case 'I'://analog up
			joyy=0;
			break;
		case 'K'://analog down
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
	}
	return oldptr(hWnd,uMsg,wParam,lParam);
}

void cfgdlg(PluginType type,void* window)
{
	printf("ndcMAPLE :No config kthx\n");
}//called when plugin is used by emu (you should do first time init here)
void* handle;
void Init_kb_map();
void dcInitPvr(void* aparam,PluginType type)
{
	maple_init_params* mpi=(maple_init_params*)aparam;
	handle=mpi->WindowHandle;
	oldptr = (dlgp*)SetWindowLongPtr((HWND)handle,GWL_WNDPROC,(LONG)sch);
	Init_kb_map();
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void dcTermPvr(PluginType type)
{
	SetWindowLongPtr((HWND)handle,GWL_WNDPROC,(LONG)oldptr);
}

//It's suposed to reset anything but vram (vram is set to 0 by emu)
void dcResetPvr(bool Manual,PluginType type)
{
	//hahah do what ? ahahahahahaha
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInitPvr(PluginType type)
{
	//maby init here ?
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTermPvr(PluginType type)
{
	//term here ?
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
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
		sk(2d,'-');
		//2F-30 "@" and "[" (the 2 keys right of P) 
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
		sk(28,VK_DOWN);
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
//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	sprintf(info->Name,"ndcMaple");
	info->PluginVersion.full=NDC_MakeVersion(1,2,3);
	

	info->Init=dcInitPvr;
	info->Term=dcTermPvr;
	info->Reset=dcResetPvr;

	info->ThreadInit=dcThreadInitPvr;
	info->ThreadTerm=dcThreadTermPvr;

	info->ShowConfig=cfgdlg;
	info->Type=PluginType::MapleDevice;
}



u8 GetBtFromSgn(s8 val);

#define w32(data) *(u32*)buffer_out_b=(data);buffer_out_b+=4;buffer_out_len+=4
#define w16(data) *(u16*)buffer_out_b=(data);buffer_out_b+=2;buffer_out_len+=2
#define w8(data) *(u8*)buffer_out_b=(data);buffer_out_b+=1;buffer_out_len+=1

void KbdDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
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
				w8((u8)testJoy_strBrand[i]);
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
			printf("UNKOWN MAPLE COMMAND \n");
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
void DreamEye_mainDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
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
				w8((u8)testJoy_strBrand[i]);
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
void DreamEye_subDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
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
				w8((u8)testJoy_strBrand[i]);
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


void MicDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	printf("MicDMA[0x%x | %d %x] : UNKOWN MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
	printf(" buffer in size : %d\n",buffer_in_len);
	if (buffer_in_len==-4)
		buffer_in_len=0;
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
				w8((u8)testJoy_strBrand[i]);
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


void MouseDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
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
				w8((u8)testJoy_strBrand[i]);
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
			printf("UNKOWN MAPLE COMMAND \n");
			break;
	}
}


void ControllerDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
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
				w8((u8)testJoy_strName[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand[i]);
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
			w16(kcode); 
			
			//triger
			//1 R
			w8(rt);
			//1 L
			w8(lt); 
			//joyx
			//1
			w8(GetBtFromSgn(joyx));
			//joyy
			//1
			w8(GetBtFromSgn(joyy));

			//1
			w8(GetBtFromSgn(joy2x)); 
			//1
			w8(GetBtFromSgn(joy2y)); 
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;

			break;

		default:
			printf("UNKOWN MAPLE COMMAND \n");
			break;
	}
}

void ControllerDMA_nul(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
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
				w8((u8)testJoy_strBrand[i]);
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
			printf("UNKOWN MAPLE COMMAND \n");
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
void VmuDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
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
				w8((u8)testJoy_strBrand[i]);
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
				VMU_info* dev=(VMU_info*)((*device_instance).DevData);

				buffer_out[0] = (2<<24);
				u32 Block = (SWAP32(buffer_in[1]))&0xffff;
				buffer_out[1] = buffer_in[1];
				if (Block>255)
				{
					printf("Block read : %d\n",Block);
					printf("BLOCK READ ERROR\n");
					Block&=256;
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
				VMU_info* dev=(VMU_info*)((*device_instance).DevData);

				u32 Block = (SWAP32(buffer_in[1]))&0xffff;
				u32 Phase = ((SWAP32(buffer_in[1]))>>16)&0xff; 
				//printf("Block wirte : %d:%d , %d bytes\n",Block,Phase,(buffer_in_len-4));
				memcpy(&dev->data[Block*512+Phase*(512/4)],&buffer_in[2],(buffer_in_len-4));
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
			printf("UNKOWN MAPLE COMMAND \n");
			break;
	}
}


void CreateInstance(maple_device*dev,maple_device_instance& inst,u8 port)
{
	if (dev->id==0)
	{
		inst.MapleDeviceDMA=ControllerDMA;
		inst.DevData=0;
	}
	else if (dev->id==1)
	{
		inst.DevData=malloc(sizeof(VMU_info));
		sprintf(((VMU_info*)inst.DevData)->file,"vmu_data_port%x.bin",port);
		FILE* f=fopen(((VMU_info*)inst.DevData)->file,"rb");
		if (f)
		{
			fread(((VMU_info*)inst.DevData)->data,1,128*1024,f);
			fclose(f);
		}
		inst.MapleDeviceDMA=VmuDMA;
	}
	else if (dev->id==2)
	{
		inst.MapleDeviceDMA=ControllerDMA_nul;
		inst.DevData=0;
	}
	else if (dev->id==3)
	{
		inst.MapleDeviceDMA=KbdDMA;
		inst.DevData=0;
	}
	else if (dev->id==4)
	{
		inst.MapleDeviceDMA=MouseDMA;
		inst.DevData=0;
	}
	else if ( dev->id==5)
	{
		inst.MapleDeviceDMA=DreamEye_mainDMA;
		inst.DevData=0;
	}
	else if ( dev->id==6)
	{
		inst.MapleDeviceDMA=DreamEye_subDMA;
		inst.DevData=0;
	}
	else if ( dev->id==7)
	{
		inst.MapleDeviceDMA=MicDMA;
		inst.DevData=0;
	}
	printf("Created instance of device %s on port 0x%x\n",dev->name,port);
}


void DestroyInstance(maple_device*dev,maple_device_instance& inst)
{
	if (inst.DevData)
		free(inst.DevData);
	printf("Deleted instance of device %s \n",dev->name);
}
//Give a list of the devices to teh emu
EXPORT void dcGetMapleInfo(maple_plugin_if* info)
{
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
}


