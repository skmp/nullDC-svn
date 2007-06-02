/*
**	DInput.h
*/
#pragma once

#include <vector>
using namespace std;




/*
	Host and functions
	The game unit side is called the "host," and the function/s configuring the peripheral to which it is connected is called "function/s".
	A function does not refer to the product itself, but rather to the elements making up the product. One product can have multiple functions.
	Peripherals are groups of functions, and access from the host is performed in peripheral units. Access to functions is performed by designating the specific function type.
	Multiple functions can be used with the one peripheral. However, with Maple Bus 1.0, a maximum of three functions can be used.
*/

struct maple_Function
{
	GUID device;		// Parent device to Button / Axe GUIDs
	GUID dev_obj[32];	// Maps FD Bits to DInput:GUID for Device !
};

/* pure  virtual class Peripheral
{
	Function device;
	Function expansion[2];	
};
*/


class InputDevice
{
public:

	u32 DevType;
	u32 Connected;

	GUID					guidDev;
	LPDIRECTINPUTDEVICE8	diDev;
	//vector<u32>				KeyMap;
	u32						KeyMap[32];

	//u32 ButtonMap[64];

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