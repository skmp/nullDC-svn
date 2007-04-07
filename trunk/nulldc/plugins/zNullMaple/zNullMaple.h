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
	DINPUT_RET_OK	=	0,

	DINPUT_KB_FIRST	=	DINPUT_RET_OK+1,
	DINPUT_KB_LAST	=	DINPUT_KB_FIRST+256,

	DINPUT_GP_BUT1	=	DINPUT_KB_LAST+1,
	DINPUT_GP_BUT32	=	DINPUT_GP_BUT1+32,

	DINPUT_GP_AX1	=	DINPUT_GP_BUT32+1,
	DINPUT_GP_AY1	=	DINPUT_GP_AX1+1,
	DINPUT_GP_AZ1	=	DINPUT_GP_AY1+1,

	DINPUT_GP_AX2	=	DINPUT_GP_AZ1+1,
	DINPUT_GP_AY2	=	DINPUT_GP_AX2+1,
	DINPUT_GP_AZ2	=	DINPUT_GP_AY2+1,

	DINPUT_GP_POV1	=	DINPUT_GP_AZ2+1,
	DINPUT_GP_POV2	=	DINPUT_GP_POV1+1,
	DINPUT_GP_POV3	=	DINPUT_GP_POV2+1,
	DINPUT_GP_POV4	=	DINPUT_GP_POV3+1,
};

static const char * MapNames[] = 
{
	"[unmapped]",

	// Keyboard //
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", "Key ?", 
	"Key  ", "Key !", "Key \"", "Key #", "Key $", "Key %", "Key &", "Key '", "Key (", "Key )", "Key *", "Key +", "Key ,", "Key -", "Key .", "Key /",
	"Key 0", "Key 1", "Key 2", "Key 3", "Key 4", "Key 5", "Key 6", "Key 7", "Key 8", "Key 9", "Key :", "Key ;", "Key <", "Key =", "Key >", "Key ?",
	"Key @", "Key A", "Key B", "Key C", "Key D", "Key E", "Key F", "Key G", "Key H", "Key I", "Key J", "Key K", "Key L", "Key M", "Key N", "Key O",
	"Key P", "Key Q", "Key R", "Key S", "Key T", "Key U", "Key V", "Key W", "Key X", "Key Y", "Key Z", "Key [", "Key \\", "Key ]", "Key ^", "Key _",
	"Key `", "Key a", "Key b", "Key c", "Key d", "Key e", "Key f", "Key g", "Key h", "Key i", "Key j", "Key k", "Key l", "Key m", "Key n", "Key o",
	"Key p", "Key q", "Key r", "Key s", "Key t", "Key u", "Key v", "Key w", "Key x", "Key y", "Key z", "Key {", "Key |", "Key }", "Key ~", "Key ",
	"Key Ä", "Key Å", "Key Ç", "Key É", "Key Ñ", "Key Ö", "Key Ü", "Key á", "Key à", "Key â", "Key ä", "Key ã", "Key å", "Key ç", "Key é", "Key è",
	"Key ê", "Key ë", "Key í", "Key ì", "Key î", "Key ï", "Key ñ", "Key ó", "Key ò", "Key ô", "Key ö", "Key õ", "Key ú", "Key ù", "Key û", "Key ü",
	"Key †", "Key °", "Key ¢", "Key £", "Key §", "Key •", "Key ¶", "Key ß", "Key ®", "Key ©", "Key ™", "Key ´", "Key ¨", "Key ≠", "Key Æ", "Key Ø",
	"Key ∞", "Key ±", "Key ≤", "Key ≥", "Key ¥", "Key µ", "Key ∂", "Key ∑", "Key ∏", "Key π", "Key ∫", "Key ª", "Key º", "Key Ω", "Key æ", "Key ø",
	"Key ¿", "Key ¡", "Key ¬", "Key √", "Key ƒ", "Key ≈", "Key ∆", "Key «", "Key »", "Key …", "Key  ", "Key À", "Key Ã", "Key Õ", "Key Œ", "Key œ",
	"Key –", "Key —", "Key “", "Key ”", "Key ‘", "Key ’", "Key ÷", "Key ◊", "Key ÿ", "Key Ÿ", "Key ⁄", "Key €", "Key ‹", "Key ›", "Key ﬁ", "Key ﬂ",
	"Key ‡", "Key ·", "Key ‚", "Key „", "Key ‰", "Key Â", "Key Ê", "Key Á", "Key Ë", "Key È", "Key Í", "Key Î", "Key Ï", "Key Ì", "Key Ó", "Key Ô",
	"Key ", "Key Ò", "Key Ú", "Key Û", "Key Ù", "Key ı", "Key ˆ", "Key ˜", "Key ¯", "Key ˘", "Key ˙", "Key ˚", "Key ¸", "Key ˝", "Key ˛", "Key ˇ",


	// GamePad //
	"Button 1 ", "Button 2 ", "Button 3 ", "Button 4 ", "Button 5 ", "Button 6 ", "Button 7 ", "Button 8 ",
	"Button 9 ", "Button 10", "Button 11", "Button 12", "Button 13", "Button 14", "Button 15", "Button 16", 
	"Button 17", "Button 18", "Button 19", "Button 20", "Button 21", "Button 22", "Button 23", "Button 24",
	"Button 25", "Button 26", "Button 27", "Button 28", "Button 29", "Button 30", "Button 31", "Button 32", 

	"Axis X1", "Axis Y1", "Axis Z1", 
	"Axis X2", "Axis Y2", "Axis Z2", 

	"POV 1", "POV 2", "POV 3", "POV 4", 

	"Slider 1", "Slider 2",

	// Mouse //
	"Mouse Axis 1",		"Mouse Axis 2",		"Mouse Axis 3", 
	"Mouse Button 1",	"Mouse Button 2",	"Mouse Button 3", 


	// End //
	"EndOfMappingNames"
};