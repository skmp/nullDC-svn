/*
**	DInput.cpp,	nullMaple
*/
#include <windows.h>
#include <dinput.h>
#include <vector>
using namespace std;

#include "zNullMaple.h"
#include "MapleBus.h"
#include "DInput.h"



LPDIRECTINPUT8		dInputObj;
vector<GUID>		dInputDevList;

InputDevice			InputDev[4];



s32 GetChInput(InputDevice &iDev)
{
	if(false==iDev.Connected)
		return -1;

	iDev.diDev->Poll();

	switch(iDev.DevType)
	{
	case DI8DEVCLASS_KEYBOARD:
		{
			if(DI_OK != iDev.diDev->GetDeviceState(256, iDev.diKeys)) 
				return -1;

			for(int k=0; k<256; k++) {
				if(iDev.diKeys[k] & 0x80)
					return 0x30000000 | (k+1);	// *FIXME* yea just remember the +1
			}

			return 0;
		}

	case DI8DEVCLASS_GAMECTRL:
		{
			if(DI_OK != iDev.diDev->GetDeviceState(sizeof(DIJOYSTATE), &iDev.diJoyState)) 
				return -1;


			//if(diJoyState.lX)
			//if(diJoyState.lY)
			//if(diJoyState.lZ)
			//if(diJoyState.lRx)
			//if(diJoyState.lRy)
			//if(diJoyState.lRz)

			//if(diJoyState.rglSlider)
			//if(diJoyState.rgdwPOV)

			for(int b=0; b<32; b++)
				if(iDev.diJoyState.rgbButtons[b])
					return 0x40000000 | (b+1);	// *FIXME* yea just remember the +b


	/*		typedef struct DIJOYSTATE {
				LONG    lX;                     // x-axis position              
				LONG    lY;                     // y-axis position              
				LONG    lZ;                     // z-axis position              
				LONG    lRx;                    // x-axis rotation              
				LONG    lRy;                    // y-axis rotation              
				LONG    lRz;                    // z-axis rotation              
				LONG    rglSlider[2];           // extra axes positions         
				DWORD   rgdwPOV[4];             // POV directions               
				BYTE    rgbButtons[32];         // 32 buttons                   
			} DIJOYSTATE, *LPDIJOYSTATE;
*/
		}


	case DI8DEVCLASS_POINTER:
		printf("Mouse Input Type, Unsupported!\n");
		return -2;

	default:
		printf("Default Input Type, WTF?\n");
		return -3;
	}

	return -6;
}

bool GetDInput(u32 port, Controller_ReadFormat *crf)
{
	u8 diKeys[256]; 

	if(4<=port)
		return false;

	if(false==InputDev[port].Connected)
		return false;

	InputDev[port].diDev->Poll();

	switch(InputDev[port].DevType)
	{
	case DI8DEVCLASS_KEYBOARD:
	{
		if(DI_OK != InputDev[port].diDev->GetDeviceState(256, diKeys)) 
			return false;

		crf->C		= (diKeys[DIK_A]		& 0x80)	? 1 : 0 ;
		crf->B		= (diKeys[DIK_S]		& 0x80)	? 1 : 0 ;
		crf->A		= (diKeys[DIK_D]		& 0x80)	? 1 : 0 ;
		crf->Start	= (diKeys[DIK_RETURN]	& 0x80)	? 1 : 0 ;
		crf->Ua		= (diKeys[DIK_UP]		& 0x80)	? 1 : 0 ;
		crf->Da		= (diKeys[DIK_DOWN]		& 0x80)	? 1 : 0 ;
		crf->La		= (diKeys[DIK_LEFT]		& 0x80)	? 1 : 0 ;
		crf->Ra		= (diKeys[DIK_RIGHT]	& 0x80)	? 1 : 0 ;

		crf->Z		= (diKeys[DIK_Z]		& 0x80)	? 1 : 0 ;
		crf->Y		= (diKeys[DIK_X]		& 0x80)	? 1 : 0 ;
		crf->X		= (diKeys[DIK_C]		& 0x80)	? 1 : 0 ;
		crf->D		= (diKeys[DIK_V]		& 0x80)	? 1 : 0 ;
		crf->Ub		= 0;
		crf->Db		= 0;
		crf->Lb		= 0;
		crf->Rb		= 0;

		return true;
	}

	case DI8DEVCLASS_GAMECTRL:
	{
		DIJOYSTATE diJoyState;
		if(DI_OK != InputDev[port].diDev->GetDeviceState(sizeof(DIJOYSTATE), &diJoyState)) 
			return false;

		crf->C		= diJoyState.rgbButtons[0];
		crf->B		= diJoyState.rgbButtons[1];
		crf->A		= diJoyState.rgbButtons[2];
		crf->Start	= diJoyState.rgbButtons[3];
		crf->Ua		= diJoyState.rgbButtons[4];
		crf->Da		= diJoyState.rgbButtons[5];
		crf->La		= diJoyState.rgbButtons[6];
		crf->Ra		= diJoyState.rgbButtons[7];

		crf->Z		= diJoyState.rgbButtons[8];
		crf->Y		= diJoyState.rgbButtons[9];
		crf->X		= diJoyState.rgbButtons[10];
		crf->D		= diJoyState.rgbButtons[11];
		crf->Ub		= 0;
		crf->Db		= 0;
		crf->Lb		= 0;
		crf->Rb		= 0;

		return true;
	}


	case DI8DEVCLASS_POINTER:
		printf("Mouse Input Type, Unsupported!\n");
		return false;

	default:
		printf("Default Input Type, WTF?\n");
		return false;
	}
}

BOOL EnumDevsCB(LPCDIDEVICEINSTANCE lpddi, LPVOID Ref)
{
	dInputDevList.push_back(lpddi->guidInstance);
	printf("Found: %s\n", lpddi->tszProductName);
	return DIENUM_CONTINUE;
}


 
 

bool InitDInput(HINSTANCE hInst)
{
	if(FAILED(DirectInput8Create(hInst,DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&dInputObj,NULL)))
		return false;

	if(DI_OK != dInputObj->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)EnumDevsCB,(void*)0, DIEDFL_ATTACHEDONLY))	{	return false;	}
	if(DI_OK != dInputObj->EnumDevices(DI8DEVCLASS_KEYBOARD, (LPDIENUMDEVICESCALLBACK)EnumDevsCB,(void*)1, DIEDFL_ATTACHEDONLY))	{	return false;	}
	if(DI_OK != dInputObj->EnumDevices(DI8DEVCLASS_POINTER , (LPDIENUMDEVICESCALLBACK)EnumDevsCB,(void*)2, DIEDFL_ATTACHEDONLY))	{	return false;	}

	for(int d=0; d<4; d++) {
		InputDev[d].diDev = NULL;
		InputDev[d].Connected = false;
	}

	dInputObj->CreateDevice(GUID_SysKeyboard, &InputDev[0].diDev, NULL);
	if(NULL == InputDev[0].diDev)
		return false;

//	if(FAILED(dInputDev[d]->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))) { return false; }
	if(FAILED(InputDev[0].diDev->SetDataFormat(&c_dfDIKeyboard)))
		return false;
	if(FAILED(InputDev[0].diDev->Acquire()))
		return false;

	InputDev[0].Connected = true;
	InputDev[0].DevType = DI8DEVCLASS_KEYBOARD;

	return true;
}

void TermDInput()
{
	for(int d=0; d<4; d++) {
		if(NULL!=InputDev[d].diDev) {
			InputDev[d].diDev->Unacquire();
			InputDev[d].diDev->Release();
			InputDev[d].diDev = NULL;
		}
	}
	dInputObj->Release();
	dInputObj = NULL;
}