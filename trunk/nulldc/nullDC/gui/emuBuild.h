/*
**	emuBuild.h | This file will include the proper system dependant headers
*/

#pragma once	// msc > 1000

#ifndef __EMU_BUILD_H__
#define __EMU_BUILD_H__

#ifdef	EMU_BUILD_WIN	

# include "emuWinUI.h"

#else

# include <ERROR>
# error "Unknown Build Paramters (emuBuild.h)"

#endif


#endif // __EMU_BUILD_H__