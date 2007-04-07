/*
**	DInput.h
*/
#pragma once




struct InputDevice
{
	struct {
		u8  Connected;
		u8  DevType;
		u16 OtherFlags;
	};

	LPDIRECTINPUTDEVICE8	diDev;

	union {
		u8 diKeys[256]; 
		DIJOYSTATE diJoyState;
	};

};

extern InputDevice InputDev[4];


s32 GetChInput(InputDevice &iDev);
bool GetDInput(u32 port, Controller_ReadFormat *crf);
bool InitDInput(HINSTANCE hInst);
void TermDInput();