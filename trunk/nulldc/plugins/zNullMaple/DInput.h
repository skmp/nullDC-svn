/*
**	DInput.h
*/
#pragma once

#include <vector>
using namespace std;



struct InputDevice
{
	struct {
		u8  Connected;
		u8  DevType;
		u16 OtherFlags;
	};

	GUID					guidDev;
	LPDIRECTINPUTDEVICE8	diDev;
	vector<u32>				KeyMap;

	union {
		u8 diKeys[256]; 
		DIJOYSTATE diJoyState;
	};
};

struct DI_DevInfo {
	GUID guid;
	char name[MAX_PATH];
};
extern InputDevice			InputDev[4];
extern vector<DI_DevInfo>	dInputDevList;

s32 GetChInput(InputDevice &iDev);
bool GetDInput(u32 port, Controller_ReadFormat *crf);
bool InitDInput(HINSTANCE hInst);
void TermDInput();

bool GetDInputNameByGUID(char * dest, int len, GUID guid);