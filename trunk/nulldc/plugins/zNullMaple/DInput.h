/*
**	DInput.h
*/
#pragma once

#include <vector>
using namespace std;



class InputDevice
{
public:

	u32 DevType;
	u32 Connected;

	GUID					guidDev;
	LPDIRECTINPUTDEVICE8	diDev;
	vector<u32>				KeyMap;

	union {
		u8 diKeys[256]; 
		DIJOYSTATE diJoyState;
	};


	bool ReAqcuire(int idx);
};

struct DI_DevInfo {
	GUID guid;
	char name[MAX_PATH];
	DWORD devType;
};
extern InputDevice			InputDev[4];
extern vector<DI_DevInfo>	diDevInfoList;

s32 GetChInput(InputDevice &iDev);
bool GetDInput(u32 port, Controller_ReadFormat *crf);
bool InitDInput(HINSTANCE hInst);
void TermDInput();

bool GetDInputNameByGUID(char * dest, int len, GUID guid);