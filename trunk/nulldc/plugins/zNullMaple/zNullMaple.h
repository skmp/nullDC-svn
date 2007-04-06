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


