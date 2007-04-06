/*
**	DInput.cpp,	nullMaple
*/
#include <windows.h>
#include <dinput.h>
#include <vector>
using namespace std;

#include "zNullMaple.h"
#include "MapleBus.h"



LPDIRECTINPUT8			dInputObj;
vector<GUID>			dInputDevList;
char					KeyBuffer[256];


struct ControlElement
{
	struct {
		u8  Connected;
		u8  DevType;
		u16 OtherFlags;
	};

	LPDIRECTINPUTDEVICE8	diDev;

} InputDev[4];


bool GetDInput(u32 port, Controller_ReadFormat *crf)
{
	u8 diKeys[256]; 

	if(4<=port)
		return false;

	if(false==InputDev[port].Connected)
		return false;

	printf("GetDInput(%d)\n", port);

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
	case DI8DEVCLASS_POINTER:
	//	return false;

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