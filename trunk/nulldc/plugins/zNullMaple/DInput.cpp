/*
**	DInput.cpp,	nullMaple
*/
#include <windows.h>
#include <dinput.h>

#include "zNullMaple.h"
#include "MapleBus.h"
#include "DInput.h"



LPDIRECTINPUT8		dInputObj;
InputDevice			InputDev[4];
vector<DI_DevInfo>	diDevInfoList;



s32 GetChInput(InputDevice &iDev)
{
	if(false==iDev.Connected)
		return -1;
	if(NULL==iDev.diDev)
		return -2;

	iDev.diDev->Poll();

	switch(iDev.DevType)
	{
	case DI8DEVCLASS_KEYBOARD:
	{
		if(DI_OK != iDev.diDev->GetDeviceState(256, iDev.diKeys)) 
			return -1;

		int test = DIK_J;

		for(int k=0; k<256; k++) {
			if(iDev.diKeys[k] & 0x80)
				return (DINPUT_KB_FIRST + k);
		}
		return 0;
	}

	case DI8DEVCLASS_GAMECTRL:
	{
		if(DI_OK != iDev.diDev->GetDeviceState(sizeof(DIJOYSTATE), &iDev.diJoyState)) 
			return -1;

	/*	if(iDev.diJoyState.lX)
		if(iDev.diJoyState.lY)
		if(iDev.diJoyState.lZ)
		if(iDev.diJoyState.lRx)
		if(iDev.diJoyState.lRy)
		if(iDev.diJoyState.lRz)

		if(iDev.diJoyState.rglSlider)	[2]
		if(iDev.diJoyState.rgdwPOV)		[4]	*/

		for(int b=0; b<32; b++)
			if(iDev.diJoyState.rgbButtons[b])
				return DINPUT_GP_BUT1 + b;
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
		return false;
	}
}

BOOL EnumObjsCB(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	printf("\tFound: %s\n", lpddoi->tszName);
	return DIENUM_CONTINUE;
}

BOOL EnumDevsCB(LPCDIDEVICEINSTANCE lpddi, LPVOID Ref)
{
	DI_DevInfo didi;
	didi.devType = lpddi->dwDevType;
	didi.guid = lpddi->guidInstance;
	strcpy(didi.name, lpddi->tszInstanceName);
	
	diDevInfoList.push_back(didi);
	return DIENUM_CONTINUE;
}

	// *FIXME* this shit is broke //
bool GetDInputNameByGUID(char * dest, int len, GUID guid)
{
	LPDIRECTINPUTDEVICE8 diDev;
	if(FAILED(dInputObj->CreateDevice(guid, &diDev, NULL))) {
		printf("GetDInputNameByGUID::CreateDevice Failed!\n");
		return false;
	} else {
		DIPROPSTRING dips;
		dips.diph.dwSize		= sizeof(DIPROPSTRING);
		dips.diph.dwHeaderSize	= sizeof(DIPROPHEADER); 
		dips.diph.dwObj			= 0; // device property 
		dips.diph.dwHow			= DIPH_DEVICE; 

		diDev->Acquire();

		HRESULT hr = diDev->GetProperty(DIPROP_PRODUCTNAME, &dips.diph);
		if(FAILED(hr)) {
			printf("GetDInputNameByGUID::GetProperty() Failed, hr=%X!\n",hr);
			printf("Vals: %X %X %X %X %X\n", DIERR_INVALIDPARAM,
				DIERR_NOTEXCLUSIVEACQUIRED, DIERR_NOTINITIALIZED, DIERR_OBJECTNOTFOUND, DIERR_UNSUPPORTED);

		}

		printf("GetNameByGUID: %ws \n", dips.wsz);
		strcpy_s(dest, len, (char *)dips.wsz);

		diDev->Unacquire();
		diDev->Release();
	}
	return true;
}
 
 

bool InitDInput(HINSTANCE hInst)
{
	if(FAILED(DirectInput8Create(hInst,DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&dInputObj,NULL)))
		return false;

	if(DI_OK != dInputObj->EnumDevices(DI8DEVCLASS_POINTER , (LPDIENUMDEVICESCALLBACK)EnumDevsCB,(void*)2, DIEDFL_ATTACHEDONLY))	{	return false;	}
	if(DI_OK != dInputObj->EnumDevices(DI8DEVCLASS_KEYBOARD, (LPDIENUMDEVICESCALLBACK)EnumDevsCB,(void*)3, DIEDFL_ATTACHEDONLY))	{	return false;	}
	if(DI_OK != dInputObj->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)EnumDevsCB,(void*)4, DIEDFL_ATTACHEDONLY))	{	return false;	}

	for(int d=0; d<4; d++)
	{
		int guidIdx = -1;
		InputDev[d].diDev = NULL;
		for(size_t x=0; x<diDevInfoList.size(); x++)
			if(diDevInfoList[x].guid == InputDev[d].guidDev)
				guidIdx = (int)x;

		InputDev[d].Connected = (-1==guidIdx) ;

		if(InputDev[d].Connected)
		{
			if(!InputDev[d].ReAqcuire(guidIdx))
				InputDev[d].Connected = false;

		/*	if(FAILED(dInputObj->CreateDevice(InputDev[d].guidDev, &InputDev[d].diDev, NULL)))
			{
				printf("Couldn't Create Obj for %s, DisConnecting!\n", diDevInfoList[guidIdx].name);
				InputDev[d].Connected = false;
			}*/
		}
	/*	if(InputDev[d].Connected)
		{
		//	if(FAILED(dInputDev[d]->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))) { return false; }

			switch(InputDev[d].DevType)
			{
			case DI8DEVCLASS_GAMECTRL:
				if(FAILED(InputDev[d].diDev->SetDataFormat(&c_dfDIJoystick)))
					return false;
				break;

			case DI8DEVCLASS_KEYBOARD:
				if(FAILED(InputDev[d].diDev->SetDataFormat(&c_dfDIKeyboard)))
					return false;
				break;

			case DI8DEVCLASS_POINTER:
			//	if(FAILED(InputDev[d].diDev->SetDataFormat(&c_dfDIPointer)))
			//		return false;
				return false;
			}
			if(FAILED(InputDev[d].diDev->Acquire()))
				return false;
		}*/
	}

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

u32 GetDevClass(u32 DevType)
{
	printf("GetDevClass(%X)\n",DevType);

	switch(DevType & 0xFF)
	{
	case DI8DEVCLASS_ALL:			return DI8DEVCLASS_POINTER;
	case DI8DEVCLASS_DEVICE:		return DI8DEVCLASS_POINTER;
	case DI8DEVCLASS_POINTER:		return DI8DEVCLASS_POINTER;
	case DI8DEVCLASS_KEYBOARD:		return DI8DEVCLASS_KEYBOARD;
	case DI8DEVCLASS_GAMECTRL:		return DI8DEVCLASS_GAMECTRL;
	case DI8DEVTYPE_DEVICE:			return DI8DEVCLASS_POINTER;
	case DI8DEVTYPE_MOUSE:			return DI8DEVCLASS_POINTER;
	case DI8DEVTYPE_KEYBOARD:		return DI8DEVCLASS_KEYBOARD;
	case DI8DEVTYPE_JOYSTICK:		return DI8DEVCLASS_GAMECTRL;
	case DI8DEVTYPE_GAMEPAD:		return DI8DEVCLASS_GAMECTRL;
	case DI8DEVTYPE_DRIVING:		return DI8DEVCLASS_GAMECTRL;
	case DI8DEVTYPE_FLIGHT:			return DI8DEVCLASS_GAMECTRL;
	case DI8DEVTYPE_1STPERSON:		return DI8DEVCLASS_GAMECTRL;
	case DI8DEVTYPE_DEVICECTRL:		return DI8DEVCLASS_POINTER;
	case DI8DEVTYPE_SCREENPOINTER:	return DI8DEVCLASS_POINTER;
	case DI8DEVTYPE_REMOTE:			return DI8DEVCLASS_POINTER;
	case DI8DEVTYPE_SUPPLEMENTAL:	return DI8DEVCLASS_GAMECTRL;
	default:
		printf("Warning: GetDevClass Def. Type (%X)!\n", DevType&0xFF);
		break;
	}
	return DI8DEVCLASS_POINTER;
}

bool InputDevice::ReAqcuire(int idx)
{
	if(diDevInfoList.size() < idx)
		return false;

	if(NULL != diDev) {
		diDev->Unacquire();
		diDev->Release();
		diDev = NULL;
	}

	Connected = true;
	guidDev = diDevInfoList[idx].guid;
	DevType = GetDevClass(diDevInfoList[idx].devType);

	printf("ReAqcuiring: %s, Type: %X\n", diDevInfoList[idx].name, DevType);
	if(FAILED(dInputObj->CreateDevice(diDevInfoList[idx].guid, &diDev, NULL)))
		return false;

	//	if(FAILED(dInputDev[d]->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))) { return false; }

	switch(DevType)
	{
	case DI8DEVCLASS_GAMECTRL:
		if(FAILED(diDev->SetDataFormat(&c_dfDIJoystick)))
			return false;
		break;

	case DI8DEVCLASS_KEYBOARD:
		if(FAILED(diDev->SetDataFormat(&c_dfDIKeyboard)))
			return false;
		break;

	case DI8DEVCLASS_POINTER:
		if(FAILED(diDev->SetDataFormat(&c_dfDIMouse)))
			return false;
		break;

	default:
		printf("DevType Default in ReAqcuire!\n");
		return false;
	}

	if(FAILED(diDev->Acquire()))
		return false;

	return true;
}