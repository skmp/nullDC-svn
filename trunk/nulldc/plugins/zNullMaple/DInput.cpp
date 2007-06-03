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

		for(int b=0; b<32; b++)
			if(iDev.diJoyState.rgbButtons[b])
				return DINPUT_GP_BUT1 + b;

		if(-1 != iDev.diJoyState.rgdwPOV[0]) {
			if(0x0000==iDev.diJoyState.rgdwPOV[0])	return DINPUT_GP_POV1_U;
			if(0x2328==iDev.diJoyState.rgdwPOV[0])	return DINPUT_GP_POV1_R;
			if(0x4650==iDev.diJoyState.rgdwPOV[0])	return DINPUT_GP_POV1_D;
			if(0x6978==iDev.diJoyState.rgdwPOV[0])	return DINPUT_GP_POV1_L;
		}
		if(-1 != iDev.diJoyState.rgdwPOV[1]) return DINPUT_GP_POV2;
		if(-1 != iDev.diJoyState.rgdwPOV[2]) return DINPUT_GP_POV3;
		if(-1 != iDev.diJoyState.rgdwPOV[3]) return DINPUT_GP_POV4;

		if((iDev.diJoyState.lX/256)&0xF8)	return DINPUT_GP_AX1;
		if((iDev.diJoyState.lY/256)&0xF8)	return DINPUT_GP_AY1;
		if((iDev.diJoyState.lZ/256)&0xF8)	return DINPUT_GP_AZ1;
		if((iDev.diJoyState.lRx/256)&0xF8)	return DINPUT_GP_AX2;
		if((iDev.diJoyState.lRy/256)&0xF8)	return DINPUT_GP_AY2;
		if((iDev.diJoyState.lRz/256)&0xF8)	return DINPUT_GP_AZ2;

	/*	if(iDev.diJoyState.rglSlider)	[2]	*/
		return 0;
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

__inline static u32 GetCompatJoyInput(DIJOYSTATE * pJS, u32 map)
{
//	printf("GetCompatJoyInput()::map: %d\n", map);

	if((DINPUT_GP_BUT1+map) <= DINPUT_GP_BUT32)			// Checking buttons, easy
	{
		if(map > 31) printf("-ERROR BUTTON MAP > 31 !\n");
		return (pJS->rgbButtons[map] & 0x80)	? 0 : 1 ;
	}
	else if((DINPUT_GP_BUT1+map) <= DINPUT_GP_AZ2)		// Checking axes
	{
		u32 axis = 5 - (DINPUT_GP_AZ2 - (DINPUT_GP_BUT1+map));

		switch(axis)
		{
		case 0:	return (pJS->lX /256)&0xFC;	// *FIXME* use real deadzone and maxes
		case 1:	return (pJS->lY /256)&0xFC;
		case 2:	return (pJS->lZ /256)&0xFC;
		case 3:	return (pJS->lRx/256)&0xFC;
		case 4:	return (pJS->lRy/256)&0xFC;
		case 5:	return (pJS->lRz/256)&0xFC;
		}

		return 0;
	}
	else if((DINPUT_GP_BUT1+map) <= DINPUT_GP_POV1_L)	// Checking POV
	{
		u32 pov = 3 - (DINPUT_GP_POV1_L - (DINPUT_GP_BUT1+map));

		if(-1 == pJS->rgdwPOV[0]) return 1;
		if((pJS->rgdwPOV[0] >= 0x0000) && (pJS->rgdwPOV[0] < 0x2328) && (pov==0))	return 0;
		if((pJS->rgdwPOV[0] >= 0x2328) && (pJS->rgdwPOV[0] < 0x4650) && (pov==1))	return 0;
		if((pJS->rgdwPOV[0] >= 0x4650) && (pJS->rgdwPOV[0] < 0x6978) && (pov==2))	return 0;
		if((pJS->rgdwPOV[0] >= 0x6978) && (pov==3))	return 0;
		return 1;
	}
	return 1;
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

		// *FIXME* kb is fucked here w/ map #
		crf->C		= (diKeys[InputDev[port].KeyMap[0x0]] & 0x80)	? 0 : 1 ;
		crf->B		= (diKeys[InputDev[port].KeyMap[0x1]] & 0x80)	? 0 : 1 ;
		crf->A		= (diKeys[InputDev[port].KeyMap[0x0]] & 0x80)	? 0 : 1 ;
		crf->Start	= (diKeys[InputDev[port].KeyMap[0xB]] & 0x80)	? 0 : 1 ;
		crf->Ua		= (diKeys[InputDev[port].KeyMap[0x5]] & 0x80)	? 0 : 1 ;
		crf->Da		= (diKeys[InputDev[port].KeyMap[0x6]] & 0x80)	? 0 : 1 ;
		crf->La		= (diKeys[InputDev[port].KeyMap[0x7]] & 0x80)	? 0 : 1 ;
		crf->Ra		= (diKeys[InputDev[port].KeyMap[0x8]] & 0x80)	? 0 : 1 ;

		crf->Z		= (diKeys[InputDev[port].KeyMap[0x0]] & 0x80)	? 0 : 1 ;
		crf->Y		= (diKeys[InputDev[port].KeyMap[0x4]] & 0x80)	? 0 : 1 ;
		crf->X		= (diKeys[InputDev[port].KeyMap[0x3]] & 0x80)	? 0 : 1 ;
		crf->D		= (diKeys[InputDev[port].KeyMap[0x0]] & 0x80)	? 0 : 1 ;
		crf->Ub		= 1;
		crf->Db		= 1;
		crf->Lb		= 1;
		crf->Rb		= 1;

		return true;
	}

	case DI8DEVCLASS_GAMECTRL:
	{
		DIJOYSTATE diJoyState;
		if(DI_OK != InputDev[port].diDev->GetDeviceState(sizeof(DIJOYSTATE), &diJoyState)) 
			return false;

		// A,B,X,Y,  U,D,L,R,  Ax1,Ax2, Start, LT,RT

		crf->C		= 1;
		crf->B		= GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x1]);
		crf->A		= GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x0]);
		crf->Start	= GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0xA]);
		crf->Ua		= GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x4]);
		crf->Da		= GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x5]);
		crf->La		= GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x6]);
		crf->Ra		= GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x7]);

		crf->Z		= 1;
		crf->Y		= GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x3]);
		crf->X		= GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x2]);
		crf->D		= 1;
		crf->Ub		= 1;
		crf->Db		= 1;
		crf->Lb		= 1;
		crf->Rb		= 1;

		crf->Av[0] = 0;	//GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0xC]);
		crf->Av[1] = 0;	//GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0xD]);
		crf->Av[2] = GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x8]);
		crf->Av[3] = GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x9]);
		crf->Av[4] = 128;	//GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x8]);
		crf->Av[5] = 128;	//GetCompatJoyInput(&diJoyState, InputDev[port].KeyMap[0x8]);

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
	//printf("\tFound: %s\n", lpddoi->tszName);
	return DIENUM_CONTINUE;
}

/*
	DirectInputDevice objects instantiate the IDirectInputDevice8 Interface. 
	The application ascertains the number and type of device objects available
	by using the IDirectInputDevice8::EnumObjects method. 
	Individual device objects are not encapsulated as code objects, 
	but are described in DIDEVICEOBJECTINSTANCE structures.
	
	All DirectInput interfaces are available in ANSI and Unicode versions. 
	If "UNICODE" is defined during compilation, the Unicode versions are used.
*/

BOOL EnumDevsCB(LPCDIDEVICEINSTANCE lpddi, LPVOID Ref)
{
	DI_DevInfo didi;
	didi.devType = lpddi->dwDevType;
	didi.guid = lpddi->guidInstance;
	strcpy(didi.name, lpddi->tszInstanceName);

/*	LPDIRECTINPUTDEVICE8 diDev;
	if(FAILED(dInputObj->CreateDevice(lpddi->guidInstance, &diDev, NULL)))
	{
		printf("Couldn't Create Obj for %s, DisConnecting!\n", lpddi->tszInstanceName);
		return DIENUM_STOP;
	}
	if(DI_OK != diDev->EnumObjects((LPDIENUMDEVICEOBJECTSCALLBACK)EnumObjsCB,(void*)0, DIDFT_ALL)) { return DIENUM_STOP; }

	diDev->Unacquire();
	diDev->Release();*/
	
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

		InputDev[d].Connected = (-1==guidIdx) ? false : true ;

		if(InputDev[d].Connected)
		{
			if(!InputDev[d].ReAqcuire(guidIdx))
				InputDev[d].Connected = false;
		}
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