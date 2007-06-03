/*
**	zNullMaple.h	- Maple Bus Plugin by David Miller -
*/
#pragma once

#include "plugin_header.h"	// plugin header, we'll typedef some shit to make it match my naming schemes etc


#define PL_CALL	FASTCALL

#define MAPLE_SUBDEVICE_DISABLE_ALL \
	(MAPLE_SUBDEVICE_DISABLE_1 | MAPLE_SUBDEVICE_DISABLE_2 | MAPLE_SUBDEVICE_DISABLE_3 | MAPLE_SUBDEVICE_DISABLE_4)


#define MDF_Controller	\
	(MDTF_Hotplug|MDTF_Sub0|MDTF_Sub1)


enum DINPUT_MAPPING
{
	DINPUT_RET_OK		=	0,

	DINPUT_KB_FIRST		=	DINPUT_RET_OK+1,
	DINPUT_KB_LAST		=	DINPUT_KB_FIRST+255,

	DINPUT_GP_BUT1		=	DINPUT_KB_LAST+1,
	DINPUT_GP_BUT32		=	DINPUT_GP_BUT1+31,

	DINPUT_GP_AX1		=	DINPUT_GP_BUT32+1,
	DINPUT_GP_AY1		=	DINPUT_GP_AX1+1,
	DINPUT_GP_AZ1		=	DINPUT_GP_AY1+1,

	DINPUT_GP_AX2		=	DINPUT_GP_AZ1+1,
	DINPUT_GP_AY2		=	DINPUT_GP_AX2+1,
	DINPUT_GP_AZ2		=	DINPUT_GP_AY2+1,

	DINPUT_GP_POV1_U	=	DINPUT_GP_AZ2+1,
	DINPUT_GP_POV1_R	=	DINPUT_GP_POV1_U+1,
	DINPUT_GP_POV1_D	=	DINPUT_GP_POV1_R+1,
	DINPUT_GP_POV1_L	=	DINPUT_GP_POV1_D+1,


	DINPUT_GP_POV2		=	DINPUT_GP_POV1_L+1,
	DINPUT_GP_POV3		=	DINPUT_GP_POV2+1,
	DINPUT_GP_POV4		=	DINPUT_GP_POV3+1,

	DINPUT_GP_SL1		=	DINPUT_GP_POV4+1,
	DINPUT_GP_SL2		=	DINPUT_GP_SL1+1,

	DINPUT_LAST
};

static const char * MapNames[] = 
{
	"[unmapped]",




	// *FIXME* THERE ARE HUGE GAPS HERE //
	"Key ?",	
	"DIK_ESCAPE", 
	"DIK_1", "DIK_2", "DIK_3", "DIK_4", "DIK_5", "DIK_6", "DIK_7", "DIK_8", "DIK_9", "DIK_0", 
	"DIK_MINUS", "DIK_EQUALS", "DIK_BACK", "DIK_TAB", 

//	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
//	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 


	"DIK_Q", "DIK_W", "DIK_E", "DIK_R", "DIK_T", "DIK_Y", "DIK_U", "DIK_I", "DIK_O", "DIK_P", 
	"DIK_LBRACKET", "DIK_RBRACKET", "DIK_RETURN", "DIK_LCONTROL", 
	"DIK_A", "DIK_S", "DIK_D", "DIK_F", "DIK_G", "DIK_H", "DIK_J", "DIK_K", "DIK_L", 
	"DIK_SEMICOLON", "DIK_APOSTROPHE", "DIK_GRAVE", "DIK_LSHIFT", "DIK_BACKSLASH", 
	"DIK_Z", "DIK_X", "DIK_C", "DIK_V", "DIK_B", "DIK_N", "DIK_M", 
	"DIK_COMMA", "DIK_PERIOD", "DIK_SLASH", "DIK_RSHIFT", "DIK_MULTIPLY", "DIK_LMENU", "DIK_SPACE", "DIK_CAPITAL", 
	"DIK_F1", "DIK_F2", "DIK_F3", "DIK_F4", "DIK_F5", "DIK_F6", "DIK_F7", "DIK_F8", "DIK_F9", "DIK_F10", 
	"DIK_NUMLOCK", "DIK_SCROLL", "DIK_NUMPAD7", "DIK_NUMPAD8", "DIK_NUMPAD9", "DIK_SUBTRACT", "DIK_NUMPAD4", 
	"DIK_NUMPAD5", "DIK_NUMPAD6", "DIK_ADD", "DIK_NUMPAD1",

/*	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", */


	"DIK_NUMPAD2", "DIK_NUMPAD3", "DIK_NUMPAD0", "DIK_DECIMAL", 

//	"Key ?", "Key ?", "Key ?", "Key ?",

	"Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 

/*	"Key ?",	
	"DIK_ESCAPE", 
	"DIK_1", "DIK_2", "DIK_3", "DIK_4", "DIK_5", "DIK_6", "DIK_7", "DIK_8", "DIK_9", "DIK_0", 
	"DIK_MINUS", "DIK_EQUALS", "DIK_BACK", "DIK_TAB", 




	"DIK_Q", "DIK_W", "DIK_E", "DIK_R", "DIK_T", "DIK_Y", "DIK_U", "DIK_I", "DIK_O", "DIK_P", 
	"DIK_LBRACKET", "DIK_RBRACKET", "DIK_RETURN", "DIK_LCONTROL", 
	"DIK_A", "DIK_S", "DIK_D", "DIK_F", "DIK_G", "DIK_H", "DIK_J", "DIK_K", "DIK_L", 
	"DIK_SEMICOLON", "DIK_APOSTROPHE", "DIK_GRAVE", "DIK_LSHIFT", "DIK_BACKSLASH", 
	"DIK_Z", "DIK_X", "DIK_C", "DIK_V", "DIK_B", "DIK_N", "DIK_M", 
	"DIK_COMMA", "DIK_PERIOD", "DIK_SLASH", "DIK_RSHIFT", "DIK_MULTIPLY", "DIK_LMENU", "DIK_SPACE", "DIK_CAPITAL", 
	"DIK_F1", "DIK_F2", "DIK_F3", "DIK_F4", "DIK_F5", "DIK_F6", "DIK_F7", "DIK_F8", "DIK_F9", "DIK_F10", 
	"DIK_NUMLOCK", "DIK_SCROLL", "DIK_NUMPAD7", "DIK_NUMPAD8", "DIK_NUMPAD9", "DIK_SUBTRACT", "DIK_NUMPAD4", 
	"DIK_NUMPAD5", "DIK_NUMPAD6", "DIK_ADD", "DIK_NUMPAD1",
	
	"DIK_NUMPAD2", "DIK_NUMPAD3", "DIK_NUMPAD0", "DIK_DECIMAL", 

	// Watch from here out

	"Key ?",	"Key ?",	// 0x54,55


	"DIK_OEM_102", 
	"DIK_F11", "DIK_F12", 

	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 	// 0x59 - 0x63
	
	"DIK_F13", "DIK_F14", "DIK_F15", 

	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?",	// 67-6F

	"DIK_KANA",
	"Key ?", "Key ?",	// 

	"DIK_ABNT_C1", "DIK_CONVERT", "DIK_NOCONVERT", "DIK_YEN", "DIK_ABNT_C2", 
	"DIK_NUMPADEQUALS", "DIK_PREVTRACK", "DIK_AT", "DIK_COLON", "DIK_UNDERLINE", "DIK_KANJI", 
	"DIK_STOP", "DIK_AX", "DIK_UNLABELED", "DIK_NEXTTRACK", "DIK_NUMPADENTER", "DIK_RCONTROL", 
	"DIK_MUTE", "DIK_CALCULATOR", "DIK_PLAYPAUSE", "DIK_MEDIASTOP", "DIK_VOLUMEDOWN", "DIK_VOLUMEUP", 
	"DIK_WEBHOME", "DIK_NUMPADCOMMA", "DIK_DIVIDE", "DIK_SYSRQ", "DIK_RMENU", 
	"DIK_PAUSE", "DIK_HOME", "DIK_UP", "DIK_PRIOR",
	"DIK_LEFT", "DIK_RIGHT", "DIK_END", "DIK_DOWN", "DIK_NEXT",
	"DIK_INSERT", "DIK_DELETE", "DIK_LWIN", "DIK_RWIN", 
	"DIK_APPS", "DIK_POWER", "DIK_SLEEP", "DIK_WAKE", 
	"DIK_WEBSEARCH", "DIK_WEBFAVORITES", "DIK_WEBREFRESH", "DIK_WEBSTOP", "DIK_WEBFORWARD", "DIK_WEBBACK", 
	"DIK_MYCOMPUTER", "DIK_MAIL", "DIK_MEDIASELECT", 
*/



	// GamePad //
	"Button 1 ", "Button 2 ", "Button 3 ", "Button 4 ", "Button 5 ", "Button 6 ", "Button 7 ", "Button 8 ",
	"Button 9 ", "Button 10", "Button 11", "Button 12", "Button 13", "Button 14", "Button 15", "Button 16", 
	"Button 17", "Button 18", "Button 19", "Button 20", "Button 21", "Button 22", "Button 23", "Button 24",
	"Button 25", "Button 26", "Button 27", "Button 28", "Button 29", "Button 30", "Button 31", "Button 32", 

	"Axis X1", "Axis Y1", "Axis Z1", 
	"Axis X2", "Axis Y2", "Axis Z2", 

	"POV 1 Up", "POV 1 Right", "POV 1 Down", "POV 1 Left", 
	"POV 2", "POV 3", "POV 4", 

	"Slider 1", "Slider 2",

	// Mouse //
	"Mouse Axis 1",		"Mouse Axis 2",		"Mouse Axis 3", 
	"Mouse Button 1",	"Mouse Button 2",	"Mouse Button 3", 


	// End //
	"EndOfMappingNames"
};