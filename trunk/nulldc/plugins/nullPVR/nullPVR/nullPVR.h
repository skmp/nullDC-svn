/*
**	nullPVR.h,	nullDC::nullPVR	(2007) ZeZu & drk||Raziel
*/
#ifndef __NULLPVR_H__
#define __NULLPVR_H__

#include <stdio.h>

#ifndef linux
#include <windows.h>
#else
#error "FIXME"
#endif


#include "plugins/plugin_header.h"











/*
**	Prototypes for these ugly bastards
*/

s32  FASTCALL pvrInit(pvr_init_params*);
void FASTCALL pvrTerm();
void FASTCALL pvrReset(bool);
s32  FASTCALL pvrLoad(emu_info*,u32);
void FASTCALL pvrUnload();
void FASTCALL pvrConfig(void*);

void FASTCALL pvrUpdate(u32 cycles);
u32  FASTCALL pvrReadReg(u32 addr,u32 size);
void FASTCALL pvrWriteReg(u32 addr,u32 data,u32 size);
void FASTCALL pvrWriteFifo(u32 address, u32* data, u32 size);
void FASTCALL pvrLockCB(vram_block *bl, u32 addr);



#endif //__NULLPVR_H___

