/*
**	emuMode.h
*/

#pragma once

#ifndef __EMUMODE_H__
#define __EMUMODE_H__


#define EMU_QUIT	(0)
#define EMU_RUNNING	(1)
#define EMU_HALTED	(3)
#define EMU_STALLED	(7)

#define CPU_INTERP	(1)
#define CPU_DYNAREC	(3)

#define UI_USE_GUI	(1)	// unused atm

#define O_NAOMI		(1)	// w00t

#endif // __EMUMODE_H__